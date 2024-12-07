#include <cstdio>
#include <cstdlib>
#include <getopt.h>

#include "pcap/file_reader.h"
#include "proto/packet_parser.h"
#include "proto/ethernet.h"
#include "proto/ipv4.h"
#include "proto/simba.h"
#include "proto/udp.h"
#include "json_serializer.h"

using namespace std::chrono_literals;

void process_pcap(FILE *fpin, FILE *fpout)
{
    using proto::PacketParser;

    pcap::FileReader reader(fpin);
    JsonSerializer serializer(fpout);

    while (true)
    {
        const auto pcap_record{reader.next_record()};

        if (!pcap_record)
        {
            break;
        }

        const auto ethernet_packet{PacketParser(*pcap_record).parse<proto::Ethernet>()};
        const auto ip_packet{PacketParser(*ethernet_packet).parse<proto::IPv4>()};

        if (!ip_packet || ip_packet->header.protocol != proto::IPv4::IP_UDP)
        {
            continue;
        }

        const auto udp_packet{PacketParser(*ip_packet).parse<proto::Udp>()};

        if (!udp_packet)
        {
            continue;
        }

        const auto md_packet{PacketParser(*udp_packet).parse<proto::simba::MarketData>()};

        if (!md_packet)
        {
            continue;
        }

        const auto sbe_messages{PacketParser(*md_packet).parse_seq<proto::simba::SbeMessage>()};

        if (sbe_messages.empty())
        {
            continue;
        }

        std::vector<proto::simba::FixMessage> fix_messages;

        for (const auto &sbe_message : sbe_messages)
        {
            const auto fix_message{PacketParser(sbe_message).parse_msg<proto::simba::FixMessage>()};

            if (fix_message.has_value())
            {
                fix_messages.push_back(*fix_message);
            }
        }

        serializer.serialize_messages(md_packet->header, fix_messages);
    }
}

struct ProgramArgs
{
    const char *input_path{nullptr};
    const char *output_path{nullptr};
};

auto parse_args(int argc, char **argv) -> ProgramArgs
{
    ProgramArgs args{};

    int opt;
    while (opt = getopt(argc, argv, "i:o:"), opt != -1)
    {
        switch (opt)
        {
            case 'i': args.input_path = optarg; break;
            case 'o': args.output_path = optarg; break;
            case '?': fprintf(stderr, "Unknown option %c", optopt); break;
            case ':': fprintf(stderr, "Argument required for option %c", optopt); break;
            default: break;
        }
    }

    return args;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: %s -i file.pcap -o file.json\n", *argv);
        return 1;
    }

    const auto args = parse_args(argc, argv);

    if (!args.input_path || !args.output_path)
    {
        printf("usage: %s -i file.pcap -o file.json\n", *argv);
        return 1;
    }

    FILE *fpin = fopen(args.input_path, "rb");

    if (!fpin)
    {
        perror("Cannot open file");
        return 1;
    }

    FILE *fpout = fopen(args.output_path, "wb");

    if (!fpout)
    {
        perror("Cannot open file");
        return 1;
    }

    try
    {
        process_pcap(fpin, fpout);
    }
    catch (const std::exception &e)
    {
        printf("Exception thrown: %s\n", e.what());
    }

    fclose(fpin);
    fclose(fpout);

    return 0;
}
