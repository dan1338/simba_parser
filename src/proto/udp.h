#pragma once

#include <cstdint>
#include "packet.h"

namespace proto {
    struct Udp
    {
        uint16_t src_port;
        uint16_t dst_port;
        uint16_t length;
        uint16_t checksum;
    } __attribute__((packed));

    static_assert(sizeof(Udp) == 8);
}
