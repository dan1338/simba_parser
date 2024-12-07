#pragma once

#include <chrono>
#include <cstdint>

namespace proto {
    struct PcapRecord
    {
        using Timestamp = std::chrono::duration<uint64_t, std::nano>;

        size_t index;
        Timestamp timestamp;
        size_t original_size;
    };
}