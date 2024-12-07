#include "file_reader.h"

#include <algorithm>
#include <span>
#include <stdexcept>

// https://www.ietf.org/archive/id/draft-gharris-opsawg-pcap-01.html#name-file-header
struct FileHeader
{
    uint32_t magic;
    uint16_t major_version;
    uint16_t minor_version;
    uint32_t reserved_1;
    uint32_t reserved_2;
    uint32_t snap_len;
    uint16_t fcs    : 4;
    uint16_t zeros  : 12;
    uint16_t link_type;
} __attribute__((packed));

static_assert(sizeof(FileHeader) == 24);

// https://www.ietf.org/archive/id/draft-gharris-opsawg-pcap-01.html#name-packet-record
struct PacketHeader
{
    uint32_t timestamp;
    uint32_t timestamp_sub;
    uint32_t captured_length;
    uint32_t original_length;
} __attribute__((packed));

static_assert(sizeof(PacketHeader) == 16);

namespace pcap {
    FileReader::FileReader(FILE *fp, bool relative_timestamps):
        _fp(fp),
        _relative_timestamps(relative_timestamps)
    {
        if (!fp)
        {
            throw std::invalid_argument("fp is null");
        }

        FileHeader file_header;

        if (fread(&file_header, 1, sizeof file_header, _fp) != sizeof file_header)
        {
            throw std::runtime_error("fread file_header failed");
        }

        if (file_header.magic == 0xA1B2C3D4)
        {
            _file_info.timestamp_is_nano = false;
        }
        else if (file_header.magic == 0xA1B23C4D)
        {
            _file_info.timestamp_is_nano = true;
        }
        else
        {
            throw std::runtime_error("invalid header magic " + std::to_string(file_header.magic));
        }

        _file_info.major_version = file_header.major_version;
        _file_info.minor_version = file_header.minor_version;
        _file_info.snap_length = file_header.snap_len;

        // ignore fcs and link type
    }

    auto FileReader::next_record() -> ::std::optional<proto::Packet<proto::PcapRecord>>
    {
        PacketHeader packet_header;

        if (fread(&packet_header, 1, sizeof packet_header, _fp) != sizeof packet_header)
        {
            if (feof(_fp))
            {
                return {}; // No more packets available
            }
            else
            {
                throw std::runtime_error("fread packet_header failed");
            }
        }

        const size_t packet_size = packet_header.captured_length;

        if (_packet_data.size() < packet_size)
        {
            const size_t new_size = std::max(packet_size, _packet_data.size() * 2);
            _packet_data.resize(new_size);
        }

        if (fread(_packet_data.data(), 1, packet_size, _fp) != packet_size)
        {
            throw std::runtime_error("fread packet_data failed");
        }

        using namespace std::chrono_literals;

        proto::Packet<proto::PcapRecord> packet;
        packet.header.index = _index_counter++;
        packet.header.timestamp = _file_info.timestamp_is_nano
            ? (packet_header.timestamp * 1s) + (packet_header.timestamp_sub * 1ns)
            : (packet_header.timestamp * 1s) + (packet_header.timestamp_sub * 1us);
        packet.header.original_size = packet_header.original_length;
        packet.data = std::span(_packet_data.data(), packet_size);

        if (_relative_timestamps)
        {
            if (!_first_timestamp)
            {
                _first_timestamp.emplace(packet.header.timestamp);
            }

            packet.header.timestamp -= *_first_timestamp;
        }

        return packet;
    }
}
