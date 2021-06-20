#include "tab.h"
#include "code.h"

int Tab::tab_size = Tab::DEFAULT_TAB_SIZE;

//
// Compute the length of a character string segment after expanding tabs.
//
int Tab::strlen(char *line, int start, int end)
{
    for (int i = start--; i <= end; i++)
    {
        if (line[i] == Code::HORIZONTAL_TAB)
        {
            int offset = (i - start) - 1;
            start -= ((tab_size - 1) - offset % tab_size);
        }
    }

    return (end - start);
}
