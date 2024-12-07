#pragma once

#include <array>
#include <cstdint>
#include <string>
#include "packet.h"

namespace proto {
    struct IPv4
    {
        using Address = std::array<uint8_t, 4>;

        union {
            struct {
                uint8_t ihl     : 4;
                uint8_t version : 4;
            };
            uint8_t field_00;
        };

        union {
            struct {
                uint8_t ecn     : 2;
                uint8_t dscp    : 6;
            };
            uint8_t field_01;
        };

        uint16_t total_length;
        uint16_t identification;

        union {
            struct {
                uint16_t fragment_offset : 13;
                uint16_t flags  : 3;
            };
            uint16_t field_06;
        };

        uint8_t ttl;

        enum Protocol : uint8_t {
            IP_ICMP = 1,
            IP_IGMP = 2,
            IP_TCP = 6,
            IP_UDP = 17,
        } protocol;

        uint16_t header_checksum;

        Address src_address;
        Address dst_address;

        // Ignore options
    } __attribute__((packed));

    static_assert(sizeof(IPv4) == 20);

    auto to_string(const IPv4::Address &addr) -> std::string;
}
