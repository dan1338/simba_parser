#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace proto {
    struct Ethernet
    {
        using Address = std::array<uint8_t, 6>;

        Address dst_address;
        Address src_address;

        enum EtherType : uint16_t {
            ETH_IPv4 = 0x0800,
            ETH_IPv6 = 0x86DD,
            ETH_DOT1Q = 0x8100,
        } type;
    } __attribute__((packed));

    static_assert(sizeof(Ethernet) == 14);

    auto to_string(const Ethernet::Address &addr) -> std::string;
    auto to_string(const Ethernet::EtherType &ether_type) -> std::string;
}
