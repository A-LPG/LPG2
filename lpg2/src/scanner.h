#ifndef scanner_INCLUDED
#define scanner_INCLUDED

#include "code.h"
#include "jikespg_sym.h"
#include "option.h"
#include "LexStream.h"
#include "symbol.h"
#include "tuple.h"

class Arguments
{
public:
    Arguments(Option *option, const char *filename, const char *exp_file, const char *exp_prefix, const char *exp_suffix)
    {
        const char *quiet_header = "-quiet",
                   *noquiet_header = "-noquiet",
                   *export_header = "-export_terminals=(\"",
                   *include_header = "-include=",
                   *package_header = "-package=",
                   *ast_directory_header = "-ast_directory=",
                   *directory_prefix_header = "-directory-prefix=",
                   *out_directory_header = "-out-directory=";
        char *quiet_arg = NewString(strlen(noquiet_header) + 1),
             *export_arg = NewString(strlen(export_header) +
                                     strlen(exp_file) +
                                     strlen(exp_prefix) +
                                     strlen(exp_suffix) + 11),
             *include_arg = NewString(strlen(include_header) + strlen(option -> include_directory) + 1),
             *package_arg = NewString(strlen(package_header) + strlen(option -> package) + 1),
             *ast_directory_arg = NewString(strlen(ast_directory_header) + strlen(option -> ast_directory) + 1),
             *directory_prefix_arg = NewString(strlen(directory_prefix_header) + strlen(option -> directory_prefix) + 1),
             *out_directory_arg = NewString(strlen(out_directory_header) + strlen(option->out_directory) + 1);

        strcpy(quiet_arg, option -> quiet ? quiet_header : noquiet_header);

        strcpy(export_arg, export_header);
        strcat(export_arg, exp_file);
        strcat(export_arg, "\", \"");
        strcat(export_arg, exp_prefix);
        strcat(export_arg, "\", \"");
        strcat(export_arg, exp_suffix);
        strcat(export_arg, "\")");

        strcpy(include_arg, include_header);
        strcat(include_arg, option -> include_directory);

        strcpy(package_arg, package_header);
        strcat(package_arg, option -> package);

        strcpy(ast_directory_arg, ast_directory_header);
        strcat(ast_directory_arg, option -> ast_directory);

        strcpy(directory_prefix_arg, directory_prefix_header);
        strcat(directory_prefix_arg, option -> directory_prefix);

        strcpy(out_directory_arg, out_directory_header);
        strcat(out_directory_arg, option -> out_directory);
        
        //
        // Initialize argc and argv.
        //
        argc = temp_string.Length() + 2;
        argv = new const char *[argc];
        argv[0] = NULL;
        for (int i = 0; i < temp_string.Length(); i++)
            argv[i + 1] = temp_string[i];
        argv[argc - 1] = filename;
    }

    ~Arguments()
    {
        for (int i = 0; i < temp_string.Length(); i++)
            delete [] temp_string[i];
        delete [] argv;
    }

    Tuple<char *> temp_string;
    char *NewString(int n) { return temp_string.Next() = new char[n]; }

    int argc;
    const char **argv;
};


class ImportArguments : public Arguments
{
public:
    ImportArguments(Option *option, const char *filename) : Arguments(option, 
                                                                      filename,
                                                                      option -> sym_file,
                                                                      option -> prefix,
                                                                      option -> suffix)
    {}
};


class FilterArguments : public Arguments
{
public:
    FilterArguments(Option *option, const char *filename) : Arguments(option, 
                                                                      filename,
                                                                      option -> exp_file,
                                                                      option -> exp_prefix,
                                                                      option -> exp_suffix)
    {}
};


//
// The Scanner object
//
class Scanner : public Code
{
public:

    Scanner(Option *option_,
            LexStream *lex_stream_,
            VariableLookupTable *variable_table_,
            MacroLookupTable *macro_table_) : option(option_),
                                              lex_stream(lex_stream_),
                                              variable_table(variable_table_),
                                              macro_table(macro_table_),
                                              action_blocks(&(option_ -> ActionBlocks())),
                                              input_file(NULL)
    {}

    ~Scanner()
    {
        for (int i = 0; i < temp_string.Length(); i++)
            delete [] temp_string[i];
    }

    void Scan();
    void Scan(int);

    inline int NumErrorTokens() { return error_tokens.Length(); }

private:
    enum StreamErrorKind
    {
        NO_ERROR,
        NO_INPUT,
        NO_TEMPLATE,
        NO_INCLUDE,
        LEGACY_KEYWORD,
        SYMBOL_WITH_KEYWORD_MARKER,
        RECURSIVE_INCLUDE,
        BAD_UNICODE,
        BAD_OCTAL_ASCII_CODE,
        ISOLATED_BACKSLASH,
        UNDELIMITED_STRING_SYMBOL,
        UNTERMINATED_STRING_SYMBOL,
        UNTERMINATED_BRACKET_SYMBOL,
        UNTERMINATED_BLOCK,
        INCLUDE_OPTIONS
    };

    Tuple<char *> temp_string;
    char *NewString(int n) { return temp_string.Next() = new char[n]; }

    class ProblemToken
    {
    public:
        int msg_code;
        const char *name;
        Token *token;

        void Initialize(int msg_code_, const char *name_, Token *token_)
        {
            msg_code = msg_code_;
            name = name_;
            token = token_;
        }
    };

    Option *option;
    LexStream *lex_stream;
    VariableLookupTable *variable_table;
    MacroLookupTable *macro_table;
    Blocks *action_blocks;

    InputFileSymbol *input_file;
    Tuple<unsigned> *line_location;
    char *input_buffer,
         *cursor;
    Token *current_token;
    int current_token_index;

    Stack<InputFileSymbol *> file_stack;
    Stack<char *> cursor_stack;
    void PushEnvironment()
    {
        file_stack.Push(input_file);
        cursor_stack.Push(cursor);
    }
    void PopEnvironment()
    {
        assert(! file_stack.IsEmpty());
        input_file = file_stack.Pop();
              line_location = input_file -> LineLocationReference();
              input_buffer = input_file -> Buffer();
        cursor = cursor_stack.Pop();
    }
    void ResetEnvironment(char *cursor)
    {
        Tuple<unsigned> *line_location = input_file -> LineLocationReference();
        char *input_buffer = input_file -> Buffer();
        unsigned location = cursor - input_buffer;
        int length;
        for (length = line_location -> Length() - 1; length > 0; length--)
            if ((*line_location)[length] <= location)
                break;
        line_location -> Reset(length + 1);
    }


    enum { SCAN_KEYWORD_SIZE = 24 + 1 };

    static int (*scan_keyword[SCAN_KEYWORD_SIZE]) (char *p1);
    static int ScanKeyword0(char *p1);
    static int ScanKeyword4(char *p1);
    static int ScanKeyword6(char *p1);
    static int ScanKeyword7(char *p1);
    static int ScanKeyword8(char *p1);
    static int ScanKeyword9(char *p1);
    static int ScanKeyword10(char *p1);
    static int ScanKeyword11(char *p1);
    static int ScanKeyword12(char *p1);
    static int ScanKeyword13(char *p1);
    static int ScanKeyword24(char *p1);

    void ReportErrors();
    char *ScanOptions();
    void SkipOptions();
    void ImportTerminals(const char *);
    void ProcessFilters(const char *);
    void Scan(char *, char *);
    void ScanComment();
    void SkipSpaces();

    void Setup();
    void ClassifyBlock(Tuple<BlockSymbol *> &);

    void (Scanner::*classify_token[256])();

    void ClassifyBadToken();
    void ClassifyKeyword();
    void ClassifyEscapedSymbol();
    void ClassifySingleQuotedSymbol();
    void ClassifyDoubleQuotedSymbol();
    void ClassifyLess();
    void ClassifySymbol();
    void ClassifyEquivalence();
    void ClassifyArrow();
    void ClassifyOr();
    void ClassifyEof();

    void ImportFiles(int, int);
    char *ProcessInclude(const char *);
    int IncludeFile();

    Tuple<ProblemToken> error_tokens,
                        warning_tokens;
    void ResetProblemTokens() { error_tokens.Reset(); warning_tokens.Reset(); }
    void AddErrorToken(int error_kind, LexStream::TokenIndex index) {
         error_tokens.Next().Initialize(error_kind, lex_stream -> NameString(index), lex_stream -> GetTokenReference(index));
    }
    void AddErrorToken(int error_kind, const char *name, Token *token) { error_tokens.Next().Initialize(error_kind, name, token); }
    void AddWarningToken(int warning_kind, LexStream::TokenIndex index) {
         warning_tokens.Next().Initialize(warning_kind, lex_stream -> NameString(index), lex_stream -> GetTokenReference(index));
    }
    void AddWarningToken(int warning_kind, const char *name, Token *token) { warning_tokens.Next().Initialize(warning_kind, name, token); }
};

#endif
