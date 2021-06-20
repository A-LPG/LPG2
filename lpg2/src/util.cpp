#include "util.h"
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#if WIN32
#include <direct.h>
    int SystemMkdir(char *dirname)
    {
        //
        // returns 0,
        //         EEXIST (directory already exists)
        //      or ENOENT (directory could not be created)
        //
        struct stat status;
        int rc = (stat(dirname, &status) == 0
                      ? ((status.st_mode & _S_IFDIR) ? 0 : ENOTDIR)
                      : _mkdir(dirname));
        return rc;
    }
#elif defined(_POSIX_SOURCE)
    int SystemMkdir(char *dirname)
    {
        struct stat status;
        int rc = (stat(dirname, &status) == 0
                      ? ((status.st_mode & S_IFDIR) ? 0 : ENOTDIR)
                      : mkdir(dirname, S_IRWXU | S_IRWXG | S_IRWXO));
        return rc;
    }
#else /* ! _POSIX_SOURCE */
    int SystemMkdir(char *dirname)
    {
        struct stat status;
        int rc = (stat(dirname, &status) == 0
                      ? ((status.st_mode & S_IFDIR) ? 0 : ENOTDIR)
                      : mkdir(dirname, S_IRWXU | S_IRWXG | S_IRWXO));
        return rc;
    }
#endif

int Util::INFINITY_ = INT_MAX;
int Util::OMEGA    = INT_MIN;
int Util::NIL      = INT_MIN + 1;

//
// QuickSort is a quicksort algorithm that takes as arguments an array of
// integers, two numbers Low and High that indicate the lower and upper bound
// positions in ARRAY to be sorted.
//
void Util::QuickSort(Tuple<int> &array, int low, int high)
{
    Stack<int> lostack,
               histack;

    lostack.Push(low);
    histack.Push(high);
    while (! lostack.IsEmpty())
    {
        int lower = lostack.Pop(),
            upper = histack.Pop();

        while (upper > lower)
        {
            int i = lower,
                pivot = array[lower];
            for (int j = lower + 1; j <= upper; j++)
            {
                if (array[j] < pivot)
                {
                    array[i] = array[j];
                    i++;
                    array[j] = array[i];
                }
            }
            array[i] = pivot;

            if (i - lower < upper - i)
            {
                lostack.Push(i + 1);
                histack.Push(upper);
                upper = i - 1;
            }
            else
            {
                histack.Push(i - 1);
                lostack.Push(lower);
                lower = i + 1;
            }
        }
    }

    return;
}
