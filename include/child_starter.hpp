#pragma once

#include <unistd.h>
#include <string>

class BaseChildStarter {
public:
    BaseChildStarter() noexcept = default;

    [[nodiscard]] virtual size_t getPid() const = 0;

    virtual void startChildProcess() = 0;

    virtual ~BaseChildStarter() = default;
};

namespace starter {
    class ExecuteProgram final : public BaseChildStarter {
    public:
        explicit ExecuteProgram(char** command_argv) noexcept;

        [[nodiscard]] size_t getPid() const override;

        void startChildProcess() override;
    private:
        size_t pid_ = 0;
        char** command_argv_ = nullptr;
    };

    class ConnectToProcess final : public BaseChildStarter {
    public:
        explicit ConnectToProcess(size_t process_pid) noexcept;

        [[nodiscard]] size_t getPid() const override;

        void startChildProcess() override;

    private:
        size_t pid_ = 0;
    };
};