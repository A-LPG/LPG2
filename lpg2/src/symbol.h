#ifndef symbol_INCLUDED
#define symbol_INCLUDED

#include "tuple.h"
#include "code.h"
#include "util.h"
#include "buffer.h"

#include <sys/stat.h>
#include <cstring>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
class Token;
class VariableSymbol;
class RuleSymbol;
class MacroSymbol;
class SimpleMacroSymbol;
class BlockSymbol;
class InputFileSymbol;
class ActionFileSymbol;

class Symbol
{
public:
    Symbol *next;

    enum  SymbolKind
    {
         NONE,
         VARIABLE,
         RULE,
         MACRO,
         SIMPLE_MACRO,
         BLOCK,
         INPUT_FILE,
         ACTION_FILE
    };

    SymbolKind Kind() const { return kind; }

    VariableSymbol    *VariableCast()    { return (VariableSymbol *)    (kind == VARIABLE ? this : NULL); }
    RuleSymbol        *RuleCast()        { return (RuleSymbol *)        (kind == RULE ? this : NULL); }
    MacroSymbol       *MacroCast()       { return (MacroSymbol *)       (kind == MACRO ? this : NULL); }
    SimpleMacroSymbol *SimpleMacroCast() { return (SimpleMacroSymbol *) (kind == SIMPLE_MACRO ? this : NULL); }
    BlockSymbol       *BlockCast()       { return (BlockSymbol *)       (kind == BLOCK ? this : NULL); }
    InputFileSymbol   *InputFileCast()   { return (InputFileSymbol *)   (kind == INPUT_FILE ? this : NULL); }
    ActionFileSymbol  *ActionFileCast()  { return (ActionFileSymbol *)  (kind == ACTION_FILE ? this : NULL); }

    unsigned HashAddress() { return hash_address; }

    virtual char *Name() {
    	return name;
    }
    virtual size_t NameLength() { return length; }

    int Index() { return pool_index; }

    Token *Location() { return location; }
    void SetLocation(Token *location_) { location = location_; }

    Symbol(SymbolKind kind_, const char *name_, int length_, int pool_index_, unsigned hash_address_) : kind(kind_),
                                                                                                        length(length_),
                                                                                                        pool_index(pool_index_),
                                                                                                        location(NULL),
                                                                                                        hash_address(hash_address_)
    {
        name = new char[length_ + 1];
        memmove(name, name_, length_ * sizeof(char));
        name[length] = Code::NULL_CHAR;

        return;
    }

    Symbol(const char *name_, int length_, int pool_index_, unsigned hash_address_) : kind(NONE),
                                                                                      length(length_),
                                                                                      pool_index(pool_index_),
                                                                                      location(NULL),
                                                                                      hash_address(hash_address_)
    {
        name = new char[length_ + 1];
        memmove(name, name_, length_ * sizeof(char));
        name[length] = Code::NULL_CHAR;

        return;
    }

    virtual ~Symbol()
    {
        delete [] name;
    }

protected:
    SymbolKind kind;
    char *name;
    int length,
        pool_index;
    Token *location;
    unsigned hash_address;
};


class VariableSymbol : public Symbol
{
public:

    int SymbolIndex() { return symbol_index; }
    void SetSymbolIndex(int index_) { symbol_index = index_; }
    int NameIndex() { return name_index; }
    void SetNameIndex(int index_) { name_index = index_; }

    VariableSymbol(const char *name_,
                   int length_,
                   int pool_index_,
                   unsigned hash_address_) : Symbol(VARIABLE, name_, length_, pool_index_, hash_address_),
                                             symbol_index(0),
                                             name_index(Util::OMEGA)
    {}

    ~VariableSymbol()
    {}

private:
    int symbol_index,
        name_index;
};


class RuleSymbol : public Symbol
{
public:
    Tuple<int> &Rules() { return rules; }
    void AddRule(int rule_no) { rules.Next() = rule_no; }

    RuleSymbol(const char *name_, int length_, int pool_index_, unsigned hash_address_)
        : Symbol(RULE, name_, length_, pool_index_, hash_address_)
    {}

    virtual ~RuleSymbol() {}

private:
    Tuple<int> rules;
};


class MacroSymbol : public Symbol
{
public:
    int Block() { return block; }
    void SetBlock(int block_) { block = block_; }

    bool IsInUse()      { return in_use; }
    void MarkInUse()    { in_use = true; }
    void MarkNotInUse() { in_use = false; }

    MacroSymbol(const char *name_, int length_, int pool_index_, unsigned hash_address_) : Symbol(MACRO, name_, length_, pool_index_, hash_address_),
                                                                                           block(0),
                                                                                           in_use(false)
    {}

    virtual ~MacroSymbol()
    {}

private:
    int block;
    bool in_use;
};


class SimpleMacroSymbol : public Symbol
{
public:
    char *Value() { return value; }
    void SetValue(const char *value_) { value = new char[strlen(value_) + 1]; strcpy(value, value_); }

    bool IsUsed() { return used; }
    void MarkUsed() { used = true; }

    SimpleMacroSymbol(const char *name_,
                      int length_,
                      int pool_index_,
                      unsigned hash_address_) : Symbol(SIMPLE_MACRO, name_, length_, pool_index_, hash_address_),
                                                value(NULL),
                                                used(false)
    {}

    virtual ~SimpleMacroSymbol()
    {
        delete [] value;
    }

private:
    char *value;
    bool used;
};



class InputFileSymbol : public Symbol
{
public:
    InputFileSymbol(const char *name_,
                    int length_,
                    int pool_index_,
                    unsigned hash_address_) : Symbol(INPUT_FILE, name_, length_, pool_index_, hash_address_)
    {
        buffer = NULL;
        buffer_start = NULL;
        buffer_length = 0;
        locked = false;
    }

    virtual ~InputFileSymbol()
    {
        delete [] buffer;
    }

    void Lock()     { locked = true; }
    void Unlock()   { locked = false; }
    bool IsLocked() { return locked; }
    void ResetInput(char* _buffer, int len);
    void ReadInput();

    Tuple<unsigned> &LineLocation() { return line_location; }
    Tuple<unsigned> *LineLocationReference() { return &line_location; }
    unsigned LineLocation(int index) { return line_location[index]; }
    char *Buffer() { return buffer_start; }
    int BufferLength() { return buffer_length; }

private:
    Tuple<unsigned> line_location;
    bool locked;
    char *buffer,
         *buffer_start;
    int buffer_length;

    //
    // If the input file is in Utf8 format, then the first three
    // characters are a marker that indicates this.
    //
    int ByteOrderMarkSize(const char *cursor)
    {
        return (cursor[0] == (char) 0xEF &&
                cursor[1] == (char) 0xBB &&
                cursor[2] == (char) 0xBF
                           ? 3
                           : 0);
    }
};


class ActionFileSymbol : public Symbol
{
public:
    ActionFileSymbol(const char *name_,
                     int length_,
                     int pool_index_,
                     unsigned hash_address_) : Symbol(ACTION_FILE, name_, length_, pool_index_, hash_address_),
                                               block(NULL)
    {
        file = fopen(name_, "wb");
        if ((! file) && strlen(name) > 0)
        {
             std::cout << "Unable to open file \"" << name_ << "\"\n";
             std::cout.flush();
             throw 12;
        }
    }

    virtual ~ActionFileSymbol() {}

    void Flush();

    void SetBlock(BlockSymbol *block_)
    {
        block = block_;
        return;
    }
    BlockSymbol *Block() { return block; }

    TextBuffer *InitialHeadersBuffer()  { return &initial_headers; }
    TextBuffer *BodyBuffer()  { return &body; }
    TextBuffer *FinalTrailersBuffer()  { return &final_trailers; }

    TextBuffer &GetNextHeaderBuffer()  { return headers.Next(); }
    TextBuffer &GetNextTrailerBuffer() { return trailers.Next(); }
    TextBuffer* BufferForTypeScriptNestAst() { return &bufferForTypeScriptNestAst; }
private:
    BlockSymbol *block;

    TextBuffer initial_headers,
               body,
               final_trailers, bufferForTypeScriptNestAst;

    Tuple<TextBuffer> headers,
                      trailers;

    FILE *file;
};


class BlockSymbol : public Symbol
{
public:
    enum 
    {
        MAIN_BLOCK,
        HEADER_BLOCK,
        TRAILER_BLOCK
    };

    BlockSymbol(const char *block_begin_,
                int block_begin_length_,
                int pool_index_,
                unsigned hash_address_) : Symbol(BLOCK, block_begin_, block_begin_length_, pool_index_, hash_address_),
                                          block_kind(MAIN_BLOCK),
                                          block_end(NULL),
                                          block_end_length(0),
                                          buffer(NULL),
                                          filename_symbol(NULL)
    {
    }

    virtual ~BlockSymbol();

    char *BlockBegin() { return name; }
    int BlockBeginLength() { return length; }

 
    void SetBlockKind(int kind_) { block_kind = kind_; }
    int BlockKind() const { return block_kind; }

    void SetBlockEnd(const char *block_end_, int block_end_length_)
    {
        block_end_length = block_end_length_;

        block_end = new char[block_end_length + 1];
        memmove(block_end, block_end_, block_end_length * sizeof(char));
        block_end[block_end_length] = Code::NULL_CHAR;

        return;
    }
    char *BlockEnd() { return block_end; }
    int BlockEndLength() { return block_end_length; }

    void SetActionfileSymbol(ActionFileSymbol *filename_symbol_)
    {
        filename_symbol = filename_symbol_;
    }
    ActionFileSymbol *ActionfileSymbol()
    {
        return filename_symbol;
    }

    void SetBuffer(TextBuffer *buffer_)
    {
        buffer = buffer_;
    }
    TextBuffer *Buffer() { return buffer; }


private:
  
    int block_kind;
    char *block_end;
    int block_end_length;
    TextBuffer *buffer;
    ActionFileSymbol *filename_symbol;
};


class HashPrimes
{
public:
    enum
    {
        DEFAULT_HASH_SIZE = 4093,
        MAX_HASH_SIZE = 32771
    };

    static int primes[];
    int prime_index;

    HashPrimes() : prime_index(0)
    {}
};


class Hash
{
public:
    //
    // Same as above function for a regular "char" string.
    //
    inline static unsigned Function(const char *head, int len)
    {
        unsigned hash_value = head[len >> 1]; // start with center (or unique) letter
        const char *tail = &head[len - 1];

        for (int i = 0; i < 5 && head < tail; i++)
        {
            unsigned k = *tail--;
            hash_value += ((k << 7) + *head++);
        }

        return hash_value;
    }
};


template <class SubSymbol>
    class LookupTable : public HashPrimes
    {
    public:
        LookupTable(int estimate = 16384);
        ~LookupTable();

        SubSymbol *FindOrInsertName(const char *, size_t);
        SubSymbol *InsertName(const char *, size_t);
        SubSymbol *FindName(const char *, size_t);

        int Size() { return symbol_pool.Length(); }

        SubSymbol *Element(const int i) { return symbol_pool[i]; }
        SubSymbol *operator[](const int i) { return Element(i); }

        void Push() { stack.Push(symbol_pool.Length()); }

        void Pop()
        {
           int previous_size = stack.Pop();
           //
           // First, remove all the elements from the hash table;
           //
           for (int i = symbol_pool.Length() - 1; i >= previous_size; i--)
           {
               SubSymbol *symbol = symbol_pool[i];
               int k = symbol -> HashAddress() % hash_size;
               assert(base[k] == symbol);
               base[k] = (SubSymbol *) symbol -> next;
               delete symbol;
           }
           //
           // Then, remove the elements from the pool.
           //
           symbol_pool.Resize(previous_size);
        }

        void Reset()
        {
            while(! stack.IsEmpty())
                Pop();
            if (symbol_pool.Length() > 0)
            {
                stack.Push(0);
                Pop();
            }
        }

    private:

        Tuple<SubSymbol *> symbol_pool;
        Array<SubSymbol *> base;
        int hash_size;

        Stack<int> stack;

        inline static unsigned Hash(const char *head, int len) { return Hash::Function(head, len); }

        void Rehash();
    };

template <class SubSymbol>
    LookupTable<SubSymbol>::LookupTable(int estimate) : symbol_pool(estimate),
                                                        hash_size(primes[prime_index])
    {
        base.Resize(hash_size);
        base.MemReset();
    }

template <class SubSymbol>
    LookupTable<SubSymbol>::~LookupTable()
    {
        for (int i = 0; i < symbol_pool.Length(); i++)
            delete symbol_pool[i];
    }


template <class SubSymbol>
    void LookupTable<SubSymbol>::Rehash()
    {
        hash_size = primes[++prime_index];
        base.Resize(hash_size);
        base.MemReset();
        for (int i = 0; i < symbol_pool.Length(); i++)
        {
            SubSymbol *ns = symbol_pool[i];
            int k = ns -> HashAddress() % hash_size;
            ns -> next = base[k];
            base[k] = ns;
        }

        return;
    }


template <class SubSymbol>
    SubSymbol *LookupTable<SubSymbol>::FindOrInsertName(const char *str, size_t len)
    {
        unsigned hash_address = Hash(str, len);
        int k = hash_address % hash_size;
        SubSymbol *symbol;
        for (symbol = base[k]; symbol; symbol = (SubSymbol *) symbol -> next)
        {
            if (len == symbol -> NameLength() && memcmp(symbol -> Name(), str, len * sizeof(char)) == 0)
                return symbol;
        }

        symbol = new SubSymbol(str, len, symbol_pool.Length(), hash_address);
        symbol_pool.Next() = symbol;

        symbol -> next = base[k];
        base[k] = symbol;

        //
        // If the number of unique elements in the hash table exceeds 2 times
        // the size of the base, and we have not yet reached the maximum
        // allowable size for a base, reallocate a larger base and rehash
        // the elements.
        //
        if ((symbol_pool.Length() > (hash_size << 1)) && (hash_size < MAX_HASH_SIZE))
            Rehash();

        return symbol;
    }

template <class SubSymbol>
    SubSymbol *LookupTable<SubSymbol>::FindName(const char *str, size_t len)
    {
        unsigned hash_address = Hash(str, len);
        int k = hash_address % hash_size;
        SubSymbol *symbol;
        for (symbol = base[k]; symbol; symbol = (SubSymbol *) symbol -> next)
        {
            if (len == symbol -> NameLength() && memcmp(symbol -> Name(), str, len * sizeof(char)) == 0)
                return symbol;
        }

        return NULL;
    }

template <class SubSymbol>
    SubSymbol *LookupTable<SubSymbol>::InsertName(const char *str, size_t len)
    {
        unsigned hash_address = Hash(str, len);
        int k = hash_address % hash_size;
        SubSymbol *symbol = new SubSymbol(str, len, symbol_pool.Length(), hash_address);
        symbol_pool.Next() = symbol;

        symbol -> next = base[k];
        base[k] = symbol;

        //
        // If the number of unique elements in the hash table exceeds 2 times
        // the size of the base, and we have not yet reached the maximum
        // allowable size for a base, reallocate a larger base and rehash
        // the elements.
        //
        if ((symbol_pool.Length() > (hash_size << 1)) && (hash_size < MAX_HASH_SIZE))
            Rehash();

        return symbol;
    }

typedef LookupTable<Symbol> SymbolLookupTable;
typedef LookupTable<VariableSymbol> VariableLookupTable;
typedef LookupTable<RuleSymbol> RuleLookupTable;
typedef LookupTable<MacroSymbol> MacroLookupTable;
typedef LookupTable<SimpleMacroSymbol> SimpleMacroLookupTable;
typedef LookupTable<BlockSymbol> BlockLookupTable;
typedef LookupTable<InputFileSymbol> InputFileLookupTable;
typedef LookupTable<ActionFileSymbol> ActionFileLookupTable;
#endif
