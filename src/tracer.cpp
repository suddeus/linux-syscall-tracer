#include <tracer.hpp>

#include <wait.h>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <elf.h>

#include "syscalls.hpp"

Tracer::Tracer(const Config& cfg) {
    if (cfg.command_argv.has_value()) {
        starter_ = std::make_unique<starter::ExecuteProgram>(cfg.command_argv.value());
    } else if (cfg.process_pid.has_value()) {
        starter_ = std::make_unique<starter::ConnectToProcess>(cfg.process_pid.value());
    } else {
        throw std::logic_error("Invalid config: Both of the program and the process pid are null");
    }

    if (cfg.traced_syscalls.has_value()) {
        traced_syscalls_ = cfg.traced_syscalls.value();
    } else {
        traced_syscalls_ = std::nullopt;
    }

    if (cfg.output_file.has_value()) {
        output_file_ = std::make_unique<std::ofstream>(cfg.output_file.value());

        if (!output_file_->is_open()) {
            throw std::logic_error("Failed while opening or creating the file");
        }
    }
    starter_->startChildProcess();
}

Tracer::~Tracer() {
    if (output_file_) {
        output_file_->close();
    }
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

            std::ostream& out_stream = getOutStream();
            if (!traced_syscalls_.has_value() || traced_syscalls_->contains(syscall_code)) {
                if (syscall.has_value()) {
                    out_stream << syscall.value().name << "(";
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
                        out_stream << formatted_args[arg_index] << (arg_index != formatted_args.size() - 1 ? ", " : "");
                    }

                    out_stream << ")";
                } else {
                    out_stream << "Unknown syscall[" << syscall_code << "](";
                    std::vector<std::string> formatted_args;
                    formatted_args.reserve(6);
                    for (const unsigned long arg_index : args) {
                        const auto register_value = handleRegisterValue(
                            SyscallArgType::HEX,
                            arg_index,
                            static_cast<pid_t>(starter_->getPid())
                            );
                        if (register_value.has_value()) {
                            formatted_args.push_back(register_value.value());
                        }
                    }
                    for (int arg_index = 0; arg_index < formatted_args.size(); arg_index++) {
                        out_stream << formatted_args[arg_index] << (arg_index != formatted_args.size() - 1 ? ", " : "");
                    }
                    out_stream << ")";
                }

                out_stream << '\n';
            }

            ptrace(PTRACE_SYSCALL, static_cast<__pid_t>(starter_->getPid()), nullptr, nullptr);
        }
    }
}

std::ostream &Tracer::getOutStream() const {
    return (output_file_ ? *output_file_ : std::cout);
}
