
//#line 315 "jikespg.g"
#include "jikespg_act.h"
#include "control.h"

jikespg_act::jikespg_act(Control *control_,
                         LexStream *lex_stream_,
                         VariableLookupTable *variable_table_,
                         MacroLookupTable *macro_table_)
            : identifier_index(0),
              eol_index(0),
              eof_index(0),
              error_index(0),

              control(control_),
              option(control_ -> option),
              lex_stream(lex_stream_),
              variable_table(variable_table_),
              macro_table(macro_table_)
{
    rule_action[0] = &jikespg_act::BadAction;

    rule_action[1] = &jikespg_act::Act1;
    rule_action[2] = &jikespg_act::NoAction;
    rule_action[3] = &jikespg_act::NoAction;
    rule_action[4] = &jikespg_act::NoAction;
    rule_action[5] = &jikespg_act::NoAction;
    rule_action[6] = &jikespg_act::NoAction;
    rule_action[7] = &jikespg_act::NoAction;
    rule_action[8] = &jikespg_act::NoAction;
    rule_action[9] = &jikespg_act::NoAction;
    rule_action[10] = &jikespg_act::NoAction;
    rule_action[11] = &jikespg_act::NoAction;
    rule_action[12] = &jikespg_act::NoAction;
    rule_action[13] = &jikespg_act::NoAction;
    rule_action[14] = &jikespg_act::NoAction;
    rule_action[15] = &jikespg_act::NoAction;
    rule_action[16] = &jikespg_act::NoAction;
    rule_action[17] = &jikespg_act::NoAction;
    rule_action[18] = &jikespg_act::NoAction;
    rule_action[19] = &jikespg_act::NoAction;
    rule_action[20] = &jikespg_act::NoAction;
    rule_action[21] = &jikespg_act::NoAction;
    rule_action[22] = &jikespg_act::NoAction;
    rule_action[23] = &jikespg_act::NoAction;
    rule_action[24] = &jikespg_act::NoAction;
    rule_action[25] = &jikespg_act::NoAction;
    rule_action[26] = &jikespg_act::NoAction;
    rule_action[27] = &jikespg_act::NoAction;
    rule_action[28] = &jikespg_act::Act28;
    rule_action[29] = &jikespg_act::NoAction;
    rule_action[30] = &jikespg_act::Act30;
    rule_action[31] = &jikespg_act::NoAction;
    rule_action[32] = &jikespg_act::Act32;
    rule_action[33] = &jikespg_act::NoAction;
    rule_action[34] = &jikespg_act::NoAction;
    rule_action[35] = &jikespg_act::Act35;
    rule_action[36] = &jikespg_act::Act36;
    rule_action[37] = &jikespg_act::NoAction;
    rule_action[38] = &jikespg_act::Act38;
    rule_action[39] = &jikespg_act::NoAction;
    rule_action[40] = &jikespg_act::Act40;
    rule_action[41] = &jikespg_act::NoAction;
    rule_action[42] = &jikespg_act::NoAction;
    rule_action[43] = &jikespg_act::NoAction;
    rule_action[44] = &jikespg_act::NoAction;
    rule_action[45] = &jikespg_act::Act45;
    rule_action[46] = &jikespg_act::NoAction;
    rule_action[47] = &jikespg_act::NoAction;
    rule_action[48] = &jikespg_act::Act48;
    rule_action[49] = &jikespg_act::Act49;
    rule_action[50] = &jikespg_act::Act50;
    rule_action[51] = &jikespg_act::NoAction;
    rule_action[52] = &jikespg_act::NoAction;
    rule_action[53] = &jikespg_act::NoAction;
    rule_action[54] = &jikespg_act::Act54;
    rule_action[55] = &jikespg_act::Act55;
    rule_action[56] = &jikespg_act::NoAction;
    rule_action[57] = &jikespg_act::Act57;
    rule_action[58] = &jikespg_act::NoAction;
    rule_action[59] = &jikespg_act::Act59;
    rule_action[60] = &jikespg_act::NoAction;
    rule_action[61] = &jikespg_act::Act61;
    rule_action[62] = &jikespg_act::NoAction;
    rule_action[63] = &jikespg_act::Act63;
    rule_action[64] = &jikespg_act::NoAction;
    rule_action[65] = &jikespg_act::Act65;
    rule_action[66] = &jikespg_act::NoAction;
    rule_action[67] = &jikespg_act::Act67;
    rule_action[68] = &jikespg_act::NoAction;
    rule_action[69] = &jikespg_act::Act69;
    rule_action[70] = &jikespg_act::Act70;
    rule_action[71] = &jikespg_act::Act71;
    rule_action[72] = &jikespg_act::Act72;
    rule_action[73] = &jikespg_act::Act73;
    rule_action[74] = &jikespg_act::Act74;
    rule_action[75] = &jikespg_act::Act75;
    rule_action[76] = &jikespg_act::NoAction;
    rule_action[77] = &jikespg_act::Act77;
    rule_action[78] = &jikespg_act::NoAction;
    rule_action[79] = &jikespg_act::NoAction;
    rule_action[80] = &jikespg_act::NoAction;
    rule_action[81] = &jikespg_act::NoAction;
    rule_action[82] = &jikespg_act::NoAction;
    rule_action[83] = &jikespg_act::NoAction;
    rule_action[84] = &jikespg_act::Act84;
    rule_action[85] = &jikespg_act::NoAction;
    rule_action[86] = &jikespg_act::Act86;
    rule_action[87] = &jikespg_act::NoAction;
    rule_action[88] = &jikespg_act::Act88;
    rule_action[89] = &jikespg_act::NoAction;
    rule_action[90] = &jikespg_act::Act90;
    rule_action[91] = &jikespg_act::NoAction;
    rule_action[92] = &jikespg_act::Act92;
    rule_action[93] = &jikespg_act::NoAction;
    rule_action[94] = &jikespg_act::NoAction;
    rule_action[95] = &jikespg_act::NoAction;
    rule_action[96] = &jikespg_act::NoAction;
    rule_action[97] = &jikespg_act::Act97;
    rule_action[98] = &jikespg_act::Act98;
    rule_action[99] = &jikespg_act::Act99;
    rule_action[100] = &jikespg_act::Act100;
    rule_action[101] = &jikespg_act::NoAction;
    rule_action[102] = &jikespg_act::NoAction;
    rule_action[103] = &jikespg_act::NoAction;
    rule_action[104] = &jikespg_act::NoAction;
    rule_action[105] = &jikespg_act::NoAction;
    rule_action[106] = &jikespg_act::NoAction;
    rule_action[107] = &jikespg_act::NoAction;
    rule_action[108] = &jikespg_act::NoAction;
    rule_action[109] = &jikespg_act::NoAction;
    rule_action[110] = &jikespg_act::NoAction;
    rule_action[111] = &jikespg_act::NoAction;
    rule_action[112] = &jikespg_act::NoAction;
    rule_action[113] = &jikespg_act::NoAction;
    rule_action[114] = &jikespg_act::Act114;
    rule_action[115] = &jikespg_act::Act115;
    rule_action[116] = &jikespg_act::Act116;
    rule_action[117] = &jikespg_act::NoAction;
    rule_action[118] = &jikespg_act::Act118;
    rule_action[119] = &jikespg_act::NoAction;
    rule_action[120] = &jikespg_act::Act120;
    rule_action[121] = &jikespg_act::NoAction;
    rule_action[122] = &jikespg_act::Act122;
    rule_action[123] = &jikespg_act::Act123;
    rule_action[124] = &jikespg_act::NoAction;
    rule_action[125] = &jikespg_act::NoAction;
    rule_action[126] = &jikespg_act::NoAction;
    rule_action[127] = &jikespg_act::NoAction;
    rule_action[128] = &jikespg_act::NoAction;
    rule_action[129] = &jikespg_act::NoAction;
    rule_action[130] = &jikespg_act::Act130;

////#line 1348 "jikespg.g"

    return;
}

