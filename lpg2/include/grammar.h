#ifndef GRAMMAR_INCLUDED
#define GRAMMAR_INCLUDED

#include "util.h"
#include "tuple.h"
#include "symbol.h"
#include "option.h"
#include "scanner.h"
#include "parser.h"
#include "set.h"
#include "Action.h"

#undef INFINITY // disable macro definition of INFINITY

class Control;
class Action;

//
// 
//
class ProcessedRuleElement
{
public:
    int image,
        token_index,  // points to the lex_stream
        type_index,   // points to a symbol image
        symbol_index, // points to a symbol element in a symbol_set in a ClassnameElement
        position;     // position of this symbol in the right-hand side of the rule
};

//
//
//
class ActionBlockElement
{
public:
    enum
    {
        INITIALIZE,
        BODY,
        FINALIZE
    };

    int rule_number,
        location;
    LexStream::TokenIndex block_token;
    TextBuffer *buffer;
};


//
//
//
class SpecialArrayElement
{
public:
    int lhs_image;
    const char *name;
    Tuple<int> rules;
};


//
//
//
class ClassnameElement
{
public:
    ClassnameElement() : rule_index(0),
                         specified_name(NULL),
                         real_name(NULL),
                         is_terminal_class(false),
                         needs_environment(false),
                         array_element_type_symbol(NULL)
    {}
    int rule_index;
    char *specified_name,
         *real_name;
    bool is_terminal_class,
         needs_environment;
    VariableSymbol *array_element_type_symbol;
    Tuple<SpecialArrayElement> special_arrays;
    Tuple<int> rule,
               ungenerated_rule,
               interface_,
               rhs_type_index;
    SymbolLookupTable symbol_set;

    const char *GetAllocationName(int lhs_image)
    {
        for (int i = 0; i < special_arrays.Length(); i++)
        {
            if (special_arrays[i].lhs_image == lhs_image)
                return special_arrays[i].name;
        }
        return real_name;
    }
};


//
//
//
class RuleAllocationElement
{
public:
    enum ListKind {
                    NOT_A_LIST,
                    EMPTY,
                    SINGLETON,
                    LEFT_RECURSIVE_EMPTY,
                    RIGHT_RECURSIVE_EMPTY,
                    LEFT_RECURSIVE_SINGLETON,
                    RIGHT_RECURSIVE_SINGLETON,
                    ADD_ELEMENT,
                    COPY_LIST
                 };

    RuleAllocationElement() : name(NULL),
                              is_terminal_class(false),
                              needs_environment(false),
                              list_kind(NOT_A_LIST),
                              list_symbol(0),
                              list_position(0),
                              element_symbol(0),
                              element_position(0)
    {}

    const char *name;
    bool is_terminal_class,
         needs_environment;
    ListKind list_kind;
    int list_symbol,
        list_position,
        element_type_symbol_index,
        element_symbol,
        element_position;
};


//
//
//
class Grammar : public Util
{
public:

    friend class Action;

    enum { DEFAULT_SYMBOL = 0 };

    //
    //
    //
    class PairElement
    {
    public:
        int left_symbol,
            right_symbol;
    };

    //
    // 
    //
    class SymbolElement
    {
    public:
        VariableSymbol *symbol;
        int external_name_index;
    };

    //
    //
    //
    class AttributeElement
    {
    public:
        int lhs_image;
        LexStream::TokenIndex block_token;
    };

    //
    // RULES is the structure that contain the rules of the grammar.
    // Every rule of the grammar is mapped into an integer, and given
    // rule, and we have access to a value RHS which is the index
    // location in the vector RHS where the right-hand-side of the rule
    // begins.  The right hand side of a certain rule represented by an
    // integer I starts at index location RULES[I].RHS in RHS, and
    // ends at index location RULES[I + 1].RHS - 1.  An extra
    // NUM_RULES + 1 element is used as a "fence" for the last rule.
    // The RHS vector as mentioned above is used to hold a complete
    // list of allthe right-hand-side symbols specified in the grammar.
    //
    class RuleElement
    {
    public:
        int first_token_index, // The index of the separator of the rule
            last_token_index,  // the last token in the rule
            lhs,
            separator_token_kind,
            rhs_index,
            source_index; // the index in the parser object of this rule.

        //
        // In case an or delimiter, '|', is used, we may still want
        // to know what equivalence symbol started the chain.
        //
        int produces_token_kind;

        bool IsPriorityProduction()
        {
            return produces_token_kind == TK_PRIORITY_ARROW ||
                   produces_token_kind == TK_PRIORITY_EQUIVALENCE;
        }
        bool IsArrowProduction()
        {
            return produces_token_kind == TK_ARROW || produces_token_kind == TK_PRIORITY_ARROW;
        }
        bool IsAlternateProduction() { return separator_token_kind == TK_OR_MARKER; }
    };

    VariableSymbol *GetSymbol(int);

    const char *Get_ast_token_classname() { return ast_token_classname; }
    int Get_ast_token_interface() { return ast_token_interface; }

     Tuple<VariableSymbol*>& GetStartSymbol() 
    {
        return  start_symbol;
    }
private:

    TextBuffer notice_buffer;
    BitSet keyword_set,
           recover_set;

    Action *action;

    Tuple<int> declared_terminals_in_g;

    //
    // The variables below are used to hold information about special
    // grammar symbols.
    //
    Tuple<VariableSymbol *> variable_symbol_pool,
                            start_symbol;
    VariableSymbol *empty_symbol,
                   *identifier_symbol,
                   *eol_symbol,
                   *eof_symbol,
                   *error_symbol,
                   *accept_symbol,
                   *null_symbol;

    VariableSymbol *allocate_variable_symbol(const char *keyword)
    {
        int length = strlen(keyword) + 1;
        char *string = new char[length + 1];

        string[0] = option -> escape;
        strcpy(string + 1, keyword);
        int i = variable_symbol_pool.NextIndex();
        variable_symbol_pool[i] = new VariableSymbol(string, length, 0, 0);

        delete [] string;

        return variable_symbol_pool[i];
    }

    int GetSymbolIndex(int);
    int AssignSymbolIndex(VariableSymbol *);
    void ProcessExportedTerminals();
    void ProcessTerminals(Tuple<int> &);
    void ProcessInitialAliases(Tuple<int> &);
    void ProcessRemainingAliases(Tuple<int> &);
    void ProcessTitleOrGlobalBlock(int, ActionBlockElement &);
    char *InsertInterface(SymbolLookupTable &, char *);
    void ProcessRules(Tuple<int> &);
    void SetName(VariableSymbol *, VariableSymbol *, bool negate = false);
    void ProcessNames();
    void DisplayString(const char *, const char);
    void DisplaySymbol(const char *);
    void DisplayInput();
    void DisplayEBNF();

    Control *control;
    Option *option;
    Blocks *action_blocks;
    LexStream *lex_stream;
    VariableLookupTable *variable_table;
    MacroLookupTable *macro_table;

    char *ast_token_classname;
    int ast_token_interface;

public:

    Grammar(Control *control_,
            Blocks *action_blocks_,
            VariableLookupTable *variable_table_,
            MacroLookupTable *macro_table_);

    ~Grammar()
    {
        for (int i = 0; i < variable_symbol_pool.Length(); i++)
            delete variable_symbol_pool[i];
        delete this -> action;

        return;
    }

    Parser parser;

    void Process();

    int num_items,
        num_symbols,
        num_single_productions,
        num_terminals,
        num_nonterminals,
        num_names,
        num_rules;

    int empty,
        identifier_image,
        eol_image,
        eof_image,
        error_image,
        accept_image,
        start_image;

    //
    // Buffer containing "notice" string to be output.
    //
    TextBuffer &NoticeBuffer() { return notice_buffer; }

    //
    // SYMBOL_INDEX is an array that maps symbol numbers to actual symbols.
    //
    Tuple<SymbolElement> symbol_index;
    Tuple<RuleElement> rules;
    Tuple<int> rhs_sym;
    Tuple<int> keywords;
    Tuple<int> recovers;
    Tuple<char *> name;
    Tuple<VariableSymbol *> exported_symbols;
    Tuple<PairElement> check_predecessor_sets_for;

    bool IsTerminal(int x)    { return x > 0 && x <= num_terminals; }
    bool IsNonTerminal(int x) { return x > num_terminals && x <= num_symbols; }
    bool IsKeyword(int x)     { return IsTerminal(x) && keyword_set[x]; }
    BitSet &KeywordSet()      { return keyword_set; }
    bool IsRecover(int x)     { return recover_set[x]; }
    BitSet &RecoverSet()      { return recover_set; }

    int RhsSize(int rule_no)  { return rules[rule_no + 1].rhs_index - rules[rule_no].rhs_index; }

    bool IsUnitProduction(int rule_no) { return rules[rule_no].IsArrowProduction() && RhsSize(rule_no) == 1; }

    Token *RetrieveTokenLocation(int i) { return symbol_index[i].symbol -> Location(); }
    LexStream *GetLexStream() { return lex_stream; }
    char *RetrieveString(int i) { return symbol_index[i].symbol -> Name(); }
    char *RetrieveName(int i)   { return name[i]; }

    int FirstRhsIndex(int rule_no) { return rules[rule_no].rhs_index; }
    int EndRhsIndex(int rule_no)   { return rules[(rule_no) + 1].rhs_index; }

    inline int FirstTerminal() { return 1; }
    inline int LastTerminal()  { return num_terminals; }

    inline int FirstNonTerminal() { return num_terminals + 1; }
    inline int LastNonTerminal()  { return num_symbols; }

    inline int FirstRule() { return 0; }
    inline int LastRule()  { return num_rules; }

    void RestoreSymbol(char *, char *);
    void PrintLargeToken(char *, const char *, const char *, int);
};
#endif /* GRAMMAR_INCLUDED */
