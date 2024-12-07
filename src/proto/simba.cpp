#include "simba.h"

#include <stdexcept>

#include "packet_parser.h"
#include "udp.h"

namespace proto {
    using namespace simba;

    template<>
    template<>
    auto PacketParser<void>::parse() -> std::optional<Packet<MarketData>>
    {
        constexpr auto market_data_minsize = sizeof(MarketData) - sizeof(MarketData::IncrementalHeader);

        if (_data.size() < market_data_minsize)
        {
            return {};
        }

        Packet<MarketData> packet;
        copy_span(&packet.header, _data.first<market_data_minsize>());

        if (_data.size() < packet.header.msg_size)
        {
            return {};
        }

        if (packet.header.is_incremental())
        {
            copy_span(&packet.header.incremental_header, _data.subspan(market_data_minsize, sizeof(MarketData::IncrementalHeader)));
            packet.data = _data.subspan(sizeof(MarketData));
        }
        else
        {
            packet.data = _data.subspan(market_data_minsize);
        }

        return packet;
    }

    template<>
    template<>
    auto PacketParser<Udp>::parse() -> std::optional<Packet<MarketData>>
    {
        return PacketParser<void>({_data}).parse<MarketData>();
    }

    template<>
    template<>
    auto PacketParser<MarketData>::parse_seq() -> std::vector<Packet<SbeMessage>>
    {
        if (_data.size() < sizeof(SbeMessage))
        {
            return {};
        }

        std::vector<Packet<SbeMessage>> packets;

        if (_packet.header.is_incremental())
        {
            size_t offset{0};

            while (_data.size() >= offset + sizeof(SbeMessage))
            {
                auto &packet = packets.emplace_back();
                copy_span(&packet.header, _data.subspan(offset).first<sizeof(SbeMessage)>());

                if (_data.size() < offset + sizeof(SbeMessage) + packet.header.block_length)
                {
                    break; // block_length too large
                }

                packet.data = _data.subspan(offset + sizeof(SbeMessage), packet.header.block_length);
                offset += sizeof(SbeMessage) + packet.header.block_length;
            }
        }
        else // SNAPSHOT
        {
            auto &packet = packets.emplace_back();
            copy_span(&packet.header, _data.first<sizeof(SbeMessage)>());

            struct GroupSize {
                uint16_t block_length;
                uint8_t num_in_group;
            } __attribute__((packed)) group_size;
            static_assert(sizeof(GroupSize) == 3);

            const size_t offset = sizeof(SbeMessage) + packet.header.block_length;

            if (_data.size() < offset + sizeof(GroupSize))
            {
                return {}; // block_length too large
            }

            copy_span(&group_size, _data.subspan(offset, sizeof(GroupSize)));

            const size_t repeat_size = group_size.block_length * group_size.num_in_group;
            const size_t total_size = packet.header.block_length + sizeof(GroupSize) + repeat_size;

            packet.data = _data.subspan(sizeof(SbeMessage), total_size);
        }

        return packets;
    }

    template<>
    template<>
    auto PacketParser<SbeMessage>::parse_msg() -> std::optional<FixMessage>
    {
        switch (_packet.header.template_id)
        {
            case SbeMessage::SBE_ORDER_UPDATE: {
                OrderUpdate msg;
                copy_span(&msg, _data.first<sizeof(OrderUpdate)>());
                return msg;
            }
            case SbeMessage::SBE_ORDER_EXECUTION: {
                OrderExecution msg;
                copy_span(&msg, _data.first<sizeof(OrderExecution)>());
                return msg;
            }
            case SbeMessage::SBE_ORDER_BOOK_SNAPSHOT: {
                GroupMsg<OrderBookSnapshot> msg;
                copy_span(&static_cast<OrderBookSnapshot&>(msg), _data.first<sizeof(OrderBookSnapshot)>());

                size_t offset = sizeof(OrderBookSnapshot);

                while (_data.size() >= offset + sizeof(OrderBookSnapshot::Entry))
                {
                    auto &entry = msg.entries.emplace_back();
                    copy_span(&entry, _data.subspan(offset, sizeof(OrderBookSnapshot::Entry)));

                    offset += sizeof(OrderBookSnapshot::Entry);
                }

                return msg;
            }
            default: {
                return {FixUnsupported{_packet.header.template_id}};
            }
        }
    }
}
