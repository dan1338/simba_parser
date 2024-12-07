#pragma once

#include <cmath>
#include <cstdint>
#include <math.h>
#include <memory>
#include <variant>
#include <vector>

#include "packet.h"

namespace proto::simba {
    struct MarketData
    {
        uint32_t msg_seq_num;
        uint16_t msg_size;

        enum Flags : uint16_t {
            MDP_LAST_FRAGMENT       = 0x01,
            MDP_START_OF_SNAPSHOT   = 0x02,
            MDP_END_OF_SNAPSHOT     = 0x04,
            MDP_IS_INCREMENTAL      = 0x08,
            MDP_POSS_DUP_FLAG       = 0x10,
        } msg_flags;

        uint64_t sending_time;

        struct IncrementalHeader
        {
            uint64_t transact_time;
            uint32_t exchange_session_id;
        } __attribute__((packed)) incremental_header;;

        auto is_incremental() const -> bool { return msg_flags & MDP_IS_INCREMENTAL; }
    } __attribute__((packed));

    static_assert(sizeof(MarketData) == 28);
    static_assert(sizeof(MarketData::IncrementalHeader) == 12);

    struct SbeMessage
    {
        uint16_t block_length;

        enum TemplateId : uint16_t {
            SBE_SECURITY_DEF_UPDATE     = 10,
            SBE_TRADING_SESSION_STATUS  = 11,
            SBE_DISCRETE_AUCTION        = 13,
            SBE_ORDER_UPDATE            = 15,
            SBE_ORDER_EXECUTION         = 16,
            SBE_ORDER_BOOK_SNAPSHOT     = 17,
            SBE_SECURITY_DEFINITION     = 18,
        } template_id;

        uint16_t schema_id;
        uint16_t version;
    } __attribute__((packed));

    static_assert(sizeof(SbeMessage) == 8);

    template<int Exponent>
    struct Decimal
    {
        int64_t mantissa;
        explicit operator double() const { return pow(10.0, Exponent) * mantissa; }
    };

    enum MdFlagsSet : uint64_t
    {
        MDFS_DAY                    = 0x01,
        MDFS_IOC                    = 0x02,
        MDFS_OTC                    = 0x04,
        MDFS_END_OF_TRANSACTION     = 0x1000,
        MDFS_CANCEL_PASSIVE_XORDER  = 0x2000,
        MDFS_FILL_OR_KILL           = 0x80000,
        MDFS_ORDER_MOVE_RESULT      = 0x100000,
        MDFS_ORDER_CANCEL_RESULT    = 0x200000,
        MDFS_ORDER_MCANCEL_RESULT   = 0x400000,
        MDFS_NEGOTIATED_ORDER       = 0x4000000,
        MDFS_MULTI_LEG_ORDER        = 0x8000000,
        MDFS_ORDER_DELETION_XTRADE  = 0x20000000,
        MDFS_NEGOTIATED_BY_REF      = 0x80000000,
        MDFS_ORDER_CANCEL_DC_RESULT = 0x100000000,
        MDFS_SYNTHETIC_ORDER        = 0x200000000000,
        MDFS_RFS_ORDER              = 0x400000000000,
        MDFS_PASSIVE_SYNTHETIC_ORDER = 0x200000000000000,
        MDFS_BOOK_OR_CANCEL_ORDER   = 0x1000000000000000,
        MDFS_TRADE_DURING_OPENING   = 0x4000000000000000
    };

    enum MdUpdateAction : uint8_t
    {
        MDUA_NEW    = '0',
        MDUA_CHANGE = '1',
        MDUA_DELETE = '2',
    };

    enum MdEntryType : uint8_t
    {
        MDET_BID        = '0',
        MDET_OFFER      = '1',
        MDET_EMPTY_BOOK = 'J',
    };

    struct OrderUpdate
    {
        int64_t md_entry_id;
        Decimal<-5> md_entry_px;
        int64_t md_entry_size;
        MdFlagsSet md_flags;
        uint64_t md_flags2;
        int32_t security_id;
        uint32_t rpt_seq;
        MdUpdateAction md_update_action;
        MdEntryType md_entry_type;
    } __attribute__((packed));

    static_assert(sizeof(OrderUpdate) == 50);

    struct OrderExecution
    {
        int64_t md_entry_id;
        Decimal<-5> md_entry_px;
        int64_t md_entry_size;
        Decimal<-5> last_px;
        int64_t last_qty;
        int64_t trade_id;
        MdFlagsSet md_flags;
        MdFlagsSet md_flags2;
        int32_t security_id;
        uint32_t rpt_seq;
        MdUpdateAction md_update_action;
        MdEntryType md_entry_type;
    } __attribute__((packed));

    static_assert(sizeof(OrderExecution) == 74);

    struct OrderBookSnapshot
    {
        int32_t security_id;
        uint32_t last_msg_seq_number;
        uint32_t rpt_seq;
        uint32_t exchange_session_id;

        struct GroupSize {
            uint16_t block_length;
            uint8_t num_in_group;
        } __attribute__((packed)) group_size;

        struct Entry {
            int64_t md_entry_id;
            uint64_t transact_time;
            Decimal<-5> md_entry_px;
            int64_t md_entry_size;
            uint64_t trade_id;
            MdFlagsSet md_flags;
            MdFlagsSet md_flags2;
            MdEntryType md_entry_type;
        } __attribute__((packed));
    } __attribute__((packed));

    static_assert(sizeof(OrderBookSnapshot) == 19);
    static_assert(sizeof(OrderBookSnapshot::GroupSize) == 3);
    static_assert(sizeof(OrderBookSnapshot::Entry) == 57);

    template<typename TMessage>
    struct GroupMsg : TMessage
    {
        std::vector<typename TMessage::Entry> entries;
    };

    struct FixUnsupported
    {
        uint16_t template_id;
    };

    using FixMessage = std::variant<
        OrderUpdate,
        OrderExecution,
        GroupMsg<OrderBookSnapshot>,
        FixUnsupported
    >;
}
