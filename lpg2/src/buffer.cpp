#include "util.h"
#include "buffer.h"

const char UnbufferedTextFile::digits[] = "0123456789ABCDEF";

//
// This Put takes as arguments an integer NUM. NUM is an integer containing at
// most 10 digits which is converted into a character string and placed in
// the buffer.  Leading zeros are eliminated and if the number is negative,
// a leading "-" is added.
//
void UnbufferedTextFile::Put(int num)
{
    BufferCheck(12);
    int  val = Util::Abs(num);
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
    memmove(output_ptr, p, size * sizeof(char));
    output_ptr += size;

    return;
}


//
// This Put takes as arguments two integers: NUM and SIZE. NUM is an integer
// containing at most SIZE digits which is converted into a character strinc
// and placed in the buffer. Leading zeros are replaced by blanks and if
// the number is negative, a leading "-" is added.
//
void UnbufferedTextFile::Put(int num, int size)
{
    BufferCheck(size);
    int val = Util::Abs(num);
    char *p = output_ptr + size;
    do
    {
        p--;
        *p = digits[val % 10];
        val /= 10;
    } while(val > 0 && p > output_ptr);

    if (num < 0 && p > output_ptr)
    {
        p--;
        *p = '-';
    }

    while(p > output_ptr)
    {
        p--;
        *p = ' ';
    }

    output_ptr += size;

    return;
}


//
// Field(int, int) is identical to Put(int, int) except that leading
// zeroes are used instead of blanks.
//
void UnbufferedTextFile::Field(int num, int size)
{
    BufferCheck(size);
    int val = Util::Abs(num);
    char *p = output_ptr + size;
    do
    {
        p--;
        *p = digits[val % 10];
        val /= 10;
    } while(val > 0 && p > output_ptr);

    if (p > output_ptr)
    {
        for (p--; p > output_ptr; p--)
            *p = ' ';
        *p = (num < 0 ? '-' : '0');
    }

    output_ptr += size;

    return;
}


//
// Field(char *str, int size) outputs a string in a field of length size.
// If the length of str is less than size then the field is filled with blanks.
// If, on the other hand, the length of str is greater than size then the
// prefix of str of length size is output.
//
void UnbufferedTextFile::Field(const char *str, int size)
{
    int length = strlen(str);
    if (length > size)
         Put(str, size);
    else
    {
        Put(str);
        for (int i = length; i < size; i++)
            PutChar(' ');
    }

    return;
}




