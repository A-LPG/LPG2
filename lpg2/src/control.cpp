#include "control.h"
#include "generator.h"
#include "table.h"
#include "CppTable.h"
#include "JavaTable.h"

#include <string.h>
#include <iostream>
#include <memory>

#include "lpg_error.h"

#include "CppTable2.h"
#include "CSharpTable.h"
#include "DartTable.h"
#include "GoTable.h"
#include "RustTable.h"
#include "lpg_version.h"
#include "Python2Table.h"
#include "Python3Table.h"
#include "TypeScriptTable.h"
using namespace std;

const char Control::HEADER_INFO[] = "The LALR Parser Generator",
           Control::VERSION[] = LPG2_VERSION_STRING;

//
//
//
void Control::ProcessGrammar(void)
{
    grammar -> Process();
}


//
// By the time this function is invoked, the source input has already
// been fully processed. More specifically, the scanner is invoked in the
// main program, jikespg.cpp. The parser and the grammar are constructed
// and invoked in the constructor of this object, Control::Control.
//
void Control::ConstructParser(void)
{
    std::unique_ptr<Generator> generator;
    std::unique_ptr<Table> table;

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

        if (option -> fail_on_conflicts &&
            (pda -> num_shift_reduce_conflicts > 0 ||
             pda -> num_reduce_reduce_conflicts > 0))
        {
            option -> EmitError(0,
                "fail_on_conflicts: conflicts detected "
                "(shift/reduce or reduce/reduce)");
            Exit(12);
        }

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
                generator.reset(new Generator(this, pda.get()));
                generator -> Process();
                switch (option->programming_language)
                {
                case Option::CPP:
                    table.reset(new CppTable(this, pda.get()));
                    break;
                case Option::CPP2:
                    table.reset(new CppTable2(this, pda.get()));
                    break;
                case Option::JAVA:
                    table.reset(new JavaTable(this, pda.get()));
                    break;
                case Option::CSHARP:
                    table.reset(new CSharpTable(this, pda.get()));
                    break;
                case Option::PYTHON2:
                    table.reset(new Python2Table(this, pda.get()));
                    break;
                case Option::PYTHON3:
                    table.reset(new Python3Table(this, pda.get()));
                    break;
                case Option::DART:
                    table.reset(new DartTable(this, pda.get()));
                    break;
                case Option::GO:
                    table.reset(new GoTable(this, pda.get()));
                    break;
                case Option::RUST:
                    table.reset(new RustTable(this, pda.get()));
                    break;
                case Option::TSC:
                    table.reset(new TypeScriptTable(this, pda.get()));
                    break;
                default:
                    option->EmitError(0, "Unsupported programming language for table generation");
                    break;
                }

                if (! table)
                    throw LpgError(12);

                generator -> Generate(table.get());
                generator.reset();

                table -> PrintTables();
                if (option -> states)
                    table -> PrintStateMaps();

                table -> PrintReport();

                table.reset();
            }
        }

        option -> FlushReport();
}


void Control::Exit(int code)
{
    //
    // Before exiting, flush the report buffer.
    //
    option -> FlushReport();

    CleanUp();

    throw LpgError(code);
}

void Control::PrintHeading(int)
{
    fprintf(option -> syslis, "\f\n\n %-39s%s\n\n", HEADER_INFO, VERSION);
    return;
}
