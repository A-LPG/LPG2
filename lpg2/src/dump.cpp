#ifdef TEST

#include "LexStream.h"
#include "code.h"
#include "jikespg_sym.h"

#include <stdio.h>
#include <iostream>
using namespace std;

void LexStream::Dump()
{
    FILE *tokfile;
    char *tokfile_name = "a.tok";

    if ((tokfile = fopen(tokfile_name, "w")) == NULL)
    {
        cout << "*** Cannot open file " << tokfile_name << "\n";
        return;
    }

    if (NumTokens() == 0)
        return;

    LexStream::TokenIndex tok = 0;
    Reset();
    do
    {
        tok = Gettoken();

        fprintf(tokfile, "%6d ", tok);
        fprintf(tokfile, " %s",FileName(tok));
        fprintf(tokfile, ", %cline %d.%d-%d.%d: %s %s\n",
                         (AfterEol(tok) ? '*' : ' '),
                         Line(tok),
                         Column(tok),
                         EndLine(tok),
                         EndColumn(tok),
                         KeywordName(Kind(tok)),
                         NameString(tok));
    } while (Kind(tok) != TK_EOF);

    fprintf(tokfile, "\n");
    fflush(tokfile);

    if (tokfile)
        fclose(tokfile);

    return;
}

#endif
