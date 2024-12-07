#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

auto load_file(const std::string &path) -> std::vector<uint8_t>
{
    if (std::ifstream file{path, std::ios::binary}; file)
    {
        return std::vector<uint8_t>(std::istreambuf_iterator{file}, {});
    }

    throw std::runtime_error("Could not open file");
}