#include "output_transaction.h"

#include "lpg_error.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <system_error>
#include <utility>

namespace
{
std::atomic<unsigned long long> output_sequence(0);

std::string ErrorMessage(const char *operation,
                         const std::string &filename,
                         const std::error_code &error)
{
    return std::string(operation) + " output file \"" + filename +
           "\": " + error.message();
}
}

OutputTransaction &OutputTransaction::Instance()
{
    static OutputTransaction transaction;
    return transaction;
}

OutputTransaction::~OutputTransaction()
{
    Rollback();
}

std::string OutputTransaction::TemporaryName(const std::string &filename,
                                             const char *tag) const
{
    const auto timestamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    const auto sequence = output_sequence.fetch_add(1);
    return filename + ".lpg2-" + tag + "-" + std::to_string(timestamp) +
           "-" + std::to_string(sequence);
}

FILE *OutputTransaction::Open(const char *filename, const char *mode)
{
    if (filename == nullptr || filename[0] == '\0')
        return nullptr;

    // Analysis / dry-run: keep writers happy without touching the
    // destination directory (may be read-only in CI mounts).
    if (!publish_enabled)
        return tmpfile();

    PendingOutput output;
    output.final_name = filename;
    output.temporary_name = TemporaryName(output.final_name, "tmp");

    FILE *file = fopen(output.temporary_name.c_str(), mode);
    if (file != nullptr)
        pending.push_back(std::move(output));

    return file;
}

void OutputTransaction::RestoreOriginals() noexcept
{
    for (auto output = pending.rbegin(); output != pending.rend(); ++output)
    {
        std::error_code ignored;
        if (output -> published)
            std::filesystem::remove(output -> final_name, ignored);

        if (output -> had_original)
        {
            ignored.clear();
            std::filesystem::rename(output -> backup_name,
                                    output -> final_name,
                                    ignored);
        }
    }
}

std::vector<std::string> OutputTransaction::Commit()
{
    try
    {
        for (PendingOutput &output : pending)
        {
            std::error_code error;
            const bool final_exists =
                std::filesystem::exists(output.final_name, error);
            if (error)
                throw LpgError(12, ErrorMessage("Cannot inspect",
                                                output.final_name,
                                                error));

            if (final_exists)
            {
                output.backup_name = TemporaryName(output.final_name, "backup");
                std::filesystem::rename(output.final_name,
                                        output.backup_name,
                                        error);
                if (error)
                    throw LpgError(12, ErrorMessage("Cannot preserve",
                                                    output.final_name,
                                                    error));
                output.had_original = true;
            }

            error.clear();
            std::filesystem::rename(output.temporary_name,
                                    output.final_name,
                                    error);
            if (error)
            {
                if (output.had_original)
                {
                    std::error_code ignored;
                    std::filesystem::rename(output.backup_name,
                                            output.final_name,
                                            ignored);
                    output.had_original = false;
                }
                throw LpgError(12, ErrorMessage("Cannot publish",
                                                output.final_name,
                                                error));
            }
            output.published = true;
        }

        for (PendingOutput &output : pending)
        {
            if (output.had_original)
            {
                std::error_code ignored;
                std::filesystem::remove(output.backup_name, ignored);
                output.had_original = false;
            }
        }
        std::vector<std::string> published;
        published.reserve(pending.size());
        for (const PendingOutput &output : pending)
            published.push_back(output.final_name);
        pending.clear();
        return published;
    }
    catch (...)
    {
        RestoreOriginals();
        for (const PendingOutput &output : pending)
        {
            std::error_code ignored;
            std::filesystem::remove(output.temporary_name, ignored);
            std::filesystem::remove(output.backup_name, ignored);
        }
        pending.clear();
        throw;
    }
}

void OutputTransaction::Rollback() noexcept
{
    RestoreOriginals();
    for (const PendingOutput &output : pending)
    {
        std::error_code ignored;
        std::filesystem::remove(output.temporary_name, ignored);
        std::filesystem::remove(output.backup_name, ignored);
    }
    pending.clear();
}
