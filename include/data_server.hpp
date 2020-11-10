#pragma once

#include "pool.hpp"

class Server : public PoolBase {

public:
    Server(const std::string& _name, const uint32_t& _capacity, const uint32_t& _max_clients = 128)
    {
        // init logger
        try {
            this->logger = spdlog::basic_logger_mt(fmt::format("shmpy.Server.{}", _name), "shmpy.log");
        } catch (spdlog::spdlog_ex& e) {
            std::cerr << "Fail to create file_sink logger, use stdout as fallback. (" << e.what() << ")" << std::endl;
            this->logger = spdlog::stdout_color_mt(fmt::format("shmpy.Server.{}", _name));
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
                "fail to create pool message queue with key: {}. This is probably cause by hash collision, try with another name. {}",
                _key, strerror(errno));
            throw msg_error("failed to create pool message queue. Try with another name!");
        }

        // assign values
        this->name = _name;
        this->id = PMSGT_SVR;
        this->msgid = new (&_meta->msgid) int(_msgid);
        this->capacity = new (&_meta->capacity) std::uint32_t(_capacity);
        this->size = new (&_meta->size) std::uint32_t(0);
        this->ref_count = new (&_meta->ref_count) std::uint32_t(1);
        this->owner_pid = new (&_meta->owner_pid) pid_t(getpid());
        this->mtx = new (&_meta->mtx) std::mutex;
        this->max_clients = new (&_meta->max_clients) std::uint32_t(_max_clients);

        // start the thread to handle msgq
        this->msgq_tr = std::thread(&Server::handle_msgqtr, this);

        logger->info("Data Server initialization complete.");
    }

    ~Server()
    {
        if (this->status == pool_status::POOL_OK) {
            msg_t msg;
            msg.msg_type = this->id;
            msg.request.sender = this->id;
            msg.request.receiver = this->id;
            msg.request.target = e_target::SVR;
            msg.request.type = e_reqtype::TERM;
            msg.request.need_reply = false;
            msg.request.action = e_action::AC_NULL;
            int _msgid = *this->msgid;
            // tell server self to destroy
            if (msgsnd(*this->msgid, &msg, sizeof(req), 0) == -1) {
                logger->error("fail to send msg. {}", strerror(errno));
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            this->msgq_tr.join();
        }
    }

private:
    void handle_msgqtr()
    {
        this->logger->info("Begin thread to capture message.");
        msg_t msg;
        resp response;
        while (this->keep_msgq) {
            msgrcv(*this->msgid, &msg, sizeof(req), PMSGT_SVR, 0);
            logger->info("Message Received. sender: {}, receiver: {}, type: {}, need_reply: {}, target: {}, action: {}", msg.request.sender, msg.request.receiver, msg.request.type, msg.request.need_reply, msg.request.target, msg.request.action);
            // if message target is Server
            if (msg.request.target == e_target::SVR) {
                // if message type is PING
                if (msg.request.type == e_reqtype::PING) {

                } else if (msg.request.type == e_reqtype::TERM) {
                    /*
                     * Handling type TERM
                     * this should only send by itself. Usually a destructor is called.
                     * this will check sender first, if sender and receive has the same id as this->id.
                     * if true. following steps will take place:
                     *  1. check sender and receiver.
                     *  2. send ACTION Detach to other client pools
                     *  3. detach all attached variable, which is stored in this->attached_vars
                     *  4. lock pool meta mutex
                     *  5. decrease pool buffer ref_count by 1
                     *  6. unlock pool meta mutex
                     *  7. detach pBuffer
                     *  8. set status to pool_status::POOL_TM
                     *  9. break while loop
                     */

                    // step 1.
                    if (this->id == msg.request.sender && this->id == msg.request.receiver) {
                        // step 2.
                        std::uint32_t i;
                        msg_t msg;
                        msg.request.sender = PMSGT_SVR;
                        msg.request.type = e_reqtype::CMD;
                        msg.request.target = e_target::CLN;
                        msg.request.need_reply = false;
                        msg.request.action = e_action::AC_DETACH;
                        for (i = 0; i < *this->ref_count - 1; i++) {
                            msg.msg_type = this->client_ids()[i];
                            msg.request.receiver = this->client_ids()[i];
                            if (msgsnd(*this->msgid, &msg, sizeof(req), 0) == -1)
                                logger->error("failed to send msg. {}", strerror(errno));
                            // TODO: mkfifo to receive clients response.
                        }
                        // step 3.
                        this->dt_allvar();
                        // step 4. 5. 6.
                        {
                            std::lock_guard<std::mutex> _G(*this->mtx);
                            *this->ref_count -= 1;
                        }
                        // get msgid before detach form pBuffer.
                        int _msgid = *this->msgid;
                        // step 7.
                        this->dt_pmeta();
                        // step 8.
                        // delete msgq
                        if (msgctl(_msgid, IPC_RMID, nullptr) == -1)
                            logger->error("failed to delete msg queue. {}", strerror(errno));
                        // delete shm
                        if (shmctl(this->shmid, IPC_RMID, nullptr) == -1)
                            logger->error("failed to delete shm. {}", strerror(errno));
                        logger->info("Detory complete.");
                        // step 9.
                        break;
                    }
                }
            }
        }
    }

private:
    /*
     * Send message to client pool, which indicates update is available.
     */
    void sndpmsg_update()
    {
    }

    void sndpmsg_update(const std::uint32_t* client_id)
    {
    }

    void sndpmsg_update(const std::initializer_list<std::uint32_t>& client_id)
    {
    }

    /*
     * Send message to client pool to call shmdt.
     */
    void sndpmsg_detach()
    {
    }

    void sndpmsg_detach(const std::uint32_t* client_id)
    {
    }

    void sndpmsg_detach(const std::initializer_list<std::uint32_t>& client_id)
    {
    }
};