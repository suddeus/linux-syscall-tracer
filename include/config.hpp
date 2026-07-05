#pragma once

#include <string>
#include <optional>
#include <vector>
#include <cstdint>

struct Config {
    std::optional<std::string> command = std::nullopt;
    std::optional<std::uint64_t> process_pid = std::nullopt;
    std::optional<std::string> output_file = std::nullopt;
    std::optional<std::vector<std::uint64_t>> traced_syscalls = std::nullopt;
};

static Config getConfig(int argc, char** argv);