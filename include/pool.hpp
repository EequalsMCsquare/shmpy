#pragma once

#include "libshmpy.hpp"
#include "msgq.hpp"
#include "var.hpp"

enum pool_status {
    POOL_OK, // pool ok
    POOL_DT, // Pool detach
    POOL_TM, // Pool terminate
};

struct PoolMeta {
    std::uint32_t max_clients;
    std::uint32_t capacity;
    std::uint32_t size;
    std::uint32_t ref_count;
    pid_t owner_pid;
    int msgid;
    std::mutex mtx;
    // client_ids
    // var_metas

    std::uint32_t* client_ids()
    {
        return (std::uint32_t*)(&mtx + 1);
    }

    VarMeta* var_metas()
    {
        return (VarMeta*)(this->client_ids() + this->max_clients);
    }
};

class PoolBase {

protected:
    pool_status status;
    std::uint32_t id;
    void* pBuffer;
    std::string name;
    int shmid;
    int* msgid; // message queue id

    std::uint32_t* max_clients;
    std::uint32_t* capacity;
    std::uint32_t* size;
    std::uint32_t* ref_count;
    pid_t* owner_pid;
    std::mutex* mtx;

    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<spdlog::sinks::sink> sink;
    std::thread msgq_tr; // message queue thread: 0 -> pool msgq, 1 -> var msgq.
    std::unordered_map<std::string_view, std::shared_ptr<Var>> attached_vars; // when a variable is attaching to current Pool, it will be register here.
    std::atomic<bool> keep_msgq = true;

    std::uint32_t* client_ids()
    {
        if (this->pBuffer != nullptr)
            return static_cast<PoolMeta*>(this->pBuffer)->client_ids();
        return nullptr;
    }

    VarMeta* var_metas()
    {
        if (this->pBuffer != nullptr)
            return static_cast<PoolMeta*>(this->pBuffer)->var_metas();
        return nullptr;
    }

    void NdArray_SET(const std::string& name, const py::array& arr)
    {
    }

    void PyDict_SET(const std::string& name, const py::dict& dict)
    {
    }

    void PyList_SET(const std::string& name, const py::list& list)
    {
    }

    void PyObj_SET(const std::string& name, const py::object& obj)
    {
    }

    void dt_var(const std::string& name)
    {
        auto _v = this->attached_vars.find(name);
        if (_v == this->attached_vars.end()) {
            this->logger->warn("variable '{}' not found.", name);
            return;
        }
        // if found
        logger->debug("begin to detach var: {}, client: {}", name, id);
        std::lock_guard<std::mutex> _G(*_v->second->mtx);
        // remove var_meta's client id
        std::uint32_t i;
        for (i = 0; i < *_v->second->ref_count; i++) {
            if (_v->second->att_pools()[i] == this->id) {
                for (; i < *_v->second->ref_count - 1; i++)
                    _v->second->att_pools()[i] = _v->second->att_pools()[i + 1];
                break;
            }
        }
        // decrease var meta reference count
        *_v->second->ref_count -= 1;
        int vshmid = *_v->second->shmid;
        // detach 
        if (shmdt(_v->second->pBuffer) == -1) {
            logger->error("failed to detach var. client: {}, var: {}. {}", this->id, name, strerror(errno));
            throw shm_error("failed to detach var.");
        }
        // check if the var meta's reference count is zero. if zero delete it.
        if (*_v->second->ref_count == 0) {
            if (shmctl(vshmid, IPC_RMID, NULL) == -1) {
                logger->error("failed to destroy var shm. client: {}, var: {}. {}", this->id, name, strerror(errno));
            }
        }
        // remove from map
        this->attached_vars.erase(name);
        logger->info("detach success! client: {}, var: {}", this->id, name);
    }

    void dt_allvar()
    {
        // iterate through all the val in the map
        for (auto& [key, val] : this->attached_vars) {
            logger->debug("begin to detach var: {}, client: {}", key, id);
            std::lock_guard<std::mutex> _G(*val->mtx);
            std::uint32_t i;
            for (i = 0; i < *val->ref_count; i++)
                if (val->att_pools()[i] == this->id) {
                    for (; i < *val->ref_count - 1; i++)
                        val->att_pools()[i] = val->att_pools()[i + 1];
                    break;
                }
            *val->ref_count -= 1;
            int vshmid = *val->shmid;
            if (shmdt(val->pBuffer) == -1) {
                logger->error("failed to detach var. client: {}, var: {}. {}", this->id, name, strerror(errno));
            }
            if (*val->ref_count == 0) {
                if (shmctl(vshmid, IPC_RMID, NULL) == -1) {
                    logger->error("failed to destroy var shm. client: {}, var: {}. {}", this->id, name, strerror(errno));
                }
            }
            this->attached_vars.erase(key);
            logger->info("detach success! client: {}, var: {}", this->id, name);
        }
        logger->info("all vars have been detached.");
    }

    // detach from pool meta
    void dt_pmeta() {
        logger->debug("detach from pool meta.");
        if (this->pBuffer == nullptr)
            logger->error("failed to detach from pool meta. pBuffer is nullptr.");
        if (shmdt(this->pBuffer) == -1) {
            logger->critical("failed to detach from pool meta. {}", strerror(errno));
        }
    }

    virtual void init_logger() = 0;
    virtual void handle_msgqtr() = 0;

public:
    PoolBase() = default;

    void set(const std::string& name, const py::object& obj)
    {
    }
    void del(const std::string& name)
    {
    }
    void insert(const std::string& name, const py::object& obj)
    {
    }

    const std::string_view get_name()
    {
        if (this->status != pool_status::POOL_OK) 
            throw pool_error("pool is not accessible.");
        return this->name;
    }

    const std::uint32_t& get_capacity()
    {
        if (this->status != pool_status::POOL_OK) 
            throw pool_error("pool is not accessible.");
        return *this->capacity;
    }

    const std::uint32_t& get_size()
    {
        if (this->status != pool_status::POOL_OK) 
            throw pool_error("pool is not accessible.");
        return *this->size;
    }

    const std::uint32_t get_ref_count()
    {
        if (this->status != pool_status::POOL_OK) 
            throw pool_error("pool is not accessible.");
        return *this->ref_count;
    }

    const pid_t& get_owner_pid()
    {
        if (this->status != pool_status::POOL_OK) 
            throw pool_error("pool is not accessible.");
        return *this->owner_pid;
    }

    const std::uint32_t& get_max_clients()
    {
        if (this->status != pool_status::POOL_OK) 
            throw pool_error("pool is not accessible.");
        return *this->max_clients;
    }
    const std::vector<std::uint32_t> get_client_ids()
    {
        if (this->status != pool_status::POOL_OK) 
            throw pool_error("pool is not accessible.");
        return std::vector<std::uint32_t>(this->client_ids(), this->client_ids() + *this->ref_count - 1);
    }

    const pool_status get_status() {
        return this->status;
    }

    const std::string_view Pyget_status() {
        switch(this->status)
        {
            case pool_status::POOL_OK:
                return "ok";
            case pool_status::POOL_DT:
                return "detached";
            case pool_status::POOL_TM:
                return "terminated";
            default:
                return "unknown";
        }
    }
};