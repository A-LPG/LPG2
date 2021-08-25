#ifndef util_INCLUDED
#define util_INCLUDED

#include <string>

#include "tuple.h"

int SystemMkdir(char *);

class Util
{
public:
  static   std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
        return str;
    }
    static int INFINITY_,
               OMEGA,
               NIL;

    static int Max(int a, int b) { return (a > b ? a : b); }
    static int Min(int a, int b) { return (a < b ? a : b); }

    static short Abs(short x) { return x < 0 ? -x : x; }
    static int Abs(int x) { return x < 0 ? -x : x; }
    static float Abs(float x) { return x < 0 ? -x : x; }
    static double Abs(double x) { return x < 0 ? -x : x; }

    static void QuickSort(Tuple<int> &, int, int);

    //
    // FILL_IN is a function that pads a buffer, STRING,  with CHARACTER a
    // certain AMOUNT of times.
    //
    static void FillIn(char string[], int amount, char character)
    {
        for (int i = 0; i <= amount; i++)
            string[i] = character;
        string[amount + 1] = '\0';

        return;
    }


    //
    // NUMBER_LEN takes a state number and returns the number of digits in that
    // number.
    //
    static int NumberLength(int state_no)
    {
        int num = 0;

        do
        {
            state_no /= 10;
            num++;
        }   while (state_no != 0);

        return num;
    }
};

//
// Convert an integer to its character string representation.
//
class IntToString
{
public:
    IntToString(int num) : value(num)
    {
        if ((unsigned) num == 0x80000000)
        {
            str = info;
            strcpy(str, "-2147483648");
        }
        else
        {
            str = &info[TAIL_INDEX];
            *str = '\0';
            int n = (num < 0 ? -num : num);
            do
            {
                *--str = ('0' + n % 10);
                n /= 10;
            } while (n != 0);

            if (num < 0)
                *--str = '-';
        }
        return;
    }

    int Value() { return value; }
    char *String() { return str; }
    int Length()   { return (&info[TAIL_INDEX]) - str; }

private:
    enum { TAIL_INDEX = 1 + 10 }; // 1 for sign, +10 significant digits
    int value;
    char info[TAIL_INDEX + 1], // +1 for '\0'
         *str;
};

#endif
