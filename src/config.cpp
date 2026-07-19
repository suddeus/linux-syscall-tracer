#include <config.hpp>

#include <iostream>
#include <string>
#include <ranges>

#include <syscalls.hpp>


namespace {

const auto HELP_TEXT = R"(Usage:
    syscall_tracer [options] command [args...]
    syscall_tracer [options] -p PID

Trace system calls made by a process.

Modes:
    command [args...]        Run a new command and trace it
    -p, --pid PID            Attach to an existing process

Options:
    -o, --output FILE        Write trace output to FILE instead of stdout
    -e LIST                  Trace only selected syscalls
                             Example: -e read,write)";

void printHelpPage() {
    std::cout << HELP_TEXT << '\n';
}

std::optional<std::set<std::uint64_t>> getTracedSyscalls(const std::string& list) {
    std::set<std::uint64_t> syscalls;

    auto syscalls_list = list | std::views::split(',');

    for (const auto& word: syscalls_list) {
        const std::string_view curr_syscall(word.begin(), word.end());

        auto it = std::ranges::find_if(
            SYSCALLS.begin(),SYSCALLS.end(), [=](const SyscallInfo& syscall) {
                return syscall.name == curr_syscall;
            }
        );

        if (it == SYSCALLS.end()) {
            std::cerr << "Unknown syscall: " << curr_syscall << '\n';
            return std::nullopt;
        }
        syscalls.insert(it->number);
    }

    return syscalls;
}

}

std::optional<Config> getConfig(const int argc, char* argv[]) {
    Config result_config;

    int curr_arg_number = 1;
    std::string curr_arg = argv[curr_arg_number];

    while (curr_arg == "-h" || curr_arg == "--help" || curr_arg == "-e" || curr_arg == "-o" || curr_arg == "--output") {
        if (curr_arg == "-h" || curr_arg == "--help") {
            printHelpPage();
            return std::nullopt;
        }
        if (curr_arg == "-e") {
            result_config.traced_syscalls = getTracedSyscalls(argv[++curr_arg_number]);
            if (!result_config.traced_syscalls.has_value()) {
                return std::nullopt;
            }
        } else if (curr_arg == "-o" || curr_arg == "--output") {
            result_config.output_file = argv[++curr_arg_number];
        }
        curr_arg = argv[++curr_arg_number];
    }

    if (curr_arg == "-p" || curr_arg == "--pid") {
        result_config.command_argc = std::nullopt;
        result_config.command_argv = std::nullopt;
        curr_arg = argv[++curr_arg_number];
        result_config.process_pid = std::strtol(curr_arg.c_str(), nullptr, 10);
    } else {
        result_config.process_pid = std::nullopt;
        result_config.command_argc = argc - curr_arg_number;
        result_config.command_argv = argv + curr_arg_number;
    }

    return result_config;
}
