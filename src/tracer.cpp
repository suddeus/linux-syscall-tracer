#include <functional>
#include <tracer.hpp>

#include <wait.h>
#include <iostream>
#include <sys/ptrace.h>
#include <algorithm>
#include <sys/user.h>
#include <sys/uio.h>
#include <elf.h>

#include "syscalls.hpp"

Tracer::Tracer(const Config& cfg) {
    if (cfg.command_argv.has_value()) {
        starter_ = std::make_unique<starter::ExecuteProgram>(cfg.command_argv.value());
    } else if (cfg.process_pid.has_value()) {
        throw std::logic_error("Attaching to existing process isn't supported yet");
    } else {
        throw std::logic_error("Invalid config: Both of the program and the process pid are null");
    }
    if (cfg.traced_syscalls.has_value()) {
        traced_syscalls_ = cfg.traced_syscalls.value();
    } else {
        traced_syscalls_ = std::nullopt;
    }
    starter_->startChildProcess();
}

void Tracer::trace() const {
    int status;
    while (waitpid(static_cast<__pid_t>(starter_->getPid()), &status, 0) > 0) {
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            break;
        }

        if (WIFSTOPPED(status)) {
            user_regs_struct regs{};
            iovec io{.iov_base = &regs, .iov_len = sizeof(regs)};
            if (ptrace(PTRACE_GETREGSET,
                    static_cast<__pid_t>(starter_->getPid()),
                    reinterpret_cast<void*>(NT_PRSTATUS),
                    &io) == -1) {
                throw std::runtime_error("Failed to get registers");
            }

            auto [syscall_code, args] = getSyscallArgs(regs);
            const auto syscall = getSyscallInfo(syscall_code);

            if (!traced_syscalls_.has_value() || traced_syscalls_->contains(syscall_code)) {
                if (syscall.has_value()) {
                    std::cout << syscall.value().name << "(";
                    std::vector<std::string> formatted_args;
                    formatted_args.reserve(syscall.value().args.size());
                    for (int arg_index = 0; arg_index < syscall.value().args.size(); arg_index++) {
                        const auto register_value = handleRegisterValue(
                            syscall.value().args[arg_index],
                            args[arg_index],
                            static_cast<pid_t>(starter_->getPid())
                            );
                        if (register_value.has_value()) {
                            formatted_args.push_back(register_value.value());
                        }
                    }
                    for (int arg_index = 0; arg_index < formatted_args.size(); arg_index++) {
                        std::cout << formatted_args[arg_index] << (arg_index != formatted_args.size() - 1 ? ", " : "");
                    }

                    std::cout << ")";
                } else {
                    std::cout << "Unknown syscall[" << syscall_code << "](";
                    std::vector<std::string> formatted_args;
                    formatted_args.reserve(6);
                    for (int arg_index = 0; arg_index < args.size(); arg_index++) {
                        const auto register_value = handleRegisterValue(
                            SyscallArgType::HEX,
                            args[arg_index],
                            static_cast<pid_t>(starter_->getPid())
                            );
                        if (register_value.has_value()) {
                            formatted_args.push_back(register_value.value());
                        }
                    }
                    for (int arg_index = 0; arg_index < formatted_args.size(); arg_index++) {
                        std::cout << formatted_args[arg_index] << (arg_index != formatted_args.size() - 1 ? ", " : "");
                    }
                    std::cout << ")";
                }

                std::cout << '\n';
            }

            ptrace(PTRACE_SYSCALL, static_cast<__pid_t>(starter_->getPid()), nullptr, nullptr);
        }
    }
}
