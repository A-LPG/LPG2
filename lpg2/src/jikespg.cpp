#include "symbol.h"
#include "option.h"
#include "scanner.h"
#include "option.h"
#include "control.h"
#include "lpg_error.h"
#include "output_transaction.h"

#include <cstring>
#include <cctype>
#include <memory>
#include <new>
#include <string>
#include <vector>

using namespace std;

static bool JsonDiagnosticsRequested(int argc, char *argv[])
{
    for (int i = 1; i < argc - 1; i++)
    {
        string argument = argv[i];
        while (! argument.empty() && argument[0] == '-')
            argument.erase(argument.begin());
        for (char &c : argument)
            c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
        if (argument == "diagnostics=json")
            return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    const bool json_diagnostics_requested =
        JsonDiagnosticsRequested(argc, argv);
    //
    // If only "lpg" or "lpg ?*" is typed, we display the help
    // screen.
    //
    if (argc == 1 ||
        argv[1][0] == '?' ||
        strcmp(argv[1], "-h") == 0 ||
        strcmp(argv[1], "-help") == 0 ||
        strcmp(argv[1], "--help") == 0)
    {
        Option::PrintOptionsList();
        return 0;
    }

    if (strcmp(argv[1], "-version") == 0 ||
        strcmp(argv[1], "--version") == 0) {
        cout << "\n"
             << Control::HEADER_INFO
             << " Version " << Control::VERSION
             << "\n(C) Copyright LPG Group. 1984, 2022.\n\n";
        return 0;
    }

    try
    {
        Option option(argc, (const char **) argv);

        LexStream lex_stream(&option);
        VariableLookupTable variable_table;

        unique_ptr<MacroLookupTable> macro_table(new MacroLookupTable());
        unique_ptr<Scanner> scanner(
            new Scanner(&option, &lex_stream, &variable_table, macro_table.get()));
        scanner -> Scan();

        if (! option.quiet && ! option.DiagnosticsJson())
        {
            cout << "\n"
                 << Control::HEADER_INFO
                 << " Version " << Control::VERSION
                 << "\n(C) Copyright LPG Group. 1984, 2021.\n\n";
        }

#ifdef TEST
        lex_stream.Dump();
#endif
        if (lex_stream.NumTokens() == 0)
            throw LpgError(12, "Grammar contains no tokens");
        if (scanner -> NumErrorTokens() > 0)
            throw LpgError(12);

        scanner.reset();

        unique_ptr<Control> control(
            new Control(&option, &lex_stream, &variable_table, macro_table.get()));
        control -> ProcessGrammar();

        macro_table.reset();

        control -> ConstructParser();
        control.reset();

        vector<string> generated_files;
        if (option.write)
            generated_files = OutputTransaction::Instance().Commit();
        else OutputTransaction::Instance().Rollback();
        if (! option.quiet && ! option.DiagnosticsJson())
        {
            cout << "Generated " << generated_files.size() << " file(s)";
            for (const string &filename : generated_files)
                cout << "\n  " << filename;
            cout << "\n";
        }
    }
    catch (bad_alloc&)
    {
        if (! json_diagnostics_requested)
            cerr << "***OS System Failure: Out of memory" << endl;
        OutputTransaction::Instance().Rollback();
        return 12;
    }
    catch (const LpgError &error)
    {
        if (! json_diagnostics_requested && error.what()[0] != '\0')
            cerr << "***ERROR: " << error.what() << endl;
        OutputTransaction::Instance().Rollback();
        return error.ExitCode();
    }
    catch (const char *str)
    {
        if (! json_diagnostics_requested)
            cerr << "***ERROR: " << str << endl;
        OutputTransaction::Instance().Rollback();
        return 12;
    }
    catch (const exception &error)
    {
        if (! json_diagnostics_requested)
            cerr << "***ERROR: " << error.what() << endl;
        OutputTransaction::Instance().Rollback();
        return 12;
    }
    catch (...)
    {
        if (! json_diagnostics_requested)
            cerr << "***ERROR: Unexpected internal failure" << endl;
        OutputTransaction::Instance().Rollback();
        return 12;
    }

    return 0;
}
