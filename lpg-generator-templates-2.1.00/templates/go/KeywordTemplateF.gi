--
-- An LPG Lexer Template Using lpg.jar
--
-- An instance of my template must have a $Export section and the export_terminals option
-- There must be only one non-terminal, the start symbol, for the keywords
-- The action for each keyword should be a call to $setResult(terminal_symbol)
--
-- Macro that may be redefined in an instance of my template
--
--     $eof_char
--
-- B E G I N N I N G   O F   T E M P L A T E   KeywordTemplateF (Similar to KeywordTemplateD)
--
%Options Programming_Language=go,margin=4
%Options table
%options action-block=("*.go", "/.", "./")
%options ParseTable=lpg.runtime.ParseTable
%Options prefix=Char_
%Options single-productions

--
-- This template requires that the name of the EOF token be set
-- to EOF and that the prefix be "Char_" to be consistent with
-- LexerTemplateD.
--
%Eof
    EOF
%End

%Define
    --
    -- Macro that may be respecified in an instance of my template
    --
    $eof_char /.$sym_type$.$prefix$EOF$suffix$./

    --
    -- Macros useful for specifying actions
    --
    $setResult /.my.keywordKind[$rule_number] = ./

    $Header
    /.
            //
            // Rule $rule_number:  $rule_text
            //
            ./

    $BeginAction /.$Header./

    $EndAction /../

    $BeginJava /.$BeginAction./

    $EndJava /.$EndAction./
%End

%Globals
    /.
    ./
%End

%Headers
    /.
    type  $action_type struct{
        *$prs_type
        inputChars []rune
        keywordKind []int
    }
    func (my *$action_type)  GetKeywordKinds() []int { return my.keywordKind }

    func (my *$action_type)  Lexer(curtok int,  lasttok int)int{
        var current_kind = my.GetKind(my.inputChars[curtok])
        var    act int 

        for act = my.TAction($prs_type$_START_STATE, current_kind);
                act > $prs_type$_NUM_RULES && act < $prs_type$_ACCEPT_ACTION;
                act = my.TAction(act, current_kind){
            curtok++
            if curtok > lasttok{
                current_kind = $eof_char
            }else{
                current_kind =my.GetKind(my.inputChars[curtok])
            }

        }

        if (act > $prs_type$_ERROR_ACTION){
            curtok++
            act -= $prs_type$_ERROR_ACTION
        }
        if act == $prs_type$_ERROR_ACTION  || curtok <= lasttok  {
            return my.keywordKind[0]
        }else{
            return my.keywordKind[act]
        }
    }

    func (my *$action_type)  SetInputChars(inputChars []rune) {
        my.inputChars = inputChars
    }
    ./
%End

%Rules
    /.
    func New$action_type(inputChars []rune, identifierKind int)*$action_type{

        my := new($action_type)
        my.$prs_type = New$prs_type()
        my.keywordKind = make([]int,$num_rules + 1)
        my.inputChars = inputChars
        my.keywordKind[0] = identifierKind
    ./
%End

%Trailers
    /.
        var i int = 0
        for ;i < len(my.keywordKind); i++{
            if my.keywordKind[i] == 0 {
                my.keywordKind[i] = identifierKind
            }
        }
        return my
    }
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
