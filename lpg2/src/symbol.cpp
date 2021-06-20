#include "symbol.h"

int HashPrimes::primes[] = {DEFAULT_HASH_SIZE, 8191, 16411, MAX_HASH_SIZE};

ReferenceSymbol::ReferenceSymbol(const std::string& _n, int _l, Symbol* _d): name(_n), line_no(_l), define_symbol(_d)
{
	if(name.find("kw_lexer_class") != std::string::npos)
	{
        name = "kw_lexer_class2";
	}
}

void InputFileSymbol::ResetInput(char* _buffer, int len)
{
    delete[] buffer;
    buffer = _buffer;
    buffer_length = len;
}

void
InputFileSymbol::ReadInput()
{
    if (buffer) // file already read in
        return;
    
    char *filename = Name();
    struct stat status;
    stat(filename, &status);
    
    FILE *srcfile = fopen(filename, "rb");
    if (srcfile != NULL)
    {
        buffer = new char[status.st_size + 4];
        size_t file_size = fread(buffer + 1, sizeof(char), status.st_size, srcfile);
        fclose(srcfile);
        int mark_size = ByteOrderMarkSize(buffer + 1);
        if (mark_size > 0)
            strncpy(buffer, buffer + 1, mark_size);
            buffer[mark_size] = '\n';
            buffer_start = buffer + mark_size;
            
            char *source_tail = &(buffer[file_size]); // point to last character read from the file.
            //
            // Remove all trailing spaces
            //
            while((source_tail > buffer_start) && Code::IsSpace(*source_tail))
                source_tail--;
        
        //
        // If the very last character is not CTL_Z then add CTL_Z
        //
        if (*source_tail != Code::CTL_Z)
        {
            if (*source_tail != Code::LINE_FEED)
                *(++source_tail) = Code::LINE_FEED; // if the last character is not end-of-line, add end-of-line
            *(++source_tail) = Code::CTL_Z;         // Mark end-of-file
        }
        *(++source_tail) = Code::NULL_CHAR; // add gate
        
        buffer_length = source_tail - buffer_start;
    }
    
    return;
}

void
ActionFileSymbol::Flush()
{
    if (file) // not the null file?
    {
        initial_headers.Flush(file);
        {
            for (int i = 0; i < headers.Length(); i++)
                headers[i].Flush(file);
                }
        body.Flush(file);
        {
            for (int i = 0; i < trailers.Length(); i++)
                trailers[i].Flush(file);
                }
        final_trailers.Flush(file);
        
        fprintf(file, "\n");
        fclose (file);
        file = NULL;
    }
    
    return;
}

BlockSymbol::~BlockSymbol()
{
	delete [] block_end;
	for (auto& it : references)
	{
		delete it;
	}
}
