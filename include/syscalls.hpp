#pragma once

#include <vector>
#include <string>
#include <cstdint>

enum class SyscallArgType {
    INT,
    UINT,
    CHAR_PTR,
    SIZE,
    UMODE,
    ULONG,
    PTR,
    HEX
};

struct SyscallInfo {
    std::uint64_t number;
    std::string name;
    std::vector<SyscallArgType> args;
};

static const std::vector <SyscallInfo> SYSCALLS = {
    {0, "read", {
        SyscallArgType::UINT,
        SyscallArgType::CHAR_PTR,
        SyscallArgType::SIZE
    }},
    {1, "write", {
        SyscallArgType::UINT,
        SyscallArgType::CHAR_PTR,
        SyscallArgType::SIZE
    }},
{2, "open", {
        SyscallArgType::CHAR_PTR,
        SyscallArgType::INT,
        SyscallArgType::UMODE
    }},
{3, "close", {
        SyscallArgType::UINT
    }}
};