#ifndef CONTROL_INCLUDED
#define CONTROL_INCLUDED

#include "option.h"
#include "grammar.h"
#include "node.h"
#include "base.h"
#include "pda.h"

//
//
//
class Control
{
public:

    Control(Option *option_,
            LexStream *lex_stream_,
            VariableLookupTable *variable_table_,
            MacroLookupTable *macro_table_) : option(option_),
                                              lex_stream(lex_stream_)
    {
        node_pool = new NodePool();
        grammar = new Grammar(this, &(option -> ActionBlocks()), variable_table_, macro_table_);
        base = new Base(this);
        pda = new Pda(this, base);

        //
        // Flush buffer to print options and initial reports
        //
        if (! option -> quiet)
            option -> FlushReport();
    }

    //
    // Close listing file and destroy the objects allocated in the constructor.
    //
    ~Control() { CleanUp(); }

    void CleanUp()
    {
        delete node_pool; node_pool = NULL;
        delete grammar; grammar = NULL;
        delete base; base = NULL;
        delete pda; pda = NULL;
    }

    enum
    {
        SYMBOL_SIZE = 256,
        PRINT_LINE_SIZE = 80
    };

    static const char HEADER_INFO[],
                      VERSION[];

    void PrintHeading(int code = 0)
    {
        fprintf(option -> syslis, "\f\n\n %-39s%s\n\n", HEADER_INFO, VERSION);
        return;
    }

    void InvalidateFile(const char *, const char *);
    void Exit(int);

    void ProcessGrammar(void);

    void ConstructParser(void);

    Option *option;
    LexStream *lex_stream;
    NodePool *node_pool;
    Base *base;
    Pda *pda;
    Grammar *grammar;
};
#endif /* CONTROL_INCLUDED */
