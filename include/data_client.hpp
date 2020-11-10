#pragma once

#include "pool.hpp"

class Client : public PoolBase {

public:
    Client(const std::string& _name)
    {

        // shmget
        key_t _key = std::hash<std::string>()(_name) % INT_MAX;
        this->shmid = shmget(_key, 0, 0600);
        if (this->shmid == -1)
            throw shm_error(fmt::format("fail to obtain shmid. {}", strerror(errno)));

        // shmat
        this->pBuffer = shmat(this->shmid, 0, 0);
        if (this->pBuffer == (void*)-1)
            throw shm_error(fmt::format("failed to attach shm. {}", strerror(errno)));
        auto _meta = static_cast<PoolMeta*>(this->pBuffer);

        // check ref_count < max attaching process
        {
            std::lock_guard<std::mutex> _G(_meta->mtx);
            if (_meta->ref_count == _meta->max_clients) {
                shmdt(this->pBuffer);
                throw shm_error("max attaching proc already reached.");
            }
            /*
             * ref_count is one, means only a server is attached to this pool meta.
             * therefore, just directly set id to 2.
             */
            if (_meta->ref_count == 1)
                this->id = 2;
            else
                this->id = _meta->client_ids()[_meta->ref_count - 2] + 1;
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

        // init logger
        try {
            this->logger = spdlog::basic_logger_mt(fmt::format("shmpy.Client {}", this->id), "shmpy.log");
        } catch (spdlog::spdlog_ex& e) {
            std::cerr << "Fail to create file_sink logger, use stdout as fallback. (" << e.what() << ")" << std::endl;
            this->logger = spdlog::stdout_color_mt(fmt::format("shmpy.Client {}", this->id));
        }
        this->logger->set_level(spdlog::level::debug);

        // start threads to handle message queue
        this->msgq_tr = std::thread(&Client::handle_msgqtr, this);
    }

    ~Client()
    {
        this->logger->info("calling ~Client");
        if (this->status == pool_status::POOL_DT) {
            this->status = pool_status::POOL_TM;
            this->msgq_tr.join();
            return;
        }
        if (this->status == pool_status::POOL_OK) {
            msg_t msg;
            msg.msg_type = this->id;
            msg.request.sender = this->id;
            msg.request.receiver = this->id;
            msg.request.target = e_target::CLN;
            msg.request.type = e_reqtype::TERM;
            msg.request.need_reply = false;
            msg.request.action = e_action::AC_NULL;
            if (msgsnd(*this->msgid, &msg, sizeof(req), 0) == -1) {
                logger->error("fail to send msg. {}", strerror(errno));
                return;
            }
            this->msgq_tr.join();
        }
    }

private:
    void handle_msgqtr()
    {
        this->logger->info("Begin thread to cpature message.");
        msg_t msg;
        resp response;
        while (true) {
            // blocking current thread to receive msgq
            msgrcv(*this->msgid, &msg, sizeof(req), this->id, 0);
            logger->info("Message Received. sender: {}, receiver: {}, type: {}, need_reply: {}, target: {}, action: {}", msg.request.sender, msg.request.receiver, msg.request.type, msg.request.need_reply, msg.request.target, msg.request.action);
            // if message target is client
            if (msg.request.target == e_target::CLN) {
                // if message request type is command
                if (msg.request.type == e_reqtype::CMD) {
                    if (msg.request.action == AC_DETACH) {
                        // ACTION Detach
                        /*
                        * When detach command is receive.
                        * client need to complete following steps:
                        */
                        // 1. detach all attached variable, which is stored in this->attached_vars
                        this->dt_allvar();
                        // 2. decrease pool meta ref_count
                        {
                            std::lock_guard<std::mutex> _G(*this->mtx);
                            *this->ref_count -= 1;
                            // 3. remove this client id from pool meta client_ids
                            std::uint32_t i;
                            for (i = 0; i < *this->ref_count; i++) {
                                if (this->client_ids()[i] == this->id) {
                                    for (; i < *this->ref_count - 1; i++)
                                        this->client_ids()[i] = this->client_ids()[i + 1];
                                    break;
                                }
                            }
                        }
                        // 4. detach pBuffer
                        if (this->pBuffer == nullptr)
                            logger->error("this->pBuffer is nullptr.");
                        if (shmdt(this->pBuffer) == -1)
                            logger->error("failed to shmdt pool meta. {}", strerror(errno));
                        this->pBuffer = nullptr;
                        // 5. set status to pool_status::POOL_DT
                        this->status = pool_status::POOL_DT;
                        // 6. check if need reply
                        if (msg.request.need_reply) {
                            // TODO:
                            int _fifo = open(msg.request.fifo_name, O_WRONLY);
                            resp response;
                            response.receiver = msg.request.sender;
                            response.sender = this->id;
                            response.status = e_status::OK;
                            write(_fifo, &response, sizeof(resp));
                        }
                        // break
                        break;
                    }
                    // ACTION Update
                    if (msg.request.action == AC_UPDATE) {
                    }
                } else if (msg.request.type == e_reqtype::PING) {
                    /*
                 * if request type is PING, fifo_name must be specified.
                 *  1. open fifo with Read only mode
                 *  2. write resp
                 *  3. close fifo
                 */
                } else if (msg.request.type == e_reqtype::TERM) {
                    /*
                     * Handling type TERM
                     * this should only send by itself. Usually a destructor is called.
                     * this will check sender first, if sender and receive has the same id as this->id.
                     * if true. following steps will take place:
                     */
                    // 1. check sender and receiver.
                    if (this->id == msg.request.sender && this->id == msg.request.receiver) {
                        // 2. detach all attached variable, which is stored in this->attached_vars
                        this->dt_allvar();
                        {
                            std::lock_guard<std::mutex> _G(*this->mtx);
                            // 3. decrease pool meta ref_count by 1
                            *this->ref_count -= 1;
                            // 4. remove this client id from pool meta client_ids
                            std::uint32_t i;
                            for (i = 0; i < *this->ref_count - 1; i++) {
                                if (this->client_ids()[i] == this->id) {
                                    for (; i < (*this->ref_count) - 1; i++)
                                        this->client_ids()[i] = this->client_ids()[i + 1];
                                    break;
                                }
                            }
                        }
                        // 5. detach pBuffer
                        this->dt_pmeta();
                        // 6. break while loop
                        break;
                    }
                }
            } else if (msg.request.target == e_target::VAR) {
                // TODO: Handle vars
            }
        }
    }
};