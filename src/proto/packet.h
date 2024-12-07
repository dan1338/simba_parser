#pragma once

#include <span>
#include <cstdint>

namespace proto {
    template<typename THeader>
    struct Packet
    {
        THeader header;
        std::span<const uint8_t> data;
    };

    template<>
    struct Packet<void>
    {
        std::span<const uint8_t> data;
    };

    using RawPacket = Packet<void>;
}
