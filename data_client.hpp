#pragma once

#include "pool.hpp"


class Client : public PoolBase {

public:
    Client(const std::string& _name) {
        // init logger
        try {
            this->logger = spdlog::basic_logger_st("Data Server", "shmpy.log");
        } catch (spdlog::spdlog_ex& e) {
            std::cerr << "Fail to create file_sink logger, use stdout as fallback. (" << e.what() << ")" << std::endl;
            this->logger = spdlog::stdout_color_st("Data Server");
        }

        // shmget
        logger->debug("begin to open shm.");
        key_t _key = std::hash<std::string>()(_name) % INT_MAX;
        this->shmid = shmget(_key, 0, IPC_CREAT | IPC_EXCL);
        logger->debug("obtained shmid (%d)", this->shmid);
        if (this->shmid == -1) {
            logger->critical("fail to obtain shmid. %s", strerror(errno));
            throw shm_error(fmt::format("fail to obtain shmid. (%d)", errno));
        }

        // shmat
        this->pBuffer = shmat(this->shmid, 0, 0);
        if (this->pBuffer == (void*)-1) {
            logger->critical("failed to attch shm. (%s)", strerror(errno));
            throw shm_error(fmt::format("failed to attach shm. (%d)", errno));
        }
        auto _meta = static_cast<PoolMeta*>(this->pBuffer);
        logger->debug("attaching success. address: %p", this->pBuffer);

        // check ref_count < max attaching process
        {
            std::lock_guard<std::mutex> _G(_meta->mtx);
            if (_meta->ref_count == _meta->max_clients) {
                shmdt(this->pBuffer);
                logger->error("max attaching proc already reached.");
                throw shm_error("max attaching proc already reached.");
            }
            _meta->ref_count += 1;
            this->id = _meta->ref_count;
        }

        // assign values
        this->capacity = &_meta->capacity;
        this->size = &_meta->size;
        this->ref_count = &_meta->ref_count;
        this->owner_pid = &_meta->owner_pid;
        this->mtx = &_meta->mtx;
        this->max_clients = &_meta->max_clients;
        this->pmsgid = &_meta->pmsgid;
        this->vmsgid = &_meta->vmsgid;

        // start threads to handle message queue
        this->msgq_tr[0] = std::thread([this]()
        {
            this->logger->info("Begin thread to cpature pool handle message.");
            pmsg_t msg;
            while (true) {
                msgrcv(*this->pmsgid, &msg, sizeof(pmsg_body), this->id, 0);
                // TODO: parse msg
                switch (msg.body.act) {
                case DESTROY:
                // 
                break;

                default:
                break;
                }
            }
        });

        this->msgq_tr[1] = std::thread([this]()
        {
            this->logger->info("Begin thread to capture var handle message.");
            vmsg_t msg;
            while (true) {
                msgrcv(*this->vmsgid, &msg, sizeof(vmsg_body), this->id, 0);
            }
        });
    }

    ~Client() {

    }
};