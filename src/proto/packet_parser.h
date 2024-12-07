#pragma once

#include <cstring>
#include <optional>
#include <vector>
#include "packet.h"

#if _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

namespace proto {
    template<typename TParent>
    class PacketParser {
    public:
        PacketParser(const Packet<TParent> &packet): _packet(packet), _data(packet.data) {}

        template<typename T> auto parse() -> std::optional<Packet<T>>;
        template<typename T> auto parse_seq() -> std::vector<Packet<T>>;
        template<typename T> auto parse_msg() -> std::optional<T>;

    private:
        const Packet<TParent> &_packet;
        const std::span<const uint8_t> _data;

        template<typename T>
        void copy_span(T *dst, const std::span<const uint8_t> src) const
        {
            std::memcpy(reinterpret_cast<uint8_t*>(dst), src.data(), sizeof(T));
        }
    };
}
