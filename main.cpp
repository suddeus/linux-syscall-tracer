#include <iostream>

#include <config.hpp>
#include <child_starter.hpp>
#include <tracer.hpp>

int main(const int argc, char* argv[]) {
    const auto config = getConfig(argc, argv);

    if (!config.has_value()) {
        return 0;
    }

    Tracer proc_tracer(config.value());

    proc_tracer.trace();

    // std::cout << "Traced Syscalls: ";
    // if (!config->traced_syscalls.has_value()) {
    //     std::cout << "null" << '\n';
    // } else {
    //     for (const auto& s : config->traced_syscalls.value()) {
    //         std::cout << s << ' ';
    //     }
    //     std::cout << '\n';
    // }
    //
    // std::cout << "Output file: " << config->output_file.value_or("null") << '\n';
    //
    // std::cout << "PID: " << (config->process_pid.has_value() ? std::to_string(config->process_pid.value()) : "null") << '\n';
    // std::cout << "Command: \n\t argc:" << config->command_argc.value_or(-1) << '\n';
    // if (config->command_argc.has_value()) {
    //     std::cout << "\t argv:" << '\n';
    //     for (int i = 0; i < config->command_argc; ++i) {
    //         std::cout << "\t\t" << *(config->command_argv.value() + i) << '\n';
    //     }
    // }

    return 0;
}
