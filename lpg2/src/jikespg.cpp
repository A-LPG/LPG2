#include "symbol.h"
#include "option.h"
#include "scanner.h"
#include "option.h"
#include "control.h"

#include <assert.h>
#include <new>

using namespace std;

//
//
//
int main(int argc, char *argv[])
{
    //
    // If only "lpg" or "lpg ?*" is typed, we display the help
    // screen.
    //
    if (argc == 1 || argv[1][0] == '?' || strcmp(argv[1],"-h") == 0)
    {
        Option::PrintOptionsList();
        return 4;
    }

    if (strcmp(argv[1],"-version") == 0) {
        cout << "\n"
             << Control::HEADER_INFO
             << " Version " << Control::VERSION
             << "\n(C) Copyright LPG Group. 1984, 2021.\n\n";
        exit(0);
    }

    //
    // We declare these objects first and initialize them to NULL in case
    // an exceptional condition occurs and they have to be deleted.
    // See try/catch below.
    //
    Control *control = NULL;
    MacroLookupTable *macro_table = NULL;
    Scanner *scanner = NULL;

    try
    {
        Option option(argc, (const char **) argv);

        LexStream lex_stream(&option);
        VariableLookupTable variable_table;

        macro_table = new MacroLookupTable();
        scanner = new Scanner(&option, &lex_stream, &variable_table, macro_table);
        scanner -> Scan();

        if (! option.quiet)
        {
            cout << "\n"
                 << Control::HEADER_INFO
                 << " Version " << Control::VERSION
                 << "\n(C) Copyright LPG Group. 1984, 2021.\n\n";
        }

#ifdef TEST
        lex_stream.Dump(); // TODO: REMOVE THIS !!!
#endif
        if (lex_stream.NumTokens() == 0 || scanner -> NumErrorTokens() > 0)
        {
            //
            // Note that scanner and macro_table are set to NULL after they are
            // deleted to avoid a dangling pointer situation that may occur in
            // case an exceptional condition occurs and they have to be deleted
            // again. See try/catch below.
            //
            delete scanner; scanner = NULL; // Note use of scanner in test above... DO NOT move this command!
            delete macro_table; macro_table = NULL;
        }
        else
        {
            delete scanner; scanner = NULL; // Note use of scanner above... DO NOT move this command!

            control = new Control(&option, &lex_stream, &variable_table, macro_table);
            control -> ProcessGrammar();

            delete macro_table; macro_table = NULL;

            control -> ConstructParser();
            delete control;
        }
    }
    catch (bad_alloc&)
    {
        cerr << "***OS System Failure: Out of memory" << endl;
        cerr.flush();

        delete scanner;
        delete macro_table;
        delete control;

        exit(12);
    }
    catch (int code)
    {
        delete scanner;
        delete macro_table;
        delete control;

        exit(code);
    }
    catch (const char *str)
    {
        delete scanner;
        delete macro_table;
        delete control;

        cerr <<"*** " << str << endl;
        cerr.flush();

        exit(12);
    }

    return 0;
}
