// Contract tests for C++ runtime token-stream incremental reset.
// Token-level damage/reuse — not tree-sitter subtree reuse.
#include "lpg2/LexStream.h"
#include "lpg2/PrsStream.h"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct StubLex : LexStream {
    explicit StubLex(const std::wstring& text)
    {
        shared_ptr_wstring chars(text);
        initialize(chars, L"incremental-test");
    }
    std::vector<std::wstring> orderedExportedSymbols() override { return {}; }
};

int test_incremental_reset_truncates_suffix()
{
    StubLex lex(L"0123456789");
    PrsStream stream(&lex);
    for (int i = 0; i < 10; ++i)
        stream.makeToken(i, i, 1);
    stream.makeToken(10, 10, 2);
    stream.setStreamLength(stream.getSize());

    const int before = stream.getSize();
    if (before < 11)
        return 2;

    Tuple<IToken*> affected = stream.incrementalResetAtCharacterOffset(5);
    if (affected.size() < 1)
        return 3;
    if (stream.getSize() >= before)
        return 4;
    for (int i = 1; i < stream.getSize(); ++i) {
        if (stream.getStartOffset(i) > 5)
            return 5;
    }
    return 0;
}

int test_reset_at_token_boundary()
{
    StubLex lex(L"abcd");
    PrsStream stream(&lex);
    stream.makeToken(0, 0, 1);
    stream.makeToken(1, 1, 1);
    stream.makeToken(2, 2, 1);
    stream.makeToken(3, 3, 1);
    stream.makeToken(4, 4, 2);
    stream.setStreamLength(stream.getSize());

    const int before = stream.getSize();
    (void)stream.incrementalResetAtCharacterOffset(2);
    if (stream.getSize() >= before)
        return 6;
    if (stream.getSize() < 2)
        return 7; // at least bad/empty-ish prefix retained historically
    return 0;
}

int bench_reset_vs_rebuild()
{
    constexpr int N = 20000;
    std::wstring text(static_cast<size_t>(N), L'x');
    StubLex lex(text);
    PrsStream stream(&lex);
    for (int i = 0; i < N; ++i)
        stream.makeToken(i, i, 1);
    stream.makeToken(N, N, 2);
    stream.setStreamLength(stream.getSize());

    auto t0 = std::chrono::steady_clock::now();
    (void)stream.incrementalResetAtCharacterOffset(N / 2);
    auto t1 = std::chrono::steady_clock::now();

    PrsStream rebuild(&lex);
    for (int i = 0; i < N; ++i)
        rebuild.makeToken(i, i, 1);
    rebuild.makeToken(N, N, 2);
    rebuild.setStreamLength(rebuild.getSize());
    auto t2 = std::chrono::steady_clock::now();

    const auto reset_us =
        std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    const auto rebuild_us =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "incremental_reset_us=" << reset_us
              << " full_rebuild_us=" << rebuild_us << "\n";
    return 0;
}

} // namespace

int main()
{
    if (int rc = test_incremental_reset_truncates_suffix())
        return rc;
    if (int rc = test_reset_at_token_boundary())
        return rc;
    if (int rc = bench_reset_vs_rebuild())
        return rc;
    return 0;
}
