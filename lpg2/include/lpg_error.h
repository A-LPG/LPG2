#ifndef LPG_ERROR_INCLUDED
#define LPG_ERROR_INCLUDED

#include <exception>
#include <string>
#include <utility>

class LpgError : public std::exception
{
public:
    explicit LpgError(int code, std::string message = std::string())
        : exit_code(code == 0 ? 12 : code),
          error_message(std::move(message))
    {}

    int ExitCode() const noexcept { return exit_code; }
    const char *what() const noexcept override { return error_message.c_str(); }

private:
    int exit_code;
    std::string error_message;
};

#endif
