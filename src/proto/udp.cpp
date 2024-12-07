#include "udp.h"

#include "ipv4.h"
#include "packet_parser.h"

namespace proto {
    template<>
    template<>
    auto PacketParser<void>::parse() -> std::optional<Packet<Udp>>
    {
        if (_data.size() < sizeof(Udp))
        {
            return {};
        }

        Packet<Udp> packet;
        copy_span(&packet.header, _data.first<sizeof(Udp)>());
        packet.header.src_port = htons(packet.header.src_port);
        packet.header.dst_port = htons(packet.header.dst_port);
        packet.header.length = htons(packet.header.length);
        packet.header.checksum = htons(packet.header.checksum);

        if (_data.size() < packet.header.length)
        {
            return {};
        }

        packet.data = _data.subspan(sizeof(Udp), packet.header.length - sizeof(Udp));

        return packet;
    }

    template<>
    template<>
    auto PacketParser<IPv4>::parse() -> std::optional<Packet<Udp>>
    {
        return PacketParser<void>({_data}).parse<Udp>();
    }
}
