#pragma once

#include <config.hpp>
#include <child_starter.hpp>
#include <memory>
#include <fstream>

class Tracer {
public:
    explicit Tracer(const Config& cfg);

    ~Tracer();

    void trace() const;

private:
    std::unique_ptr<BaseChildStarter> starter_;
    std::optional<std::set<std::uint64_t>> traced_syscalls_;
    std::unique_ptr<std::ofstream> output_file_;

    [[nodiscard]] std::ostream& getOutStream() const;
};
