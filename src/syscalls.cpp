#include <syscalls.hpp>

SyscallArgs getSyscallArgs(const user_regs_struct& regs) {
    SyscallArgs syscall_args{};

#if defined(__x86_64__)
    syscall_args.syscall_code = regs.orig_rax;
    syscall_args.args[0] = regs.rdi;
    syscall_args.args[1] = regs.rsi;
    syscall_args.args[2] = regs.rdx;
    syscall_args.args[3] = regs.r10;
    syscall_args.args[4] = regs.r8;
    syscall_args.args[5] = regs.r9;
#elif defined(__aarch32__)
    syscall_args.syscall_code = regs.regs[7];
    syscall_args.args[0] = regs.regs[0];
    syscall_args.args[1] = regs.regs[1];
    syscall_args.args[2] = regs.regs[2];
    syscall_args.args[3] = regs.regs[3];
    syscall_args.args[4] = regs.regs[4];
    syscall_args.args[5] = regs.regs[5];
#elif defined(__aarch64__)
    syscall_args.syscall_code = regs.regs[8];
    syscall_args.args[0] = regs.regs[0];
    syscall_args.args[1] = regs.regs[1];
    syscall_args.args[2] = regs.regs[2];
    syscall_args.args[3] = regs.regs[3];
    syscall_args.args[4] = regs.regs[4];
    syscall_args.args[5] = regs.regs[5];
#else
# error "syscall_tracer supports only Linux x86-64, AArch32 and AArch64"
#endif

    return syscall_args;
}

std::optional<SyscallInfo> getSyscallInfo(std::uint64_t syscall_code) {
    const auto syscall = std::ranges::find_if(
        SYSCALLS,
          [syscall_code](const SyscallInfo& curr_syscall) -> bool {
              return curr_syscall.number == syscall_code;
          }
    );

    if (syscall == SYSCALLS.cend()) {
        return std::nullopt;
    }
    return *syscall;
}

std::optional<std::string> handleRegisterValue(const SyscallArgType type, const RegisterValue value, const pid_t process_pid) {
    if (!TYPE_HANDLERS.contains(type)) {
        return TYPE_HANDLERS[SyscallArgType::HEX](value, process_pid);
    }
    return TYPE_HANDLERS[type](value, process_pid);
}
