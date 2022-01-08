#ifndef blocks_INCLUDED
#define blocks_INCLUDED

#include "tuple.h"
#include "symbol.h"

class Option;

class Blocks
{
public:

    Blocks() : block_table(13),
                    filename_table(13)
    {
        block_map.Resize(256); // max 256 8-bit characters
    }

    BlockSymbol *InsertBlock(Token *token_location,
                             int kind,
                             ActionFileSymbol *filename_symbol,
                             const char *block_begin,
                             int block_begin_length,
                             const char *block_end,
                             int block_end_length)
    {
        BlockSymbol *block = block_table.InsertName(block_begin, block_begin_length);
        block -> SetLocation(token_location);
        block -> SetBlockKind(kind);
        block -> SetBlockEnd(block_end, block_end_length);
        block -> SetActionfileSymbol(filename_symbol);
        if (kind == BlockSymbol::MAIN_BLOCK)
            filename_symbol -> SetBlock(block);
        else if (kind == BlockSymbol::HEADER_BLOCK)
        {
            TextBuffer &buffer = filename_symbol -> GetNextHeaderBuffer();
            block -> SetBuffer(&buffer);
        }
        else
        {
            assert (kind == BlockSymbol::TRAILER_BLOCK);
            TextBuffer &buffer = filename_symbol -> GetNextTrailerBuffer();
            block -> SetBuffer(&buffer);
        }

        int c = *(block -> BlockBegin()); // let c be first letter in block-begin string
        block_map[c].Next() = block;

        return block;
    }

    BlockSymbol *FindBlockname(const char *block_begin, int block_begin_length)
    {
	    return block_table.FindName(block_begin, block_begin_length);
    }

    BlockSymbol *FindOrInsertIgnoredBlock(const char *block_begin, int block_begin_length)
    {
        return ignored_table.FindOrInsertName(block_begin, block_begin_length);
    }
    bool IsIgnoredBlock(const char *block_begin, int block_begin_length)
    {
        return (ignored_table.FindName(block_begin, block_begin_length) != NULL);
    }

    ActionFileSymbol *FindFilename(const char *name, int length) { return filename_table.FindName(name, length); }

    ActionFileSymbol *FindOrInsertFilename(const char *name, int length) { return filename_table.FindOrInsertName(name, length); }

    Tuple<BlockSymbol *> &ActionBlocks(int c)
    {
        return block_map[c];
    }

    int NumActionBlocks()
    {
        return block_table.Size();
    }

    BlockSymbol *operator[](const int i)
    {
        assert(i >= 0 && i < NumActionBlocks());

        return block_table[i] -> BlockCast();
    }

    void PutNotice(TextBuffer &notice_buffer)
    {
        for (int i = 0; i < filename_table.Size(); i++)
            filename_table[i] -> InitialHeadersBuffer() -> Put(notice_buffer);
    }

    void Flush()
    {
        for (int i = 0; i < filename_table.Size(); i++)
            filename_table[i] -> Flush();
    }

private:

    Array< Tuple<BlockSymbol*> > block_map;

    BlockLookupTable block_table,
                     ignored_table;
    ActionFileLookupTable filename_table;
};

#endif
