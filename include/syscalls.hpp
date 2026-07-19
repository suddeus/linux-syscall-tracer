#pragma once

#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <map>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <cstring>
#include <functional>
#include <optional>
#include <fcntl.h>

using RegisterValue = std::uint64_t;

struct SyscallArgs {
    RegisterValue syscall_code;
    std::array<RegisterValue, 6> args;
};

enum class SyscallArgType {
    INT,
    UINT,
    CHAR_PTR,
    READABLE_CHAR_PTR,
    SIZE,
    UMODE,
    ULONG,
    PTR,
    OPEN_FLAGS,
    DIR_FD,
    HEX
};

struct SyscallInfo {
    std::uint64_t number;
    std::string name;
    std::vector<SyscallArgType> args;
    SyscallArgType return_type;
};

SyscallArgs getSyscallArgs(const user_regs_struct& regs);

static const std::vector <SyscallInfo> SYSCALLS = {
#if defined(__x86_64__)
    {0, "read", {
        SyscallArgType::UINT,
        SyscallArgType::CHAR_PTR,
        SyscallArgType::SIZE
    }, SyscallArgType::SIZE},
    {1, "write", {
        SyscallArgType::UINT,
        SyscallArgType::READABLE_CHAR_PTR,
        SyscallArgType::SIZE
    }, SyscallArgType::SIZE},
    {2, "open", {
        SyscallArgType::READABLE_CHAR_PTR,
        SyscallArgType::OPEN_FLAGS,
        SyscallArgType::UMODE
    }, SyscallArgType::INT},
    {3, "close", {
        SyscallArgType::UINT
    }, SyscallArgType::INT}
#elif defined(__aarch64__)
    {63, "read", {
        SyscallArgType::UINT,
        SyscallArgType::CHAR_PTR,
        SyscallArgType::SIZE
    }, SyscallArgType::SIZE},
    {64, "write", {
        SyscallArgType::UINT,
        SyscallArgType::READABLE_CHAR_PTR,
        SyscallArgType::SIZE
    }, SyscallArgType::SIZE},
    {56, "openat", {
        SyscallArgType::DIR_FD,
        SyscallArgType::READABLE_CHAR_PTR,
        SyscallArgType::OPEN_FLAGS,
        SyscallArgType::UMODE
    }, SyscallArgType::INT},
    {57, "close", {
        SyscallArgType::UINT
    }, SyscallArgType::INT}
#else
# error "syscall_tracer supports only Linux x86-64 and AArch64"
#endif
};

std::optional<SyscallInfo> getSyscallInfo(std::uint64_t syscall_code);

inline std::map<SyscallArgType, std::function<std::optional<std::string>(RegisterValue, pid_t)>> TYPE_HANDLERS = {
    {SyscallArgType::INT, [](const RegisterValue value, pid_t) -> std::optional<std::string> {
        return std::to_string(static_cast<std::int64_t>(value));
    }},
    {SyscallArgType::UINT, [](const RegisterValue value, pid_t) -> std::optional<std::string> {
        return std::to_string(value);
    }},
    {SyscallArgType::CHAR_PTR, [](const RegisterValue value, const pid_t) -> std::optional<std::string> {
        std::ostringstream message;
        message << "0x" << std::hex << value << std::dec;
        return message.str();
    }},
    {SyscallArgType::READABLE_CHAR_PTR, [](const RegisterValue value, const pid_t process_pid) -> std::optional<std::string> {
        if (value == 0) {
            return "NULL";
        }

        constexpr std::size_t max_length = 4096;
        std::string result;

        constexpr std::size_t word_size = sizeof(long);
        const auto start = static_cast<std::uintptr_t>(value);
        auto address = start & ~(static_cast<std::uintptr_t>(word_size) - 1);
        std::size_t skip = start - address;

        while (result.size() < max_length) {
            errno = 0;
            const long word = ptrace(
                PTRACE_PEEKDATA,
                process_pid,
                reinterpret_cast<void*>(address),
                nullptr);

            if (word == -1 && errno != 0) {
                return result.empty() ? std::nullopt
                                      : std::optional{"\"" + result + "\""};
            }

            char bytes[word_size];
            std::memcpy(bytes, &word, word_size);

            for (std::size_t i = skip; i < word_size && result.size() < max_length; ++i) {
                if (bytes[i] == '\0') {
                    return "\"" + result + "\"";
                }
                if (bytes[i] == '\n') {
                    result += "\\n";
                } else {
                    result += bytes[i];
                }
            }

            address += word_size;
            skip = 0;
        }

        result += "...";
        return "\"" + result + "\"";
    }},
    {SyscallArgType::SIZE, [](const RegisterValue value, pid_t) -> std::optional<std::string> {
        return std::to_string(value);
    }},
    {SyscallArgType::UMODE, [](const RegisterValue value, pid_t) -> std::optional<std::string> {
        const unsigned short mode = value;
        std::string result_mode = "---------";

        result_mode[0] = (mode & 0400) ? 'r' : '-';
        result_mode[1] = (mode & 0200) ? 'w' : '-';
        result_mode[2] = (mode & 0100) ? 'x' : '-';
        result_mode[3] = (mode & 0040) ? 'r' : '-';
        result_mode[4] = (mode & 0020) ? 'w' : '-';
        result_mode[5] = (mode & 0010) ? 'x' : '-';
        result_mode[6] = (mode & 0004) ? 'r' : '-';
        result_mode[7] = (mode & 0002) ? 'w' : '-';
        result_mode[8] = (mode & 0001) ? 'x' : '-';

        if (mode & 04000) result_mode[2] = (mode & 0100) ? 's' : 'S';
        if (mode & 02000) result_mode[5] = (mode & 0010) ? 's' : 'S';
        if (mode & 01000) result_mode[8] = (mode & 0001) ? 't' : 'T';

        if (result_mode == "---------") {
            return std::nullopt;
        }
        return result_mode;
    }},
    {SyscallArgType::ULONG, [](const RegisterValue value, pid_t) -> std::optional<std::string> {
        return std::to_string(value);
    }},
    {SyscallArgType::PTR, [](const RegisterValue value, pid_t) -> std::optional<std::string> {
        std::ostringstream message;
        message << "0x" << std::hex << value << std::dec;
        return message.str();
    }},
    {SyscallArgType::OPEN_FLAGS, [](const RegisterValue value, pid_t) -> std::optional<std::string> {
        auto remaining = static_cast<unsigned int>(value);
        std::vector<std::string> flags;

        switch (remaining & O_ACCMODE) {
            case O_RDONLY: flags.emplace_back("O_RDONLY"); break;
            case O_WRONLY: flags.emplace_back("O_WRONLY"); break;
            case O_RDWR:   flags.emplace_back("O_RDWR"); break;
            default:       flags.emplace_back("O_ACCMODE_UNKNOWN"); break;
        }
        remaining &= ~static_cast<unsigned int>(O_ACCMODE);

        const auto take = [&flags, &remaining](const unsigned int flag, const char* name) {
            if ((remaining & flag) == flag) {
                flags.emplace_back(name);
                remaining &= ~flag;
            }
        };

#ifdef O_TMPFILE
        take(O_TMPFILE, "O_TMPFILE"); // Includes O_DIRECTORY.
#endif
#ifdef O_SYNC
        take(O_SYNC, "O_SYNC"); // Includes O_DSYNC on Linux.
#endif
        take(O_CREAT, "O_CREAT");
        take(O_EXCL, "O_EXCL");
        take(O_NOCTTY, "O_NOCTTY");
        take(O_TRUNC, "O_TRUNC");
        take(O_APPEND, "O_APPEND");
        take(O_NONBLOCK, "O_NONBLOCK");
#ifdef O_DSYNC
        take(O_DSYNC, "O_DSYNC");
#endif
#ifdef O_ASYNC
        take(O_ASYNC, "O_ASYNC");
#endif
#ifdef O_DIRECT
        take(O_DIRECT, "O_DIRECT");
#endif
#ifdef O_LARGEFILE
        take(O_LARGEFILE, "O_LARGEFILE");
#endif
#ifdef O_DIRECTORY
        take(O_DIRECTORY, "O_DIRECTORY");
#endif
#ifdef O_NOFOLLOW
        take(O_NOFOLLOW, "O_NOFOLLOW");
#endif
#ifdef O_NOATIME
        take(O_NOATIME, "O_NOATIME");
#endif
#ifdef O_CLOEXEC
        take(O_CLOEXEC, "O_CLOEXEC");
#endif
#ifdef O_PATH
        take(O_PATH, "O_PATH");
#endif

        if (remaining != 0) {
            std::ostringstream unknown;
            unknown << "0x" << std::hex << remaining;
            flags.push_back(unknown.str());
        }

        std::ostringstream formatted;
        for (std::size_t index = 0; index < flags.size(); ++index) {
            formatted << flags[index] << (index + 1 == flags.size() ? "" : "|");
        }
        return formatted.str();
    }},
    {SyscallArgType::DIR_FD, [](const RegisterValue value, pid_t) -> std::optional<std::string> {
        const int dfd = static_cast<int>(value);
        return dfd == AT_FDCWD ? "AT_FDCWD" : std::to_string(dfd);
    }},
    {SyscallArgType::HEX, [](const RegisterValue value, pid_t) -> std::optional<std::string> {
        std::ostringstream message;
        message << "0x" << std::hex << value << std::dec;
        return message.str();
    }},
};

std::optional<std::string> handleRegisterValue(SyscallArgType type, RegisterValue value, pid_t process_pid);
