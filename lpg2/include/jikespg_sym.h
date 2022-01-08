#ifndef jikespg_sym_INCLUDED
#define jikespg_sym_INCLUDED

    enum
    {
        TK_DROPSYMBOLS_KEY = 34,
        TK_DROPACTIONS_KEY = 35,
        TK_DROPRULES_KEY = 36,
        TK_NOTICE_KEY = 11,
        TK_AST_KEY = 12,
        TK_GLOBALS_KEY = 13,
        TK_DEFINE_KEY = 14,
        TK_TERMINALS_KEY = 15,
        TK_SOFTKEYWORDS_KEY = 16,
        TK_EOL_KEY = 6,
        TK_EOF_KEY = 9,
        TK_ERROR_KEY = 7,
        TK_IDENTIFIER_KEY = 8,
        TK_ALIAS_KEY = 17,
        TK_EMPTY_KEY = 3,
        TK_START_KEY = 18,
        TK_TYPES_KEY = 19,
        TK_RULES_KEY = 20,
        TK_NAMES_KEY = 21,
        TK_END_KEY = 4,
        TK_HEADERS_KEY = 22,
        TK_TRAILERS_KEY = 23,
        TK_EXPORT_KEY = 24,
        TK_IMPORT_KEY = 25,
        TK_INCLUDE_KEY = 26,
        TK_RECOVER_KEY = 27,
        TK_DISJOINTPREDECESSORSETS_KEY = 28,
        TK_EQUIVALENCE = 30,
        TK_PRIORITY_EQUIVALENCE = 31,
        TK_ARROW = 32,
        TK_PRIORITY_ARROW = 33,
        TK_OR_MARKER = 10,
        TK_MACRO_NAME = 5,
        TK_SYMBOL = 1,
        TK_BLOCK = 2,
        TK_EOF = 29,
        TK_ERROR_SYMBOL = 37,

        NUM_TOKENS = 37,

        IS_VALID_FOR_PARSER = 1
    };

#endif /* jikespg_sym_INCLUDED */
