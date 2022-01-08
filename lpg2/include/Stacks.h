#ifndef stacks_INCLUDED
#define stacks_INCLUDED

#include "jikespg_prs.h" // parsing action functions
#include "LexStream.h" 

#include <limits.h>

class Stacks
{
protected:
    Stacks() : state_stack_top(-1)
    {}

    TokenObject curtok;

    //
    // Given a rule of the form     A ::= x1 x2 ... xn     where n >= 0
    //
    // the function Token(i) yields the symbol xi, if xi is a terminal
    // or ti, if xi is a nonterminal that produced a string of the form
    // xi => ti w. If xi is a nonterminal that produced the empty string then
    // if i < n then Token(i) is the same as Token(i+1); otherwise, Token(i)
    // returns the terminal symbol that immediately follows A (the lookahead)
    // in the input.
    //
    inline LexStream::TokenIndex Token(int i)
    {
        return location_stack[state_stack_top + (i - 1)];
    }

    //
    // stack is the "state stack" used by the parser to store
    // the "context" for the next parsing action. stack_length
    // represent its size in terms of the number of elements that
    // is allocated for it. state_stack_top represents the number
    // of stack elements that are actually in use at any point.
    //
    Array<int> stack;
    int state_stack_top;

    //
    // location_stack is a stack that is "parallel" to the 
    // (state_)stack that is used to keep track of the location
    // of the first token on which an action was executed in the
    // corresponding state.
    //
    Array<Location> location_stack;

    enum { STACK_INCREMENT = 256 };

    //
    // This function is invoked to reallocate a larger stack.
    //
    void ReallocateStacks()
    {
        int new_size = stack.Size() + STACK_INCREMENT;
    
        assert(new_size <= SHRT_MAX);
        stack.Resize(new_size);
        location_stack.Resize(new_size);
    
        return;
    }
};
#endif
