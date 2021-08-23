%Terminals
    u0000
    u0001
    u0002
    u0003
    u0004
    u0005
    u0006
    u0007
    u0008
    u0009 -- HT
    u000A -- LF
    u000B
    u000C -- FF
    u000D -- CR
    u000E
    u000F
    u0010
    u0011
    u0012
    u0013
    u0014
    u0015
    u0016
    u0017
    u0018
    u0019
    u001A
    u001B
    u001C
    u001D
    u001E
    u001F
    u0020 ::= ' '
    u0021 ::= '!'
    u0022 ::= '"'
    u0023 ::= '#'
    u0024 ::= '$'
    u0025 ::= '%'
    u0026 ::= '&'
    u0027 ::= "'"
    u0028 ::= '('
    u0029 ::= ')'
    u002A ::= '*'
    u002B ::= '+'
    u002C ::= ','
    u002D ::= '-'
    u002E ::= '.'
    u002F ::= '/'
    u0030 ::= '0'
    u0031 ::= '1'
    u0032 ::= '2'
    u0033 ::= '3'
    u0034 ::= '4'
    u0035 ::= '5'
    u0036 ::= '6'
    u0037 ::= '7'
    u0038 ::= '8'
    u0039 ::= '9'
    u003A ::= ':'
    u003B ::= ';'
    u003C ::= '<'
    u003D ::= '='
    u003E ::= '>'
    u003F ::= '?'
    u0040 ::= '@'
    u0041 ::= 'A'
    u0042 ::= 'B'
    u0043 ::= 'C'
    u0044 ::= 'D'
    u0045 ::= 'E'
    u0046 ::= 'F'
    u0047 ::= 'G'
    u0048 ::= 'H'
    u0049 ::= 'I'
    u004A ::= 'J'
    u004B ::= 'K'
    u004C ::= 'L'
    u004D ::= 'M'
    u004E ::= 'N'
    u004F ::= 'O'
    u0050 ::= 'P'
    u0051 ::= 'Q'
    u0052 ::= 'R'
    u0053 ::= 'S'
    u0054 ::= 'T'
    u0055 ::= 'U'
    u0056 ::= 'V'
    u0057 ::= 'W'
    u0058 ::= 'X'
    u0059 ::= 'Y'
    u005A ::= 'Z'
    u005B ::= '['
    u005C ::= '\'
    u005D ::= ']'
    u005E ::= '^'
    u005F ::= '_'
    u0060 ::= '`'
    u0061 ::= 'a'
    u0062 ::= 'b'
    u0063 ::= 'c'
    u0064 ::= 'd'
    u0065 ::= 'e'
    u0066 ::= 'f'
    u0067 ::= 'g'
    u0068 ::= 'h'
    u0069 ::= 'i'
    u006A ::= 'j'
    u006B ::= 'k'
    u006C ::= 'l'
    u006D ::= 'm'
    u006E ::= 'n'
    u006F ::= 'o'
    u0070 ::= 'p'
    u0071 ::= 'q'
    u0072 ::= 'r'
    u0073 ::= 's'
    u0074 ::= 't'
    u0075 ::= 'u'
    u0076 ::= 'v'
    u0077 ::= 'w'
    u0078 ::= 'x'
    u0079 ::= 'y'
    u007A ::= 'z'
    u007B ::= '{'
    u007C ::= '|'
    u007D ::= '}'
    u007E ::= '~'
    u007F
    
    UNUSED
%End
$Trailers 
/. 
    #
    #
    #
    class $super_stream_class(LpgLexStream):
        
        
        #
        #
        #
        tokenKind =  [0]*(0x10000)   # 0x10000 == 65536
    
        tokenKind[0x0000] = $sym_type.$prefix$u0000$suffix$           # 000    0x00
        tokenKind[0x0001] = $sym_type.$prefix$u0001$suffix$           # 001    0x01
        tokenKind[0x0002] = $sym_type.$prefix$u0002$suffix$           # 002    0x02
        tokenKind[0x0003] = $sym_type.$prefix$u0003$suffix$           # 003    0x03
        tokenKind[0x0004] = $sym_type.$prefix$u0004$suffix$           # 004    0x04
        tokenKind[0x0005] = $sym_type.$prefix$u0005$suffix$           # 005    0x05
        tokenKind[0x0006] = $sym_type.$prefix$u0006$suffix$           # 006    0x06
        tokenKind[0x0007] = $sym_type.$prefix$u0007$suffix$           # 007    0x07
        tokenKind[0x0008] = $sym_type.$prefix$u0008$suffix$           # 008    0x08
        tokenKind[0x0009] = $sym_type.$prefix$HT$suffix$              # 009    0x09
        tokenKind[0x000A] = $sym_type.$prefix$LF$suffix$              # 010    0x0A
        tokenKind[0x000B] = $sym_type.$prefix$u000B$suffix$           # 011    0x0B
        tokenKind[0x000C] = $sym_type.$prefix$FF$suffix$              # 012    0x0C
        tokenKind[0x000D] = $sym_type.$prefix$CR$suffix$              # 013    0x0D
        tokenKind[0x000E] = $sym_type.$prefix$u000E$suffix$           # 014    0x0E
        tokenKind[0x000F] = $sym_type.$prefix$u000F$suffix$           # 015    0x0F
        tokenKind[0x0010] = $sym_type.$prefix$u0010$suffix$           # 016    0x10
        tokenKind[0x0011] = $sym_type.$prefix$u0011$suffix$           # 017    0x11
        tokenKind[0x0012] = $sym_type.$prefix$u0012$suffix$           # 018    0x12
        tokenKind[0x0013] = $sym_type.$prefix$u0013$suffix$           # 019    0x13
        tokenKind[0x0014] = $sym_type.$prefix$u0014$suffix$           # 020    0x14
        tokenKind[0x0015] = $sym_type.$prefix$u0015$suffix$           # 021    0x15
        tokenKind[0x0016] = $sym_type.$prefix$u0016$suffix$           # 022    0x16
        tokenKind[0x0017] = $sym_type.$prefix$u0017$suffix$           # 023    0x17
        tokenKind[0x0018] = $sym_type.$prefix$u0018$suffix$           # 024    0x18
        tokenKind[0x0019] = $sym_type.$prefix$u0019$suffix$           # 025    0x19
        tokenKind[0x001A] = $sym_type.$prefix$u001A$suffix$           # 026    0x1A
        tokenKind[0x001B] = $sym_type.$prefix$u001B$suffix$           # 027    0x1B
        tokenKind[0x001C] = $sym_type.$prefix$u001C$suffix$           # 028    0x1C
        tokenKind[0x001D] = $sym_type.$prefix$u001D$suffix$           # 029    0x1D
        tokenKind[0x001E] = $sym_type.$prefix$u001E$suffix$           # 030    0x1E
        tokenKind[0x001F] = $sym_type.$prefix$u001F$suffix$           # 031    0x1F
        tokenKind[0x0020] = $sym_type.$prefix$u0020$suffix$           # 032    0x20
        tokenKind[0x0021] = $sym_type.$prefix$u0021$suffix$           # 033    0x21
        tokenKind[0x0022] = $sym_type.$prefix$u0022$suffix$           # 034    0x22
        tokenKind[0x0023] = $sym_type.$prefix$u0023$suffix$           # 035    0x23
        tokenKind[0x0024] = $sym_type.$prefix$u0024$suffix$           # 036    0x24
        tokenKind[0x0025] = $sym_type.$prefix$u0025$suffix$           # 037    0x25
        tokenKind[0x0026] = $sym_type.$prefix$u0026$suffix$           # 038    0x26
        tokenKind[0x0027] = $sym_type.$prefix$u0027$suffix$           # 039    0x27
        tokenKind[0x0028] = $sym_type.$prefix$u0028$suffix$           # 040    0x28
        tokenKind[0x0029] = $sym_type.$prefix$u0029$suffix$           # 041    0x29
        tokenKind[0x002A] = $sym_type.$prefix$u002A$suffix$           # 042    0x2A
        tokenKind[0x002B] = $sym_type.$prefix$u002B$suffix$           # 043    0x2B
        tokenKind[0x002C] = $sym_type.$prefix$u002C$suffix$           # 044    0x2C
        tokenKind[0x002D] = $sym_type.$prefix$u002D$suffix$           # 045    0x2D
        tokenKind[0x002E] = $sym_type.$prefix$u002E$suffix$           # 046    0x2E
        tokenKind[0x002F] = $sym_type.$prefix$u002F$suffix$           # 047    0x2F
        tokenKind[0x0030] = $sym_type.$prefix$u0030$suffix$           # 048    0x30
        tokenKind[0x0031] = $sym_type.$prefix$u0031$suffix$           # 049    0x31
        tokenKind[0x0032] = $sym_type.$prefix$u0032$suffix$           # 050    0x32
        tokenKind[0x0033] = $sym_type.$prefix$u0033$suffix$           # 051    0x33
        tokenKind[0x0034] = $sym_type.$prefix$u0034$suffix$           # 052    0x34
        tokenKind[0x0035] = $sym_type.$prefix$u0035$suffix$           # 053    0x35
        tokenKind[0x0036] = $sym_type.$prefix$u0036$suffix$           # 054    0x36
        tokenKind[0x0037] = $sym_type.$prefix$u0037$suffix$           # 055    0x37
        tokenKind[0x0038] = $sym_type.$prefix$u0038$suffix$           # 056    0x38
        tokenKind[0x0039] = $sym_type.$prefix$u0039$suffix$           # 057    0x39
        tokenKind[0x003A] = $sym_type.$prefix$u003A$suffix$           # 058    0x3A
        tokenKind[0x003B] = $sym_type.$prefix$u003B$suffix$           # 059    0x3B
        tokenKind[0x003C] = $sym_type.$prefix$u003C$suffix$           # 060    0x3C
        tokenKind[0x003D] = $sym_type.$prefix$u003D$suffix$           # 061    0x3D
        tokenKind[0x003E] = $sym_type.$prefix$u003E$suffix$           # 062    0x3E
        tokenKind[0x003F] = $sym_type.$prefix$u003F$suffix$           # 063    0x3F
        tokenKind[0x0040] = $sym_type.$prefix$u0040$suffix$           # 064    0x40
        tokenKind[0x0041] = $sym_type.$prefix$u0041$suffix$           # 065    0x41
        tokenKind[0x0042] = $sym_type.$prefix$u0042$suffix$           # 066    0x42
        tokenKind[0x0043] = $sym_type.$prefix$u0043$suffix$           # 067    0x43
        tokenKind[0x0044] = $sym_type.$prefix$u0044$suffix$           # 068    0x44
        tokenKind[0x0045] = $sym_type.$prefix$u0045$suffix$           # 069    0x45
        tokenKind[0x0046] = $sym_type.$prefix$u0046$suffix$           # 070    0x46
        tokenKind[0x0047] = $sym_type.$prefix$u0047$suffix$           # 071    0x47
        tokenKind[0x0048] = $sym_type.$prefix$u0048$suffix$           # 072    0x48
        tokenKind[0x0049] = $sym_type.$prefix$u0049$suffix$           # 073    0x49
        tokenKind[0x004A] = $sym_type.$prefix$u004A$suffix$           # 074    0x4A
        tokenKind[0x004B] = $sym_type.$prefix$u004B$suffix$           # 075    0x4B
        tokenKind[0x004C] = $sym_type.$prefix$u004C$suffix$           # 076    0x4C
        tokenKind[0x004D] = $sym_type.$prefix$u004D$suffix$           # 077    0x4D
        tokenKind[0x004E] = $sym_type.$prefix$u004E$suffix$           # 078    0x4E
        tokenKind[0x004F] = $sym_type.$prefix$u004F$suffix$           # 079    0x4F
        tokenKind[0x0050] = $sym_type.$prefix$u0050$suffix$           # 080    0x50
        tokenKind[0x0051] = $sym_type.$prefix$u0051$suffix$           # 081    0x51
        tokenKind[0x0052] = $sym_type.$prefix$u0052$suffix$           # 082    0x52
        tokenKind[0x0053] = $sym_type.$prefix$u0053$suffix$           # 083    0x53
        tokenKind[0x0054] = $sym_type.$prefix$u0054$suffix$           # 084    0x54
        tokenKind[0x0055] = $sym_type.$prefix$u0055$suffix$           # 085    0x55
        tokenKind[0x0056] = $sym_type.$prefix$u0056$suffix$           # 086    0x56
        tokenKind[0x0057] = $sym_type.$prefix$u0057$suffix$           # 087    0x57
        tokenKind[0x0058] = $sym_type.$prefix$u0058$suffix$           # 088    0x58
        tokenKind[0x0059] = $sym_type.$prefix$u0059$suffix$           # 089    0x59
        tokenKind[0x005A] = $sym_type.$prefix$u005A$suffix$           # 090    0x5A
        tokenKind[0x005B] = $sym_type.$prefix$u005B$suffix$           # 091    0x5B
        tokenKind[0x005C] = $sym_type.$prefix$u005C$suffix$           # 092    0x5C
        tokenKind[0x005D] = $sym_type.$prefix$u005D$suffix$           # 093    0x5D
        tokenKind[0x005E] = $sym_type.$prefix$u005E$suffix$           # 094    0x5E
        tokenKind[0x005F] = $sym_type.$prefix$u005F$suffix$           # 095    0x5F
        tokenKind[0x0060] = $sym_type.$prefix$u0060$suffix$           # 096    0x60
        tokenKind[0x0061] = $sym_type.$prefix$u0061$suffix$           # 097    0x61
        tokenKind[0x0062] = $sym_type.$prefix$u0062$suffix$           # 098    0x62
        tokenKind[0x0063] = $sym_type.$prefix$u0063$suffix$           # 099    0x63
        tokenKind[0x0064] = $sym_type.$prefix$u0064$suffix$           # 100    0x64
        tokenKind[0x0065] = $sym_type.$prefix$u0065$suffix$           # 101    0x65
        tokenKind[0x0066] = $sym_type.$prefix$u0066$suffix$           # 102    0x66
        tokenKind[0x0067] = $sym_type.$prefix$u0067$suffix$           # 103    0x67
        tokenKind[0x0068] = $sym_type.$prefix$u0068$suffix$           # 104    0x68
        tokenKind[0x0069] = $sym_type.$prefix$u0069$suffix$           # 105    0x69
        tokenKind[0x006A] = $sym_type.$prefix$u006A$suffix$           # 106    0x6A
        tokenKind[0x006B] = $sym_type.$prefix$u006B$suffix$           # 107    0x6B
        tokenKind[0x006C] = $sym_type.$prefix$u006C$suffix$           # 108    0x6C
        tokenKind[0x006D] = $sym_type.$prefix$u006D$suffix$           # 109    0x6D
        tokenKind[0x006E] = $sym_type.$prefix$u006E$suffix$           # 110    0x6E
        tokenKind[0x006F] = $sym_type.$prefix$u006F$suffix$           # 111    0x6F
        tokenKind[0x0070] = $sym_type.$prefix$u0070$suffix$           # 112    0x70
        tokenKind[0x0071] = $sym_type.$prefix$u0071$suffix$           # 113    0x71
        tokenKind[0x0072] = $sym_type.$prefix$u0072$suffix$           # 114    0x72
        tokenKind[0x0073] = $sym_type.$prefix$u0073$suffix$           # 115    0x73
        tokenKind[0x0074] = $sym_type.$prefix$u0074$suffix$           # 116    0x74
        tokenKind[0x0075] = $sym_type.$prefix$u0075$suffix$           # 117    0x75
        tokenKind[0x0076] = $sym_type.$prefix$u0076$suffix$           # 118    0x76
        tokenKind[0x0077] = $sym_type.$prefix$u0077$suffix$           # 119    0x77
        tokenKind[0x0078] = $sym_type.$prefix$u0078$suffix$           # 120    0x78
        tokenKind[0x0079] = $sym_type.$prefix$u0079$suffix$           # 121    0x79
        tokenKind[0x007A] = $sym_type.$prefix$u007A$suffix$           # 122    0x7A
        tokenKind[0x007B] = $sym_type.$prefix$u007B$suffix$           # 123    0x7B
        tokenKind[0x007C] = $sym_type.$prefix$u007C$suffix$           # 124    0x7C
        tokenKind[0x007D] = $sym_type.$prefix$u007D$suffix$           # 125    0x7D
        tokenKind[0x007E] = $sym_type.$prefix$u007E$suffix$           # 126    0x7E
        tokenKind[0x007F] = $sym_type.$prefix$u007F$suffix$           # 127    0x7F

        tokenKind[0xFFFF] = $sym_type.$prefix$EOF$suffix$

        #
        # Every other character not yet assigned is treated initially as unused
        #
        for i in range(0x007F, 0xFFFF):
            if  tokenKind[i] == 0: 
                tokenKind[i] = $sym_type.$prefix$UNUSED$suffix$

        
                
        def  getKind(self, i) : # Classify character at ith location
        
            return ( 0xffff if i >= self.getStreamLength() else 
                        $super_stream_class.tokenKind[getIntValue(i)])
        

        def  orderedExportedSymbols(self) :
             return $exp_type.orderedTerminalSymbols 


        def __init__(self, fileName, inputChars= None, tab= 4) :
            super($super_stream_class, self).__init__(fileName, inputChars, tab)
./
%End
%Headers
    --
    -- Additional methods for the action class not provided in the template
    --
    /.
        #
        # The Lexer contains an array of characters as the input stream to be parsed.
        # There are methods to retrieve and classify characters.
        # The lexparser "token" is implemented simply as the index of the next character in the array.
        # The Lexer : the abstract class LpgLexStream with an implementation of the abstract
        # method getKind.  The template defines the Lexer class and the lexer() method.
        # A driver creates the action class, "Lexer", passing an Option object to the constructor.
        #
        

   
        ECLIPSE_TAB_VALUE = 4

        def  getKeywordKinds(self) : 
             return self.kwLexer.getKeywordKinds() 


        def initialize(self,filename, content = None) :
        
            LpgLexStream.initialize(filename,content)
            if self.kwLexer is None:
                self.kwLexer = new $kw_lexer_class(self.getInputChars(), $_IDENTIFIER)
            else :
                self.kwLexer.setInputChars(self.getInputChars())
        
        
        def makeToken(self,kind):
        
            startOffset = self.getLeftSpan()
            endOffset = self.getRightSpan()
            self.makeToken(startOffset, endOffset, kind)
            if self.printTokens :
               self.printValue(startOffset, endOffset)
        

        def makeComment(self,kind ):
        
            startOffset = self.getLeftSpan()
            endOffset = self.getRightSpan()
            LpgLexStream.getPrsStream().makeAdjunct(startOffset, endOffset, kind)
        

        def skipToken(self) : 
        
            if self.printTokens:
               self.printValue(self.getLeftSpan(), self.getRightSpan())
        
        
        def checkForKeyWord(self) : 
        
            startOffset = self.getLeftSpan()
            endOffset = self.getRightSpan()
            kwKind = self.kwLexer.lexer(startOffset, endOffset)
            self.makeToken(startOffset, endOffset, kwKind)
            if self.printTokens: 
               self.printValue(startOffset, endOffset)
        
        
        
        def printValue(self,startOffset , endOffset ) : 
        
            s = self.lexStream.getInputChars()[startOffset : endOffset - startOffset + 1]
            print(s, end='')
        


    ./
%End
