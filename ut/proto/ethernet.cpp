#include <gtest/gtest.h>

#include "proto/packet_parser.h"
#include "proto/ethernet.h"

using namespace proto;

static constexpr unsigned char packet_dump[] = {
    0x1, 0x0, 0x5e, 0x43, 0x14, 0x56, 0x78, 0xac, 0x44, 0x3e, 0x22, 0x42, 0x8, 0x0
};

static constexpr auto dst_address = Ethernet::Address{0x01, 0x00, 0x5e, 0x43, 0x14, 0x56};
static constexpr auto src_address = Ethernet::Address{0x78, 0xac, 0x44, 0x3e, 0x22, 0x42};
static constexpr auto ether_type = Ethernet::ETH_IPv4;

TEST(TestEthernet, ParsePacket)
{
    const auto ethernet_packet{PacketParser(RawPacket{packet_dump}).parse<Ethernet>()};

    EXPECT_TRUE(ethernet_packet.has_value());

    EXPECT_EQ(ethernet_packet->header.dst_address, dst_address);
    EXPECT_EQ(ethernet_packet->header.src_address, src_address);
    EXPECT_EQ(ethernet_packet->header.type, ether_type);

    EXPECT_TRUE(ethernet_packet->data.empty());
}
