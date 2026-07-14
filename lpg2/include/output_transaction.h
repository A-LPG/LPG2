#ifndef OUTPUT_TRANSACTION_INCLUDED
#define OUTPUT_TRANSACTION_INCLUDED

#include <cstdio>
#include <string>
#include <vector>

class OutputTransaction
{
public:
    static OutputTransaction &Instance();

    FILE *Open(const char *filename, const char *mode = "wb");
    std::vector<std::string> Commit();
    void Rollback() noexcept;

    OutputTransaction(const OutputTransaction &) = delete;
    OutputTransaction &operator=(const OutputTransaction &) = delete;

private:
    struct PendingOutput
    {
        std::string final_name;
        std::string temporary_name;
        std::string backup_name;
        bool had_original = false;
        bool published = false;
    };

    OutputTransaction() = default;
    ~OutputTransaction();

    std::string TemporaryName(const std::string &filename,
                              const char *tag) const;
    void RestoreOriginals() noexcept;

    std::vector<PendingOutput> pending;
};

#endif
