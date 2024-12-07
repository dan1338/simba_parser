#include "json_serializer.h"

#include <stdexcept>

JsonSerializer::JsonSerializer(FILE *fp, bool flush_each_line): _fp(fp), _flush_each_line(flush_each_line)
{
    if (!_fp)
    {
        throw std::invalid_argument("fp is null");
    }

    setvbuf(_fp, nullptr, _IOFBF, 4096 * 32);

    fputc('[', _fp);
}

JsonSerializer::~JsonSerializer()
{
    fputc(']', _fp);
}

// Before this function I had each field in a separate fprintf but obviously it wasn't particularly fast
// This appears to improve the performance by ~60%, but ideally I'd prefer to use a library like fmt
template<bool trailing_comma>
static void write_json_kvpairs(FILE *fp, const std::vector<std::pair<std::string, std::string>> &kvpairs)
{
    std::string s;
    s.reserve(0x1000);

    for (size_t i = 0; i < kvpairs.size(); ++i)
    {
        s += '"' + kvpairs[i].first + '"';
        s += ':' + kvpairs[i].second;

        if (i + 1 != kvpairs.size() || trailing_comma)
        {
            s += ',';
        }
    }

    fputs(s.c_str(), fp);
}

void JsonSerializer::serialize_messages(const proto::simba::MarketData &md_packet, const std::span<const proto::simba::FixMessage> &fix_messages)
{
    if (!_is_first_message)
    {
        fputs(",{", _fp);
    }
    else
    {
        fputc('{', _fp);
    }

    write_json_kvpairs<false>(_fp, {
        {"msg_seq_num", std::to_string(md_packet.msg_seq_num)},
        {"msg_size", std::to_string(md_packet.msg_size)},
        {"msg_flags", std::to_string(md_packet.msg_flags)},
        {"sending_time", std::to_string(md_packet.sending_time)},
        {"fix_messages", "["},
    });

    for (size_t i = 0; i < fix_messages.size(); i++)
    {
        serialize(fix_messages[i]);

        if (i + 1 != fix_messages.size())
        {
            fputc(',', _fp);
        }
    }

    fputs("]}", _fp);

    if (_flush_each_line)
    {
        fflush(_fp);
    }

    _is_first_message = false;
}

void JsonSerializer::serialize(const proto::simba::OrderExecution &order_execution)
{
    write_json_kvpairs<false>(_fp, {
        {"md_entry_id", std::to_string(order_execution.md_entry_id)},
        {"md_entry_px", std::to_string(static_cast<double>(order_execution.md_entry_px))},
        {"md_entry_size", std::to_string(order_execution.md_entry_size)},
        {"last_px", std::to_string(static_cast<double>(order_execution.last_px))},
        {"last_qty", std::to_string(order_execution.last_qty)},
        {"trade_id", std::to_string(order_execution.trade_id)},
        {"md_entry_flags", std::to_string(order_execution.md_flags)},
        {"md_entry_flags2", std::to_string(order_execution.md_flags2)},
        {"security_id", std::to_string(order_execution.security_id)},
        {"rpt_seq", std::to_string(order_execution.rpt_seq)},
        {"md_update_action", std::to_string(order_execution.md_update_action)},
        {"md_entry_type", std::to_string(order_execution.md_entry_type)},
    });
}

void JsonSerializer::serialize(const proto::simba::OrderUpdate &order_update)
{
    write_json_kvpairs<false>(_fp, {
        {"md_entry_id", std::to_string(order_update.md_entry_id)},
        {"md_entry_px", std::to_string(static_cast<double>(order_update.md_entry_px))},
        {"md_entry_size", std::to_string(order_update.md_entry_size)},
        {"md_entry_flags", std::to_string(order_update.md_flags)},
        {"md_entry_flags2", std::to_string(order_update.md_flags2)},
        {"security_id", std::to_string(order_update.security_id)},
        {"rpt_seq", std::to_string(order_update.rpt_seq)},
        {"md_update_action", std::to_string(order_update.md_update_action)},
        {"md_entry_type", std::to_string(order_update.md_entry_type)}
    });
}

void JsonSerializer::serialize(const proto::simba::GroupMsg<proto::simba::OrderBookSnapshot> &order_book_snapshot)
{
    write_json_kvpairs<false>(_fp, {
        {"security_id", std::to_string(order_book_snapshot.security_id)},
        {"last_msg_seq_number", std::to_string(order_book_snapshot.last_msg_seq_number)},
        {"rpt_seq", std::to_string(order_book_snapshot.rpt_seq)},
        {"exchange_session_id", std::to_string(order_book_snapshot.exchange_session_id)},
        {"group_size",
            "{\"block_length\": " + std::to_string(order_book_snapshot.group_size.block_length) +
            ", \"num_in_group\": " + std::to_string(order_book_snapshot.group_size.num_in_group) + "}"
        },
        {"entries", "["}
    });

    for (size_t i = 0; i < order_book_snapshot.entries.size(); i++)
    {
        const auto &entry{order_book_snapshot.entries[i]};

        fputc('{', _fp);
        write_json_kvpairs<false>(_fp, {
            {"md_entry_id", std::to_string(entry.md_entry_id)},
            {"transact_time", std::to_string(entry.transact_time)},
            {"md_entry_px", std::to_string(static_cast<double>(entry.md_entry_px))},
            {"md_entry_size", std::to_string(entry.md_entry_size)},
            {"trade_id", std::to_string(entry.trade_id)},
            {"md_flags", std::to_string(entry.md_flags)},
            {"md_flags2", std::to_string(entry.md_flags2)},
            {"md_entry_type", std::to_string(entry.md_entry_type)}
        });

        if (i + 1 == order_book_snapshot.entries.size())
        {
            fputc('}', _fp);
        }
        else
        {
            fputs("},", _fp);
        }
    }

    fputc(']', _fp);
}

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

void JsonSerializer::serialize(const proto::simba::FixMessage &fix_message)
{
    using namespace proto::simba;

    std::visit(overloaded{
       [&](const OrderExecution &msg) {
           fputs(R"({"type":"OrderExecution","value":{)", _fp);
           serialize(msg);
           fputs("}}", _fp);
       },
       [&](const OrderUpdate &msg) {
           fputs(R"({"type":"OrderUpdate","value":{)", _fp);
           serialize(msg);
           fputs("}}", _fp);
       },
       [&](const GroupMsg<OrderBookSnapshot> &msg) {
           fputs(R"({"type":"OrderBookSnapshot","value":{)", _fp);
           serialize(msg);
           fputs("}}", _fp);
       },
       [&](const FixUnsupported &msg) {
           fprintf(_fp, R"({"type":"FixUnsupported","template_id":%hu})", msg.template_id);
       }
    }, fix_message);
}
