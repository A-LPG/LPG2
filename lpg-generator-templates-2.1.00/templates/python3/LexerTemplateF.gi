--
-- An instance of this template must have a $Export section and the export_terminals option
--
-- Macros that may be redefined in an instance of self template
--
--     $eof_token
--     $additional_interfaces
--     $super_stream_class -- subclass LpgLexStream for getKind
--     $prs_stream_class -- use /.PrsStream./ if not subclassing
--     $super_class
--
-- B E G I N N I N G   O F   T E M P L A T E   LexerTemplateF
--
%options programming_Language=python3,margin=4
%options table
%options action-block=("*.py", "/.", "./")
%options ParseTable=ParseTable
%options prefix=Char_

--
-- This template requires that the name of the EOF token be set
-- to EOF and that the prefix be "Char_" to be consistent with
-- KeywordTemplateD.
--
$Eof
    EOF
%End

--
-- This template also requires that the name of the parser EOF
-- Token to be exported be set to EOF_TOKEN
--
$Export
    EOF_TOKEN
%End

%Define
    --
    -- Macros that are be needed in an instance of self template
    --
    $eof_token /.$_EOF_TOKEN./
    
    $additional_interfaces /../
    $super_stream_class /.$file_prefix$LpgLexStream./
    $prs_stream_class /.IPrsStream./
    $super_class /.object./

    $prs_stream /. # macro prs_stream is deprecated. Use function getPrsStream
                  self.getPrsStream()./
    $setSym1 /. # macro setSym1 is deprecated. Use function setResult
                self.self.lexParser.setSym1./
    $setResult /. # macro setResult is deprecated. Use function setResult
                  self.self.lexParser.setSym1./
    $getSym /. # macro getSym is deprecated. Use function getLastToken
               self.self.lexParser.getSym./
    $getToken /. # macro getToken is deprecated. Use function getToken
                 self.self.lexParser.getToken./
    $getLeftSpan /. # macro getLeftSpan is deprecated. Use function getLeftSpan
                    self.self.lexParser.getFirstToken./
    $getRightSpan /. # macro getRightSpan is deprecated. Use function getRightSpan
                     self.self.lexParser.getLastToken./

    --
    -- Macros useful for specifying actions
    --
    $Header
    /.
            #
            # Rule $rule_number:  $rule_text
            #
            ./

    $DefaultAction
    /.$Header def Act$rule_number():./

    $BeginAction /.$DefaultAction./

    $EndAction
    /.
             self.__rule_action[$rule_number] =Act$rule_number
    ./

    $BeginJava
    /.$BeginAction
                $symbol_declarations./

    $EndJava /.$EndAction./

    $NoAction
    /.$Header
                     ./

    $BeginActions
    /.
        def initRuleAction(self) : 
    ./


    $EndActions
    /../
%End

%Globals
/.
from lpg2  import  RuleAction, ParseTable, LexParser, ILexStream, IPrsStream, Monitor,LpgLexStream
from $prs_type  import  $prs_type 
from $sym_type  import  $sym_type  
from $kw_lexer_class  import  $kw_lexer_class 
./

%End

%Headers
    /.
    class $action_type (RuleAction$additional_interfaces):
    
        
        
        prs : ParseTable =  $prs_type()
        def getParseTable(self) -> ParseTable : return $action_type.prs 


        def getParser(self) ->LexParser :  return self.lexParser 

        def getToken(self, i: int)  -> int: return self.lexParser.getToken(i) 
        def getRhsFirstTokenIndex(self, i: int) -> int: return self.lexParser.getFirstToken(i) 
        def getRhsLastTokenIndex(self, i: int)  -> int: return self.lexParser.getLastToken(i) 

        def getLeftSpan(self) -> int: return self.lexParser.getToken(1) 
        def getRightSpan(self) -> int : return self.lexParser.getLastToken() 

        def resetKeywordLexer(self) : 
        
            if (not self.kwLexer):
                  self.kwLexer =  $kw_lexer_class(self.lexStream.getInputChars(), $_IDENTIFIER)
            self.kwLexer.setInputChars(self.lexStream.getInputChars())
        
  
      
        
        def reset(self, filename : str,  tab : int = 4, input_chars : str = None) : 
        
            self.lexStream =  $super_stream_class(filename,input_chars, tab)
            self.lexParser.reset(self.lexStream, $action_type.prs,  self)
            self.resetKeywordLexer()
        
        
        def ruleAction(self,ruleNumber : int) :
            act = self.__rule_action[ruleNumber]
            if act:
                act() 

        def __init__(self, filename : str,  tab : int =  4 ,input_chars : str = None):
        
            super().__init__()
            self.__rule_action = [None]* ($num_rules + 2)
            self.kwLexer :  $kw_lexer_class = None
            self.printTokens : bool =False
            self.lexParser  : LexParser =  LexParser()
            self.lexStream: $super_stream_class 
            self.initRuleAction()
            self.lexStream =  $super_stream_class(filename,input_chars, tab)
            self.lexParser.reset(  self.lexStream, $action_type.prs,  self)
            self.resetKeywordLexer()
        

       

        def getILexStream(self) ->ILexStream :  return  self.lexStream 

        def initializeLexer(self,prsStream : $prs_stream_class ,  start_offset : int, end_offset : int) :  
        
            if (self.lexStream.getInputChars() == None):
                raise  ValueError("LexStream was not initialized")
            self.lexStream.setPrsStream(prsStream)
            prsStream.makeToken(start_offset, end_offset, 0) # Token list must start with a bad token
        

        def addEOF(self,prsStream : $prs_stream_class, end_offset : int ) : 
        
            prsStream.makeToken(end_offset, end_offset, $eof_token) # and end with the end of file token
            prsStream.setStreamLength(prsStream.getSize())
        

        def lexerWithPosition(self,prsStream: $prs_stream_class , start_offset : int , end_offset : int, monitor : Monitor = None) : 
        
            if (start_offset <= 1):
                self.initializeLexer(prsStream, 0, -1)
            else :
                self.initializeLexer(prsStream, start_offset - 1, start_offset - 1)

            self.lexParser.parseCharacters(start_offset, end_offset,monitor)

            self.addEOF(prsStream, ( self.lexStream.getStreamIndex() if end_offset >= self.lexStream.getStreamIndex() else  end_offset + 1))
        

        def lexer(self,prsStream: $prs_stream_class ,  monitor : Monitor = None) : 
        
            self.initializeLexer(prsStream, 0, -1)
            self.lexParser.parseCharactersWhitMonitor(monitor)
            self.addEOF(prsStream, self.lexStream.getStreamIndex())
        
       

        '''/**
         * If a parse stream was not passed to self Lexical analyser then we
         * simply report a lexical error. Otherwise, we produce a bad token.
         */'''
        def reportLexicalError(self, startLoc : int,  endLoc : int) :  
            prs_stream = self.lexStream.getIPrsStream()
            if (not prs_stream):
                self.lexStream.reportLexicalError(startLoc, endLoc)
            else: 
                #
                # Remove any token that may have been processed that fall in the
                # range of the lexical error... then add one error token that spans
                # the error range.
                #
                i: int = prs_stream.getSize() - 1
                while ( i > 0 ): 
                    if (prs_stream.getStartOffset(i) >= startLoc):
                         prs_stream.removeLastToken()
                    else:
                         break
                    i-=1
                
                prs_stream.makeToken(startLoc, endLoc, 0) # add an error token to the self.prsStream
                    
        
    ./
%End

%Rules
    /.$BeginActions./
%End

%Trailers
    /.
        $EndActions
    
    ./
%End

--
-- E N D   O F   T E M P L A T E
--
