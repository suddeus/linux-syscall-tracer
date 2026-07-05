#pragma once

#include <string>
#include <optional>
#include <vector>
#include <set>
#include <cstdint>

struct Config {
    std::optional<std::string> command = std::nullopt;
    std::optional<std::uint64_t> process_pid = std::nullopt;
    std::optional<std::string> output_file = std::nullopt;
    std::optional<std::set<std::uint64_t>> traced_syscalls = std::nullopt;
};

std::optional<Config> getConfig(int argc, char* argv[]);