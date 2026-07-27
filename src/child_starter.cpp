#include <child_starter.hpp>

#include <sys/ptrace.h>
#include <csignal>
#include <exception>
#include <stdexcept>
#include <wait.h>

namespace starter {
    ExecuteProgram::ExecuteProgram(char** command_argv) noexcept
        : command_argv_(command_argv) {}

    void ExecuteProgram::startChildProcess() {
        const size_t child_pid = fork();

        if (child_pid == -1) {
            throw std::runtime_error("Error while fork");
        }

        if (child_pid == 0) {
            // Child
            if (ptrace(PT_TRACE_ME, 0, nullptr) == -1) {
                throw std::runtime_error("Error while ptrace");
            }

            raise(SIGSTOP); // Stop until parent is ready

            execvp(command_argv_[0], command_argv_);
            _exit(-1); // execvp error
        }

        // Parent
        pid_ = child_pid;
        int status;
        waitpid(static_cast<__pid_t>(child_pid), &status, 0);

        ptrace(PTRACE_SYSCALL, pid_, nullptr, nullptr);
    }

    size_t ExecuteProgram::getPid() const {
        return pid_;
    }


    ConnectToProcess::ConnectToProcess(const size_t process_pid) noexcept
    : pid_(process_pid) {}

    void ConnectToProcess::startChildProcess() {
        if (ptrace(PTRACE_ATTACH, pid_, NULL, NULL) < 0) {
            throw std::runtime_error("Failed while attach");
        }
    }

    size_t ConnectToProcess::getPid() const {
        return pid_;
    }
}
