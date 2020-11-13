#pragma once

#include "libshmpy.hpp"
#include "msgq.hpp"
#include "var.hpp"

using namespace shmpy::literal::mem;

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

    VarMeta* find_var(const std::string& __name)
    {
        std::uint32_t i;
        for (i = 0; i < this->size; i++) {
            if (!__name.compare(this->var_metas()[i].name))
                return this->var_metas() + i;
        }
        return nullptr;
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
    std::thread msgq_tr; // message queue thread: 0 -> pool msgq, 1 -> var msgq.
    std::unordered_map<std::string, std::shared_ptr<Var>> attached_vars; // when a variable is attaching to current Pool, it will be register here.
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

    void Integer_SET(const std::string& name, const py::int_& num)
    {
    }

    void Float_SET(const std::string& name, const py::float_& num)
    {
    }

    void Bool_SET(const std::string& name, const py::bool_& _b)
    {
    }

    void String_SET(const std::string& name, const py::str& str)
    {
    }

    void PyBuff_SET(const std::string& name, const py::buffer& buff)
    {
        ssize_t i;
        // calculate how many bytes is required by data
        ssize_t nbytes = 1;
        auto buff_info = buff.request(false);
        for (i = 0; i < buff_info.ndim; i++) {
            if (buff_info.shape[i] <= 0) {
                logger->critical("ndbuffay shape must > 0.");
                throw var_error("ndbuffay shape must > 0");
            }
            nbytes *= buff_info.shape[i];
        }
        nbytes *= buff_info.itemsize;
        // calculate bytes of buffer protocol struct
        // 1. constexpr fixed size
        std::size_t total_bytes = sizeof(shmpy::buff_protocol::itemsize) + sizeof(shmpy::buff_protocol::size) + sizeof(shmpy::buff_protocol::format) + sizeof(shmpy::buff_protocol::ndims) + sizeof(shmpy::buff_protocol::readonly) + sizeof(ssize_t) * buff_info.ndim * 2 + nbytes;
        logger->info("buffay total kb required: {:.4f}", total_bytes / static_cast<double>(1_KB));
        auto _var = this->create_var(std::move(name), true, shmpy::_DTYPE::_ND_ARRAY, total_bytes);
        auto _bp = static_cast<shmpy::buff_protocol*>(_var->pBuffer);
        _bp->itemsize = buff_info.itemsize;
        _bp->size = buff_info.size;
        strncpy(_bp->format, buff_info.format.c_str(), 3);
        _bp->ndims = buff_info.ndim;
        memcpy(_bp->shape(), buff_info.shape.data(), _bp->ndims * sizeof(ssize_t));
        memcpy(_bp->strides(), buff_info.strides.data(), _bp->ndims * sizeof(ssize_t));
        _bp->readonly = buff_info.readonly;
        memcpy(_bp->ptr(), buff_info.ptr, buff_info.size * buff_info.itemsize);
        *this->size += 1;
    }

    py::array NdArray_Parse(const std::shared_ptr<Var> _var)
    {
        if (*_var->dtype != shmpy::_DTYPE::_ND_ARRAY)
            throw var_error("variable dtype is not NdArray.");
        auto buff = static_cast<shmpy::buff_protocol*>(_var->pBuffer);
        std::vector<ssize_t> _shape(buff->shape(), buff->shape() + buff->ndims);
        std::vector<ssize_t> _strides(buff->strides(), buff->strides() + buff->ndims);
        return py::array(py::memoryview::from_buffer(buff->ptr(), buff->itemsize, buff->format, _shape, _strides));
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

    std::shared_ptr<Var>
    create_var(const std::string& _name, const bool isPyBP, const shmpy::_DTYPE dtype, const size_t& nbytes,
        bool override = false)
    {
        /*
         * This function will firstly create a VarMeta, and call shmget to create a
         * shared memory. and store the VarMeta into pool meta and Var will be store in 
         * Pool->attached_vars
         */

        // 1. check pool status
        if (this->status != pool_status::POOL_OK)
            throw pool_error("pool is not accessible.");
        // 2. Check if Pool reach max capacity
        if (this->size == this->capacity)
            throw pool_error("pool reach max capacity.");
        // 3. Var name length <= 63
        if (_name.length() >= 63)
            throw var_error("var name length is too long. name.length < 64.");
        // 4. create shm
        key_t _key = std::hash<std::string>()(fmt::format("{}/{}", this->name, _name)) % INT_MAX;
        int _shmid = shmget(_key, nbytes, IPC_CREAT | IPC_EXCL | 0600);
        if (_shmid == -1) {
            logger->error(strerror(errno));
            throw shm_error(errno);
        }
        // 5. attach shm
        void* _buffer = shmat(_shmid, nullptr, 0);
        if (_buffer == (void*)-1) {
            logger->error(strerror(errno));
            throw shm_error(errno);
        }
        // 6. setup Var & VarMeta
        VarMeta* _vmeta = this->var_metas() + *this->size;
        std::shared_ptr<Var> _var = std::make_shared<Var>();
        strncpy(_vmeta->name, _name.c_str(), 63);
        _var->pBuffer = _buffer;
        _var->name = _vmeta->name;
        _var->shmid = new (&_vmeta->shmid) int(_shmid);
        _var->isPyBuffProtocol = new (&_vmeta->isPyBuffProtocol) bool(isPyBP);
        _var->dtype = new (&_vmeta->dtype) shmpy::_DTYPE(dtype);
        _var->nbytes = new (&_vmeta->nbytes) std::size_t(nbytes);
        _var->mtx = new (&_vmeta->mtx) std::mutex();
        _var->mtx->try_lock();
        _var->ref_count = new (&_vmeta->ref_count) std::uint32_t(1);
        _var->att_pools()[0] = this->id;
        _var->mtx->unlock();

        // 7. insert the Var into attached_vars
        if (override)
            this->attached_vars[_name] = _var;
        else
            this->attached_vars.insert(std::make_pair(std::move(_name), _var));

        /*
         * Notice:
         *  currently, the actual data haven't been copied to shm.
         */

        // 8. return
        return _var;
    }

    std::shared_ptr<Var> at_var(const std::string& name)
    {
        /*
         * This will try find the VarMeta in the PoolMeta's var_metas.
         * If var meta is found. it will attach it to current process.
         * if var meta does not exist, var_error will be thrown.
         * after it's attached, a Var will be created and insert into
         * this->attatched_vars and return it.
         */
        VarMeta* _vmeta;
        _vmeta = static_cast<PoolMeta*>(this->pBuffer)->find_var(name);
        if (_vmeta == nullptr)
            throw var_error("Variable does not exist.");
        // attach shm to this process.
        void* _vBuffer = shmat(_vmeta->shmid, nullptr, 0);
        if (_vBuffer == (void*)-1)
            throw shm_error(fmt::format("fail to attach shm. {}", strerror(errno)));
        auto _var = std::make_shared<Var>(_vmeta, _vBuffer);

        _var->mtx->try_lock();
        _var->att_pools()[_vmeta->ref_count] = this->id;
        *_var->ref_count += 1;
        _var->mtx->unlock();
        this->attached_vars.insert(std::make_pair(name, _var));
        return _var;
    }

    void dt_var(const std::string& name)
    {
        /*
         * dt_var will first try find the var in this->attached_vars.
         * If not found, the func will just return.
         */
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
                    logger->error("failed to destroy var shm. client: {}, var: {}. {}", this->id, name,
                        strerror(errno));
                }
            }
            logger->info("detach success! client: {}, var: {}", this->id, name);
        }
        this->attached_vars.clear();
        logger->info("all vars have been detached.");
    }

    // detach from pool meta
    void dt_pmeta()
    {
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
        auto _tmp = this->attached_vars.find(name);
        if (_tmp != this->attached_vars.end()) {
            logger->critical("variable name must be unique. received: {}", name);
            throw var_error("variable name must be unique");
        }
        if (py::isinstance(obj, py::module_::import("numpy").attr("ndarray")))
            PyBuff_SET(std::move(name), std::move(obj));
    }

    py::object get(const std::string& _name)
    {
        // try to find it in this->attached_vars.
        std::shared_ptr<Var> _var;
        auto _viter = this->attached_vars.find(_name);
        if (_viter == this->attached_vars.end())
            _var = this->at_var(_name);
        else
            _var = _viter->second;
        // check handle _vmeta type.
        if (*_var->dtype == shmpy::_DTYPE::_ND_ARRAY)
            return NdArray_Parse(_var);
        return py::none();
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

    const pool_status get_status()
    {
        return this->status;
    }

    const std::string_view Pyget_status()
    {
        switch (this->status) {
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