#ifndef option_INCLUDED
#define option_INCLUDED

#include "util.h"
#include "code.h"
#include "tuple.h"
#include "blocks.h"
#include "symbol.h"

class LexStream;
class Token;
class VariableSymbol;
class OptionParser;
class OptionProcessor;

class Option : public Code, public Util
{
public:

    FILE *syslis;

    enum
    {
        //
        // Possible values for option "names"
        //
        MINIMUM = 1,
        MAXIMUM = 2,
        OPTIMIZED = 3,

        //
        // Possible values for rule_classnames
        //
        SEQUENTIAL = 1,
        STABLE = 2,

        //
        // Possible values for option "programming_language"
        //
        NONE = 0,
        XML = 1,
        C = 2,
        CPP = 3,
        JAVA = 4,
        PLX = 5,
        PLXASM = 6,
        ML = 7,
        CPP2 = 8,
        //
        // Possible values for option "trace"
        //
        // NONE = 0,
        //
        CONFLICTS = 1,
        FULL = 2,

        //
        // Possible values for option "automatic_ast"
        //
        // NONE = 0,
        //
        NESTED = 1,
        TOPLEVEL = 2,

        //
        // Possible values for option "variables"
        //
        // NONE = 0,
        //
        BOTH = 1,
        NON_TERMINALS = 2,
        TERMINALS = 3,

        //
        // Possible values for option "visitor"
        //
        // NONE = 0,
        //
        DEFAULT = 1,
        PREORDER = 2
    };

    int return_code;

    const char *home_directory;
    Tuple<const char *> include_search_directory,
                        template_search_directory,
                        filter_file,
                        import_file;

    const char *template_directory,
               *ast_directory_prefix;

    bool attributes,
         backtrack,
         legacy,
         list,
         glr,
         slr,
         verbose,
         first,
         follow,
         priority,
         edit,
         states,
         xref,
         nt_check,
         conflicts,
         read_reduce,
         remap_terminals,
         goto_default,
         shift_default,
         byte,
         warnings,
         single_productions,
         error_maps,
         debug,
         parent_saved,
         precedence,
         scopes,
         serialize,
         soft_keywords,
         table;

    bool for_parser;
    int lalr_level,
        margin,
        max_cases,
        names,
        rule_classnames,
        trace,
        programming_language,
        automatic_ast,
        variables,
        visitor;

    char escape,
         or_marker;

    const char *factory,
               *file_prefix,

               *grm_file,
               *lis_file,
               *tab_file,

               *dat_directory,
               *dat_file,
               *dcl_file,
               *def_file,
               *directory_prefix,
               *prs_file,
               *sym_file,
			  * top_level_ast_file,
               *imp_file,
               *exp_file,
               *exp_prefix,
               *exp_suffix,

               *out_directory,
               *ast_directory,
               *ast_package;

    //private:
    const char *ast_type;
    //public:
    const char *exp_type,
               *prs_type,
               *sym_type,
			   *top_level_ast_file_prefix,
               *dcl_type,
               *imp_type,
               *def_type,
               *action_type,
               *visitor_type,

               *filter,
               *import_terminals,
               *include_directory,
               *template_name,
               *extends_parsetable,
               *parsetable_interfaces,
               *package,
               *prefix,
               *suffix;

    bool quiet;

    TextBuffer report;
    LexStream *lex_stream;

    void SetLexStream(LexStream *lex_stream_) { this -> lex_stream = lex_stream_; }

    void FlushReport()
    {
        assert(syslis);
        report.Print(syslis);
        report.Flush(stdout);
    }

    Token *GetTokenLocation(const char *, int);
    void EmitHeader(Token *, const char *);
    void EmitHeader(Token *, Token *, const char *);
    void Emit(Token *, const char *, const char *);
    void Emit(Token *, const char *, Tuple<const char *> &);
    void Emit(Token *, Token *, const char *, const char *);
    void Emit(Token *, Token *, const char *, Tuple<const char *> &);
    void EmitError(int, const char *);
    void EmitError(int, Tuple<const char *> &);
    void EmitWarning(int, const char *);
    void EmitWarning(int, Tuple<const char *> &);
    void EmitInformative(int, const char *);
    void EmitInformative(int, Tuple<const char *> &);

    void EmitError(Token *token, const char *msg)                { Emit(token, "Error: ", msg); return_code = 12; }
    void EmitError(Token *token, Tuple<const char *> &msg)       { Emit(token, "Error: ", msg); return_code = 12; }
    void EmitWarning(Token *token, const char *msg)              { Emit(token, "Warning: ", msg); }
    void EmitWarning(Token *token, Tuple<const char *> &msg)     { Emit(token, "Warning: ", msg); }
    void EmitInformative(Token *token, const char *msg)          { Emit(token, "Informative: ", msg); }
    void EmitInformative(Token *token, Tuple<const char *> &msg) { Emit(token, "Informative: ", msg); }

    void EmitError(Token *startToken, Token *endToken, const char *msg)                { Emit(startToken, endToken, "Error: ", msg); return_code = 12; }
    void EmitError(Token *startToken, Token *endToken, Tuple<const char *> &msg)       { Emit(startToken, endToken, "Error: ", msg); return_code = 12; }
    void EmitWarning(Token *startToken, Token *endToken, const char *msg)              { Emit(startToken, endToken, "Warning: ", msg); }
    void EmitWarning(Token *startToken, Token *endToken, Tuple<const char *> &msg)     { Emit(startToken, endToken, "Warning: ", msg); }
    void EmitInformative(Token *startToken, Token *endToken, const char *msg)          { Emit(startToken, endToken, "Informative: ", msg); }
    void EmitInformative(Token *startToken, Token *endToken, Tuple<const char *> &msg) { Emit(startToken, endToken, "Informative: ", msg); }

    void InvalidValueError(const char *, const char *, int);
    void InvalidTripletValueError(const char *, int, const char *, const char *);

    //
    // Turn all backslashes into forward slashes in filename.
    //
    void NormalizeSlashes(char *filename)
    {
        for (char *s = filename; *s != '\0'; s++)
        {
            if  (*s == '\\')
                 *s = '/';
        }
    }

    Option(int argc_, const char **argv_);
    ~Option();

    void ProcessUserOptions(InputFileSymbol *, char *, int);
    void ProcessCommandOptions();
    void CompleteOptionProcessing();
    void PrintOptionsInEffect();

    static void PrintOptionsList(void);

    Blocks &ActionBlocks() { return action_blocks; }
    BlockSymbol *DefaultBlock(void) { return default_block; }
    BlockSymbol* AstBlock(void) { return ast_block; }
    ActionFileSymbol *DefaultActionFile(void) { return default_action_file; }
    const char *DefaultActionPrefix(void) { return default_action_prefix; }

    const char *GetFilename(const char *);

private:
    friend class OptionProcessor;

    int argc;
    const char **argv;

    OptionParser *optionParser;
    OptionProcessor *optionProcessor;

    class BlockInfo
    {
    public:
        Token *location;
        const char *filename,
                   *block_begin,
                   *block_end;
        void Set(Token *location_, const char *filename_, const char *block_begin_, const char *block_end_)
        {
            location = location_;
            filename = filename_;
            block_begin = block_begin_;
            block_end = block_end_;
        }
    };
    Tuple<BlockInfo> action_options,
                     header_options,
                     trailer_options;

    Token *dat_directory_location,
          *out_directory_location,
          *ast_directory_location,
          *escape_location,
          *or_marker_location;

    Blocks action_blocks;

    static const char *default_block_begin;
    static const char *default_block_end;


    const  char* default_ast_block_begin;
    const   char* default_ast_block_end;

	
    InputFileSymbol *input_file_symbol;
    const char *buffer_ptr,
               *parm_ptr;

    BlockSymbol* ast_block;
    BlockSymbol *default_block;
    ActionFileSymbol *default_action_file;
    const char *default_action_prefix;

    Tuple<char *> temp_string;
    char *NewString(int n) { return temp_string.Next() = new char[n]; }
    char *NewString(const char *in)
    {
        char *out = new char[strlen(in) + 1];
        temp_string.Next() = out;
        strcpy(out, in);
        return out;
    }
    char *NewString(const char *in, int length)
    {
        char *out = new char[length + 1];
        temp_string.Next() = out;
        strncpy(out, in, length);
        out[length] = NULL_CHAR;
        return out;
    }

    const char *AllocateString(const char *);
    const char *AllocateString(const char *, char);
    const char *AllocateString(const char *, const char *);
    const char *AllocateString(const char *, int);
    bool IsDelimiter(char);
    const char *CleanUp(const char *);
    const char *ValuedOption(const char *);
    const char *GetValue(const char *, const char *&);
    const char *GetStringValue(const char *, const char *&);
    int OptionMatch(const char *, const char *, const char *);

    const char *(Option::*classify_option[128])(const char *, bool);

    const char *ClassifyA(const char *, bool = true);
    const char *ClassifyB(const char *, bool = true);
    const char *ClassifyC(const char *, bool = true);
    const char *ClassifyD(const char *, bool = true);
    const char *ClassifyE(const char *, bool = true);
    const char *ClassifyF(const char *, bool = true);
    const char *ClassifyG(const char *, bool = true);
    const char *ClassifyH(const char *, bool = true);
    const char *ClassifyI(const char *, bool = true);
    const char *ClassifyL(const char *, bool = true);
    const char *ClassifyM(const char *, bool = true);
    const char *ClassifyN(const char *, bool = true);
    const char *ClassifyO(const char *, bool = true);
    const char *ClassifyP(const char *, bool = true);
    const char *ClassifyQ(const char *, bool = true);
    const char *ClassifyR(const char *, bool = true);
    const char *ClassifyS(const char *, bool = true);
    const char *ClassifyT(const char *, bool = true);
    const char *ClassifyV(const char *, bool = true);
    const char *ClassifyW(const char *, bool = true);
    const char *ClassifyX(const char *, bool = true);
    const char *ClassifyBadOption(const char *, bool = true);
    const char *AdvancePastOption(const char *);

    const char *ReportAmbiguousOption(const char *, const char *);
    const char *ReportMissingValue(const char *, const char *);
    const char *ReportValueNotRequired(const char *, const char *);

    void ProcessBlock(BlockInfo &);
    void ProcessHeader(BlockInfo &);
    void ProcessTrailer(BlockInfo &);
    void CheckBlockMarker(Token *, const char *);
    void CheckGlobalOptionsConsistency();
    void CheckAutomaticAst();

    void ProcessOptions(const char *);
    void ProcessPath(Tuple<const char *> &, const char *, const char * = NULL);
    const char *GetPrefix(const char *);
    const char *GetFile(const char *, const char *, const char *);
    const char *GetType(const char *);
    const char *ExpandFilename(const char *);
    void CheckDirectory(Token *, const char *);
};

#endif
