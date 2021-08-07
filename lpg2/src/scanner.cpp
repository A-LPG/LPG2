#include <sys/stat.h>
#include "control.h"
#include "scanner.h"
#include "parser.h"
#include "jikespg_sym.h"

int (*Scanner::scan_keyword[SCAN_KEYWORD_SIZE]) (char *p1) =
{
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword4,
    ScanKeyword0,
    ScanKeyword6,
    ScanKeyword7,
    ScanKeyword8,
    ScanKeyword9,
    ScanKeyword10,
    ScanKeyword11,
    ScanKeyword12,
    ScanKeyword13,
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword0,
    ScanKeyword24
};

void Scanner::Setup()
{
    //
    // If this assertion fails, the Token structure in stream.h must be redesigned !!!
    //
    assert(Parser::NUM_TERMINALS < 64);

    for (int c = 0; c < 256; c++)
        classify_token[c] = (IsSpace(c) ? &Scanner::ClassifyBadToken : &Scanner::ClassifySymbol);

    classify_token[(int) NULL_CHAR] = &Scanner::ClassifyEof;
    classify_token[(int) CTL_Z] = &Scanner::ClassifyEof;

    classify_token[option->lpg_escape] = &Scanner::ClassifyMacroNameSymbol;
    classify_token[(int) option -> or_marker] = &Scanner::ClassifyOr;

    classify_token[(int) '%']  = &Scanner::ClassifyKeyword;
    classify_token[(int) ':']  = &Scanner::ClassifyEquivalence;
    classify_token[(int) '-']  = &Scanner::ClassifyArrow;
    classify_token[(int) '\''] = &Scanner::ClassifySingleQuotedSymbol;
    classify_token[(int) '\"'] = &Scanner::ClassifyDoubleQuotedSymbol;
    classify_token[(int) '<']  = &Scanner::ClassifyLess;
}




//
//
//
void Scanner::ReportErrors()
{
    Tuple <const char *> msg;
    char escape[2] = { option -> escape, '\0' };

    for (int i = 0; i < warning_tokens.Length(); i++)
    {
        msg.Reset();
        Token *token = warning_tokens[i].token;
        const char *name = warning_tokens[i].name;
        assert(name);
        switch(warning_tokens[i].msg_code)
        {
            case LEGACY_KEYWORD:
                 msg.Next() = "The use of the escaped symbol \"";
                 msg.Next() = escape;
                 msg.Next() = name + 1; // lex_stream -> NameString(token_index) + 1;
                 msg.Next() = "\" as a keyword has been deprecated. The proper spelling is %";
                 msg.Next() = name + 1; // lex_stream -> NameString(token_index) + 1;
                 break;
            case SYMBOL_WITH_KEYWORD_MARKER:
                 msg.Next() = "The symbol \"";
                 msg.Next() = name; // lex_stream -> NameString(token_index);
                 msg.Next() = "\" starts with the keyword marker %. It should be quoted.";
                 break;
            case INCLUDE_OPTIONS:
                 msg.Next() = "Some options may have been imported from the included file \"";
                 msg.Next() = name;
                 msg.Next() = "\"";
                 break;
            default:
                 assert(false);
        }

        option -> EmitWarning(token, msg);
    }

    if (error_tokens.Length() > 0)
    {
        for (int i = 0; i < error_tokens.Length(); i++)
        {
            msg.Reset();
            Token *token = error_tokens[i].token;
            const char *name = error_tokens[i].name;
            switch(error_tokens[i].msg_code)
            {
                case NO_INPUT:
                     msg.Next() = "No input file specified";
                     break;
                case NO_TEMPLATE:
                     msg.Next() = "The template file ";
                     msg.Next() = option -> template_name;
                     msg.Next() = " could not be read.";
                     break;
                case NO_INCLUDE:
                     msg.Next() = "The include file ";
                     msg.Next() = name; // lex_stream -> NameString(token_index);
                     msg.Next() = " could not be read.";
                     break;
                case RECURSIVE_INCLUDE:
                     msg.Next() = "Attempt to recursively include file ";
                     msg.Next() = name; // lex_stream -> NameString(token_index);
                     break;
                case BAD_UNICODE:
                     msg.Next() = "Symbol \"";
                     msg.Next() = name; // lex_stream -> NameString(token_index);
                     msg.Next() = "\" contains an invalid unicode character.";
                     break;
                case BAD_OCTAL_ASCII_CODE:
                     msg.Next() = "Symbol \"";
                     msg.Next() = name; // lex_stream -> NameString(token_index);
                     msg.Next() = "\" contains an invalid ASCII octal code.";
                     break;
                case ISOLATED_BACKSLASH:
                     msg.Next() = "Symbol \"";
                     msg.Next() = name; // lex_stream -> NameString(token_index);
                     msg.Next() = "\" contains an isolated backslash.";
                     break;
                case UNDELIMITED_STRING_SYMBOL:
                     msg.Next() = "Quoted symbol not properly delimited. The closing quote must be followed by a space";
                     break;
                case UNTERMINATED_STRING_SYMBOL:
                     msg.Next() = "Quoted symbol not properly terminated";
                     break;
                case UNTERMINATED_BRACKET_SYMBOL:
                     msg.Next() = "Symbol enclosed in angular brackets not properly terminated";
                     break;
                case UNTERMINATED_BLOCK:
                     msg.Next() = "Block not properly terminated";
                     break;
                default:
                     assert(false);
            }
            option -> EmitError(token, msg);
        }

        throw 12;
    }

    return;
}


//
// Scan the main file
//
void Scanner::Scan()
{
    lex_stream -> ResetTokenStream();

    input_file = lex_stream -> FindOrInsertFile(option -> grm_file);
    if (! input_file)
    {
        option -> report.Put("***Error: Input file \"");
        option -> report.Put(option -> grm_file);
        option -> report.Put("\" could not be read");
        option -> report.Flush(stdout);
    }
    else
    {
        input_file -> Lock();
        input_file -> ReadInput();
        if (input_file -> Buffer() == NULL)
        {
            option -> report.Put("***Error: Input file \"");
            option -> report.Put(option -> grm_file);
            option -> report.Put("\" could not be read");
            option -> report.Flush(stdout);
            return;
        }

        current_token_index = lex_stream -> GetNextToken(input_file, 0); // Get 0th token and set its location to 0!
        current_token = lex_stream -> GetTokenReference(current_token_index);
        current_token -> SetKind(0);

        //
        // Assign the null string symbol to the 0th token.
        //
        current_token -> SetSymbol(variable_table -> FindOrInsertName(input_file -> Buffer(), 0));
        current_token -> SetEndLocation(0);

        //
        // Process all options if any that was specified by the user or set
        // defaults otherwise. First, we process the options that were hard-wired
        // in the input. Then, we process any option that was issued from the
        // command line.
        //
        char *main_cursor = ScanOptions(),
             *template_cursor = NULL;

        option -> ProcessCommandOptions(); // process options from command line

        //
        // If we need to include a template, load its environment and
        // process its options.
        //
        InputFileSymbol *template_file = NULL;
        if (option -> template_name)
        {
            //
            // First, look in the template search directory path for the
            // template file name.
            //
            template_file = lex_stream -> FindOrInsertFile(option -> template_search_directory, option -> template_name);

            //
            // If we were not successful in the template search directory path,
            // look in the include search directory path.
            //
            if (template_file == NULL)
                template_file = lex_stream -> FindOrInsertFile(option -> include_search_directory, option -> template_name);
        	
            if (template_file == NULL)
                template_file = lex_stream->FindOrInsertFile( option->template_name);

            //
            // If we could not open the template file, issue an error.
            //
            if (template_file == NULL)
                AddErrorToken(NO_TEMPLATE, current_token_index);
            else
            {
                template_file -> Lock();
                template_file -> ReadInput();
                if (template_file -> Buffer() == NULL)
                {
                    Tuple <const char *> msg;

                    msg.Next() = "Input file \"";
                    msg.Next() = option -> template_name;
                    msg.Next() = "\" could not be read";
                    option -> EmitError(0, msg);

                }
                else
                {
                    PushEnvironment();
                    input_file = template_file;
                    template_cursor = ScanOptions(); // scan the template options
                }
 
            }
        }

        option -> CompleteOptionProcessing();
        if (option -> return_code != 0) // if any bad option was found, stop
        {
            option -> report.Flush(stdout);
            throw option -> return_code;
        }
        option -> PrintOptionsInEffect();

        Setup();

        //
        // If a template is present, continue scanning it...
        //
        if (template_file)
        {
            Scan(template_cursor, &(input_buffer[template_file -> BufferLength()]));
            while(lex_stream -> Kind(current_token_index) == TK_EOF)
                current_token_index--;
            lex_stream -> ResetTokenStream(current_token_index + 1);
            template_file -> Unlock(); // unlock the template file.
            PopEnvironment();
        }

        //
        // Restore the main file environment and continue scanning 
        //
        Scan(main_cursor, &(input_buffer[input_file -> BufferLength()]));

        {
            for (int k = 0; k < this -> option -> import_file.Length(); k++)
                ImportTerminals(option -> import_file[k]);
        }

        {
            for (int k = 0; k < this -> option -> filter_file.Length(); k++)
                ProcessFilters(option -> filter_file[k]);
        }
    }

    //
    // If any problem tokens were scanned, flag them.
    //
    ReportErrors();

    return;
}


//
// Scan the file specified
//
void Scanner::Scan(int file_index)
{
    Setup();

    input_file = lex_stream -> FindOrInsertFile(option -> include_search_directory, lex_stream -> NameString(file_index));

    int error_code = IncludeFile();
    if (error_code != NO_ERROR)
        AddErrorToken(error_code, file_index);
    current_token_index = lex_stream -> GetNextToken(input_file, cursor - input_buffer);
    current_token = lex_stream -> GetTokenReference(current_token_index);
    current_token -> SetKind(TK_EOF);

    //
    // If any problem tokens were scanned, flag them.
    //
    ReportErrors();

    return;
}


//
// This is one of the main entry point for the lexical analyser.
// Its input is the name of a regular text file. Its output is a stream
// of tokens.
//
void Scanner::Scan(char *buffer_start, char *buffer_tail)
{
    cursor = buffer_start;

    //
    // CURSOR is assumed to point to the next character to be scanned.
    // Using CURSOR,we jump to the proper classification function
    // which scans and classifies the token and returns the location of
    // the character immediately following it.
    //
    do
    {
        SkipSpaces();

        //
        // Allocate space for next token and set its location.
        //
        current_token_index = lex_stream -> GetNextToken(input_file, cursor - input_buffer);
        current_token = lex_stream -> GetTokenReference(current_token_index);
        Tuple<BlockSymbol *> &blocks = action_blocks -> ActionBlocks((unsigned char) *cursor);
        if (blocks.Length() > 0)
             ClassifyBlock(blocks);
        else (this ->* classify_token[(unsigned char) *cursor])();
    } while (cursor < buffer_tail);

    //
    // Add a gate after the last line.
    //
    line_location -> Next() = buffer_tail - input_buffer;

    return;
}


/*
//
//
//
char *Scanner::ScanOptions(InputFileSymbol *file)
{
    char *start = file -> Buffer();

    cursor = start;
    SkipSpaces();
    if (strxeq(cursor, "%include"))
    {
        AddErrorToken(INCLUDE_OPTIONS, 0);
        return start;
    }

    for (SkipSpaces(); strxeq(cursor, "%options"); SkipSpaces())
    {
        char *start = cursor + strlen("%options"),
             *ptr;
        if (! (IsSpace(*start) || (start[0] == '-' && start[1] == '-')))
            break;
        //
        // Note that we start scanning at "start" in case start points to a
        // new line character or a comment.
        //
        for (ptr = start; ! (IsNewline(*ptr) || (ptr[0] == '-' && ptr[1] == '-')); ptr++)  // skip all until \n
            ;
        option -> ProcessUserOptions(file, start, ptr - start);

        cursor = ptr;
    }

    return cursor;
}
*/


//
//
//
char *Scanner::ScanOptions()
{
    input_buffer = input_file -> Buffer();
    cursor = input_buffer;
    line_location = input_file -> LineLocationReference();
    line_location -> Reset();
    line_location -> Next() = 0; // mark starting location of line # 0

    const char *include = "%include",
               *options = "%options",
               *end     = "%end";
    int include_length = strlen(include),
        options_length = strlen(options),
        end_length     = strlen(end);

    while (true)
    {
        SkipSpaces();

        char *start_cursor = cursor;
        if (strxeq(cursor, include))
        {
            cursor += include_length;
            SkipSpaces();
            char *ptr = cursor + 1;
            while ((! IsSpace(*ptr)) && (*ptr != option ->lpg_escape))
                ptr++;
            char *filename = NewString(ptr - cursor + 1);
            strncpy(filename, cursor, ptr - cursor);
            filename[ptr - cursor] = '\0';
            InputFileSymbol *include_file = lex_stream -> FindOrInsertFile(option -> include_search_directory, filename);
            if (include_file == NULL || include_file -> IsLocked())
            {
                ResetEnvironment(start_cursor);
                return start_cursor; // we need to rescan this INCLUDE file
            }
            Token *error_token = lex_stream -> GetErrorToken(input_file, cursor - input_buffer);
            error_token -> SetEndLocation(ptr - 1 - input_buffer);
            error_token -> SetKind(0);

            AddWarningToken(INCLUDE_OPTIONS, include_file -> Name(), error_token);

            include_file -> ReadInput();
            if (include_file -> Buffer() == NULL)
            {
                ResetEnvironment(start_cursor);
                return start_cursor;
            }
            cursor = ptr; // we will need to continue from where we left off
            PushEnvironment();
            include_file -> Lock();
            input_file = include_file;

            char *return_ptr = ScanOptions();

            include_file -> Unlock(); // unlock the include file.
            PopEnvironment();
            if (return_ptr < &(include_file -> Buffer()[include_file -> BufferLength()])) // did not reach end of include file?
            {
                ResetEnvironment(start_cursor);
                return start_cursor;
            }
            SkipSpaces();
            if (strxeq(cursor, end))
                cursor += end_length;
        }
        else if (strxeq(cursor, options))
        {
            char *start = cursor + options_length,
                 *ptr;
            if (! (IsSpace(*start) || (start[0] == '-' && start[1] == '-')))
                break;
            //
            // Note that we start scanning at "start" in case start points to a
            // new line character or a comment.
            //
            for (ptr = start; ! (IsNewline(*ptr) || (ptr[0] == '-' && ptr[1] == '-')); ptr++)  // skip all until \n or comment
                ;
            option -> ProcessUserOptions(input_file, start, ptr - start);

            cursor = ptr;
        }
        else break;
    }

    return cursor;
}


//
//
//
void Scanner::SkipOptions()
{
    for (SkipSpaces(); strxeq(cursor, "%options"); SkipSpaces())
    {
        char *start = cursor + strlen("%options"),
             *ptr;
        if (! (IsSpace(*start) || (start[0] == '-' && start[1] == '-')))
            break;
        //
        // Note that we start scanning at "start" in case start points to a
        // new line character or a comment.
        //
        for (ptr = start; ! (IsNewline(*ptr) || (ptr[0] == '-' && ptr[1] == '-')); ptr++)  // skip all until \n
            ;
        cursor = ptr;
    }

    return;
}


//
//
//
void Scanner::ImportTerminals(const char *filename)
{
    InputFileSymbol *file_symbol = this -> lex_stream -> FindOrInsertFile(option -> include_search_directory, filename);
    const char *name = (file_symbol == NULL ? filename : file_symbol -> Name());
    ImportArguments arguments(this -> option, name);
    Option option(arguments.argc, arguments.argv);
    LexStream lex_stream(&option);
    VariableLookupTable variable_table;
    MacroLookupTable macro_table;
    Scanner scanner(&option, &lex_stream, &variable_table, &macro_table);
    scanner.Scan();

    if (lex_stream.NumTokens() == 0 || scanner.NumErrorTokens() > 0)
        throw 12;
    else
    {
        //
        // This is the code one would write in order to retrieve only the
        // terminals exported from the imported file without processing the
        // whole grammar.
        //
        //     Parser parser(&option, &lex_stream, &variable_table, &macro_table);
        //     parser.Parse();
        //
        //     //
        //     // Transfer each imported terminal symbol into the local variable
        //     // table and add it to the list of imported symbols.
        //     //
        //     for (int i = 0; i < parser.export.Length(); i++)
        //     {
        //     Tuple<int> imports;
        //     for (int i = 0; i < lex_stream.NumImportedFilters(); i++)
        //         imports.Next() = lex_stream.ImportedFilter(i);
        //     for (int i = 0; i < parser.exports.Length(); i++)
        //         imports.Next() = parser.exports[i];
        //
        //     for (int i = 0; i < imports.Length(); i++)
        //     {
        //         int import = imports[i];
        //
        //         VariableSymbol *symbol = lex_stream.GetVariableSymbol(import);
        //         InputFileSymbol *file_symbol = this -> lex_stream -> FindOrInsertFile(lex_stream.FileName(import));
        //
        //         current_token_index = this -> lex_stream -> GetNextToken(file_symbol, lex_stream.StartLocation(import));
        //         current_token = this -> lex_stream -> GetTokenReference(current_token_index);
        //         current_token -> SetKind(lex_stream.Kind(import));
        //         current_token -> SetSymbol(this -> variable_table -> FindOrInsertName(symbol -> Name(), symbol -> NameLength()));
        //
        //         this -> lex_stream -> AddImportedTerminal(current_token_index);
        //     }
        //  
    
        //
        // Process the imported grammar.
        //
        Control control(&option, &lex_stream, &variable_table, &macro_table);
        control.ProcessGrammar();
        control.ConstructParser();

        //
        // Merge the symbols inherited from filter files
        // with the symbols that are to be exported by the parser.
        //
            Tuple<int> imports;
        {
            for (int i = 0; i < lex_stream.NumImportedFilters(); i++)
                imports.Next() = lex_stream.ImportedFilter(i);
        }
        {
            for (int i = 0; i < control.grammar -> parser.exports.Length(); i++)
                imports.Next() = control.grammar -> parser.exports[i];
        }

        //
        // Contruct the final list of imported symbols in lex_stream.
        //
        for (int i = 0; i < imports.Length(); i++)
        {
            int import = imports[i];

            VariableSymbol *symbol = lex_stream.GetVariableSymbol(import);
            InputFileSymbol *old_file_symbol = lex_stream.GetFileSymbol(import),
                            *file_symbol = this -> lex_stream -> FindOrInsertFile(old_file_symbol -> Name());
            if (file_symbol -> LineLocation().Length() == 0) // first encounter of this file?
            {
                for (int i = 0; i < old_file_symbol -> LineLocation().Length(); i++)
                    file_symbol -> LineLocation().Next() = old_file_symbol -> LineLocation()[i];
            }

            current_token_index = this -> lex_stream -> GetNextToken(file_symbol, lex_stream.StartLocation(import));
            current_token = this -> lex_stream -> GetTokenReference(current_token_index);
            current_token -> SetEndLocation(lex_stream.EndLocation(import));
            current_token -> SetKind(lex_stream.Kind(import));
            current_token -> SetSymbol(this -> variable_table -> FindOrInsertName(symbol -> Name(), symbol -> NameLength()));

            this -> lex_stream -> AddImportedTerminal(current_token_index);
        }
    }
    
    return;
}


//
//
//
void Scanner::ProcessFilters(const char *filename)
{
    InputFileSymbol *file_symbol = this -> lex_stream -> FindOrInsertFile(option -> include_search_directory, filename);
    const char *name = (file_symbol == NULL ? filename : file_symbol -> Name());
    FilterArguments arguments(this -> option, name);
    Option option(arguments.argc, arguments.argv);

    LexStream lex_stream(&option);
    VariableLookupTable variable_table;
    MacroLookupTable macro_table;
    Scanner scanner(&option, &lex_stream, &variable_table, &macro_table);
    scanner.Scan();

    if (lex_stream.NumTokens() == 0 || scanner.NumErrorTokens() > 0)
        throw 12;
    else
    {
        this -> lex_stream -> AddFilterMacro(filename, (option.DefaultActionFile()
                                                              ? option.DefaultActionFile() -> Name()
                                                              : option.file_prefix));

        //
        // This is the code one would write in order to retrieve only the
        // terminals exported from the imported file without processing the
        // whole grammar.
        //
        //     Parser parser(&option, &lex_stream, &variable_table, &macro_table);
        //     parser.Parse();
        //
        //     //
        //     // Transfer each imported terminal symbol into the local variable
        //     // table and add it to the list of imported symbols.
        //     //
        //     for (int i = 0; i < parser.export.Length(); i++)
        //     {
        //         int import = parser.exports[i];
        // 
        //         VariableSymbol *symbol = lex_stream.GetVariableSymbol(import);
        //         InputFileSymbol *file_symbol = this -> lex_stream -> FindOrInsertFile(lex_stream.FileName(import));
        // 
        //         current_token_index = this -> lex_stream -> GetNextToken(file_symbol, lex_stream.StartLocation(import));
        //         current_token = this -> lex_stream -> GetTokenReference(current_token_index);
        //         current_token -> SetKind(lex_stream.Kind(import));
        //         current_token -> SetSymbol(this -> variable_table -> FindOrInsertName(symbol -> Name(), symbol -> NameLength()));
        // 
        //         this -> lex_stream -> AddImportedFilter(current_token_index);
        //     }
        //  

        //
        // Process the imported grammar.
        //
        Control control(&option, &lex_stream, &variable_table, &macro_table);
        control.ProcessGrammar();
        control.ConstructParser();
    
        //
        // Merge the symbols inherited from filter files
        // with the symbols that are to be exported by the parser.
        //
        Tuple<int> imports;
        {
            for (int i = 0; i < lex_stream.NumImportedFilters(); i++)
                imports.Next() = lex_stream.ImportedFilter(i);
        }
        {
            for (int i = 0; i < control.grammar -> parser.exports.Length(); i++)
                imports.Next() = control.grammar -> parser.exports[i];
        }

        //
        // Contruct the final list of imported symbols in lex_stream.
        //
        for (int i = 0; i < imports.Length(); i++)
        {
            int import = imports[i];

            VariableSymbol *symbol = lex_stream.GetVariableSymbol(import);
            InputFileSymbol *old_file_symbol = lex_stream.GetFileSymbol(import),
                            *file_symbol = this -> lex_stream -> FindOrInsertFile(old_file_symbol -> Name());
            if (file_symbol -> LineLocation().Length() != old_file_symbol -> LineLocation().Length())
            {
                assert(file_symbol -> LineLocation().Length() == 0);
                for (int i = 0; i < old_file_symbol -> LineLocation().Length(); i++)
                    file_symbol -> LineLocation().Next() = old_file_symbol -> LineLocation()[i];
            }
            current_token_index = this -> lex_stream -> GetNextToken(file_symbol, lex_stream.StartLocation(import));
            current_token = this -> lex_stream -> GetTokenReference(current_token_index);
            current_token -> SetEndLocation(lex_stream.EndLocation(import));
            current_token -> SetKind(lex_stream.Kind(import));
            current_token -> SetSymbol(this -> variable_table -> FindOrInsertName(symbol -> Name(), symbol -> NameLength()));

            this -> lex_stream -> AddImportedFilter(current_token_index);
        }
    }

    return;
}


//
//
//
void Scanner::ScanComment()
{
    for (cursor += 2; ! IsNewline(*cursor); cursor++)  // skip all until \n
        ;
    return;
}


//
// This procedure is invoked to skip useless spaces in the input.
// It assumes upon entry that CURSOR points to the next character to
// be scanned.  Before returning it sets CURSOR to the location of the
// first non-space character following its initial position.
//
void Scanner::SkipSpaces()
{
    do
    {
        while (IsSpaceButNotNewline(*cursor))
            cursor++;
        while (IsNewline(*cursor)) // starting a new line?
        {
            if (*cursor == '\n') // For the sequence \r\n only count \n.
                line_location -> Next() = cursor + 1 - input_buffer;
            cursor++;
            while (IsSpaceButNotNewline(*cursor))
                cursor++;
        }

        while (cursor[0] == '-' && cursor[1] == '-')
              ScanComment();
    } while (IsSpace(*cursor));

    return;
}


//
// Scan a symbol of length I and determine if it is a keyword.
//
int Scanner::ScanKeyword0(char *p1)
{
    return TK_SYMBOL;
}

int Scanner::ScanKeyword4(char *p1)
{
    if (p1[1] == 'e' || p1[1] == 'E')
    {
        if (p1[2] == 'o' || p1[2] == 'O')
        {
             if (p1[3] == 'l' || p1[3] == 'L')
                  return TK_EOL_KEY;
             else if (p1[3] == 'f' || p1[3] == 'F')
                  return TK_EOF_KEY;
        }
        else if ((p1[2] == 'n' || p1[2] == 'N') &&
                 (p1[3] == 'd' || p1[3] == 'D'))
             return TK_END_KEY;
    }
    else if ((p1[1] == 'a' || p1[1] == 'A') &&
             (p1[2] == 's' || p1[2] == 'S') &&
             (p1[3] == 't' || p1[3] == 'T'))
         return TK_AST_KEY;

    return TK_SYMBOL;
}

int Scanner::ScanKeyword6(char *p1)
{
    switch (p1[1])
    {
        case 'a': case 'A':
            if ((p1[2] == 'l' || p1[2] == 'L') &&
                (p1[3] == 'i' || p1[3] == 'I') &&
                (p1[4] == 'a' || p1[4] == 'A') &&
                (p1[5] == 's' || p1[5] == 'S'))
                return TK_ALIAS_KEY;
            break;
        case 'e': case 'E':
            if ((p1[2] == 'm' || p1[2] == 'M') &&
                (p1[3] == 'p' || p1[3] == 'P') &&
                (p1[4] == 't' || p1[4] == 'T') &&
                (p1[5] == 'y' || p1[5] == 'Y'))
                return TK_EMPTY_KEY;
            else if ((p1[2] == 'r' || p1[2] == 'R') &&
                     (p1[3] == 'r' || p1[3] == 'R') &&
                     (p1[4] == 'o' || p1[4] == 'O') &&
                     (p1[5] == 'r' || p1[5] == 'R'))
                return TK_ERROR_KEY;
            break;
        case 'n': case 'N':
            if ((p1[2] == 'a' || p1[2] == 'A') &&
                (p1[3] == 'm' || p1[3] == 'M') &&
                (p1[4] == 'e' || p1[4] == 'E') &&
                (p1[5] == 's' || p1[5] == 'S'))
                return TK_NAMES_KEY;
            break;
        case 'r': case 'R':
            if ((p1[2] == 'u' || p1[2] == 'U') &&
                (p1[3] == 'l' || p1[3] == 'L') &&
                (p1[4] == 'e' || p1[4] == 'E') &&
                (p1[5] == 's' || p1[5] == 'S'))
                return TK_RULES_KEY;
            break;
        case 's': case 'S':
            if ((p1[2] == 't' || p1[2] == 'T') &&
                (p1[3] == 'a' || p1[3] == 'A') &&
                (p1[4] == 'r' || p1[4] == 'R') &&
                (p1[5] == 't' || p1[5] == 'T'))
                return TK_START_KEY;
            break;
        case 't': case 'T':
            if ((p1[2] == 'y' || p1[2] == 'Y') &&
                (p1[3] == 'p' || p1[3] == 'P') &&
                (p1[4] == 'e' || p1[4] == 'E') &&
                (p1[5] == 's' || p1[5] == 'S'))
                return TK_TYPES_KEY;
            break;
    }

    return TK_SYMBOL;
}

int Scanner::ScanKeyword7(char *p1)
{
    if ((p1[1] == 'd' || p1[1] == 'D') &&
        (p1[2] == 'e' || p1[2] == 'E') &&
        (p1[3] == 'f' || p1[3] == 'F') &&
        (p1[4] == 'i' || p1[4] == 'I') &&
        (p1[5] == 'n' || p1[5] == 'N') &&
        (p1[6] == 'e' || p1[6] == 'E'))
        return TK_DEFINE_KEY;
    else if ((p1[1] == 'e' || p1[1] == 'E') &&
             (p1[2] == 'x' || p1[2] == 'X') &&
             (p1[3] == 'p' || p1[3] == 'P') &&
             (p1[4] == 'o' || p1[4] == 'O') &&
             (p1[5] == 'r' || p1[5] == 'R') &&
             (p1[6] == 't' || p1[6] == 'T'))
        return TK_EXPORT_KEY;
    else if ((p1[1] == 'i' || p1[1] == 'I') &&
             (p1[2] == 'm' || p1[2] == 'M') &&
             (p1[3] == 'p' || p1[3] == 'P') &&
             (p1[4] == 'o' || p1[4] == 'O') &&
             (p1[5] == 'r' || p1[5] == 'R') &&
             (p1[6] == 't' || p1[6] == 'T'))
        return TK_IMPORT_KEY;
    else if ((p1[1] == 'n' || p1[1] == 'N') &&
             (p1[2] == 'o' || p1[2] == 'O') &&
             (p1[3] == 't' || p1[3] == 'T') &&
             (p1[4] == 'i' || p1[4] == 'I') &&
             (p1[5] == 'c' || p1[5] == 'C') &&
             (p1[6] == 'e' || p1[6] == 'E'))
        return TK_NOTICE_KEY;

    return TK_SYMBOL;
}

int Scanner::ScanKeyword8(char *p1)
{
    if((p1[1] == 'g' || p1[1] == 'G') &&
       (p1[2] == 'l' || p1[2] == 'L') &&
       (p1[3] == 'o' || p1[3] == 'O') &&
       (p1[4] == 'b' || p1[4] == 'B') &&
       (p1[5] == 'a' || p1[5] == 'A') &&
       (p1[6] == 'l' || p1[6] == 'L') &&
       (p1[7] == 's' || p1[7] == 'S'))
        return TK_GLOBALS_KEY;
    else if((p1[1] == 'h' || p1[1] == 'H') &&
            (p1[2] == 'e' || p1[2] == 'E') &&
            (p1[3] == 'a' || p1[3] == 'A') &&
            (p1[4] == 'd' || p1[4] == 'D') &&
            (p1[5] == 'e' || p1[5] == 'E') &&
            (p1[6] == 'r' || p1[6] == 'R') &&
            (p1[7] == 's' || p1[7] == 'S'))
        return TK_HEADERS_KEY;
    else if((p1[1] == 'i' || p1[1] == 'I') &&
            (p1[2] == 'n' || p1[2] == 'N') &&
            (p1[3] == 'c' || p1[3] == 'C') &&
            (p1[4] == 'l' || p1[4] == 'L') &&
            (p1[5] == 'u' || p1[5] == 'U') &&
            (p1[6] == 'd' || p1[6] == 'D') &&
            (p1[7] == 'e' || p1[7] == 'E'))
        return TK_INCLUDE_KEY;
    else if((p1[1] == 'r' || p1[1] == 'R') &&
            (p1[2] == 'e' || p1[2] == 'E') &&
            (p1[3] == 'c' || p1[3] == 'C') &&
            (p1[4] == 'o' || p1[4] == 'O') &&
            (p1[5] == 'v' || p1[5] == 'V') &&
            (p1[6] == 'e' || p1[6] == 'E') &&
            (p1[7] == 'r' || p1[7] == 'R'))
        return TK_RECOVER_KEY;

    return TK_SYMBOL;
}

int Scanner::ScanKeyword9(char *p1)
{
    if((p1[1] == 'k' || p1[1] == 'K') &&
       (p1[2] == 'e' || p1[2] == 'E') &&
       (p1[3] == 'y' || p1[3] == 'Y') &&
       (p1[4] == 'w' || p1[4] == 'W') &&
       (p1[5] == 'o' || p1[5] == 'O') &&
       (p1[6] == 'r' || p1[6] == 'R') &&
       (p1[7] == 'd' || p1[7] == 'D') &&
       (p1[8] == 's' || p1[8] == 'S'))
        return TK_SOFTKEYWORDS_KEY;
    else if((p1[1] == 't' || p1[1] == 'T') &&
            (p1[2] == 'r' || p1[2] == 'R') &&
            (p1[3] == 'a' || p1[3] == 'A') &&
            (p1[4] == 'i' || p1[4] == 'I') &&
            (p1[5] == 'l' || p1[5] == 'L') &&
            (p1[6] == 'e' || p1[6] == 'E') &&
            (p1[7] == 'r' || p1[7] == 'R') &&
            (p1[8] == 's' || p1[8] == 'S'))
        return TK_TRAILERS_KEY;

    return TK_SYMBOL;
}

int Scanner::ScanKeyword10(char *p1)
{
    if((p1[1] == 'd' || p1[1] == 'D') &&
       (p1[2] == 'r' || p1[2] == 'R') &&
       (p1[3] == 'o' || p1[3] == 'O') &&
       (p1[4] == 'p' || p1[4] == 'P') &&
       (p1[5] == 'r' || p1[5] == 'R') &&
       (p1[6] == 'u' || p1[6] == 'U') &&
       (p1[7] == 'l' || p1[7] == 'L') &&
       (p1[8] == 'e' || p1[8] == 'E') &&
       (p1[9] == 's' || p1[9] == 'S'))
        return TK_DROPRULES_KEY;
    else if((p1[1] == 't' || p1[1] == 'T') &&
       (p1[2] == 'e' || p1[2] == 'E') &&
       (p1[3] == 'r' || p1[3] == 'R') &&
       (p1[4] == 'm' || p1[4] == 'M') &&
       (p1[5] == 'i' || p1[5] == 'I') &&
       (p1[6] == 'n' || p1[6] == 'N') &&
       (p1[7] == 'a' || p1[7] == 'A') &&
       (p1[8] == 'l' || p1[8] == 'L') &&
       (p1[9] == 's' || p1[9] == 'S'))
        return TK_TERMINALS_KEY;

    return TK_SYMBOL;
}

int Scanner::ScanKeyword11(char *p1)
{
    if((p1[1] == 'i' || p1[1] == 'I') &&
       (p1[2] == 'd' || p1[2] == 'D') &&
       (p1[3] == 'e' || p1[3] == 'E') &&
       (p1[4] == 'n' || p1[4] == 'N') &&
       (p1[5] == 't' || p1[5] == 'T') &&
       (p1[6] == 'i' || p1[6] == 'I') &&
       (p1[7] == 'f' || p1[7] == 'F') &&
       (p1[8] == 'i' || p1[8] == 'I') &&
       (p1[9] == 'e' || p1[9] == 'E') &&
       (p1[10] == 'r' || p1[10] == 'R'))
        return TK_IDENTIFIER_KEY;

    return TK_SYMBOL;
}


int Scanner::ScanKeyword12(char *p1)
{
    if((p1[1] == 'd' || p1[1] == 'D') &&
       (p1[2] == 'r' || p1[2] == 'R') &&
       (p1[3] == 'o' || p1[3] == 'O') &&
       (p1[4] == 'p' || p1[4] == 'P') &&
       (p1[5] == 's' || p1[5] == 'S') &&
       (p1[6] == 'y' || p1[6] == 'Y') &&
       (p1[7] == 'm' || p1[7] == 'M') &&
       (p1[8] == 'b' || p1[8] == 'B') &&
       (p1[9] == 'o' || p1[9] == 'O') &&
       (p1[10] == 'l' || p1[10] == 'L') &&
       (p1[11] == 's' || p1[11] == 'S'))
        return TK_DROPSYMBOLS_KEY;
    else if((p1[1] == 'd' || p1[1] == 'D') &&
       (p1[2] == 'r' || p1[2] == 'R') &&
       (p1[3] == 'o' || p1[3] == 'O') &&
       (p1[4] == 'p' || p1[4] == 'P') &&
       (p1[5] == 'a' || p1[5] == 'A') &&
       (p1[6] == 'c' || p1[6] == 'C') &&
       (p1[7] == 't' || p1[7] == 'T') &&
       (p1[8] == 'i' || p1[8] == 'I') &&
       (p1[9] == 'o' || p1[9] == 'O') &&
       (p1[10] == 'n' || p1[10] == 'N') &&
       (p1[11] == 's' || p1[11] == 'S'))
        return TK_DROPACTIONS_KEY;

    return TK_SYMBOL;
}


int Scanner::ScanKeyword13(char *p1)
{
    if((p1[1] == 's' || p1[1] == 'S') &&
       (p1[2] == 'o' || p1[2] == 'O') &&
       (p1[3] == 'f' || p1[3] == 'F') &&
       (p1[4] == 't' || p1[4] == 'T') &&
       (p1[5] == 'k' || p1[5] == 'K') &&
       (p1[6] == 'e' || p1[6] == 'E') &&
       (p1[7] == 'y' || p1[7] == 'Y') &&
       (p1[8] == 'w' || p1[8] == 'W') &&
       (p1[9] == 'o' || p1[9] == 'O') &&
       (p1[10] == 'r' || p1[10] == 'R') &&
       (p1[11] == 'd' || p1[11] == 'D') &&
       (p1[12] == 's' || p1[12] == 'S'))
        return TK_SOFTKEYWORDS_KEY;

    return TK_SYMBOL;
}

int Scanner::ScanKeyword24(char *p1)
{
    if((p1[1] == 'd' || p1[1] == 'D') &&
       (p1[2] == 'i' || p1[2] == 'I') &&
       (p1[3] == 's' || p1[3] == 'S') &&
       (p1[4] == 'j' || p1[4] == 'J') &&
       (p1[5] == 'o' || p1[5] == 'O') &&
       (p1[6] == 'i' || p1[6] == 'I') &&
       (p1[7] == 'n' || p1[7] == 'N') &&
       (p1[8] == 't' || p1[8] == 'T') &&
       (p1[9] == 'p' || p1[9] == 'P') &&
       (p1[10] == 'r' || p1[10] == 'R') &&
       (p1[11] == 'e' || p1[11] == 'E') &&
       (p1[12] == 'd' || p1[12] == 'D') &&
       (p1[13] == 'e' || p1[13] == 'E') &&
       (p1[14] == 'c' || p1[14] == 'C') &&
       (p1[15] == 'e' || p1[15] == 'E') &&
       (p1[16] == 's' || p1[16] == 'S') &&
       (p1[17] == 's' || p1[17] == 'S') &&
       (p1[18] == 'o' || p1[18] == 'O') &&
       (p1[19] == 'r' || p1[19] == 'R') &&
       (p1[20] == 's' || p1[20] == 'S') &&
       (p1[21] == 'e' || p1[21] == 'E') &&
       (p1[22] == 't' || p1[22] == 'T') &&
       (p1[23] == 's' || p1[23] == 'S'))
        return TK_DISJOINTPREDECESSORSETS_KEY;

    return TK_SYMBOL;
}


//
//
//
void Scanner::ClassifyBlock(Tuple<BlockSymbol *> &blocks)
{
    for (int i = 0; i < blocks.Length(); i++)
    {
        BlockSymbol *block = blocks[i];
        if (strncmp(cursor, block -> BlockBegin(), block -> BlockBeginLength()) == 0)
        {
            char *block_end = block -> BlockEnd();
            int length = block -> BlockEndLength();

            for (cursor = cursor + length; *cursor != NULL_CHAR && (! strxeq(cursor, block_end)); cursor++)
            {
                // if (IsNewline(*cursor))
                if (*cursor == '\n') // when we encounter the sequence \r\n, we only count \n
                    line_location -> Next() = cursor - input_buffer + 1; // character after newline.
            }

            if (*cursor == NULL_CHAR)
                 AddErrorToken(UNTERMINATED_BLOCK, current_token_index);
            else cursor = cursor + length;

            current_token -> SetKind(TK_BLOCK);
            current_token -> SetSymbol(block);
            current_token -> SetEndLocation((cursor - 1) - input_buffer);

            return;
        }
    }

    ClassifySymbol();

    return;
}


//
//
//
void Scanner::ClassifyMacroNameSymbol()
{
    char *ptr = cursor + 1;

    while (IsAlnum(*ptr) && *ptr != option->lpg_escape)
        ptr++;
    int len = ptr - cursor,
        kind = (scan_keyword[len < SCAN_KEYWORD_SIZE ? len : 0])(cursor);
    current_token -> SetEndLocation((ptr - 1) - input_buffer);

    if (kind == TK_SYMBOL || (! (option -> legacy))) // not a keyword?
    {
        current_token -> SetKind(TK_MACRO_NAME); // ... then it's a macro
       // *cursor = option->escape;
        current_token -> SetSymbol(macro_table -> FindOrInsertName(cursor, len));
       // *cursor = option->lpg_escape;
    }
    else if (option -> escape != '%') // if legacy option is on treat as keyword only
    {
        assert(option -> legacy);
        current_token -> SetKind(kind); // it's a keyword
        AddWarningToken(LEGACY_KEYWORD, current_token_index);
        if (kind == TK_INCLUDE_KEY)
            ptr = ProcessInclude(ptr);
    }

    cursor = ptr;

    return;
}


//
//
//
void Scanner::ClassifyKeyword()
{
    char *ptr = cursor + 1;

    while ((! IsSpace(*ptr)) && (*ptr != option ->lpg_escape))
        ptr++;
    int len = ptr - cursor;

    current_token -> SetKind((scan_keyword[len < SCAN_KEYWORD_SIZE ? len : 0])(cursor));
    current_token -> SetEndLocation((ptr - 1) - input_buffer);
    if (current_token -> Kind() == TK_SYMBOL)
    {
        current_token->SetSymbol(variable_table->FindOrInsertName(cursor, len));
        AddWarningToken(SYMBOL_WITH_KEYWORD_MARKER, current_token_index);

    }
    else if (current_token -> Kind() == TK_INCLUDE_KEY)
         ptr = ProcessInclude(ptr);

    cursor = ptr;

    return;
}


//
//
//
void Scanner::ClassifySymbol()
{
    char *ptr = cursor + 1;

    while ((! IsSpace(*ptr)) && (*ptr != option ->lpg_escape))
        ptr++;
    int len = ptr - cursor;

    current_token -> SetKind(TK_SYMBOL);
    current_token -> SetEndLocation((ptr - 1) - input_buffer);
    current_token -> SetSymbol(variable_table -> FindOrInsertName(cursor, len));

    cursor = ptr;

    return;
}


//
//
//
void Scanner::ClassifyEquivalence()
{
    if (cursor[1] == ':' && cursor[2] == '=' && IsSpace(cursor[3]))
    {
        current_token -> SetKind(TK_EQUIVALENCE);
        current_token -> SetEndLocation((cursor + 2) - input_buffer);
        cursor += 3;
    }
    else if (cursor[1] == ':' && cursor[2] == '=' &&
             cursor[3] == '?' && IsSpace(cursor[4]))
    {
        current_token -> SetKind(TK_PRIORITY_EQUIVALENCE);
        current_token -> SetEndLocation((cursor + 3) - input_buffer);
        cursor += 4;
    }
    else ClassifySymbol();

    return;
}


//
//
//
void Scanner::ClassifyArrow()
{
    if (cursor[1] == '>' && IsSpace(cursor[2]))
    {
        current_token -> SetKind(TK_ARROW);
        current_token -> SetEndLocation((cursor + 1) - input_buffer);
        cursor += 2;
    }
    else if (cursor[1] == '>' && cursor[2] == '?' && IsSpace(cursor[3]))
    {
        current_token -> SetKind(TK_PRIORITY_ARROW);
        current_token -> SetEndLocation((cursor + 2) - input_buffer);
        cursor += 3;
    }
    else ClassifySymbol();

    return;
}


//
//
//
void Scanner::ClassifyOr()
{
    if (IsSpace(cursor[1]))
    {
        current_token -> SetKind(TK_OR_MARKER);
        current_token -> SetEndLocation(cursor - input_buffer);
        cursor++;
    }
    else ClassifySymbol();

    return;
}


void Scanner::ClassifySingleQuotedSymbol()
{
    char delimiter = *cursor,
         *ptr;

    for (ptr = cursor + 1; ! IsNewline(*ptr); ptr++)
    {
        if (ptr[0] == delimiter)
            if (ptr[1] == delimiter)
                 ptr++;
            else break;
    }

    int length = ptr - cursor - 1;
    if (length == 0)
    {
        current_token -> SetKind(TK_EMPTY_KEY);
        current_token -> SetEndLocation(ptr - input_buffer);
    }
    else
    {
        char *name = NewString(length + 1),
             *p1 = name,
             *p2 = cursor + 1;

        current_token -> ResetInfoAndSetLocation(input_file, p2 - input_buffer);

        do
        {
            *(p1++) = *p2;
            p2 += (p2[0] == delimiter && p2[1] == delimiter ? 2 : 1); // turn 2 consecutive quotes into one.
        } while(p2 < ptr);

        current_token -> SetKind(TK_SYMBOL);
        current_token -> SetSymbol(variable_table -> FindOrInsertName(name, p1 - name));
        current_token -> SetEndLocation((ptr - 1) - input_buffer);
    }

    if (*ptr == delimiter)
    {
        cursor = ptr + 1;
        //
        // TODO: This test appeared to have been here for legacy reasons. 
        // Thus, it has been disabled until someone screams and then it will be
        // reevaluated.
        //
        // if (! IsSpace(*cursor))
        //     AddErrorToken(UNDELIMITED_STRING_SYMBOL, current_token_index);
    }
    else
    {
        cursor = ptr;
        AddErrorToken(UNTERMINATED_STRING_SYMBOL, current_token_index);
    }

    return;
}


void Scanner::ClassifyDoubleQuotedSymbol()
{
    char delimiter = *cursor,
         *ptr;

    for (ptr = cursor + 1; ! IsNewline(*ptr); ptr++)
    {
        if (ptr[0] == delimiter)
            if (ptr[1] == delimiter)
                 ptr++;
            else break;
    }

    int length = ptr - cursor - 1;
    if (length == 0)
    {
        current_token -> SetKind(TK_EMPTY_KEY);
        current_token -> SetEndLocation(ptr - input_buffer);
    }
    else
    {
        char *name = NewString(length + 1),
             *p1 = name,
             *p2 = cursor + 1;

        current_token -> ResetInfoAndSetLocation(input_file, p2 - input_buffer);

        do
        {
            if (p2[0] == '\\')
            {
                switch(p2[1])
                {
                    case 'b':
                        *(p1++) = '\b';
                        p2 += 2;
                        break;
                    case 't':
                        *(p1++) = '\t';
                        p2 += 2;
                        break;
                    case 'n':
                        *(p1++) = '\n';
                        p2 += 2;
                        break;
                    case 'f':
                        *(p1++) = '\f';
                        p2 += 2;
                        break;
                    case 'r':
                        *(p1++) = '\r';
                        p2 += 2;
                        break;
                    case '\"':
                        *(p1++) = '\"';
                        p2 += 2;
                        break;
                    case '\'':
                        *(p1++) = '\'';
                        p2 += 2;
                        break;
                    case '\\':
                        *(p1++) = '\\';
                        p2 += 2;
                        break;
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                        {
                            int num = 0,
                                i;
                            for (i = 1; i < 4 ; i++)
                            {
                                char c = p2[i];
                                if (IsDigit(c) && c < '8')
                                     num = num * 8 + (c - '0');
                                else break;
                            }

                            //
                            // If the character is a ASCII code save it.
                            //
                            if (i > 1 && num <= 128)
                            {
                                *(p1++) = num;
                                p2 += i;
                            }
                            else
                            {
                                *(p1++) = *(p2++);
                                AddErrorToken(BAD_OCTAL_ASCII_CODE, current_token_index);
                            }
                        }
                        break;
                    case 'u':
                        {
                            int num = 0,
                                i;
                            for (i = 2; i < 6; i++)
                            {
                                char c = p2[i];
                                if (IsDigit(c))
                                     num = num * 16 + (c - '0');
                                else if (IsAlpha(c)) 
                                {
                                    c = ToUpper(c);
                                    if (ToUpper(c) <= 'F')
                                         num = num * 16 + 10 + (c - 'A');
                                    else break;
                                }
                                else break;
                            }

                            //
                            // If the character is a valid unicode character, convert
                            // it to its UTF8 format. 
                            //
                            if (i == 6)
                            {
                                if (num == 0)
                                {
                                     *(p1++) = (char) 0xC0;
                                     *(p1++) = (char) 0x80;
                                }
                                else if (num <= 0x007F)
                                     *(p1++) = (char) num;
                                else if (num <= 0x07FF)
                                {
                                     *(p1++) = (char) ((char) 0xC0 | (char) ((num >> 6) & 0x0000001F)); // bits 6-10
                                     *(p1++) = (char) ((char) 0x80 | (char) (num & 0x0000003F));        // bits 0-5
                                }
                                else
                                {
                                     *(p1++) = (char) ((char) 0xE0 | (char) ((num >> 12) & 0x0000000F));
                                     *(p1++) = (char) ((char) 0x80 | (char) ((num >> 6) & 0x0000003F));
                                     *(p1++) = (char) ((char) 0x80 | (char) (num & 0x0000003F));
                                }
                                p2 += i;
                            }
                            else
                            {
                                *(p1++) = *(p2++);
                                AddErrorToken(BAD_UNICODE, current_token_index);
                            }
                        }
                        break;
                    default:
                        *(p1++) = *(p2++);
                        AddErrorToken(ISOLATED_BACKSLASH, current_token_index);
                        break;
                }
            }
            else
            {
                *(p1++) = *p2;
                p2 += (p2[0] == delimiter && p2[1] == delimiter ? 2 : 1); // turn 2 consecutive quotes into one.
            }
        } while(p2 < ptr);

        current_token -> SetKind(TK_SYMBOL);
        current_token -> SetSymbol(variable_table -> FindOrInsertName(name, p1 - name));
        current_token -> SetEndLocation((ptr - 1) - input_buffer);
    }

    if (*ptr == delimiter)
    {
        cursor = ptr + 1;
        //
        // TODO: This test appeared to have been here for legacy reasons. 
        // Thus, it has been disabled until someone screams and then it will be
        // reevaluated.
        //
        //if (! IsSpace(*cursor))
        //    AddErrorToken(UNDELIMITED_STRING_SYMBOL, current_token_index);
    }
    else
    {
        cursor = ptr;
        AddErrorToken(UNTERMINATED_STRING_SYMBOL, current_token_index);
    }

    return;
}


void Scanner::ClassifyLess()
{
    char *ptr = cursor + 1;

    if (IsAlpha(*ptr))
    {
        for (ptr++; *ptr != '>' && (! IsNewline(*ptr)); ptr++)
            ;
        if (*ptr == '>')
             ptr++;
        else AddErrorToken(UNTERMINATED_BRACKET_SYMBOL, current_token_index);

        current_token -> SetKind(TK_SYMBOL);
        current_token -> SetSymbol(variable_table -> FindOrInsertName(cursor, ptr - cursor));
        current_token -> SetEndLocation((ptr - 1) - input_buffer);
        cursor = ptr;
    }
    else ClassifySymbol();

    return;
}


void Scanner::ClassifyEof()
{
    current_token -> SetKind(TK_EOF);

    cursor++;

    return;
}


void Scanner::ClassifyBadToken()
{
    assert(! "this can happen?");

    return;
}


char *Scanner::ProcessInclude(const char *start)
{
    //
    // . Save position of INCLUDE keyword token 
    // . Scan the include section
    // . Include the files specified in the include section
    //
    int include_key_index = current_token_index;
    char *ptr = (char *) start;
    cursor = ptr;
    SkipSpaces();
    if (*cursor != option ->lpg_escape)
    {
        current_token_index = lex_stream -> GetNextToken(input_file, cursor - input_buffer);
        current_token = lex_stream -> GetTokenReference(current_token_index);
        Tuple<BlockSymbol *> &blocks = action_blocks -> ActionBlocks((unsigned char) *cursor);
        if (blocks.Length())
             ClassifyBlock(blocks);
        else (this ->* classify_token[(unsigned char) *cursor])();
        SkipSpaces();
        ptr = cursor;
        if (current_token -> Kind() == TK_SYMBOL)
        {
            char *filename = NewString(lex_stream -> NameStringLength(current_token_index) + 1);
            strcpy(filename, lex_stream -> NameString(current_token_index));

            Token save_include_key = *(lex_stream -> GetTokenReference(include_key_index)),
                  save_filename = *current_token;

            lex_stream -> ResetTokenStream(include_key_index);
            if (*cursor == '%' || (*cursor == option ->lpg_escape && option -> legacy))
            {
                char *p = cursor + 1;
                while (IsAlnum(*p) && *p != option ->lpg_escape)
                    p++;
                int len = p - cursor;
                if ((scan_keyword[len < SCAN_KEYWORD_SIZE ? len : 0])(cursor) == TK_END_KEY)
                    ptr = p; // move the pointer
            }

            PushEnvironment();

            input_file = lex_stream -> FindOrInsertFile(option -> include_search_directory, filename);
            int error_code = IncludeFile();

            if (error_code != NO_ERROR)
            {
                current_token_index = lex_stream -> GetNextToken(input_file, save_include_key.StartLocation());
                current_token = lex_stream -> GetTokenReference(current_token_index);
                *current_token = save_include_key;

                current_token_index = lex_stream -> GetNextToken(input_file, save_filename.StartLocation());
                current_token = lex_stream -> GetTokenReference(current_token_index);
                *current_token = save_filename;

                AddErrorToken(error_code, current_token_index);

                PopEnvironment();
                ptr = cursor;
            }
            else PopEnvironment();
        }
    }

    return ptr;
}


int Scanner::IncludeFile()
{
    input_buffer = NULL;
    cursor = NULL;

    //
    // If we could not open the include file, issue an error.
    //
    if (input_file == NULL)
        return NO_INCLUDE;
    else
    {
        //
        // If the file is already in the process of being included, issue an error and stop.
        //
        if (input_file -> IsLocked())
            return RECURSIVE_INCLUDE;
        else
        {
            //
            // Lock the include_file to avoid looping...
            //
            input_file -> Lock();
            input_file -> ReadInput();
            input_buffer = input_file -> Buffer();
            cursor = input_buffer;
            if (input_buffer == NULL)
                return NO_INCLUDE;
            line_location = input_file -> LineLocationReference();
            line_location -> Reset();
            line_location -> Next() = 0; // mark starting location of line # 0
            SkipOptions();
            Scan(cursor, &(input_buffer[input_file -> BufferLength()]));
            while(lex_stream -> Kind(current_token_index) == TK_EOF)
                current_token_index--;
            lex_stream -> ResetTokenStream(current_token_index + 1);
            input_file -> Unlock();
        }
    }

    return NO_ERROR;
}
