#ifndef buffer_INCLUDED
#define buffer_INCLUDED

#include <stdio.h>
#include <assert.h>
#include <string.h>


//
//
//
class TextBuffer
{
    enum {
             BUFFER_SIZE = 65536
         };

    Tuple <char *> buffers;
    char *output_ptr,
         *output_tail;

public:

    TextBuffer() : output_ptr(NULL),
                   output_tail(NULL)
    {}

    ~TextBuffer()
    {
        for (int i = 0; i < buffers.Length(); i++)
            delete [] buffers[i];
        buffers.Reset();
    }

    //
    // Write whatever information that is in the buffers out to file.
    //
    inline void Print(FILE *file)
    {
        if (buffers.Length() > 0)
        {
            for (int i = 0; i < buffers.Length() - 1; i++)
                fwrite(buffers[i], sizeof(char), BUFFER_SIZE, file);
            if (output_ptr != NULL)
            {
                char *output_buffer = buffers[buffers.Length() - 1];
                fwrite(output_buffer, sizeof(char), output_ptr - output_buffer, file);
            }
            fflush(file);
        }

        return;
    }

    //
    // Write whatever information that is in the input buffer out to this buffer.
    //
    inline void Put(TextBuffer &input_text)
    {
        if (input_text.buffers.Length() > 0)
        {
            for (int i = 0; i < input_text.buffers.Length() - 1; i++)
                this -> Put(input_text.buffers[i], BUFFER_SIZE);
            if (input_text.output_ptr != NULL)
            {
                char *output_buffer = input_text.buffers[input_text.buffers.Length() - 1];
                this -> Put(output_buffer, input_text.output_ptr - output_buffer);
            }
        }

        return;
    }

    //
    // Write whatever information that is in the buffers out to file
    // and reset the buffers.
    //
    inline void Flush(FILE *file)
    {
        Print(file);
        for (int i = 0; i < buffers.Length(); i++)
            delete [] buffers[i];
        buffers.Reset();
        output_ptr = NULL;
        output_tail = NULL;
    }

    //
    // Put specified value as a character in the buffer
    //
    inline void PutChar(int c)
    {
     //   printf("%c", c);
        if (output_ptr == output_tail)
        {
            output_ptr = new char[BUFFER_SIZE];
            output_tail = &output_ptr[BUFFER_SIZE];
            buffers.Next() = output_ptr;
        }
        *output_ptr++ = c;

        return;
    }

    //
    // Put null-terminated string in the buffer
    //
    inline void Put(const char *str)
    {
        for (const char *p = str; *p; p++)
            PutChar(*p);
        return;
    }

    //
    // Put prefix of str with specified size in the buffer
    //
    inline void Put(const char *str, int size)
    {
        for (int i = 0; i < size; i++)
            PutChar(str[i]);
        return;
    }

    //
    // Put null-terminated string literal in the buffer
    //
    inline void PutStringLiteral(const char *str)
    {
        for (const char *p = str; *p; p++)
        {
            switch(*p)
            {
                case '\b':
                    PutChar('\\');
                    PutChar('b');
                    break;
                case '\t':
                    PutChar('\\');
                    PutChar('t');
                    break;
                case '\n':
                    PutChar('\\');
                    PutChar('n');
                    break;
                case '\f':
                    PutChar('\\');
                    PutChar('f');
                    break;
                case '\r':
                    PutChar('\\');
                    PutChar('r');
                    break;
                case '\\':
                    PutChar('\\');
                    PutChar('\\');
                    break;
                case '\"':
                    PutChar('\\');
                    PutChar('\"');
                    break;
                case '\'':
                    PutChar('\\');
                    PutChar('\'');
                    break;
                default:
                    PutChar(*p);
                    break;
            }
        }
        return;
    }

    //
    // This Put takes as arguments an integer NUM. NUM is an integer containing at
    // most 10 digits which is converted into a character string and placed in
    // the buffer.  Leading zeros are eliminated and if the number is negative,
    // a leading "-" is added.
    //
    void Put(int num)
    {
        const char *digits = "0123456789";
        int  val = (num < 0 ? -num : num);
        char tmp[12],
             *p = &tmp[11];
        *p = '\0';
        do
        {
            p--;
            *p = digits[val % 10];
            val /= 10;
        } while(val > 0);

        if (num < 0)
        {
            p--;
            *p = '-';
        }

        int size = &tmp[11] - p;
        Put(p, size);

        return;
    }
};


//
//
//
class UnbufferedTextFile
{
    enum {
             BUFFER_SIZE = 65536
         };

    static const char digits[];
    char *output_ptr,
         output_buffer[BUFFER_SIZE];

    FILE *&file;

    void BufferCheck(int size = 256)
    {
        assert(size < BUFFER_SIZE);
        if ((BUFFER_SIZE - (output_ptr - &output_buffer[0])) < size)
        {
            fwrite(output_buffer, sizeof(char), output_ptr - &output_buffer[0], file);
            output_ptr = &output_buffer[0];
        }

        return;
    }

public:

    UnbufferedTextFile(FILE **file_) : file(*file_)
    {
        output_ptr = &output_buffer[0];
    }

    ~UnbufferedTextFile()
    {
	if (file != NULL) {
	    Flush();
	}
    }

    //
    // Write whatever information that is in the buffer out to file.
    //
    inline void Flush()
    {
        BufferCheck(BUFFER_SIZE - 1);
        fflush(file);
    }

    //
    // Dump the content of the text buffer into this file.
    //
    inline void Put(TextBuffer &input_text)
    {
        Flush(); // first write all outstanding buffered info.
        input_text.Print(file);
    }

    //
    // Put null-terminated string in the buffer
    //
    inline void Put(const char *str)
    {
      
        int size = strlen(str);
        BufferCheck(size);
        memmove(output_ptr, str, size * sizeof(char));
        output_ptr += size;

        return;
    }


    //
    // Put prefix of str with specified size in the buffer
    //
    inline void Put(const char *str, int size)
    {
        BufferCheck(size);
        memmove(output_ptr, str, size * sizeof(char));
        output_ptr += size;

        return;
    }


    //
    // Put specified character in the buffer
    //
    inline void Put(const char c)
    {
        BufferCheck();
        *output_ptr++ = c;

        return;
    }


    //
    // Put specified value as a character in the buffer
    //
    inline void PutChar(int c)
    {
        BufferCheck();
        *output_ptr++ = c;

        return;
    }


    //
    // Put hex representation of specified character in the buffer
    //
    inline void PutHex(const char c)
    {
        int num = c & 0x000000FF;
        PutChar(digits[num >> 4]);
        PutChar(digits[num & 0x0000000F]);

        return;
    }


    //
    // Remove the last character from the buffer
    //
    inline void UnputChar()
    {
        assert(output_ptr > &output_buffer[0]); // make sure there is something to unput...
        output_ptr--;
    }


    //
    // Put null-terminated string literal in the buffer
    //
    inline void PutStringLiteral(const char *str)
    {
        for (const char *p = str; *p; p++)
        {
            switch(*p)
            {
                case '\b':
                    PutChar('\\');
                    PutChar('b');
                    break;
                case '\t':
                    PutChar('\\');
                    PutChar('t');
                    break;
                case '\n':
                    PutChar('\\');
                    PutChar('n');
                    break;
                case '\f':
                    PutChar('\\');
                    PutChar('f');
                    break;
                case '\r':
                    PutChar('\\');
                    PutChar('r');
                    break;
                case '\\':
                    PutChar('\\');
                    PutChar('\\');
                    break;
                case '\"':
                    PutChar('\\');
                    PutChar('\"');
                    break;
                case '\'':
                    PutChar('\\');
                    PutChar('\'');
                    break;
                default:
                    PutChar(*p);
                    break;
            }
        }
        return;
    }


    //
    // Pad the buffer with specified number of blanks. The default is 12.
    //
    inline void Pad(int size = 12)
    {
        BufferCheck(size);
        memset(output_ptr, ' ', size * sizeof(char));
        output_ptr += size;

        return;
    }

    void Put(int);
    void Put(int, int);
    void Field(int, int);
    void Field(const char *, int);
};

//
//
//
class UnbufferedBinaryFile
{
    enum {
             BUFFER_SIZE = 65536
         };

    char *output_ptr,
         output_buffer[BUFFER_SIZE];

    FILE *&file;

public:

    UnbufferedBinaryFile(FILE **file_) : file(*file_)
    {
        output_ptr = &output_buffer[0];
    }

    ~UnbufferedBinaryFile()
    {
        Flush();
    }

    //
    // Write whatever information that is in the buffer out to file.
    //
    inline void Flush()
    {
        if (output_ptr != &output_buffer[0])
        {
            fwrite(output_buffer, sizeof(char), output_ptr - &output_buffer[0], file);
            output_ptr = &output_buffer[0];
            fflush(file);
        }
  
        return;
    }

    //
    // Put null-terminated string in the buffer
    //
    inline void Put(const unsigned char c)
    {
        *output_ptr++ = c;
        if (output_ptr == &output_buffer[BUFFER_SIZE])
            Flush();
    }
};

#endif

