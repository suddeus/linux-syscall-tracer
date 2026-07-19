#pragma once

#include <string>
#include <optional>
#include <vector>
#include <set>
#include <cstdint>

struct Config {
    std::optional<int> command_argc = std::nullopt;
    std::optional<char**> command_argv = std::nullopt;
    std::optional<std::uint64_t> process_pid = std::nullopt;
    std::optional<std::string> output_file = std::nullopt;
    std::optional<std::set<std::uint64_t>> traced_syscalls = std::nullopt;
};

std::optional<Config> getConfig(int argc, char* argv[]);