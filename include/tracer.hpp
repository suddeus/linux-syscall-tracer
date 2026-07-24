#pragma once

#include <config.hpp>
#include <child_starter.hpp>
#include <memory>

class Tracer {
public:
    explicit Tracer(const Config& cfg);

    void trace() const;

private:
    std::unique_ptr<BaseChildStarter> starter_;
    std::optional<std::set<std::uint64_t>> traced_syscalls_;
};