#include <iostream>

#include <config.hpp>

int main(const int argc, char* argv[]) {
    const auto config = getConfig(argc, argv);

    if (!config.has_value()) {
        return 0;
    }

    std::cout << "Traced Syscalls: ";
    if (!config->traced_syscalls.has_value()) {
        std::cout << "null" << '\n';
    } else {
        for (const auto& s : config->traced_syscalls.value()) {
            std::cout << s << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "Output file: " << config->output_file.value_or("null") << '\n';

    std::cout << "PID: " << (config->process_pid.has_value() ? std::to_string(config->process_pid.value()) : "null") << '\n';
    std::cout << "Command: " << config->command.value_or("null") << '\n';

    return 0;
}