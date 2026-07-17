#include "calculator.h"
#include "calculatorsym.h"
#include "lpg2/LexStream.h"

#include <iostream>
#include <vector>

struct StubLexStream : LexStream {
    StubLexStream()
    {
        shared_ptr_wstring chars(std::wstring(L"1+2*3 "));
        initialize(chars, L"calculator");
    }

    std::vector<std::wstring> orderedExportedSymbols() override
    {
        return calculatorsym::orderedTerminalSymbols;
    }
};

static void seed(IPrsStream* stream, const std::vector<int>& kinds)
{
    stream->makeToken(0, 0, 0);
    for (int i = 0; i < (int)kinds.size(); ++i)
        stream->makeToken(i + 1, i + 1, kinds[i]);
    stream->setStreamLength(stream->getSize());
}

int main()
{
    // Valid: NUMBER PLUS NUMBER STAR NUMBER EOF  →  1+2*3
    {
        StubLexStream lex;
        calculator parser(&lex);
        seed(parser.getIPrsStream(), {
            calculatorsym::TK_NUMBER,
            calculatorsym::TK_PLUS,
            calculatorsym::TK_NUMBER,
            calculatorsym::TK_STAR,
            calculatorsym::TK_NUMBER,
            calculatorsym::TK_EOF_TOKEN,
        });
        calculator::Ast* root = parser.parser();
        if (!root) {
            std::cerr << "expected successful parse of 1+2*3\n";
            return 2;
        }
    }

    // Invalid: PLUS EOF
    {
        StubLexStream lex;
        calculator parser(&lex);
        seed(parser.getIPrsStream(), {
            calculatorsym::TK_PLUS,
            calculatorsym::TK_EOF_TOKEN,
        });
        if (parser.parser() != nullptr) {
            std::cerr << "expected reject of leading PLUS\n";
            return 3;
        }
    }

    std::cout << "calculator cpp: ok\n";
    return 0;
}
