#include "ipv4.h"

#include "packet_parser.h"
#include "ethernet.h"
#include <stdexcept>

namespace offsets {
    static constexpr auto field_00 = 0x00;
    static constexpr auto field_01 = 0x01;
    static constexpr auto total_length = 0x02;
    static constexpr auto identification = 0x04;
    static constexpr auto field_06 = 0x06;
    static constexpr auto ttl = 0x08;
    static constexpr auto protocol = 0x09;
    static constexpr auto header_checksum = 0x0a;
    static constexpr auto src_address = 0x0c;
    static constexpr auto dst_address = 0x10;

    static constexpr auto noopt_payload = 0x14;
}

namespace proto {
    template<>
    template<>
    auto PacketParser<void>::parse() -> std::optional<Packet<IPv4>>
    {
        if (_data.size() < offsets::noopt_payload)
        {
            return {};
        }

        Packet<IPv4> packet;
        copy_span(&packet.header, _data.first<sizeof(IPv4)>());
        packet.header.total_length = htons(packet.header.total_length);
        packet.header.identification = htons(packet.header.identification);
        packet.header.field_06 = htons(packet.header.field_06);
        packet.header.header_checksum = htons(packet.header.header_checksum);

        if (packet.header.version != 4)
        {
            return {};
        }

        const size_t header_length = packet.header.ihl * 4;

        if (header_length < offsets::noopt_payload || _data.size() < packet.header.total_length)
        {
            return {};
        }

        packet.data = _data.subspan(header_length);

        return packet;
    }

    template<>
    template<>
    auto PacketParser<Ethernet>::parse() -> std::optional<Packet<IPv4>>
    {
        return PacketParser<void>({_data}).parse<IPv4>();
    }

    auto to_string(const IPv4::Address &addr) -> std::string
    {
        char s[32];
        snprintf(s, sizeof s, "%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3]);

        return std::string{s};
    }
}