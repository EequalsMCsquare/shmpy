#pragma once

#include "pool.hpp"

class Client : public PoolBase
{

public:
    Client(const std::string &_name)
    {
        // init logger
        try
        {
            this->logger = spdlog::basic_logger_st("Data Client", "shmpy.log");
        }
        catch (spdlog::spdlog_ex &e)
        {
            std::cerr << "Fail to create file_sink logger, use stdout as fallback. (" << e.what() << ")" << std::endl;
            this->logger = spdlog::stdout_color_st("Data Client");
        }
        this->logger->set_level(spdlog::level::debug);

        // shmget
        logger->debug("begin to open shm.");
        key_t _key = std::hash<std::string>()(_name) % INT_MAX;
        this->shmid = shmget(_key, 0, 0600);
        logger->debug("obtained shmid (%d)", this->shmid);
        if (this->shmid == -1)
        {
            logger->critical("fail to obtain shmid. %s", strerror(errno));
            throw shm_error(fmt::format("fail to obtain shmid. (%d)", errno));
        }

        // shmat
        this->pBuffer = shmat(this->shmid, 0, 0);
        if (this->pBuffer == (void *)-1)
        {
            logger->critical("failed to attch shm. (%s)", strerror(errno));
            throw shm_error(fmt::format("failed to attach shm. (%d)", errno));
        }
        auto _meta = static_cast<PoolMeta *>(this->pBuffer);
        logger->debug("attaching success. address: %p", this->pBuffer);

        // check ref_count < max attaching process
        {
            std::lock_guard<std::mutex> _G(_meta->mtx);
            if (_meta->ref_count == _meta->max_clients)
            {
                shmdt(this->pBuffer);
                logger->error("max attaching proc already reached.");
                throw shm_error("max attaching proc already reached.");
            }
            if (_meta->ref_count == 1)
                this->id = 2;
            else
                this->id = _meta->client_ids()[_meta->ref_count - 1] + 1;
            _meta->client_ids()[_meta->ref_count - 1] = this->id;
            _meta->ref_count += 1;
        }

        // assign values
        this->name = _name;
        this->capacity = &_meta->capacity;
        this->size = &_meta->size;
        this->ref_count = &_meta->ref_count;
        this->owner_pid = &_meta->owner_pid;
        this->mtx = &_meta->mtx;
        this->max_clients = &_meta->max_clients;
        this->msgid = &_meta->msgid;
        this->status = POOL_OK;

        // start threads to handle message queue
        this->msgq_tr = std::thread([this]() {
            this->logger->info("Begin thread to cpature pool handle message.");
            msg_t msg;
            resp response;
            while (this->keep_msgq)
            {
                msgrcv(*this->msgid, &msg, sizeof(req), this->id, 0);

                // if message target is client
                if (msg.request.target == e_target::CLN)
                {
                    if (msg.request.type == e_reqtype::CMD)
                    {
                        switch (msg.request.action)
                        {
                        case AC_DETACH:
                            logger->info("detach request received. sender: {}, need_reply: {}, action: {}", msg.request.sender, msg.request.need_reply, msg.request.action);
                            this->dt_allvar();
                            this->status = POOL_DT;
                            break;
                        }
                    }
                    else if (msg.request.type == e_reqtype::PING)
                    {
                        // use fifo to snd resp
                    }
                    else if (msg.request.type == e_reqtype::TERM)
                    {
                        logger->info("terminate reqeust received. sender: {}, need_reply: {}, action: {}", msg.request.sender, msg.request.need_reply, msg.request.action);
                        this->dt_allvar();
                        this->status = pool_status::POOL_TM;
                        break;
                    }
                }
                else if (msg.request.target == e_target::VAR) {
                    if (msg.request.type == e_reqtype::CMD) {
                        switch (msg.request.action)
                        {
                        case e_action::AC_DETACH:
                            // detach vars.
                            break;
                        case e_action::AC_UPDATE:
                            // update vars.
                            break;
                        }
                    }

                }
            }
        });
    }

    ~Client()
    {
    }
};