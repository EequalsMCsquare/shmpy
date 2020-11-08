#pragma once

#include "libshmpy.hpp"
#include "msgq.hpp"
#include "var.hpp"


struct PoolMeta {
    std::uint32_t max_clients;
    std::uint32_t capacity;
    std::uint32_t size;
    std::uint32_t ref_count;
    pid_t owner_pid;
    int pmsgid;
    int vmsgid;
    std::mutex mtx;

    std::uint32_t* client_id;
    VarMeta* var_metas;
};

class PoolBase {

protected:
    std::uint32_t id;
    void* pBuffer;
    std::string_view name;
    int shmid;
    int* pmsgid; // pool message queue id
    int* vmsgid;

    std::uint32_t* max_clients;
    std::uint32_t* capacity;
    std::uint32_t* size;
    std::uint32_t* ref_count;
    pid_t* owner_pid;
    std::mutex* mtx;

    std::shared_ptr<spdlog::logger> logger;
    std::array<std::thread, 2> msgq_tr; // message queue thread: 0 -> pool msgq, 1 -> var msgq.
    std::unordered_map<std::string_view, Var> attached_vars;


    // Methods
    std::uint32_t* client_ids() {
        // Lock
        std::lock_guard<std::mutex> __G(*this->mtx);

        if (this->mtx != nullptr && this->pBuffer != nullptr)
            return (std::uint32_t*)((this->mtx) + 1);
        return nullptr;
    }

    VarMeta* var_metas() {
        // Lock
        std::lock_guard<std::mutex> _G(*this->mtx);

        if (this->pBuffer != nullptr)
            return static_cast<VarMeta*>((void*)(this->client_ids() + *this->max_clients));
        return nullptr;
    }

    void NdArray_SET(const std::string& name, const py::array& arr) {

    }

    void PyDict_SET(const std::string& name, const py::dict& dict) {

    }

    void PyList_SET(const std::string& name, const py::list& list) {

    }

    void PyObj_SET(const std::string& name, const py::object& obj) {

    }


public:
    PoolBase() = default;
    ~PoolBase() {}

    void set(const std::string& name, const py::object& obj) {

    }
    void del(const std::string& name) {

    }
    void insert(const std::string& name, const py::object& obj) {

    }

    const std::string_view& get_name() {
        return this->name;
    }

    const std::uint32_t& get_capacity() {
        return *this->capacity;
    }

    const std::uint32_t get_size() {
        return *this->size;
    }

    const std::uint32_t get_ref_count() {
        return *this->ref_count;
    }

    const pid_t& get_owner_pid() {
        return *this->owner_pid;
    }

    const std::uint32_t get_max_clients() {
        return *this->max_clients;
    }
};