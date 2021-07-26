#ifndef stream_INCLUDED
#define stream_INCLUDED

#include "jikespg_sym.h"
#include "jikespg_prs.h"
#include "code.h"
#include "option.h"
#include "symbol.h"
#include "tuple.h"
#include "tab.h"

class Control;
class Scanner;
class jikespg_act;

class LexStream;
class Token
{


    //
    // It is expected that a location will be set for every token.
    // Note that a good way to reset all the bits in "info" is to
    // use the function ResetInfoAndSetLocation defined below, instead
    // of using SetLocation
    //
    inline void SetLocation(InputFileSymbol *file_symbol_, unsigned location)
    {
        file_symbol = file_symbol_;
        assert(location <= 0x00FFFFFF);
        info = (info & 0x0000003F) | (location << 8);
    }

public:
	    Token(): info(0), additional_info(), end_location(0), file_symbol(nullptr)
    {
    }
    unsigned info;
    union
    {
        Symbol *symbol;
        int right_brace;
    } additional_info;
    unsigned end_location;
    InputFileSymbol *file_symbol;

    //
    // To just reset the info, this function should be invoked with a location value of 0.
    //
    inline void ResetInfoAndSetLocation(InputFileSymbol *file_symbol_, unsigned start_location)
    {
        file_symbol = file_symbol_;
        assert(start_location >= 0 && start_location <= 0x00FFFFFF);
        info = (start_location << 8);
        additional_info.symbol = NULL;
        end_location = start_location;
    }

    inline unsigned StartLocation()              { return (info >> 8); }
    inline unsigned EndLocation()                { return end_location; }
    inline void SetEndLocation(unsigned end_loc) { end_location = end_loc; }
    inline void SetKind(unsigned kind)           { assert(kind <= 0x0000003F); info = (info & 0xFFFFFFC0) | kind; }
    inline unsigned Kind()                       { return (info & 0x0000003F); }
    inline char *FileName()                      { return file_symbol -> Name(); }
    inline int FileNameLength()                  { return file_symbol -> NameLength(); }
    inline unsigned Line()                       { return FindLine(file_symbol -> LineLocation(), StartLocation()); }
    inline unsigned EndLine()                    { return FindLine(file_symbol -> LineLocation(), EndLocation());  }
    inline unsigned Column()                     { return FindColumn(file_symbol, StartLocation()); }
    inline unsigned EndColumn()                  { return FindColumn(file_symbol, EndLocation()); }
    InputFileSymbol *FileSymbol()                { return file_symbol; }
    Symbol *GetSymbol()                          { return additional_info.symbol; }

    /*
    //
    // Bits 7 and 8 are currently unused.
    //
    inline void SetMainToken()                   { info &= 0xFFFFFF3F; } // set bits 7 and 8 to 0
    inline bool IsMainToken()                    { return ((info & 0x000000C0) == 0); }
    inline void SetImportedToken()               { info |= 0x00000040; }
    inline bool IsImportedToken()                { return (info & 0x00000040); }
    inline void SetTemplateToken()               { info |= 0x00000080; }
    inline bool IsTemplateToken()                { return (info & 0x00000080); }
    */

    inline void SetSymbol(::Symbol *symbol)
    {
        additional_info.symbol = symbol;
        if (symbol -> Location() == NULL) // first time seen this symbol?
            symbol -> SetLocation(this); // set its location.
    }

    static unsigned FindLine(Tuple<unsigned> &line_location, unsigned location)
    {
        int lo = 0,
            hi = line_location.Length() - 1;

        if (hi == 0)
            return 0;

        //
        // we can place the exit test at the bottom of the loop
        // since the line_location array will always contain at least
        // one element.
        //
        do
        {
            int mid = (lo + hi) / 2;

            if (line_location[mid] == location)
                return mid;
            if (line_location[mid] < location)
                 lo = mid + 1;
            else hi = mid - 1;
        } while (lo < hi);

        return (line_location[lo] > location ? lo - 1 : lo);
    }

    static unsigned FindColumn(InputFileSymbol *file_symbol, unsigned location)
    {
        if (! file_symbol -> Buffer())
            file_symbol -> ReadInput();
        int index = FindLine(file_symbol -> LineLocation(), location);
        return (index == 0 ? 0 : Tab::strlen(file_symbol -> Buffer(), file_symbol -> LineLocation(index), location));
    }
};

//
//
//
class FilterMacroElement
{
public:
    char *macro_name,
         *macro_value;

    FilterMacroElement() : macro_name(NULL),
                           macro_value(NULL)
    {}

    ~FilterMacroElement()
    {
        delete [] macro_name;
        delete [] macro_value;
    }

    void SetMacroName(const char *name)
    {
        const char *forward_slash = strrchr(name, '/'),
                   *backward_slash = strrchr(name, '\\'),
                   *slash = (forward_slash > backward_slash ? forward_slash : backward_slash),
                   *colon = strrchr(name, ':'), // Windows files may use format: d:filename
                   *separator = (colon > slash ? colon : slash),
                   *start = (separator ? separator + 1 : name),
                   *dot = strrchr(name, '.');
        int length = (dot == NULL ? strlen(start) : dot - start);
        macro_name = new char[length + 1];
        memcpy(macro_name, start, length);
        macro_name[length] = '\0';
    }

    void SetMacroValue(const char *value)
    {
        const char *forward_slash = strrchr(value, '/'),
                   *backward_slash = strrchr(value, '\\'),
                   *slash = (forward_slash > backward_slash ? forward_slash : backward_slash),
                   *colon = strrchr(value, ':'), // Windows files may use format: d:filename
                   *separator = (colon > slash ? colon : slash),
                   *start = (separator ? separator + 1 : value),
                   *dot = strrchr(value, '.');
        int length = (dot == NULL ? strlen(start) : dot - start);
        macro_value = new char[length + 1];
        memcpy(macro_value, start, length);
        macro_value[length] = '\0';
    }
};


//
// LexStream holds a stream of tokens generated from an input and
// provides methods to retrieve information from the stream.
//
class LexStream
{
public:
    typedef int TokenIndex;

    inline TokenIndex Next(TokenIndex i)
         { return (++i < token_stream.Length() ? i : token_stream.Length() - 1); }

    inline TokenIndex Previous(TokenIndex i) { return (i <= 0 ? 0 : i - 1); }

    inline TokenIndex Peek() { return Next(index); }

    inline void Reset(TokenIndex i = 1) { index = Previous(i); }

    inline TokenIndex Gettoken() { return index = Next(index); }

    inline TokenIndex Gettoken(TokenIndex end_token)
         { return index = (index < end_token ? Next(index) : token_stream.Length() - 1); }

    inline TokenIndex Badtoken() { return 0; }

    inline unsigned Kind(TokenIndex i) { return token_stream[i].Kind(); }

    inline unsigned StartLocation(TokenIndex i) { return token_stream[i].StartLocation(); }

    inline unsigned EndLocation(TokenIndex i) { return token_stream[i].EndLocation(); }

    inline unsigned Line(TokenIndex i) { return token_stream[i].Line(); }

    inline unsigned EndLine(TokenIndex i) { return token_stream[i].EndLine(); }

    inline unsigned Column(TokenIndex i) { return token_stream[i].Column(); }

    inline unsigned EndColumn(TokenIndex i) { return token_stream[i].EndColumn(); }

    inline bool AfterEol(TokenIndex i) { return (i < 1 ? true : Line(i - 1) != Line(i)); }

    inline char *FileName(TokenIndex i) { return token_stream[i].file_symbol -> Name(); }
    inline int FileNameLength(TokenIndex i) { return token_stream[i].file_symbol -> NameLength(); }

    char *InputBuffer(TokenIndex i)
    {
        if (! token_stream[i].file_symbol -> Buffer())
            token_stream[i].file_symbol -> ReadInput();
        return token_stream[i].file_symbol -> Buffer();
    }
    int InputBufferLength(TokenIndex i)
    {
        if (! token_stream[i].file_symbol -> Buffer())
            token_stream[i].file_symbol -> ReadInput();
        return token_stream[i].file_symbol -> BufferLength();
    }

    //
    //
    //
    const char *NameString(TokenIndex i)
    {
        VariableSymbol *variable = GetVariableSymbol(i);
    	if(variable)
    	{
            return  variable->Name();
    	}
        MacroSymbol *macro = GetMacroSymbol(i);
        if (macro)
        {
            return  macro->Name();
        }
        BlockSymbol* block = GetBlockSymbol(i);
    	if(Kind(i) == TK_BLOCK)
    	{
           return  block->BlockBegin();
    	}
        return   KeywordName(token_stream[i].Kind());

    /*    return (variable ? variable -> Name()
                         : macro ? macro -> Name()
                                 : Kind(i) == TK_BLOCK ? block -> BlockBegin()
                                                       : KeywordName(token_stream[i].Kind()));*/
    }

    //
    //
    //
    int NameStringLength(TokenIndex i)
    {
        VariableSymbol *variable = GetVariableSymbol(i);
        MacroSymbol *macro = GetMacroSymbol(i);

        return (variable ? variable -> NameLength()
                         : macro ? macro -> NameLength()
                                 : Kind(i) == TK_BLOCK ? EndLocation(i) - StartLocation(i)
                                                       : strlen(KeywordName(token_stream[i].Kind())));
    }

    //
    //
    //
    InputFileSymbol *GetFileSymbol(TokenIndex i)
    {
        return token_stream[i].file_symbol;
    }

    //
    //
    //
    VariableSymbol *GetVariableSymbol(TokenIndex i)
    {
        Symbol *symbol = token_stream[i].additional_info.symbol;
        return (symbol ? symbol -> VariableCast() : NULL);
    }

    //
    //
    //
    MacroSymbol *GetMacroSymbol(TokenIndex i)
    {
        Symbol *symbol = token_stream[i].additional_info.symbol;
        return (symbol ? symbol -> MacroCast() : NULL);
    }

    //
    //
    //
    BlockSymbol *GetBlockSymbol(TokenIndex i)
    {
        Symbol *symbol = token_stream[i].additional_info.symbol;
        return (symbol ? symbol -> BlockCast() : NULL);
    }

    inline int LineSegmentLength(TokenIndex i)
    {
        if (! token_stream[i].file_symbol -> Buffer())
            token_stream[i].file_symbol -> ReadInput();
        int line_end = token_stream[i].file_symbol -> LineLocation(Line(i) + 1) - 1;
        return Tab::strlen(token_stream[i].file_symbol -> Buffer(), token_stream[i].StartLocation(), line_end);
    }

    inline int NumTokens() { return token_stream.Length(); }
    inline int ImportedTerminal(int i) { return imported_terminals[i]; }
    inline int NumImportedTerminals() { return imported_terminals.Length(); }

    inline int ImportedFilter(int i) { return imported_filters[i]; }
    inline int NumImportedFilters() { return imported_filters.Length(); }

    inline FilterMacroElement &FilterMacro(int i) { return filter_macros[i]; }
    inline int NumFilterMacros() { return filter_macros.Length(); }

    //
    // Constructors and Destructor.
    //
    LexStream(Option *option_) : option(option_),
                                 token_stream(12, 16)
    {
        option_ -> SetLexStream(this);

        keyword_name.Resize(NUM_TOKENS + 1);
        keyword_name[0] = NULL;
        for (int k = 1; k < keyword_name.Size(); k++)
        {
            int i = jikespg_prs::terminal_index[k],
                length = jikespg_prs::name_length(i);
            char *str = new char[length + 1];
            strncpy(str, &jikespg_prs::string_buffer[jikespg_prs::name_start[i]], length);
            str[length] = '\0';

            keyword_name[k] = (const char *) str;
        }
    }

    ~LexStream()
    {
        for (int i = 0; i < keyword_name.Size(); i++)
             delete [] ((char *) keyword_name[i]);
    }

    void Dump(); // temporary function used to dump token stream.

    TokenIndex GetNextToken(InputFileSymbol *file_symbol, unsigned location = 0)
    {
        TokenIndex index = token_stream.NextIndex();
        token_stream[index].ResetInfoAndSetLocation(file_symbol, location);

        return index;
    }

    Token *GetTokenReference(int index) { return &(token_stream[index]); }

    Token *GetErrorToken(InputFileSymbol *file_symbol, unsigned location = 0)
    {
        Token *error_token = &(error_stream.Next());
        error_token -> ResetInfoAndSetLocation(file_symbol, location);

        return error_token;
    }

    void ResetTokenStream(int size = 0) { token_stream.Reset(size); }
    void AddImportedTerminal(int i) { imported_terminals.Next() = i; }
    void AddImportedFilter(int i) { imported_filters.Next() = i; }
    void AddFilterMacro(const char *name, const char *value)
    {
        FilterMacroElement &filter = filter_macros.Next();
        filter.SetMacroName(name);
        filter.SetMacroValue(value);
    }

    InputFileSymbol *FindOrInsertFile(Tuple<const char *> &, const char *name); 

    InputFileSymbol *FindOrInsertFile(const char *name)
    {
        Tuple<const char *> default_search_directory;
        default_search_directory.Next() = "";
        return FindOrInsertFile(default_search_directory, name == NULL ? "" : name);
    }

private:

    Option *option;

    InputFileLookupTable file_table;

    TokenIndex index{0};
    Tuple<Token> token_stream,
                 error_stream;
    Tuple<int> imported_terminals,
               imported_filters;
    Tuple<FilterMacroElement> filter_macros;
    Array<const char*> keyword_name;

    const char *KeywordName(int i) { return keyword_name[i]; }
};

typedef LexStream::TokenIndex TokenObject;
typedef LexStream::TokenIndex Location;

#endif
