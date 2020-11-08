#pragma once

#include "pool.hpp"

class Server : public PoolBase {

public:
    Server(const std::string& _name, const uint32_t& _capacity, const uint32_t& _max_clients = 128) {

        this->id = 1;
        uint64_t reqbytes = sizeof(PoolMeta) + (sizeof(VarMeta) + sizeof(pid_t) * _max_clients) * _capacity + sizeof(pid_t) * _max_clients + 32;
        logger->debug("Required bytes %lf", reqbytes / 1024.);

        // Create shared memory using SystemV
        std::hash<std::string> _hash;
        key_t _key = _hash(_name) % INT_MAX;
        this->shmid = shmget(_key, reqbytes, IPC_CREAT | IPC_EXCL);
        if (this->shmid == -1) {
            logger->critical(
                "Fail to create shm with key: %d. This is probably caused by hash collision, try with another name. %s",
                _key, strerror(errno));
            throw shm_error("Failed to create shm. Try with another name!");
        }

        // shmat
        this->pBuffer = (PoolMeta*)shmat(this->shmid, 0, 0);
        if (this->pBuffer == (void*)-1) {
            logger->critical("failed to attaching shm. %s", strerror(errno));
            throw shm_error("Failed to attaching.");
        }
        auto _meta = static_cast<PoolMeta*>(this->pBuffer);
        logger->debug("attach success. address: %p", this->pBuffer);

        // create message queue
        int _pmsgid, _vmsgid;
        _key = _hash(fmt::format("{}/{}", _name, "pools")) % INT_MAX;
        _pmsgid = msgget(_key, IPC_EXCL | IPC_CREAT | 0600);
        if (_pmsgid == -1) {
            logger->critical(
                "fail to create pool message queue with key: %d. This is probably cause by hash collision, try with another name. %s",
                _key, strerror(errno));
            throw msg_error("failed to create pool message queue. Try with another name!");
        }

        _key = _hash(fmt::format("{}/{}", _name, "vars")) % INT_MAX;
        _vmsgid = msgget(_key, IPC_EXCL | IPC_CREAT | 0600);
        if (_vmsgid == -1) {
            logger->critical(
                "fail to create pool message queue with key: %d. This is probably cause by hash collision, try with another name. %s",
                _key, strerror(errno));
            throw msg_error("failed to create pool message queue. Try with another name!");
        }

        // start thread to handle msgq
        this->msgq_tr[0] = std::thread([&_pmsgid, this]()
        {
            this->logger->info("Begin thread to capture pool handle messages");
            pmsg_t msg;
            while (true) {
                msgrcv(_pmsgid, &msg, sizeof(pmsg_body), PMSGT_SRV, 0);
                // TODO: parse msg
            }
        });

        this->msgq_tr[1] = std::thread([&_vmsgid, this]()
        {
            this->logger->info("Begin thread to cpature var handle message");
            vmsg_t msg;
            while (true) {
                /*
                 * Since msgtype should be any int64_t which is > 0,
                 * therefore, msgtype is handled by a Pool, and msgtype > 1 mean var meta's index.
                 */
                msgrcv(_vmsgid, &msg, sizeof(vmsg_body), PMSGT_SRV, 0);
                // TODO: parse msg
            }
        });

        // assign values
        this->pmsgid = new(&_meta->pmsgid) int(_pmsgid);
        this->vmsgid = new(&_meta->vmsgid) int(_vmsgid);
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