#pragma once

#include "pool.hpp"

class Server : public PoolBase {

public:
    Server(const std::string& _name, const uint32_t& _capacity, const uint32_t& _max_clients = 128) {

        // init logger
        try {
            this->logger = spdlog::basic_logger_st("Data Server", "shmpy.log");
        } catch (spdlog::spdlog_ex& e) {
            std::cerr << "Fail to create file_sink logger, use stdout as fallback. (" << e.what() << ")" << std::endl;
            this->logger = spdlog::stdout_color_st("Data Server");
        }
        this->logger->set_level(spdlog::level::debug);

        this->id = 1;
        uint64_t reqbytes = sizeof(PoolMeta) + (sizeof(VarMeta) + sizeof(pid_t) * _max_clients) * _capacity + sizeof(pid_t) * _max_clients + 32;
        logger->debug("Required bytes {}", reqbytes / 1024.);

        // Create shared memory using SystemV
        std::hash<std::string> _hash;
        key_t _key = _hash(_name) % INT_MAX;
        this->shmid = shmget(_key, reqbytes, IPC_CREAT | IPC_EXCL | 0600);
        if (this->shmid == -1) {
            logger->critical(
                "Fail to create shm with key: {}. This is probably caused by hash collision, try with another name. {}",
                _key, strerror(errno));
            throw shm_error("Failed to create shm. Try with another name!");
        }

        // shmat
        this->pBuffer = (PoolMeta*)shmat(this->shmid, 0, 0);
        if (this->pBuffer == (void*)-1) {
            logger->critical("failed to attaching shm. {}", strerror(errno));
            throw shm_error("Failed to attaching.");
        }
        auto _meta = static_cast<PoolMeta*>(this->pBuffer);
        logger->debug("attach success. address: {}", this->pBuffer);

        // create message queue
        int _msgid;
        _key = _hash(fmt::format("{}/{}", _name, "pools")) % INT_MAX;
        _msgid = msgget(_key, IPC_EXCL | IPC_CREAT | 0600);
        if (_msgid == -1) {
            logger->critical(
                "fail to create pool message queue with key: %d. This is probably cause by hash collision, try with another name. %s",
                _key, strerror(errno));
            throw msg_error("failed to create pool message queue. Try with another name!");
        }

        // start thread to handle msgq
        this->msgq_tr = std::thread([&_msgid, this]()
        {
            this->logger->info("Begin thread to capture pool handle messages");
            msg_t msg;
            while (true) {
                msgrcv(_msgid, &msg, sizeof(req), PMSGT_SRV, 0);
                // TODO: parse msg
            }
        });

        // assign values
        this->name = _name;
        this->msgid = new(&_meta->msgid) int(_msgid);
        this->capacity = new(&_meta->capacity) std::uint32_t(_capacity);
        this->size = new(&_meta->size) std::uint32_t(0);
        this->ref_count = new(&_meta->ref_count) std::uint32_t(1);
        this->owner_pid = new(&_meta->owner_pid) pid_t(getpid());
        this->mtx = new(&_meta->mtx) std::mutex;
        this->max_clients = new(&_meta->max_clients) std::uint32_t(_max_clients);

        logger->info("Data Server initialization complete.");
    }

    void dispose() {
        // tell all the attaching process to dt and desctroy all the var and destroy shm

    }

    ~Server() {

    }

private:
    /*
     * Send message to client pool, which indicates update is available.
     */
    void sndpmsg_update() {

    }

    void sndpmsg_update(const std::uint32_t* client_id) {

    }

    void sndpmsg_update(const std::initializer_list<std::uint32_t>& client_id) {

    }

    /*
     * Send message to client pool to call shmdt.
     */
    void sndpmsg_detach() {

    }

    void sndpmsg_detach(const std::uint32_t* client_id) {

    }

    void sndpmsg_detach(const std::initializer_list<std::uint32_t>& client_id) {

    }
};