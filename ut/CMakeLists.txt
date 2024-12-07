enable_testing()

add_executable(ut_proto
        proto/ethernet.cpp
        proto/ipv4.cpp
        proto/udp.cpp
        proto/simba.cpp
        utils.h)
target_link_libraries(ut_proto GTest::gtest_main libsimba)

include(GoogleTest)
gtest_discover_tests(ut_proto)
