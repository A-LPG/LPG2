#include "control.h"
#include "generator.h"
#include "table.h"
#include "CTable.h"
#include "CppTable.h"
#include "JavaTable.h"
#include "PlxTable.h"
#include "PlxasmTable.h"
#include "MlTable.h"
#include "XmlTable.h"

#include <string.h>
#include <iostream>

#include "CppTable2.h"
#include "CSharpTable.h"
#include "DartTable.h"
#include "GoTable.h"
#include "Python2Table.h"
#include "Python3Table.h"
#include "TypeScriptTable.h"
using namespace std;

const char Control::HEADER_INFO[]  = "The LALR Parser Generator",
           Control::VERSION[] = "2.2.00 (" __DATE__ ")";

//
//
//
void Control::ProcessGrammar(void)
{
    try
    {
        grammar -> Process();
    }
    catch(int code)
    {
        CleanUp();
        throw code;
    }
    catch (const char *)
    {
        CleanUp();
        throw 12;
    }
}


//
// By the time this function is invoked, the source input has already
// been fully processed. More specifically, the scanner is invoked in the
// main program, jikespg.cpp. The parser and the grammar are constructed
// and invoked in the constructor of this object, Control::Control.
//
void Control::ConstructParser(void)
{
	
    Generator *generator = NULL;
    Table *table = NULL;

    try
    {
        //
        // If the user only wanted to edit his grammar, we quit the program.
        //
        if (option -> edit)
        {
            if (option -> first || option -> follow || option -> xref)
                base -> Process();

            if (! option -> quiet)
            {
                option -> report.Put("\nNumber of Terminals: ");
                option -> report.Put(grammar -> num_terminals - 1); //-1 for %empty
                option -> report.PutChar('\n');
                option -> report.Put("Number of Nonterminals: ");
                option -> report.Put(grammar -> num_nonterminals - 1); // -1 for %ACC
                option -> report.PutChar('\n');
                option -> report.Put("Number of Productions: ");
                option -> report.Put(grammar -> num_rules + 1);
                option -> report.PutChar('\n');

                if (option -> single_productions)
                {
                    option -> report.Put("Number of Single Productions: ");
                    option -> report.Put(grammar -> num_single_productions);
                    option -> report.PutChar('\n');
                }

                option -> report.Put("Number of Items: ");
                option -> report.Put(grammar -> num_items);
                option -> report.PutChar('\n');

                option -> FlushReport();
            }

            return;
        }

        base -> Process(); // Build basic maps
        pda -> Process();  // Build State Automaton

        if (! option -> quiet)
        {
            option -> report.Put("\nNumber of Terminals: ");
            option -> report.Put(grammar -> num_terminals - 1);
            option -> report.PutChar('\n');
            option -> report.Put("Number of Nonterminals: ");
            option -> report.Put(grammar -> num_nonterminals - 1);
            option -> report.PutChar('\n');
            option -> report.Put("Number of Productions: ");
            option -> report.Put(grammar -> num_rules + 1);
            option -> report.PutChar('\n');

            if (option -> single_productions)
            {
                option -> report.Put("Number of Single Productions: ");
                option -> report.Put(grammar -> num_single_productions);
                option -> report.PutChar('\n');
            }

            option -> report.Put("Number of Items: ");
            option -> report.Put(grammar -> num_items);
            option -> report.PutChar('\n');
            if (option -> scopes)
            {
                option -> report.Put("Number of Scopes: ");
                option -> report.Put(pda -> scope_prefix.Size());
                option -> report.PutChar('\n');
            }

            option -> report.Put("Number of States: ");
            option -> report.Put(pda -> num_states);
            option -> report.PutChar('\n');

            if (pda -> max_la_state > pda -> num_states)
            {
                option -> report.Put("Number of look-ahead states: ");
                option -> report.Put(pda -> max_la_state - pda -> num_states);
                option -> report.PutChar('\n');
            }

            option -> report.Put("Number of Shift actions: ");
            option -> report.Put(pda -> num_shifts);
            option -> report.PutChar('\n');

            option -> report.Put("Number of Goto actions: ");
            option -> report.Put(pda -> num_gotos);
            option -> report.PutChar('\n');

            if (option -> read_reduce)
            {
                option -> report.Put("Number of Shift/Reduce actions: ");
                option -> report.Put(pda -> num_shift_reduces);
                option -> report.PutChar('\n');
                option -> report.Put("Number of Goto/Reduce actions: ");
                option -> report.Put(pda -> num_goto_reduces);
                option -> report.PutChar('\n');
            }

            option -> report.Put("Number of Reduce actions: ");
            option -> report.Put(pda -> num_reductions);
            option -> report.PutChar('\n');

            if (! pda -> not_lrk)
            {
                option -> report.Put("Number of Shift-Reduce conflicts: ");
                option -> report.Put(pda -> num_shift_reduce_conflicts);
                option -> report.PutChar('\n');
                option -> report.Put("Number of Reduce-Reduce conflicts: ");
                option -> report.Put(pda -> num_reduce_reduce_conflicts);
                option -> report.PutChar('\n');
            }

            if (grammar -> keywords.Length() > 0)
            {
                option -> report.Put("Number of Keyword/Identifier Shift conflicts: ");
                option -> report.Put(pda -> num_shift_shift_conflicts);
                option -> report.PutChar('\n');
                option -> report.Put("Number of Keyword/Identifier Shift-Reduce conflicts: ");
                option -> report.Put(pda -> num_soft_shift_reduce_conflicts);
                option -> report.PutChar('\n');
                option -> report.Put("Number of Keyword/Identifier Reduce-Reduce conflicts: ");
                option -> report.Put(pda -> num_soft_reduce_reduce_conflicts);
                option -> report.PutChar('\n');
            }

            option -> FlushReport();
        }

        //
        // If the removal of single productions is requested, do
        // so now.
        // If STATES option is on, we print the states.
        //
        if (option -> states)
            pda -> PrintStates();

        //
        // If the tables are requested, we process them.
        //
        if (option -> table)
        {
            if (option -> goto_default && option -> nt_check)
                option -> EmitError(0, "The options GOTO_DEFAULT and NT_CHECK are incompatible. Tables not generated");
            else
            {
                generator = new Generator(this, pda);
                generator -> Process();
                switch (option->programming_language)
                {
                case Option::C:
                    table = new CTable(this, pda);
                    break;
                case Option::CPP:
                    table = new CppTable(this, pda);
                    break;
                case Option::CPP2:
                    table = new CppTable2(this, pda);
                    break;
                case Option::JAVA:
                    table = new JavaTable(this, pda);
                    break;
                case Option::CSHARP:
                    table = new CSharpTable(this, pda);
                    break;
                case Option::PYTHON2:
                    table = new Python2Table(this, pda);
                    break;
                case Option::PYTHON3:
                    table = new Python3Table(this, pda);
                    break;
                case Option::DART:
                    table = new DartTable(this, pda);
                    break;
                case Option::GO:
                    table = new GoTable(this, pda);
                    break;
                case Option::TSC:
                    table = new TypeScriptTable(this, pda);
                    break;
                case Option::PLX:
                    table = new PlxTable(this, pda);
                    break;
                case Option::PLXASM:
                    table = new PlxasmTable(this, pda);
                    break;
                case Option::ML:
                    table = new MlTable(this, pda);
                    break;
                case Option::XML:
                    table = new XmlTable(this, pda);
                    break;
                default:
                       assert(false);
                       break;
                }
            	
                generator -> Generate(table);
                delete generator; generator = NULL;

                table -> PrintTables();
                if (option -> states)
                    table -> PrintStateMaps();

                table -> PrintReport();

                delete table; table = NULL;
            }
        }

        option -> FlushReport();
    }
    catch(int code)
    {
        delete generator;
        delete table;

        CleanUp();
        throw code;
    }
    catch (const char *)
    {
        delete generator;
        delete table;

        CleanUp();
        throw 12;
    }

    return;
}


void Control::InvalidateFile(const char *filename, const char *filetype)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL) // could not open file...
        return;

    UnbufferedTextFile buffer(&file);

    switch(option -> programming_language)
    {
        case Option::CPP:
        case Option::C:
             break;
        case Option::PLXASM:
        case Option::PLX:
             break;
        case Option::JAVA:
             if (strlen(option -> package) > 0)
             {
                 buffer.Put("package ");
                 buffer.Put(option -> package);
                 buffer.Put(";\n\n");
             }
             buffer.Put("/**\n"
                        " * This class is invalid because LPG stopped while processing\n"
                        " * the grammar file \"");
             buffer.Put(option -> grm_file);
             buffer.Put("\"\n");
             buffer.Put(" */\n"
                        "public class Bad");
             buffer.Put(filetype);
             buffer.Put(" {}\n\n");
             break;
        case Option::ML:
             break;
        case Option::XML:
             break;
        default:
             break;
    }

    buffer.Flush();
    fclose(file);

    return;
}


void Control::Exit(int code)
{
    InvalidateFile(option -> prs_file, option -> prs_type);
    InvalidateFile(option -> sym_file, option -> sym_type);
    if (grammar -> exported_symbols.Length() > 0)
        InvalidateFile(option -> exp_file, option -> exp_type);
    switch(option -> programming_language)
    {
        case Option::CPP:
        case Option::C:
             InvalidateFile(option -> dcl_file, option -> dcl_type);
             break;
        case Option::PLXASM:
        case Option::PLX:
             InvalidateFile(option -> dcl_file, option -> dcl_type);
             InvalidateFile(option -> def_file, option -> def_type);
             InvalidateFile(option -> imp_file, option -> imp_type);
             break;
        case Option::JAVA:
             break;
        case Option::ML:
             InvalidateFile(option -> dcl_file, option -> dcl_type);
             InvalidateFile(option -> imp_file, option -> imp_type);
             break;
        case Option::XML:
             break;
        default:
             break;
    }

    //
    // Before exiting, flush the report buffer.
    //
    option -> FlushReport();

    CleanUp();

    throw code;

    return;
}

void Control::PrintHeading(int)
{
    fprintf(option -> syslis, "\f\n\n %-39s%s\n\n", HEADER_INFO, VERSION);
    return;
}
