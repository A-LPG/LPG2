#include "LexStream.h"
#include "control.h"
#include "code.h"
#include "tab.h"

#include <sys/stat.h>
#include <iostream>
using namespace std;

InputFileSymbol *LexStream::FindOrInsertFile(Tuple<const char *> &search_directory, const char *filename)
{
    //
    // Look in the search directory path for the filename.
    //
    char *full_filename = NULL;
    int filename_length = strlen(filename);
    for (int i = 0; i < search_directory.Length(); i++)
    {
        const char *directory_name = search_directory[i];
        int directory_name_length = strlen(directory_name),
            length = directory_name_length + filename_length;
        full_filename = new char[length + 2];
        strcpy(full_filename, directory_name);
        int index = directory_name_length;
        if (directory_name_length > 0 &&
            directory_name[directory_name_length - 1] != '\\' &&
            directory_name[directory_name_length - 1] != '/')
            full_filename[index++] = '/';
        if (*filename == '<') // remove brackets from bracketed symbols: <...>
        {
            strncpy(&(full_filename[index]), filename + 1, filename_length - 2);
            full_filename[index + (filename_length - 2)] = '\0';
        }
        else strcpy(&(full_filename[index]), filename);

        struct stat status;
        if (stat(full_filename, &status) == 0)
            break;
    	
        delete [] full_filename;
        full_filename = NULL;
    }

    InputFileSymbol *file_symbol = NULL;
    if (full_filename != NULL)
    {
        //
        // Turn all backslashes into forward slashes in filename.
        //
        for (char *s = full_filename; *s != '\0'; s++)
        {
            if (*s == '\\')
                *s = '/';
        }

        //
        // No need to normalize full_filename as if two separate
        // names are used to access the same file, eventually, we
        // will have to revisit one of the names again. Therefore,
        // at worst, it may take a bit longer to detect the loop,
        // but sooner or later, it will be detected.
        //
        file_symbol = file_table.FindOrInsertName(full_filename, strlen(full_filename));
    }

    delete [] full_filename;

    return file_symbol;
} 
