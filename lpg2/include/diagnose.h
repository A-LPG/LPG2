#ifndef diagnose_INCLUDED
#define diagnose_INCLUDED

#include "jikespg_prs.h" // parsing action functions
#include "parser.h"

class DiagnoseParser : public jikespg_prs
{
public:     
    DiagnoseParser(LexStream *lex_stream_) : lex_stream(lex_stream_),
                                             state_pool(256, 4)
    {
        buffer.Resize(BUFF_SIZE);
        list.Resize(NUM_SYMBOLS + 1);
        list.MemReset();
        DiagnoseParse();

        return;
    }

private:

    static int (*t_action) (int, int, LexStream *);

    inline Location Loc(TokenObject i) { return i; }

    struct PrimaryRepairInfo
    {
        int distance,
            buffer_position,
            code,
            misspell_index,
            symbol;

        PrimaryRepairInfo() : distance(0),
                              buffer_position(0),
                              code(0),
                              misspell_index(0),
                              symbol(0)
        {}
    };
 
    struct SecondaryRepairInfo
    {
        int distance,
            buffer_position,
            code,
            stack_position,
            num_deletions,
            symbol;
 
        bool recovery_on_next_stack;

        SecondaryRepairInfo() : distance(0),
                              buffer_position(0),
                              code(0),
                              stack_position(0),
                              num_deletions(0),
                              symbol(0),
                              recovery_on_next_stack(0)
        {}
    };

    struct RepairCandidate
    {
        int symbol;
        Location location;

        RepairCandidate() : symbol(0)
        {}
    };
 
    struct StateInfo
    {
        int state,
            next;
    };
 
    struct ErrorInfo
    {
        LexStream::TokenIndex left_token,
                              right_token;
        int                   name_index,
                              msg_code,
                              scope_name_index;
    };
 
    LexStream *lex_stream;

    enum
    {
        ERROR_CODE,
        BEFORE_CODE,
        INSERTION_CODE,
        INVALID_CODE,
        SUBSTITUTION_CODE,
        DELETION_CODE,
        MERGE_CODE,
        MISPLACED_CODE,
        SCOPE_CODE,
        MANUAL_CODE,
        SECONDARY_CODE,
        EOF_CODE,

        BUFF_UBOUND  = 31,
        BUFF_SIZE    = 32,
        MAX_DISTANCE = 30,
        MIN_DISTANCE = 3,

        STACK_INCREMENT = 256,
        NIL = -1
    };

    //
    //
    //
    int state_stack_top,
        temp_stack_top,
        prev_stack_top,
        next_stack_top,
        scope_stack_top;

    Array<int> stack,
               temp_stack,
               next_stack,
               prev_stack,
               scope_index,
               scope_position;

    //
    // LOCATION_STACK is a stack that is "parallel" to
    // (STATE_)STACK that is used to keep
    // track of the location of the first token on which an action
    // was executed in the corresponding state.
    //
    Array<Location> location_stack;

    Tuple<StateInfo> state_pool;
    Array<int> list,
               state_seen; // this variable is managed entirely by the function "scope_trial"
    Array<TokenObject> buffer;

    void DiagnoseParse();

    void ReallocateStacks();

    inline int NameLength(int i) { return name_start[i + 1] - name_start[i]; }

    RepairCandidate ErrorRecovery(TokenObject error_token);
    RepairCandidate PrimaryPhase(TokenObject error_token);
    int MergeCandidate(int state, int buffer_position);
    void CheckPrimaryDistance(PrimaryRepairInfo &, Array<int> &, int);
    RepairCandidate PrimaryDiagnosis(PrimaryRepairInfo &);
    int GetTermIndex(Array<int> &, int, int, int);
    int GetNtermIndex(int, int, int);
    int Misspell(int, TokenObject);
    RepairCandidate SecondaryPhase(TokenObject error_token);
    void MisplacementRecovery(SecondaryRepairInfo &, Array<int> &, int, int, bool);
    void SecondaryRecovery(SecondaryRepairInfo &, Array<int> &, int, int, bool);
    void SecondaryDiagnosis(SecondaryRepairInfo &);
 
    void RepairParse(TokenObject);

    void ScopeTrial(PrimaryRepairInfo &, Array<int> &, int);
    void ScopeTrialCheck(PrimaryRepairInfo &, Array<int> &, int, int);
    bool SecondaryCheck(Array<int> &, int, int buffer_position, int distance);
    int ParseCheck(Array<int> &stack, int stack_top, int first_token, int buffer_position);

    void PrintPrimaryMessage(int,
                             int,
                             Location,
                             Location,
                             int);

    void PrintSecondaryMessage(int,
                               int,
                               Location,
                               Location,
                               int);

    void ReportError(int msg_code,
                     int name_index,
                     LexStream::TokenIndex left_token,
                     LexStream::TokenIndex right_token,
                     int scope_name_index = 0);
};

#endif
