#include <gtest/gtest.h>
#include <variant>

#include "proto/packet_parser.h"
#include "proto/simba.h"

#include "../utils.h"

using namespace proto;
using namespace simba;

TEST(TestSimba, ParsePacketIncremental)
{
    const auto packet_dump = load_file("../../ut/proto/bin/md_incremental_1.bin");
    const auto md_packet{PacketParser(RawPacket{packet_dump}).parse<MarketData>()};

    EXPECT_TRUE(md_packet.has_value());
    EXPECT_TRUE(md_packet->header.is_incremental());

    const auto sbe_messages{PacketParser(*md_packet).parse_seq<SbeMessage>()};

    EXPECT_EQ(sbe_messages.size(), 1);

    const auto fix_message{PacketParser(sbe_messages[0]).parse_msg<FixMessage>()};

    EXPECT_TRUE(fix_message.has_value());
    EXPECT_TRUE(std::holds_alternative<FixUnsupported>(*fix_message));
    EXPECT_EQ(std::get<FixUnsupported>(*fix_message).template_id, 10);
}

TEST(TestSimba, ParsePacketSnapshot)
{
    const auto packet_dump = load_file("../../ut/proto/bin/md_snapshot.bin");
    const auto md_packet{PacketParser(RawPacket{packet_dump}).parse<MarketData>()};

    EXPECT_TRUE(md_packet.has_value());
    EXPECT_FALSE(md_packet->header.is_incremental());

    const auto sbe_messages{PacketParser(*md_packet).parse_seq<SbeMessage>()};

    EXPECT_EQ(sbe_messages.size(), 1);

    const auto fix_message{PacketParser(sbe_messages[0]).parse_msg<FixMessage>()};

    EXPECT_TRUE(fix_message.has_value());
    EXPECT_TRUE(std::holds_alternative<GroupMsg<OrderBookSnapshot>>(*fix_message));
    
    const auto order_book_snapshot{std::get<GroupMsg<OrderBookSnapshot>>(*fix_message)};

    EXPECT_EQ(order_book_snapshot.security_id, 3036264);
    EXPECT_EQ(order_book_snapshot.last_msg_seq_number, 4089);
    EXPECT_EQ(order_book_snapshot.rpt_seq, 24);
    EXPECT_EQ(order_book_snapshot.exchange_session_id, 6902);
    EXPECT_EQ(order_book_snapshot.entries.size(), 23);
}
