#ifndef spell_INCLUDED
#define spell_INCLUDED

class Spell
{
    static inline int Min(int x, int y) { return (x < y ? x : y); }

public:
    static int Index(const char *str1, const char *str2)
    {
        int len1 = strlen(str1),
            len2 = strlen(str2);

        char *s1 = new char[len1 + 1],
             *s2 = new char[len2 + 1];

        for (int i = 0; i < len1; i++)
            s1[i] = tolower(str1[i]);
        s1[len1] = '\0';

        for (int j = 0; j < len2; j++)
            s2[j] = tolower(str2[j]);
        s2[len2] = '\0';

        if (len1 == 1 && len2 == 1)
        {
            //
            //  Singleton mispellings:
            //
            //  ;      <---->     ,
            //
            //  ;      <---->     :
            //
            //  .      <---->     ,
            //
            //  '      <---->     "
            //
            if ((s1[0] == ';'  &&  s2[0] == ',')  ||
                (s1[0] == ','  &&  s2[0] == ';')  ||
                (s1[0] == ';'  &&  s2[0] == ':')  ||
                (s1[0] == ':'  &&  s2[0] == ';')  ||
                (s1[0] == '.'  &&  s2[0] == ',')  ||
                (s1[0] == ','  &&  s2[0] == '.')  ||
                (s1[0] == '\'' &&  s2[0] == '\"')  ||
                (s1[0] == '\"' &&  s2[0] == '\''))
                    delete [] s1;
                    delete [] s2;

                    return 3;
        }

        //
        // Scan the two strings. Increment "match" count for each match.
        // When a transposition is encountered, increase "match" count
        // by two but count it as an error. When a typo is found, skip
        // it and count it as an error. Otherwise we have a mismatch; if
        // one of the strings is longer, increment its index, otherwise,
        // increment both indices and continue.
        //
        // This algorithm is an adaptation of a boolean misspelling
        // algorithm proposed by Juergen Uhl.
        //
        int count = 0,
            prefix_length = 0,
            num_errors = 0,
            i1 = 0,
            i2 = 0;

        while ((i1 < len1)  &&  (i2 < len2))
        {
            if (s1[i1] == s2[i2])
            {
                count++;
                i1++;
                i2++;
                if (num_errors == 0)
                    prefix_length++;
            }
            else if (s1[i1 + 1] == s2[i2]  &&  s1[i1] == s2[i2 + 1])
            {
                count += 2;
                i1 += 2;
                i2 += 2;
                num_errors++;
            }
            else if (s1[i1 + 1] == s2[i2 + 1])
            {
                i1++;
                i2++;
                num_errors++;
            }
            else
            {
                if ((len1 - i1) > (len2 - i2))
                     i1++;
                else if ((len2 - i2) > (len1 - i1))
                     i2++;
                else
                {
                    i1++;
                    i2++;
                }
                num_errors++;
            }
        }

        if (i1 < len1  ||  i2 < len2)
            num_errors++;

        if (num_errors > (Min(len1, len2) / 6 + 1))
             count = prefix_length;

        delete [] s1;
        delete [] s2;

        return (count * 10 / (len1 + num_errors));
    }
};

#endif // #ifndef spell_INCLUDED
