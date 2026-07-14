#include "minimalprs.h"


const char minimalprs::is_nullable[] = {0,
            0,0,0,0
        };

const unsigned char minimalprs::prostheses_index[] = {0,
            2,1
        };

const char minimalprs::is_keyword[] = {0,
            0,0
        };

const unsigned char minimalprs::base_check[] = {0,
            1
        };

const unsigned char *minimalprs::rhs = base_check;

const unsigned char minimalprs::base_action[] = {
            1,1,1,4,3,7,7
        };

const unsigned char *minimalprs::lhs = base_action;

const unsigned char minimalprs::term_check[] = {0,
            0,1,0,0,2
        };

const unsigned char minimalprs::term_action[] = {0,
            7,8,7,7,6
        };
