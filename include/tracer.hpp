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
};