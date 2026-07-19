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

            if (syscall.has_value()) {
                std::cout << syscall.value().name << "(";
                for (int arg_index = 0; arg_index < syscall.value().args.size(); arg_index++) {
                    std::cout << handleRegisterValue(
                        syscall.value().args[arg_index],
                        args[arg_index],
                        static_cast<pid_t>(starter_->getPid())
                        ) << (arg_index == syscall.value().args.size() - 1 ? "" : ", ");
                }
                std::cout << ")";
            } else {
                std::cout << "Unknown syscall[" << syscall_code << "](";
                for (int arg_index = 0; arg_index < args.size(); arg_index++) {
                    std::cout << handleRegisterValue(
                        SyscallArgType::HEX,
                        args[arg_index],
                        static_cast<pid_t>(starter_->getPid())
                        ) << (arg_index == args.size() - 1 ? "" : ", ");
                }
                std::cout << ")";
            }

            std::cout << '\n';

            ptrace(PTRACE_SYSCALL, static_cast<__pid_t>(starter_->getPid()), nullptr, nullptr);
        }
    }
}
