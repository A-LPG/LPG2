#ifndef OUTPUT_TRANSACTION_INCLUDED
#define OUTPUT_TRANSACTION_INCLUDED

#include <cstdio>
#include <string>
#include <vector>

class OutputTransaction
{
public:
    static OutputTransaction &Instance();

    // When false (e.g. -nowrite / --dry-run), Open uses an anonymous
    // tmpfile and never creates sibling .lpg2-tmp-* next to products.
    void SetPublishEnabled(bool enabled) { publish_enabled = enabled; }
    bool PublishEnabled() const { return publish_enabled; }

    FILE *Open(const char *filename, const char *mode = "wb");
    std::vector<std::string> Commit();
    void Rollback() noexcept;

    OutputTransaction(const OutputTransaction &) = delete;
    OutputTransaction &operator=(const OutputTransaction &) = delete;

private:
    bool publish_enabled = true;
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
