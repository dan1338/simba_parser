#include "ethernet.h"

#include <stdexcept>
#include "packet_parser.h"
#include "pcap.h"

namespace proto {
    template<>
    template<>
    auto PacketParser<void>::parse() -> std::optional<Packet<Ethernet>>
    {
        if (_data.size() < sizeof(Ethernet))
        {
            return {};
        }

        Packet<Ethernet> packet;
        copy_span(&packet.header, _data.first<sizeof(Ethernet)>());
        packet.header.type = static_cast<Ethernet::EtherType>(ntohs(packet.header.type));

        if (packet.header.type == Ethernet::ETH_DOT1Q)
        {
            return {}; // unsupported
        }

        packet.data = _data.subspan(sizeof(Ethernet));

        return packet;
    }

    template<>
    template<>
    auto PacketParser<PcapRecord>::parse() -> std::optional<Packet<Ethernet>>
    {
        return PacketParser<void>({_data}).parse<Ethernet>();
    }

    auto to_string(const Ethernet::Address &addr) -> std::string
    {
        char s[32];
        snprintf(s, sizeof s, "%02x:%02x:%02x:%02x:%02x:%02x",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

        return std::string{s};
    }

    auto to_string(const Ethernet::EtherType &ether_type) -> std::string
    {
        switch (ether_type)
        {
            case Ethernet::ETH_IPv4: return "IPv4";
            case Ethernet::ETH_IPv6: return "IPv6";
            case Ethernet::ETH_DOT1Q: return "802.1Q";
        }

        throw std::invalid_argument("ether_type");
    }
}
