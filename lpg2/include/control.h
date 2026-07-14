#ifndef CONTROL_INCLUDED
#define CONTROL_INCLUDED

#include "option.h"
#include "grammar.h"
#include "node.h"
#include "base.h"
#include "pda.h"

#include <memory>

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
                                              lex_stream(lex_stream_),
                                              node_pool(new NodePool()),
                                              grammar(new Grammar(this,
                                                  &(option -> ActionBlocks()),
                                                  variable_table_,
                                                  macro_table_)),
                                              base(new Base(this)),
                                              pda(new Pda(this, base.get()))
    {
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
        pda.reset();
        base.reset();
        grammar.reset();
        node_pool.reset();
    }

    enum
    {
        SYMBOL_SIZE = 256,
        PRINT_LINE_SIZE = 80
    };

    static const char HEADER_INFO[],
                      VERSION[];

    void PrintHeading(int code = 0);

    void Exit(int);

    void ProcessGrammar(void);

    void ConstructParser(void);

    Option *option;
    LexStream *lex_stream;
    std::unique_ptr<NodePool> node_pool;
    std::unique_ptr<Grammar> grammar;
    std::unique_ptr<Base> base;
    std::unique_ptr<Pda> pda;
};
#endif /* CONTROL_INCLUDED */
