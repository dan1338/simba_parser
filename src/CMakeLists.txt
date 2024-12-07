add_library(libsimba
        proto/ethernet.cpp
        proto/ethernet.h
        proto/ipv4.cpp
        proto/ipv4.h
        proto/udp.cpp
        proto/udp.h
        proto/simba.cpp
        proto/simba.h
        proto/packet.h
        proto/pcap.h
        pcap/file_reader.cpp
        pcap/file_reader.h
        proto/packet_parser.h
)

target_include_directories(libsimba PUBLIC .)

if (WIN32)
    target_link_libraries(libsimba PUBLIC ws2_32)
endif ()

add_executable(simba main.cpp
        json_serializer.cpp
        json_serializer.h)
target_link_libraries(simba PRIVATE libsimba)
