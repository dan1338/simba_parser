#pragma once

#include <cstdio>
#include "proto/simba.h"

class JsonSerializer
{
public:
    JsonSerializer(FILE *fp, bool flush_each_line = false);
    ~JsonSerializer();

    void serialize_messages(const proto::simba::MarketData &md_packet, const std::span<const proto::simba::FixMessage> &fix_messages);

private:
    FILE *_fp;
    bool _flush_each_line;

    bool _is_first_message{true};

    void serialize(const proto::simba::OrderExecution &order_execution);
    void serialize(const proto::simba::OrderUpdate &order_update);
    void serialize(const proto::simba::GroupMsg<proto::simba::OrderBookSnapshot> &order_book_snapshot);

    void serialize(const proto::simba::FixMessage &fix_message);
};

