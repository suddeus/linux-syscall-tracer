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

    return 0;
}
