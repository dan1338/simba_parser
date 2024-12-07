#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <cstdio>

#include "proto/packet.h"
#include "proto/pcap.h"

namespace pcap {
    struct FileInfo
    {
        int major_version;
        int minor_version;
        size_t snap_length;
        bool timestamp_is_nano;
    };

    class FileReader
    {
    public:
        explicit FileReader(FILE *fp, bool relative_timestamps=true);
        auto get_file_info() const -> const FileInfo& { return _file_info; }
        auto next_record() -> std::optional<proto::Packet<proto::PcapRecord>>;

    private:
        FILE *_fp;
        bool _relative_timestamps;

        FileInfo _file_info;
        size_t _index_counter{0};
        std::optional<proto::PcapRecord::Timestamp> _first_timestamp;
        std::vector<uint8_t> _packet_data;
    };
}
