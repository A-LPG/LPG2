#ifndef tab_INCLUDED
#define tab_INCLUDED

#include <string.h>

class Tab
{
public:
    enum { DEFAULT_TAB_SIZE = 8 };

    inline static int TabSize() { return tab_size; }
    inline static void SetTabSize(int value) { tab_size = value; }

    static int strlen(char *line, int start, int end);

private:
    static int tab_size;
};
#endif

