#pragma once

/*
 * 1 is pre-defined message type which is specific used to communicate with Server.
 * other vlaue > 1 is used as client message type. Each client should have different
 * message type.
 */
#define PMSGT_SRV 1 

enum action {
    DESTROY,
    DETACH,
    UPDATE
};

struct pmsg_body {
    bool nd_reply;
    action act;
};

struct vmsg_body {
    bool nd_reply;
    unsigned char size;
    action act;
    long to;
    std::uint32_t var_indexs[16];
};

struct pmsg_t {
    long msg_type;
    pmsg_body body;
};

struct vmsg_t {
    long msg_type;
    vmsg_body body;
};