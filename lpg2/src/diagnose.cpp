#include "util.h"
#include "diagnose.h"

#include <assert.h>
#include <iostream>
using namespace std;

int (*DiagnoseParser::t_action) (int, int, LexStream *) = Parser::t_action;

void DiagnoseParser::ReallocateStacks()
{
    int new_size = stack.Size() + STACK_INCREMENT;

    assert(new_size <= SHRT_MAX);

    stack.Resize(new_size);
    location_stack.Resize(new_size);
    temp_stack.Resize(new_size);
    next_stack.Resize(new_size);
    prev_stack.Resize(new_size);
    scope_index.Resize(new_size);
    scope_position.Resize(new_size);

    return;
}


void DiagnoseParser::DiagnoseParse()
{
    lex_stream -> Reset();
    ReallocateStacks();

    //
    // Start parsing
    //
    state_stack_top = 0;
    int act = START_STATE;
    stack[state_stack_top] = act;

    TokenObject current_token = lex_stream -> Gettoken();
    int current_kind = lex_stream -> Kind(current_token);
    location_stack[state_stack_top] = Loc(current_token);

    //
    // Process a terminal
    //
    do
    {
        //
        // Synchronize state stacks and update the location stack
        //
        int prev_pos = -1;
        prev_stack_top = -1;

        int next_pos = -1;
        next_stack_top = -1;

        int pos = state_stack_top;
        temp_stack_top = state_stack_top - 1;
        for (int i = 0; i <= state_stack_top; i++)
            temp_stack[i] = stack[i];

        act = t_action(act, current_kind, lex_stream);
        //
        // When a reduce action is encountered, we compute all REDUCE
        // and associated goto actions induced by the current token.
        // Eventually, a SHIFT, SHIFT-REDUCE, ACCEPT or ERROR action is
        // computed...
        //
        while (act <= NUM_RULES)
        {
            do
            {
                temp_stack_top -= (rhs[act]-1);
                act = nt_action(temp_stack[temp_stack_top], lhs[act]);
            } while(act <= NUM_RULES);
            //
            // ... Update the maximum useful position of the
            // (STATE_)STACK, push goto state into stack, and
            // compute next action on current symbol ...
            //
            if (temp_stack_top + 1 >= stack.Size())
                ReallocateStacks();
            pos = Util::Min(pos, temp_stack_top);
            temp_stack[temp_stack_top + 1] = act;
            act = t_action(act, current_kind, lex_stream);
        }

        //
        // At this point, we have a shift, shift-reduce, accept or error
        // action.  STACK contains the configuration of the state stack
        // prior to executing any action on current_token. next_stack contains
        // the configuration of the state stack after executing all
        // reduce actions induced by current_token.  The variable pos indicates
        // the highest position in STACK that is still useful after the
        // reductions are executed.
        //
        while(act > ERROR_ACTION || act < ACCEPT_ACTION) // SHIFT-REDUCE action or SHIFT action ?
        {
            next_stack_top = temp_stack_top + 1;
            for (int i = next_pos + 1; i <= next_stack_top; i++)
                next_stack[i] = temp_stack[i];

            for (int k = pos + 1; k <= next_stack_top; k++)
                location_stack[k] = location_stack[state_stack_top];

            //
            // If we have a shift-reduce, process it as well as
            // the goto-reduce actions that follow it.
            //
            if (act > ERROR_ACTION)
            {
                act -= ERROR_ACTION;
                do
                {
                    next_stack_top -= (rhs[act]-1);
                    act = nt_action(next_stack[next_stack_top], lhs[act]);
                } while(act <= NUM_RULES);
                pos = Util::Min(pos, next_stack_top);
            }

            if (next_stack_top + 1 >= stack.Size())
                ReallocateStacks();

            temp_stack_top = next_stack_top;
            next_stack[++next_stack_top] = act;
            next_pos = next_stack_top;

            //
            // Simulate the parser through the next token without
            // destroying STACK or next_stack.
            //
            current_token = lex_stream -> Gettoken();
            current_kind = lex_stream -> Kind(current_token);
            act = t_action(act, current_kind, lex_stream);
            while(act <= NUM_RULES)
            {
                //
                // ... Process all goto-reduce actions following
                // reduction, until a goto action is computed ...
                //
                do
                {
                    int lhs_symbol = lhs[act];
                    temp_stack_top -= (rhs[act]-1);
                    act = (temp_stack_top > next_pos
                                          ? temp_stack[temp_stack_top]
                                          : next_stack[temp_stack_top]);
                    act = nt_action(act, lhs_symbol);
                }   while(act <= NUM_RULES);

                //
                // ... Update the maximum useful position of the
                // (STATE_)STACK, push GOTO state into stack, and
                // compute next action on current symbol ...
                //
                if (temp_stack_top + 1 >= stack.Size())
                    ReallocateStacks();

                next_pos = Util::Min(next_pos, temp_stack_top);
                temp_stack[temp_stack_top + 1] = act;
                act = t_action(act, current_kind, lex_stream);
            }

            //
            // No error was detected, Read next token into
            // PREVTOK element, advance CURRENT_TOKEN pointer and
            // update stacks.
            //
            if (act != ERROR_ACTION)
            {
                prev_stack_top = state_stack_top;
                for (int i = prev_pos + 1; i <= prev_stack_top; i++)
                    prev_stack[i] = stack[i];
                prev_pos = pos;

                state_stack_top = next_stack_top;
                for (int k = pos + 1; k <= state_stack_top; k++)
                    stack[k] = next_stack[k];
                location_stack[state_stack_top] = Loc(current_token);
                pos = next_pos;
            }
        }

        //
        // At this stage, either we have an ACCEPT or an ERROR
        // action.
        //
        if (act == ERROR_ACTION)
        {
            //
            // An error was detected.
            //
            RepairCandidate candidate = ErrorRecovery(current_token);

            act = stack[state_stack_top];

            //
            // If the recovery was successful on a nonterminal candidate,
            // parse through that candidate and "read" the next token.
            //
            if (!candidate.symbol)
                break;
            else if (candidate.symbol > NT_OFFSET)
            {
                int lhs_symbol = candidate.symbol - NT_OFFSET;
                act = nt_action(act, lhs_symbol);
                while(act <= NUM_RULES)
                {
                    state_stack_top -= (rhs[act]-1);
                    act = nt_action(stack[state_stack_top], lhs[act]);
                }
                stack[++state_stack_top] = act;
                current_token = lex_stream -> Gettoken();
                current_kind = lex_stream -> Kind(current_token);
                location_stack[state_stack_top] = Loc(current_token);
            }
            else
            {
                current_kind = candidate.symbol;
                location_stack[state_stack_top] = candidate.location;
            }
        }
    } while (act != ACCEPT_ACTION);

    return;
}


//
//  This routine is invoked when an error is encountered.  It
// tries to diagnose the error and recover from it.  If it is
// successful, the state stack, the current token and the buffer
// are readjusted; i.e., after a successful recovery,
// state_stack_top points to the location in the state stack
// that contains the state on which to recover; current_token
// identifies the symbol on which to recover.
//
// Up to three configurations may be available when this routine
// is invoked. PREV_STACK may contain the sequence of states
// preceding any action on prevtok, STACK always contains the
// sequence of states preceding any action on current_token, and
// NEXT_STACK may contain the sequence of states preceding any
// action on the successor of current_token.
//
DiagnoseParser::RepairCandidate DiagnoseParser::ErrorRecovery(TokenObject error_token)
{
    TokenObject prevtok = lex_stream -> Previous(error_token);

    //
    // Try primary phase recoveries. If not successful, try secondary
    // phase recoveries.  If not successful and we are at end of the
    // file, we issue the end-of-file error and quit. Otherwise, ...
    //
    RepairCandidate candidate = PrimaryPhase(error_token);
    if (candidate.symbol)
        return candidate;

    candidate = SecondaryPhase(error_token);
    if (candidate.symbol)
        return candidate;

    if (lex_stream -> Kind(error_token) == EOFT_SYMBOL)
    {
        ReportError(EOF_CODE,
                    terminal_index[EOFT_SYMBOL],
                    Loc(prevtok),
                    prevtok);
        candidate.symbol = 0;
        candidate.location = Loc(error_token);
        return candidate;
    }

    //
    // At this point, primary and (initial attempt at) secondary
    // recovery did not work.  We will now get into "panic mode" and
    // keep trying secondary phase recoveries until we either find
    // a successful recovery or have consumed the remaining input
    // tokens.
    //
    while(lex_stream -> Kind(buffer[BUFF_UBOUND]) != EOFT_SYMBOL)
    {
        candidate = SecondaryPhase(buffer[MAX_DISTANCE - MIN_DISTANCE + 2]);
        if (candidate.symbol)
            return candidate;
    }

    //
    // We reached the end of the file while panicking. Delete all
    // remaining tokens in the input.
    //
    int i;
    for (i = BUFF_UBOUND; lex_stream -> Kind(buffer[i]) == EOFT_SYMBOL; i--)
        ;

    ReportError(DELETION_CODE,
                terminal_index[lex_stream -> Kind(prevtok)],
                Loc(error_token),
                buffer[i]);

    candidate.symbol = 0;
    candidate.location = Loc(buffer[i]);

    return candidate;
}


//
// This function tries primary and scope recovery on each
// available configuration.  If a successful recovery is found
// and no secondary phase recovery can do better, a diagnosis is
// issued, the configuration is updated and the function returns
// "true".  Otherwise, it returns "false".
//
DiagnoseParser::RepairCandidate DiagnoseParser::PrimaryPhase(TokenObject error_token)
{
    //
    // Initialize the buffer.
    //
    int i = (next_stack_top >= 0 ? 3 : 2);
    buffer[i] = error_token;

    for (int j = i; j > 0; j--)
        buffer[j - 1] = lex_stream -> Previous(buffer[j]);

    for (int k = i + 1; k < BUFF_SIZE; k++)
        buffer[k] = lex_stream -> Next(buffer[k - 1]);

    //
    // If NEXT_STACK_TOP > 0 then the parse was successful on CURRENT_TOKEN
    // and the error was detected on the successor of CURRENT_TOKEN. In
    // that case, first check whether or not primary recovery is
    // possible on next_stack ...
    //
    PrimaryRepairInfo repair;
    if (next_stack_top >= 0)
    {
        repair.buffer_position = 3;
        CheckPrimaryDistance(repair, next_stack, next_stack_top);
    }

    //
    // ... Next, try primary recovery on the current token...
    //
    PrimaryRepairInfo base_repair = repair;
    base_repair.buffer_position = 2;
    CheckPrimaryDistance(base_repair, stack, state_stack_top);
    if (base_repair.distance > repair.distance ||
        base_repair.misspell_index > repair.misspell_index)
        repair = base_repair;

    //
    // Finally, if prev_stack_top >= 0 then try primary recovery on
    // the prev_stack configuration.
    //
    if (prev_stack_top >= 0)
    {
        PrimaryRepairInfo prev_repair = repair;
        prev_repair.buffer_position = 1;
        CheckPrimaryDistance(prev_repair, prev_stack, prev_stack_top);
        if (prev_repair.distance > repair.distance ||
            prev_repair.misspell_index > repair.misspell_index)
            repair = prev_repair;
    }

    //
    // Before accepting the best primary phase recovery obtained,
    // ensure that we cannot do better with a similar secondary
    // phase recovery.
    //
    RepairCandidate candidate;
    if (next_stack_top >= 0) // next_stack available
    {
        if (SecondaryCheck(next_stack,next_stack_top,3,repair.distance))
              return candidate;
    }
    else if (SecondaryCheck(stack, state_stack_top, 2, repair.distance))
             return candidate;

    //
    // First, adjust distance if the recovery is on the error token;
    // it is important that the adjustment be made here and not at
    // each primary trial to prevent the distance tests from being
    // biased in favor of deferred recoveries which have access to
    // more input tokens...
    //
    repair.distance = repair.distance - repair.buffer_position + 1;

    //
    // ...Next, adjust the distance if the recovery is a deletion or
    // (some form of) substitution...
    //
    if (repair.code == INVALID_CODE      ||
        repair.code == DELETION_CODE     ||
        repair.code == SUBSTITUTION_CODE ||
        repair.code == MERGE_CODE)
         repair.distance--;

    //
    // ... After adjustment, check if the most successful primary
    // recovery can be applied.  If not, continue with more radical
    // recoveries...
    //
    if (repair.distance < MIN_DISTANCE)
        return candidate;

    //
    // When processing an insertion error, if the token preceeding
    // the error token is not available, we change the repair code
    // into a BEFORE_CODE to instruct the reporting routine that it
    // indicates that the repair symbol should be inserted before
    // the error token.
    //
    if (repair.code == INSERTION_CODE)
    {
        if (! lex_stream -> Kind(buffer[repair.buffer_position - 1]))
            repair.code = BEFORE_CODE;
    }

    //
    // Select the proper sequence of states on which to recover,
    // update stack accordingly and call diagnostic routine.
    //
    if (repair.buffer_position == 1)
    {
        state_stack_top = prev_stack_top;
        for (int i = 0; i <= state_stack_top; i++)
            stack[i] = prev_stack[i];
    }
    else if (next_stack_top >= 0 && repair.buffer_position >= 3)
    {
        state_stack_top = next_stack_top;
        for (int i = 0; i <= state_stack_top; i++)
            stack[i] = next_stack[i];
        location_stack[state_stack_top] = Loc(buffer[3]);
    }

    return PrimaryDiagnosis(repair);
}


//
//     This function checks whether or not a given state has a
// candidate, whose string representaion is a merging of the two
// tokens at positions buffer_position and buffer_position+1 in
// the buffer.  If so, it returns the candidate in question;
// otherwise it returns 0.
//
int DiagnoseParser::MergeCandidate(int state, int buffer_position)
{
    int len1 = lex_stream -> NameStringLength(buffer[buffer_position]),
        len2 = lex_stream -> NameStringLength(buffer[buffer_position + 1]),
        len  = len1 + len2;

    char *str = new char[len + 1];

    strncpy(str, lex_stream -> NameString(buffer[buffer_position]), len1);
    strncpy(&(str[len1]), lex_stream -> NameString(buffer[buffer_position + 1]), len2);
    str[len] = '\0';

    for (int k = asi(state); asr[k] != 0; k++)
    {
        int l = terminal_index[asr[k]];

        if (len == NameLength(l))
        {
            const char *name = &string_buffer[name_start[l]];

            char *p = &str[0];
            while (*p != '\0')
            {
                char c = *(name++);

                if (Code::ToLower(*p) != Code::ToLower(c))
                    break;
                p++;
            }
            if (*p == '\0')
            {
                delete [] str;
                return asr[k];
            }
        }
    }

    delete [] str;

    return 0;
}


//
// This procedure takes as arguments a parsing configuration
// consisting of a state stack (stack and stack_top) and a fixed
// number of input tokens (starting at buffer_position) in the
// input BUFFER; and some reference arguments: repair_code,
// distance, misspell_index, candidate, and stack_position
// which it sets based on the best possible recovery that it
// finds in the given configuration.  The effectiveness of a
// a repair is judged based on two criteria:
//
//   1) the number of tokens that can be parsed after the repair
//      is applied: distance.
//   2) how close to perfection is the candidate that is chosen:
//      misspell_index.
// When this procedure is entered, distance, misspell_index and
// repair_code are assumed to be initialized.
//
void DiagnoseParser::CheckPrimaryDistance(PrimaryRepairInfo &repair, Array<int> &stck, int stack_top)
{
    //
    //  First, try scope and manual recovery.
    //
    PrimaryRepairInfo scope_repair = repair;
    ScopeTrial(scope_repair, stck, stack_top);
    if (scope_repair.distance > repair.distance)
        repair = scope_repair;

    //
    //  Next, try merging the error token with its successor.
    //
    int symbol = MergeCandidate(stck[stack_top], repair.buffer_position);
    if (symbol != 0)
    {
        int j = ParseCheck(stck, stack_top,
                           symbol, repair.buffer_position+2);
        if ((j > repair.distance) ||
            (j == repair.distance && repair.misspell_index < 10))
        {
            repair.misspell_index = 10;
            repair.symbol = symbol;
            repair.distance = j;
            repair.code = MERGE_CODE;
        }
    }

    //
    // Next, try deletion of the error token.
    //
    int j = ParseCheck(stck, stack_top,
                       lex_stream -> Kind(buffer[repair.buffer_position+1]),
                   repair.buffer_position+2);
    int k = (lex_stream -> Kind(buffer[repair.buffer_position]) == EOLT_SYMBOL &&
             lex_stream -> AfterEol(buffer[repair.buffer_position+1])
                         ? 10
                         : 0);
    if (j > repair.distance || (j == repair.distance && k > repair.misspell_index))
    {
        repair.misspell_index = k;
        repair.code = DELETION_CODE;
        repair.distance = j;
    }

    //
    // Update the error configuration by simulating all reduce and
    // goto actions induced by the error token. Then assign the top
    // most state of the new configuration to next_state.
    //
    int next_state = stck[stack_top],
        max_pos = stack_top;
    temp_stack_top = stack_top - 1;

    lex_stream -> Reset(buffer[repair.buffer_position + 1]);
    int tok = lex_stream -> Kind(buffer[repair.buffer_position]),
        act = t_action(next_state, tok, lex_stream);
    while(act <= NUM_RULES)
    {
        do
        {
            int lhs_symbol = lhs[act];
            temp_stack_top -= (rhs[act]-1);
            act = (temp_stack_top > max_pos
                                  ? temp_stack[temp_stack_top]
                                  : stck[temp_stack_top]);
            act = nt_action(act, lhs_symbol);
        }   while(act <= NUM_RULES);
        max_pos = Util::Min(max_pos, temp_stack_top);
        temp_stack[temp_stack_top + 1] = act;
        next_state = act;
        act = t_action(next_state, tok, lex_stream);
    }

    //
    //  Next, place the list of candidates in proper order.
    //
    int root = 0;
    for (int i = asi(next_state); asr[i] != 0; i++)
    {
        symbol = asr[i];
        if (symbol != EOFT_SYMBOL && symbol != ERROR_SYMBOL)
        {
            if (root == 0)
                list[symbol] = symbol;
            else
            {
                list[symbol] = list[root];
                list[root] = symbol;
            }
            root = symbol;
        }
    }

    if (stck[stack_top] != next_state)
    {
        for (int i = asi(stck[stack_top]); asr[i] != 0; i++)
        {
            symbol = asr[i];
            if (symbol != EOFT_SYMBOL && symbol != ERROR_SYMBOL &&
                                         list[symbol] == 0)
            {
                if (root == 0)
                    list[symbol] = symbol;
                else
                {
                    list[symbol] = list[root];
                    list[root] = symbol;
                }
                root = symbol;
            }
        }
    }

    int head = list[root];
    list[root] = 0;
    root = head;

    //
    //  Next, try insertion for each possible candidate available in
    // the current state, except EOFT and ERROR_SYMBOL.
    //
    symbol = root;
    while(symbol != 0)
    {
        int m = ParseCheck(stck, stack_top, symbol, repair.buffer_position),
            n = (symbol == EOLT_SYMBOL && lex_stream -> AfterEol(buffer[repair.buffer_position])
                         ? 10
                         : 0);
        if (m > repair.distance ||
            (m == repair.distance && n > repair.misspell_index))
        {
            repair.misspell_index = n;
            repair.distance = m;
            repair.symbol = symbol;
            repair.code = INSERTION_CODE;
        }

        symbol = list[symbol];
    }

    //
    //  Next, Try substitution for each possible candidate available
    // in the current state, except EOFT and ERROR_SYMBOL.
    //
    symbol = root;
    while(symbol != 0)
    {
        int m = ParseCheck(stck, stack_top, symbol, repair.buffer_position + 1),
            n = (symbol == EOLT_SYMBOL && lex_stream -> AfterEol(buffer[repair.buffer_position+1])
                         ? 10
                         : Misspell(symbol, buffer[repair.buffer_position]));
        if (m > repair.distance ||
            (m == repair.distance && n > repair.misspell_index))
        {
            repair.misspell_index = k;
            repair.distance = m;
            repair.symbol = symbol;
            repair.code = SUBSTITUTION_CODE;
        }

        int s = symbol;
        symbol = list[symbol];
        list[s] = 0; // reset element
    }

    //
    // Next, we try to insert a nonterminal candidate in front of the
    // error token, or substituting a nonterminal candidate for the
    // error token. Precedence is given to insertion.
    //
     for (int nt_index = nasi(stck[stack_top]); nasr[nt_index] != 0; nt_index++)
     {
         symbol = nasr[nt_index] + NT_OFFSET;
         int n = ParseCheck(stck, stack_top, symbol, repair.buffer_position+1);
         if (n > repair.distance)
         {
             repair.misspell_index = 0;
             repair.distance = n;
             repair.symbol = symbol;
             repair.code = INVALID_CODE;
         }

         n = ParseCheck(stck, stack_top, symbol, repair.buffer_position);
         if (n > repair.distance || (n == repair.distance && repair.code == INVALID_CODE))
         {
             repair.misspell_index = 0;
             repair.distance = n;
             repair.symbol = symbol;
             repair.code = INSERTION_CODE;
         }
     }

    return;
}


//
// This procedure is invoked to issue a diagnostic message and
// adjust the input buffer.  The recovery in question is either
// the insertion of one or more scopes, the merging of the error
// token with its successor, the deletion of the error token,
// the insertion of a single token in front of the error token
// or the substitution of another token for the error token.
//
DiagnoseParser::RepairCandidate DiagnoseParser::PrimaryDiagnosis(PrimaryRepairInfo &repair)
{
    //
    //  Issue diagnostic.
    //
    TokenObject prevtok = buffer[repair.buffer_position - 1],
                current_token  = buffer[repair.buffer_position];

    switch(repair.code)
    {
        case INSERTION_CODE: case BEFORE_CODE:
        {
            int name_index = (repair.symbol > NT_OFFSET)
                                            ? GetNtermIndex(stack[state_stack_top],
                                                            repair.symbol,
                                                            repair.buffer_position)
                                            : GetTermIndex(stack,
                                                           state_stack_top,
                                                           repair.symbol,
                                                           repair.buffer_position);
            TokenObject t = (repair.code == INSERTION_CODE ? prevtok : current_token);
            ReportError(repair.code, name_index, Loc(t), t);
            break;
        }
        case INVALID_CODE:
        {
            int name_index = GetNtermIndex(stack[state_stack_top],
                                           repair.symbol,
                                           repair.buffer_position + 1);
            ReportError(repair.code, name_index, Loc(current_token), current_token);
            break;
        }
        case SUBSTITUTION_CODE:
        {
            int name_index;
            if (repair.misspell_index >= 6)
                name_index = terminal_index[repair.symbol];
            else
            {
                name_index = GetTermIndex(stack, state_stack_top,
                                          repair.symbol,
                                          repair.buffer_position + 1);
                if (name_index != terminal_index[repair.symbol])
                    repair.code = INVALID_CODE;
            }
            ReportError(repair.code, name_index, Loc(current_token), current_token);
            break;
        }
        case MERGE_CODE:
        {
            ReportError(repair.code,
                         terminal_index[repair.symbol],
                         Loc(current_token),
                         lex_stream -> Next(current_token));
            break;
        }
        case SCOPE_CODE:
        {
            for (int i = 0; i < scope_stack_top; i++)
            {
                ReportError(repair.code,
                            -scope_index[i],
                            location_stack[scope_position[i]],
                            prevtok,
                            non_terminal_index[scope_lhs[scope_index[i]]]);
            }

            repair.symbol = scope_lhs[scope_index[scope_stack_top]] + NT_OFFSET;
            state_stack_top = scope_position[scope_stack_top];
            ReportError(repair.code,
                        -scope_index[scope_stack_top],
                        location_stack[scope_position[scope_stack_top]],
                        prevtok,
                        GetNtermIndex(stack[state_stack_top],
                                      repair.symbol,
                                      repair.buffer_position)
                       );
            break;
        }
        default: // deletion
        {
            ReportError(repair.code,
                        terminal_index[ERROR_SYMBOL],
                        Loc(current_token),
                        current_token);
        }
    }

    //
    //  Update buffer.
    //
    RepairCandidate candidate;
    switch (repair.code)
    {
        case INSERTION_CODE: case BEFORE_CODE: case SCOPE_CODE:
        {
            candidate.symbol = repair.symbol;
            candidate.location = Loc(buffer[repair.buffer_position]);
            lex_stream -> Reset(buffer[repair.buffer_position]);

            break;
        }

        case INVALID_CODE: case SUBSTITUTION_CODE:
        {
            candidate.symbol = repair.symbol;
            candidate.location = Loc(buffer[repair.buffer_position]);
            lex_stream -> Reset(buffer[repair.buffer_position + 1]);

            break;
        }

        case MERGE_CODE:
        {
            candidate.symbol = repair.symbol;
            candidate.location = Loc(buffer[repair.buffer_position]);
            lex_stream -> Reset(buffer[repair.buffer_position + 2]);

            break;
        }

        default: // deletion
        {
            candidate.location = Loc(buffer[repair.buffer_position + 1]);
            candidate.symbol =
                      lex_stream -> Kind(buffer[repair.buffer_position + 1]);
            lex_stream -> Reset(buffer[repair.buffer_position + 2]);

            break;
        }
    }

    return candidate;
}


//
// This function takes as parameter an integer STACK_TOP that
// points to a STACK element containing the state on which a
// primary recovery will be made; the terminal candidate on which
// to recover; and an integer: buffer_position, which points to
// the position of the next input token in the BUFFER.  The
// parser is simulated until a shift (or shift-reduce) action
// is computed on the candidate.  Then we proceed to compute the
// the name index of the highest level nonterminal that can
// directly or indirectly produce the candidate.
//
int DiagnoseParser::GetTermIndex(Array<int> &stck, int stack_top,
                                 int tok, int buffer_position)
{
    //
    // Initialize stack index of temp_stack and initialize maximum
    // position of state stack that is still useful.
    //
    int act = stck[stack_top],
        max_pos = stack_top,
        highest_symbol = tok;

    temp_stack_top = stack_top - 1;

    //
    // Compute all reduce and associated actions induced by the
    // candidate until a SHIFT or SHIFT-REDUCE is computed. ERROR
    // and ACCEPT actions cannot be computed on the candidate in
    // this context, since we know that it is suitable for recovery.
    //
    lex_stream -> Reset(buffer[buffer_position]);
    act = t_action(act, tok, lex_stream);
    while(act <= NUM_RULES)
    {
        //
        // Process all goto-reduce actions following reduction,
        // until a goto action is computed ...
        //
        do
        {
            int lhs_symbol = lhs[act];
            temp_stack_top -= (rhs[act]-1);
            act = (temp_stack_top > max_pos
                                  ? temp_stack[temp_stack_top]
                                  : stck[temp_stack_top]);
            act = nt_action(act, lhs_symbol);
        } while(act <= NUM_RULES);

        //
        // Compute new maximum useful position of (STATE_)stack,
        // push goto state into the stack, and compute next
        // action on candidate ...
        //
        max_pos = Util::Min(max_pos, temp_stack_top);
        temp_stack[temp_stack_top + 1] = act;
        act = t_action(act, tok, lex_stream);
    }

    //
    // At this stage, we have simulated all actions induced by the
    // candidate and we are ready to shift or shift-reduce it. First,
    // set tok and next_ptr appropriately and identify the candidate
    // as the initial highest_symbol. If a shift action was computed
    // on the candidate, update the stack and compute the next
    // action. Next, simulate all actions possible on the next input
    // token until we either have to shift it or are about to reduce
    // below the initial starting point in the stack (indicated by
    // max_pos as computed in the previous loop).  At that point,
    // return the highest_symbol computed.
    //
    temp_stack_top++; // adjust top of stack to reflect last goto
                      // next move is shift or shift-reduce.
    int threshold = temp_stack_top;

    tok = lex_stream -> Kind(buffer[buffer_position]);
    lex_stream -> Reset(buffer[buffer_position + 1]);

    if (act > ERROR_ACTION)   // shift-reduce on candidate?
        act -= ERROR_ACTION;
    else if (act < ACCEPT_ACTION) // shift on candidate !
    {
        temp_stack[temp_stack_top + 1] = act;
        act = t_action(act, tok, lex_stream);
    }

    while(act <= NUM_RULES)
    {
        //
        // Process all goto-reduce actions following reduction,
        // until a goto action is computed ...
        //
        do
        {
            int lhs_symbol = lhs[act];
            temp_stack_top -= (rhs[act]-1);

            if (temp_stack_top < threshold)
                return (highest_symbol > NT_OFFSET
                               ? non_terminal_index[highest_symbol - NT_OFFSET]
                               : terminal_index[highest_symbol]);

            if (temp_stack_top == threshold)
                highest_symbol = lhs_symbol + NT_OFFSET;
            act = (temp_stack_top > max_pos
                                  ? temp_stack[temp_stack_top]
                                  : stck[temp_stack_top]);
            act = nt_action(act, lhs_symbol);
        } while(act <= NUM_RULES);

        temp_stack[temp_stack_top + 1] = act;
        act = t_action(act, tok, lex_stream);
    }

    //
    // Quit:
    //
    return (highest_symbol > NT_OFFSET
                         ? non_terminal_index[highest_symbol - NT_OFFSET]
                         : terminal_index[highest_symbol]);
}

//
// This function takes as parameter a starting state number:
// start, a nonterminal symbol, A (candidate), and an integer,
// buffer_position,  which points to the position of the next
// input token in the BUFFER.
// It returns the highest level non-terminal B such that
// B =>*rm A.  I.e., there does not exists a nonterminal C such
// that C =>+rm B. (Recall that for an LALR(k) grammar if
// C =>+rm B, it cannot be the case that B =>+rm C)
//
int DiagnoseParser::GetNtermIndex(int start, int sym, int buffer_position)
{
    int highest_symbol = sym - NT_OFFSET,
        tok = lex_stream -> Kind(buffer[buffer_position]);
    lex_stream -> Reset(buffer[buffer_position + 1]);

    //
    // Initialize stack index of temp_stack and initialize maximum
    // position of state stack that is still useful.
    //
    temp_stack_top = 0;
    temp_stack[temp_stack_top] = start;

    int act = nt_action(start, highest_symbol);
    if (act > NUM_RULES) // goto action?
    {
        temp_stack[temp_stack_top + 1] = act;
        act = t_action(act, tok, lex_stream);
    }

    while(act <= NUM_RULES)
    {
        //
        // Process all goto-reduce actions following reduction,
        // until a goto action is computed ...
        //
        do
        {
            temp_stack_top -= (rhs[act]-1);
            if (temp_stack_top < 0)
                return non_terminal_index[highest_symbol];
            if (temp_stack_top == 0)
                highest_symbol = lhs[act];
            act = nt_action(temp_stack[temp_stack_top], lhs[act]);
        } while(act <= NUM_RULES);
        temp_stack[temp_stack_top + 1] = act;
        act = t_action(act, tok, lex_stream);
    }

    return non_terminal_index[highest_symbol];
}


//
//     Check whether or not there is a high probability that a
// given string is a misspelling of another.
// Certain singleton symbols (such as ":" and ";") are also
// considered to be misspelling of each other.
//
int DiagnoseParser::Misspell(int sym, TokenObject tok)
{
    int len = lex_stream -> NameStringLength(tok);

    //
    //
    //
    int n = NameLength(terminal_index[sym]),
        i = name_start[terminal_index[sym]];
    char *s1 = new char[n + 1];
    for (int l = 0; l < n; l++)
    {
        char c = string_buffer[i++];
        s1[l] = Code::ToLower(c);
    }
    s1[n] = '\0';

    //
    //
    //
    int m = Util::Min(len, MAX_TERM_LENGTH);
    char *s2 = new char[m + 1];
    for (int k = 0; k < m; k++)
    {
        char c = (char) lex_stream -> NameString(tok)[k];
        s2[k] = Code::ToLower(c);
    }
    s2[m] = '\0';

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
    //
    if (n == 1  &&  m == 1)
    {
        if ((s1[0] == ';'  &&  s2[0] == ',')  ||
            (s1[0] == ','  &&  s2[0] == ';')  ||
            (s1[0] == ';'  &&  s2[0] == ':')  ||
            (s1[0] == ':'  &&  s2[0] == ';')  ||
            (s1[0] == '.'  &&  s2[0] == ',')  ||
            (s1[0] == ','  &&  s2[0] == '.')  ||
            (s1[0] == '\'' &&  s2[0] == '\"')  ||
            (s1[0] == '\"'  &&  s2[0] == '\''))
        {
                delete [] s1;
                delete [] s2;

                return 3;
        }
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
        num_errors = 0;

    i = 0;
    int j = 0;
    while ((i < n)  &&  (j < m))
    {
        if (s1[i] == s2[j])
        {
            count++;
            i++;
            j++;
            if (num_errors == 0)
                prefix_length++;
        }
        else if (s1[i+1] == s2[j]  &&  s1[i] == s2[j+1])
        {
            count += 2;
            i += 2;
            j += 2;
            num_errors++;
        }
        else if (s1[i+1] == s2[j+1])
        {
            i++;
            j++;
            num_errors++;
        }
        else
        {
            if ((n - i) > (m - j))
                 i++;
            else if ((m - j) > (n - i))
                 j++;
            else
            {
                i++;
                j++;
            }
            num_errors++;
        }
    }

    if (i < n  ||  j < m)
        num_errors++;

    if (num_errors > (Util::Min(n, m) / 6 + 1))
         count = prefix_length;

    delete [] s1;
    delete [] s2;

    return(count * 10 / (Util::Max(n, len) + num_errors));
}

//
//
//
void DiagnoseParser::ScopeTrial(PrimaryRepairInfo &repair, Array<int> &stck, int stack_top)
{
    state_seen.Resize(stack.Size());
    state_seen.Initialize(NIL);

    state_pool.Reset();

    ScopeTrialCheck(repair, stck, stack_top, 0);

    repair.code = SCOPE_CODE;
    repair.misspell_index = 10;

    return;
}


//
// SCOPE_TRIAL_CHECK is a recursive procedure that takes as arguments
// a state configuration: STACK and STACK_TOP, and an integer:
// INDX. In addition, a global variable SCOPE_DISTANCE indicates
// the distance to "beat" and SCOPE_BUFFER_POSITION identifies
// the starting position of the next input tokens in BUFFER.
// SCOPE_TRIAL determines whether or not scope recovery is
// possible.  If so, it uses two global arrays: SCOPE_INDEX and
// SCOPE_LOCATION to store the necessary information about the
// scopes to be closed. A global variable, scope_stack_top, identifies
// the highest element used in these arrays.
// The global variable SCOPE_DISTANCE is also updated to reflect
// the new result.
//
void DiagnoseParser::ScopeTrialCheck(PrimaryRepairInfo &repair, Array<int> &stck, int stack_top, int indx)
{
    int act = stck[stack_top];

    for (int j = state_seen[stack_top]; j != NIL; j = state_pool[j].next)
    {
        if (state_pool[j].state == act)
            return;
    }

    int k = state_pool.NextIndex();
    state_pool[k].state = act;
    state_pool[k].next  = state_seen[stack_top];
    state_seen[stack_top] = k;

    for (int i = 0; i < SCOPE_SIZE; i++)
    {
        //
        // Use the scope lookahead symbol to force all reductions
        // inducible by that symbol.
        //
        int act = stck[stack_top];
        temp_stack_top = stack_top - 1;
        int max_pos = stack_top,
            tok = scope_la[i];
        lex_stream -> Reset(buffer[repair.buffer_position]);
        act = t_action(act, tok, lex_stream);
        while(act <= NUM_RULES)
        {
            //
            // ... Process all goto-reduce actions following
            // reduction, until a goto action is computed ...
            //
            do
            {
                int lhs_symbol = lhs[act];
                temp_stack_top -= (rhs[act]-1);
                act =  (temp_stack_top > max_pos
                            ? temp_stack[temp_stack_top]
                            : stck[temp_stack_top]);
                act = nt_action(act, lhs_symbol);
            }  while(act <= NUM_RULES);
            if (temp_stack_top + 1 >= stack.Size())
                return;
            max_pos = Util::Min(max_pos, temp_stack_top);
            temp_stack[temp_stack_top + 1] = act;
            act = t_action(act, tok, lex_stream);
        }

        //
        // If the lookahead symbol is parsable, then we check
        // whether or not we have a match between the scope
        // prefix and the transition symbols corresponding to
        // the states on top of the stack.
        //
        if (act != ERROR_ACTION)
        {
            int j,
                k = scope_prefix[i];
            for (j = temp_stack_top + 1;
                 j >= (max_pos + 1) &&
                 in_symbol(temp_stack[j]) == scope_rhs[k]; j--)
                 k++;
            if (j == max_pos)
            {
                for (j = max_pos;
                     j >= 1 && in_symbol(stck[j]) == scope_rhs[k];
                     j--)
                    k++;
            }
            //
            // If the prefix matches, check whether the state
            // newly exposed on top of the stack, (after the
            // corresponding prefix states are popped from the
            // stack), is in the set of "source states" for the
            // scope in question and that it is at a position
            // below the threshold indicated by MARKED_POS.
            //
            int marked_pos = (max_pos < stack_top ? max_pos + 1 : stack_top);
            if (scope_rhs[k] == 0 && j < marked_pos) // match?
            {
                int stack_position = j;
                for (j = scope_state_set[i];
                     stck[stack_position] != scope_state[j] &&
                     scope_state[j] != 0;
                     j++)
                    ;
                //
                // If the top state is valid for scope recovery,
                // the left-hand side of the scope is used as
                // starting symbol and we calculate how far the
                // parser can advance within the forward context
                // after parsing the left-hand symbol.
                //
                if (scope_state[j] != 0)      // state was found
                {
                    int previous_distance = repair.distance,
                        distance = ParseCheck(stck,
                                              stack_position,
                                              scope_lhs[i]+NT_OFFSET,
                                              repair.buffer_position);
                    //
                    // if the recovery is not successful, we
                    // update the stack with all actions induced
                    // by the left-hand symbol, and recursively
                    // call SCOPE_TRIAL_CHECK to try again.
                    // Otherwise, the recovery is successful. If
                    // the new distance is greater than the
                    // initial SCOPE_DISTANCE, we update
                    // SCOPE_DISTANCE and set scope_stack_top to INDX
                    // to indicate the number of scopes that are
                    // to be applied for a succesful  recovery.
                    // NOTE that this procedure cannot get into
                    // an infinite loop, since each prefix match
                    // is guaranteed to take us to a lower point
                    // within the stack.
                    //
                    if ((distance - repair.buffer_position + 1) < MIN_DISTANCE)
                    {
                        int top = stack_position,
                            act = nt_action(stck[top], scope_lhs[i]);
                        while(act <= NUM_RULES)
                        {
                            top -= (rhs[act]-1);
                            act = nt_action(stck[top], lhs[act]);
                        }
                        top++;

                        j = act;
                        act = stck[top];  // save
                        stck[top] = j;    // swap
                        ScopeTrialCheck(repair, stck, top, indx+1);
                        stck[top] = act; // restore
                    }
                    else if (distance > repair.distance)
                    {
                        scope_stack_top = indx;
                        repair.distance = distance;
                    }

                    if (lex_stream -> Kind(buffer[repair.buffer_position]) == EOFT_SYMBOL &&
                        repair.distance == previous_distance)
                    {
                        scope_stack_top = indx;
                        repair.distance = MAX_DISTANCE;
                    }

                    //
                    // If this scope recovery has beaten the
                    // previous distance, then we have found a
                    // better recovery (or this recovery is one
                    // of a list of scope recoveries). Record
                    // its information at the proper location
                    // (INDX) in SCOPE_INDEX and SCOPE_STACK.
                    //
                    if (repair.distance > previous_distance)
                    {
                        scope_index[indx] = i;
                        scope_position[indx] = stack_position;
                        return;
                    }
                }
            }
        }
    }

    return;
}

//
// This function computes the ParseCheck distance for the best
// possible secondary recovery for a given configuration that
// either deletes none or only one symbol in the forward context.
// If the recovery found is more effective than the best primary
// recovery previously computed, then the function returns true.
// Only misplacement, scope and manual recoveries are attempted;
// simple insertion or substitution of a nonterminal are tried
// in CHECK_PRIMARY_DISTANCE as part of primary recovery.
//
bool DiagnoseParser::SecondaryCheck(Array<int> &stck, int stack_top, int buffer_position, int distance)
{
    for (int top = stack_top - 1; top >= 0; top--)
    {
        int j = ParseCheck(stck,
                           top,
                           lex_stream -> Kind(buffer[buffer_position]),
                           buffer_position + 1);
        if (((j - buffer_position + 1) > MIN_DISTANCE) &&
            (j > distance))
            return true;
    }

    PrimaryRepairInfo scope_repair;
    scope_repair.buffer_position = buffer_position + 1;
    scope_repair.distance = distance;
    ScopeTrial(scope_repair, stck, stack_top);

    return ((scope_repair.distance - buffer_position) > MIN_DISTANCE && scope_repair.distance > distance);
}


//
// Secondary_phase is a boolean function that checks whether or
// not some form of secondary recovery is applicable to one of
// the error configurations. First, if "next_stack" is available,
// misplacement and secondary recoveries are attempted on it.
// Then, in any case, these recoveries are attempted on "stack".
// If a successful recovery is found, a diagnosis is issued, the
// configuration is updated and the function returns "true".
// Otherwise, the function returns false.
//
DiagnoseParser::RepairCandidate DiagnoseParser::SecondaryPhase(TokenObject error_token)
{
    SecondaryRepairInfo repair,
                        misplaced_repair;

    //
    // If the next_stack is available, try misplaced and secondary
    // recovery on it first.
    //
    int next_last_index = 0;
    if (next_stack_top >= 0)
    {
        Location  save_location;

        buffer[2] = error_token;
        buffer[1] = lex_stream -> Previous(buffer[2]);
        buffer[0] = lex_stream -> Previous(buffer[1]);

        for (int k = 3; k < BUFF_UBOUND; k++)
            buffer[k] = lex_stream -> Next(buffer[k - 1]);

        buffer[BUFF_UBOUND] = lex_stream -> Badtoken();// elmt not available

        //
        // If we are at the end of the input stream, compute the
        // index position of the first EOFT symbol (last useful
        // index).
        //
        for (next_last_index = MAX_DISTANCE - 1;
             next_last_index >= 1 &&
             lex_stream -> Kind(buffer[next_last_index]) == EOFT_SYMBOL;
             next_last_index--)
            ;
        next_last_index = next_last_index + 1;

        save_location = location_stack[next_stack_top];
        location_stack[next_stack_top] = Loc(buffer[2]);
        misplaced_repair.num_deletions = next_stack_top;
        MisplacementRecovery(misplaced_repair, next_stack, next_stack_top, next_last_index, true);
        if (misplaced_repair.recovery_on_next_stack)
            misplaced_repair.distance++;

        repair.num_deletions = next_stack_top + BUFF_UBOUND;
        SecondaryRecovery(repair,
                          next_stack, next_stack_top,
                          next_last_index, true);
        if (repair.recovery_on_next_stack)
            repair.distance++;

        location_stack[next_stack_top] = save_location;
    }
    else             // next_stack not available, initialize ...
    {
        misplaced_repair.num_deletions = state_stack_top;
        repair.num_deletions = state_stack_top + BUFF_UBOUND;
    }

    //
    // Try secondary recovery on the "stack" configuration.
    //
    buffer[3] = error_token;

    buffer[2] = lex_stream -> Previous(buffer[3]);
    buffer[1] = lex_stream -> Previous(buffer[2]);
    buffer[0] = lex_stream -> Previous(buffer[1]);

    for (int k = 4; k < BUFF_SIZE; k++)
        buffer[k] = lex_stream -> Next(buffer[k - 1]);

    int last_index;
    for (last_index = MAX_DISTANCE - 1;
         last_index >= 1 && lex_stream -> Kind(buffer[last_index]) == EOFT_SYMBOL;
         last_index--)
        ;
    last_index++;

    MisplacementRecovery(misplaced_repair, stack, state_stack_top, last_index, false);

    SecondaryRecovery(repair, stack, state_stack_top, last_index, false);

    //
    // If a successful misplaced recovery was found, compare it with
    // the most successful secondary recovery.  If the misplaced
    // recovery either deletes fewer symbols or parse-checks further
    // then it is chosen.
    //
    if (misplaced_repair.distance > MIN_DISTANCE)
    {
        if (misplaced_repair.num_deletions <= repair.num_deletions ||
            (misplaced_repair.distance - misplaced_repair.num_deletions) >=
            (repair.distance - repair.num_deletions))
        {
            repair.code = MISPLACED_CODE;
            repair.stack_position = misplaced_repair.stack_position;
            repair.buffer_position = 2;
            repair.num_deletions = misplaced_repair.num_deletions;
            repair.distance = misplaced_repair.distance;
            repair.recovery_on_next_stack = misplaced_repair.recovery_on_next_stack;
        }
    }

    //
    // If the successful recovery was on next_stack, update: stack,
    // buffer, location_stack and last_index.
    //
    if (repair.recovery_on_next_stack)
    {
        state_stack_top = next_stack_top;
        for (int i = 0; i <= state_stack_top; i++)
            stack[i] = next_stack[i];

        buffer[2] = error_token;
        buffer[1] = lex_stream -> Previous(buffer[2]);
        buffer[0] = lex_stream -> Previous(buffer[1]);

        for (int k = 3; k < BUFF_UBOUND; k++)
            buffer[k] = lex_stream -> Next(buffer[k - 1]);

        buffer[BUFF_UBOUND] = lex_stream -> Badtoken();// elmt not available

        location_stack[next_stack_top] = Loc(buffer[2]);
        last_index = next_last_index;
    }

    //
    // Next, try scope recoveries after deletion of one, two, three,
    // four ... buffer_position tokens from the input stream.
    //
    if (repair.code == SECONDARY_CODE || repair.code == DELETION_CODE)
    {
        PrimaryRepairInfo scope_repair;
        for (scope_repair.buffer_position = 2;
             scope_repair.buffer_position <= repair.buffer_position &&
             repair.code != SCOPE_CODE; scope_repair.buffer_position++)
        {
            ScopeTrial(scope_repair, stack, state_stack_top);
            int j = (scope_repair.distance == MAX_DISTANCE
                                            ? last_index
                                            : scope_repair.distance),
                k = scope_repair.buffer_position - 1;
            if ((j - k) > MIN_DISTANCE &&
                (j - k) > (repair.distance - repair.num_deletions))
            {
                int i = scope_index[scope_stack_top];       // upper bound
                repair.code = SCOPE_CODE;
                repair.symbol = scope_lhs[i] + NT_OFFSET;
                repair.stack_position = state_stack_top;
                repair.buffer_position = scope_repair.buffer_position;
            }
        }
    }

    //
    // If no successful recovery is found and we have reached the
    // end of the file, check whether or not scope recovery is
    // applicable at the end of the file after discarding some
    // states.
    //
    if (repair.code == 0 && lex_stream -> Kind(buffer[last_index]) == EOFT_SYMBOL)
    {
        PrimaryRepairInfo scope_repair;
        scope_repair.buffer_position = last_index;
        for (int top = state_stack_top; top >= 0 && repair.code == 0; top--)
        {
            ScopeTrial(scope_repair, stack, top);
            if (scope_repair.distance > 0)
            {
                int i = scope_index[scope_stack_top];    // upper bound
                repair.code = SCOPE_CODE;
                repair.symbol = scope_lhs[i] + NT_OFFSET;
                repair.stack_position = top;
                repair.buffer_position = scope_repair.buffer_position;
            }
        }
    }

    //
    // If a successful repair was not found, quit!  Otherwise, issue
    // diagnosis and adjust configuration...
    //
    RepairCandidate candidate;    
    if (repair.code == 0)
        return candidate;

    SecondaryDiagnosis(repair);

    //
    // Update buffer based on number of elements that are deleted.
    //
    switch(repair.code)
    {
        case MANUAL_CODE: case MISPLACED_CODE:
             candidate.location = Loc(buffer[2]);
             candidate.symbol = lex_stream -> Kind(buffer[2]);
             lex_stream -> Reset(lex_stream -> Next(buffer[2]));

             break;

        case DELETION_CODE:
             candidate.location = Loc(buffer[repair.buffer_position]);
             candidate.symbol =
                       lex_stream -> Kind(buffer[repair.buffer_position]);
             lex_stream -> Reset(lex_stream -> Next(buffer[repair.buffer_position]));

             break;

        default: // SCOPE_CODE || SECONDARY_CODE
             candidate.symbol = repair.symbol;
             candidate.location = Loc(buffer[repair.buffer_position]);
             lex_stream -> Reset(buffer[repair.buffer_position]);

             break;
    }

    return candidate;
}


//
// This boolean function checks whether or not a given
// configuration yields a better misplacement recovery than
// the best misplacement recovery computed previously.
//
void DiagnoseParser::MisplacementRecovery(SecondaryRepairInfo &repair,  Array<int> &stck, int stack_top, int last_index, bool stack_flag)
{
    Location previous_loc = Loc(buffer[2]);
    int stack_deletions = 0;

    for (int top = stack_top - 1; top >= 0; top--)
    {
        if (location_stack[top] < previous_loc)
            stack_deletions++;
        previous_loc = location_stack[top];

        int j = ParseCheck(stck, top, lex_stream -> Kind(buffer[2]), 3);
        if (j == MAX_DISTANCE)
             j = last_index;
        if ((j > MIN_DISTANCE) && (j - stack_deletions) > (repair.distance - repair.num_deletions))
        {
            repair.stack_position = top;
            repair.distance = j;
            repair.num_deletions = stack_deletions;
            repair.recovery_on_next_stack = stack_flag;
        }
    }

    return;
}


//
// This boolean function checks whether or not a given
// configuration yields a better secondary recovery than the
// best misplacement recovery computed previously.
//
void DiagnoseParser::SecondaryRecovery(SecondaryRepairInfo &repair, Array<int> &stck, int stack_top, int last_index, bool stack_flag)
{
    Location  previous_loc = Loc(buffer[2]);
    int stack_deletions = 0;

    for (int top = stack_top; top >= 0 && repair.num_deletions >= stack_deletions; top--)
    {
        if (location_stack[top] < previous_loc)
            stack_deletions++;
        previous_loc = location_stack[top];

        for (int i = 2;
             i <= (last_index - MIN_DISTANCE + 1) &&
             (repair.num_deletions >= (stack_deletions + i - 1)); i++)
        {
            int j = ParseCheck(stck, top, lex_stream -> Kind(buffer[i]), i + 1);

            if (j == MAX_DISTANCE)
                 j = last_index;
            if ((j - i + 1) > MIN_DISTANCE)
            {
                int k = stack_deletions + i - 1;
                if ((k < repair.num_deletions) ||
                    (j - k) > (repair.distance - repair.num_deletions) ||
                    ((repair.code == SECONDARY_CODE) &&
                     (j - k) == (repair.distance -
                                 repair.num_deletions)))
                {
                    repair.code = DELETION_CODE;
                    repair.distance = j;
                    repair.stack_position = top;
                    repair.buffer_position = i;
                    repair.num_deletions = k;
                    repair.recovery_on_next_stack = stack_flag;
                }
            }

            for (int l = nasi(stck[top]); l >= 0 && nasr[l] != 0; l++)
            {
                int symbol = nasr[l] + NT_OFFSET;
                j = ParseCheck(stck, top, symbol, i);
                if (j == MAX_DISTANCE)
                     j = last_index;
                if ((j - i + 1) > MIN_DISTANCE)
                {
                    int k = stack_deletions + i - 1;
                    if (k < repair.num_deletions || (j - k) > (repair.distance - repair.num_deletions))
                    {
                        repair.code = SECONDARY_CODE;
                        repair.symbol = symbol;
                        repair.distance = j;
                        repair.stack_position = top;
                        repair.buffer_position = i;
                        repair.num_deletions = k;
                        repair.recovery_on_next_stack = stack_flag;
                    }
                }
            }
        }
    }

    return;
}


//
// This procedure is invoked to issue a secondary diagnosis and
// adjust the input buffer.  The recovery in question is either
// an automatic scope recovery, a manual scope recovery, a
// secondary substitution or a secondary deletion.
//
void DiagnoseParser::SecondaryDiagnosis(SecondaryRepairInfo &repair)
{
    //
    //  Issue diagnostic.
    //
    switch(repair.code)
    {
        case SCOPE_CODE:
        {
            if (repair.stack_position < state_stack_top)
            {
                ReportError(DELETION_CODE,
                            terminal_index[ERROR_SYMBOL],
                            location_stack[repair.stack_position],
                            buffer[1]);
            }
            for (int i = 0; i < scope_stack_top; i++)
            {
                ReportError(SCOPE_CODE,
                            -scope_index[i],
                            location_stack[scope_position[i]],
                            buffer[1],
                            non_terminal_index[scope_lhs[scope_index[i]]]);
            }

            repair.symbol = scope_lhs[scope_index[scope_stack_top]] + NT_OFFSET;
            state_stack_top = scope_position[scope_stack_top];
            ReportError(SCOPE_CODE,
                        -scope_index[scope_stack_top],
                        location_stack[scope_position[scope_stack_top]],
                        buffer[1],
                        GetNtermIndex(stack[state_stack_top],
                                      repair.symbol,
                                      repair.buffer_position)
                       );
            break;
        }
        default:
        {
            ReportError(repair.code,
                        (repair.code == SECONDARY_CODE
                                      ? GetNtermIndex(stack[repair.stack_position],
                                                      repair.symbol,
                                                      repair.buffer_position)
                                      : terminal_index[ERROR_SYMBOL]),
                        location_stack[repair.stack_position],
                        buffer[repair.buffer_position - 1]);
            state_stack_top = repair.stack_position;
        }
    }

    return;
}




//
// Try to parse until first_token and all tokens in BUFFER have
// been consumed, or an error is encountered. Return the number
// of tokens that were expended before the parse blocked.
//
int DiagnoseParser::ParseCheck(Array<int> &stck, int stack_top, int first_token, int buffer_position)
{
    int max_pos,
        buffer_index,
        current_kind;

    //
    // Initialize pointer for temp_stack and initialize maximum
    // position of state stack that is still useful.
    //
    int act = stck[stack_top];
    if (first_token > NT_OFFSET)
    {
        int lhs_symbol = first_token - NT_OFFSET;
        temp_stack_top = stack_top;
        max_pos = stack_top;
        buffer_index = buffer_position;
        current_kind = lex_stream -> Kind(buffer[buffer_index]);
        lex_stream -> Reset(lex_stream -> Next(buffer[buffer_index]));
        act = nt_action(act, lhs_symbol);
        if (act <= NUM_RULES)
        {
            do
            {
                temp_stack_top -= (rhs[act] - 1);
                lhs_symbol = lhs[act];
                act = (temp_stack_top > max_pos
                                      ? temp_stack[temp_stack_top]
                                      : stck[temp_stack_top]);
                act = nt_action(act, lhs_symbol);
            } while(act <= NUM_RULES);

            max_pos = Util::Min(max_pos, temp_stack_top);
        }
    }
    else
    {
        temp_stack_top = stack_top - 1;
        max_pos = temp_stack_top;
        buffer_index = buffer_position - 1;
        current_kind = first_token;
        lex_stream -> Reset(buffer[buffer_position]);
    }

    //
    // process_terminal:
    //
    for (;;)
    {
        if (++temp_stack_top >= stack.Size())  // Stack overflow!!!
            return buffer_index;
        temp_stack[temp_stack_top] = act;

        act = t_action(act, current_kind, lex_stream);

        if (act <= NUM_RULES)               // reduce action
            temp_stack_top--;
        else if (act < ACCEPT_ACTION ||     // shift action
                 act > ERROR_ACTION)        // shift-reduce action
        {
            if (buffer_index == MAX_DISTANCE)
                return buffer_index;
            buffer_index++;
            current_kind = lex_stream -> Kind(buffer[buffer_index]);
            lex_stream -> Reset(lex_stream -> Next(buffer[buffer_index]));
            if (act > ERROR_ACTION)
                 act -= ERROR_ACTION;
            else continue; // process_terminal:
        }
        else if (act == ACCEPT_ACTION)           // accept action
             return MAX_DISTANCE;
        else return buffer_index;                // error action

	//
	// process_non_terminal:
	//
        do
        {
            int lhs_symbol = lhs[act];
            temp_stack_top -= (rhs[act]-1);
            act = (temp_stack_top > max_pos
                                  ? temp_stack[temp_stack_top]
                                  : stck[temp_stack_top]);
            act = nt_action(act, lhs_symbol);
        } while(act <= NUM_RULES);

        max_pos = Util::Min(max_pos, temp_stack_top);
    } // process_terminal

    return 0;
}

//
// This procedure is invoked by an LPG PARSER or a semantic
// routine to process an error message.  The LPG parser always
// passes the value 0 to msg_level to indicate an error.
// This routine simply stores all necessary information about
// the message into an array: error.
//
void DiagnoseParser::ReportError(int msg_code,
                                 int name_index,
                                 LexStream::TokenIndex left_token,
                                 LexStream::TokenIndex right_token,
                                 int scope_name_index)
{
    Location left_token_loc = (left_token > Loc(right_token) ? Loc(right_token) : left_token),
             right_token_loc = Loc(right_token);

    if (left_token_loc < right_token_loc)
         PrintSecondaryMessage(msg_code,
                               name_index,
                               left_token_loc,
                               right_token_loc,
                               scope_name_index);
    else PrintPrimaryMessage(msg_code,
                             name_index,
                             left_token_loc,
                             right_token_loc,
                             scope_name_index);

    return;
}


//
// This procedure is invoked to form a primary error message. The
// parameter k identifies the error to be processed.  The global
// variable: msg, is used to store the message.
//
void DiagnoseParser::PrintPrimaryMessage(int msg_code,
                                         int name_index,
                                         Location left_token_loc,
                                         Location right_token_loc,
                                         int scope_name_index)
{
    const char *name = NULL;
    int i,
        len = 0;

    if (name_index >= 0)
    {
        len = NameLength(name_index);
        name = &string_buffer[name_start[name_index]];
    }

    int left_line_no    = lex_stream -> Line(left_token_loc),
        left_column_no  = lex_stream -> Column(left_token_loc),
        right_line_no   = lex_stream -> EndLine(right_token_loc),
        right_column_no = lex_stream -> EndColumn(right_token_loc),
        left_location   = lex_stream -> StartLocation(left_token_loc),
        right_location  = lex_stream -> EndColumn(right_token_loc);

    cout << lex_stream -> FileName(left_token_loc)
         << ':' << left_line_no  << ':' << left_column_no
         << ':' << right_line_no << ':' << right_column_no
         << ':' << left_location << ':' << right_location
         << ": ";

    switch(msg_code)
    {
        case ERROR_CODE:
             cout << "Parsing terminated at this token";
             break;
        case BEFORE_CODE:
             for (i = 0; i < len; i++)
                 cout << name[i];
             cout << " inserted before this token";
             break;
        case INSERTION_CODE:
             for (i = 0; i < len; i++)
                 cout << name[i];
             cout << " expected after this token";
             break;
        case DELETION_CODE:
             if (left_token_loc == right_token_loc)
                  cout << "Unexpected symbol ignored";
             else cout << "Unexpected symbols ignored";
             break;
        case INVALID_CODE:
             if (len == 0)
                 cout << "Unexpected input discarded";
             else
             {
                 cout << "Invalid ";
                 for (i = 0; i < len; i++)
                     cout << name[i];
             }
             break;
        case SUBSTITUTION_CODE:
             for (i = 0; i < len; i++)
                 cout << name[i];
             cout << " expected instead of this token";
             break;
        case SCOPE_CODE:
             cout << '\"';
             for (i = scope_suffix[- (int) name_index];
                  scope_rhs[i] != 0; i++)
             {
                 len  = NameLength(scope_rhs[i]);
                 name = &string_buffer[name_start[scope_rhs[i]]];
                 for (int j = 0; j < len; j++)
                     cout << name[j];
                 if (scope_rhs[i+1]) // any more symbols to print?
                     cout << ' ';
             }
             cout << '\"';
             cout << " inserted to complete scope";
             //
             // TODO: This should not be an option
             //
             if (scope_name_index)
             {
                 len  = NameLength(scope_name_index);
                 name = &string_buffer[name_start[scope_name_index]];
                 for (int j = 0; j < len; j++) // any more symbols to print?
                      cout << name[j];
             }
             else cout << "scope";
             break;
        case MANUAL_CODE:
             cout << '\"';
             for (i = 0; i < len; i++)
                 cout << name[i];
             cout << "\" inserted to complete structure";
             break;
        case MERGE_CODE:
             cout << "symbols merged to form ";
             for (i = 0; i < len; i++)
                 cout << name[i];
             break;
        case EOF_CODE:
             for (i = 0; i < len; i++)
                 cout << name[i];
             cout << " reached after this token";
             break;
        default:
             if (msg_code == MISPLACED_CODE)
                  cout << "misplaced construct(s)";
             else if (len == 0)
                  cout << "unexpected input discarded";
             else
             {
                 for (i = 0; i < len; i++)
                     cout << name[i];
                 cout << " expected instead";
             }
             break;
    }

    cout << '\n';
    cout.flush();

    return;
}

//
// This procedure is invoked to form a secondary error message.
// The parameter k identifies the error to be processed.  The
// global variable: msg, is used to store the message.
//
void DiagnoseParser::PrintSecondaryMessage(int msg_code,
                                           int name_index,
                                           Location left_token_loc,
                                           Location right_token_loc,
                                           int scope_name_index)
{
    const char *name = "";
    int i,
        len = 0;

    if (name_index >= 0)
    {
        len = NameLength(name_index);
        name = &string_buffer[name_start[name_index]];
    }

    int left_line_no    = lex_stream -> Line(left_token_loc),
        left_column_no  = lex_stream -> Column(left_token_loc),
        right_line_no   = lex_stream -> EndLine(right_token_loc),
        right_column_no = lex_stream -> EndColumn(right_token_loc),
        left_location   = lex_stream -> StartLocation(left_token_loc),
        right_location  = lex_stream -> EndColumn(right_token_loc);

    cout << lex_stream -> FileName(left_token_loc)
         << ':' << left_line_no  << ':' << left_column_no
         << ':' << right_line_no << ':' << right_column_no
         << ':' << left_location << ':' << right_location
         << ": ";

    switch(msg_code)
    {
        case MISPLACED_CODE:
            cout << "Misplaced construct(s)";
            break;
        case SCOPE_CODE:
            cout << '\"';
            for (i = scope_suffix[- (int) name_index]; scope_rhs[i] != 0; i++)
            {
                len  = NameLength(scope_rhs[i]);
                name = &string_buffer[name_start[scope_rhs[i]]];
                for (int j = 0; j < len; j++) // any more symbols to print?
                     cout << name[j];
                if (scope_rhs[i+1])
                    cout << ' ';
            }
            cout << "\" inserted to complete ";
            //
            // TODO: This should not be an option
            //
            if (scope_name_index)
            {
                len  = NameLength(scope_name_index);
                name = &string_buffer[name_start[scope_name_index]];
                for (int j = 0; j < len; j++) // any more symbols to print?
                     cout << name[j];
            }
            else cout << "phrase";
            break;
        case  MANUAL_CODE:
            cout << '\"';
            for (i = 0; i < len; i++)
                cout << name[i];
            cout << "\" inserted to complete structure";
            break;
        case MERGE_CODE:
            cout << "Symbols merged to form ";
            for (i = 0; i < len; i++)
                cout << name[i];
            break;
        default:
            if (msg_code == DELETION_CODE || len == 0)
                cout << "Unexpected input discarded";
            else
            {
                for (i = 0; i < len; i++)
                    cout << name[i];
                cout << " expected instead";
            }
    }

    cout << '\n';
    cout.flush();

    return;
}
