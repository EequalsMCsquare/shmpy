#pragma once

/*
 * 1 is pre-defined message type which is specific used to communicate with Server.
 * other vlaue > 1 is used as client message type. Each client should have different
 * message type.
 */
#define PMSGT_SRV 1 

enum e_action {
    AC_DETACH,
    AC_UPDATE
};

enum e_target { 
    CLN, // Client
    SVR, // Server
    VAR // Variable
};

enum e_reqtype {
    CMD, // command
    PING, // ping
    TERM // terminate. this reqtype should only send by the destructor.
};

enum e_status {
    OK,
    FAIL,
};

struct resp {
    std::uint32_t sender;
    std::uint32_t receiver;
    e_status status;  
};

struct req {
    std::uint32_t sender;
    std::uint32_t receiver;
    e_reqtype type;
    bool need_reply;
    e_target target;
    e_action action;
    char fifo_name[16];
    char size;
    std::uint32_t var_indexs[16];
};

struct msg_t {
    long msg_type;
    req request;
};
