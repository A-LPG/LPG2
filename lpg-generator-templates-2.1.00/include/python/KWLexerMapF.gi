%Terminals
    DollarSign ::= '$'
    Percent ::= '%'
    _
    a b c d e f g h i j k l m n o p q r s t u v w x y z
    A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
%End

%Headers
    /.
        okenKind : list=  [0]*128
       
        tokenKind[ord('$'[0])] = $sym_type.$prefix$DollarSign$suffix$
        tokenKind[ord('%'[0])] = $sym_type.$prefix$Percent$suffix$
        tokenKind[ord('_'[0])] = $sym_type.$prefix$_$suffix$

        tokenKind[ord('a'[0])] = $sym_type.$prefix$a$suffix$
        tokenKind[ord('b'[0])] = $sym_type.$prefix$b$suffix$
        tokenKind[ord('c'[0])] = $sym_type.$prefix$c$suffix$
        tokenKind[ord('d'[0])] = $sym_type.$prefix$d$suffix$
        tokenKind[ord('e'[0])] = $sym_type.$prefix$e$suffix$
        tokenKind[ord('f'[0])] = $sym_type.$prefix$f$suffix$
        tokenKind[ord('g'[0])] = $sym_type.$prefix$g$suffix$
        tokenKind[ord('h'[0])] = $sym_type.$prefix$h$suffix$
        tokenKind[ord('i'[0])] = $sym_type.$prefix$i$suffix$
        tokenKind[ord('j'[0])] = $sym_type.$prefix$j$suffix$
        tokenKind[ord('k'[0])] = $sym_type.$prefix$k$suffix$
        tokenKind[ord('l'[0])] = $sym_type.$prefix$l$suffix$
        tokenKind[ord('m'[0])] = $sym_type.$prefix$m$suffix$
        tokenKind[ord('n'[0])] = $sym_type.$prefix$n$suffix$
        tokenKind[ord('o'[0])] = $sym_type.$prefix$o$suffix$
        tokenKind[ord('p'[0])] = $sym_type.$prefix$p$suffix$
        tokenKind[ord('q'[0])] = $sym_type.$prefix$q$suffix$
        tokenKind[ord('r'[0])] = $sym_type.$prefix$r$suffix$
        tokenKind[ord('s'[0])] = $sym_type.$prefix$s$suffix$
        tokenKind[ord('t'[0])] = $sym_type.$prefix$t$suffix$
        tokenKind[ord('u'[0])] = $sym_type.$prefix$u$suffix$
        tokenKind[ord('v'[0])] = $sym_type.$prefix$v$suffix$
        tokenKind[ord('w'[0])] = $sym_type.$prefix$w$suffix$
        tokenKind[ord('x'[0])] = $sym_type.$prefix$x$suffix$
        tokenKind[ord('y'[0])] = $sym_type.$prefix$y$suffix$
        tokenKind[ord('z'[0])] = $sym_type.$prefix$z$suffix$

        tokenKind[ord('A'[0])] = $sym_type.$prefix$A$suffix$
        tokenKind[ord('B'[0])] = $sym_type.$prefix$B$suffix$
        tokenKind[ord('C'[0])] = $sym_type.$prefix$C$suffix$
        tokenKind[ord('D'[0])] = $sym_type.$prefix$D$suffix$
        tokenKind[ord('E'[0])] = $sym_type.$prefix$E$suffix$
        tokenKind[ord('F'[0])] = $sym_type.$prefix$F$suffix$
        tokenKind[ord('G'[0])] = $sym_type.$prefix$G$suffix$
        tokenKind[ord('H'[0])] = $sym_type.$prefix$H$suffix$
        tokenKind[ord('I'[0])] = $sym_type.$prefix$I$suffix$
        tokenKind[ord('J'[0])] = $sym_type.$prefix$J$suffix$
        tokenKind[ord('K'[0])] = $sym_type.$prefix$K$suffix$
        tokenKind[ord('L'[0])] = $sym_type.$prefix$L$suffix$
        tokenKind[ord('M'[0])] = $sym_type.$prefix$M$suffix$
        tokenKind[ord('N'[0])] = $sym_type.$prefix$N$suffix$
        tokenKind[ord('O'[0])] = $sym_type.$prefix$O$suffix$
        tokenKind[ord('P'[0])] = $sym_type.$prefix$P$suffix$
        tokenKind[ord('Q'[0])] = $sym_type.$prefix$Q$suffix$
        tokenKind[ord('R'[0])] = $sym_type.$prefix$R$suffix$
        tokenKind[ord('S'[0])] = $sym_type.$prefix$S$suffix$
        tokenKind[ord('T'[0])] = $sym_type.$prefix$T$suffix$
        tokenKind[ord('U'[0])] = $sym_type.$prefix$U$suffix$
        tokenKind[ord('V'[0])] = $sym_type.$prefix$V$suffix$
        tokenKind[ord('W'[0])] = $sym_type.$prefix$W$suffix$
        tokenKind[ord('X'[0])] = $sym_type.$prefix$X$suffix$
        tokenKind[ord('Y'[0])] = $sym_type.$prefix$Y$suffix$
        tokenKind[ord('Z'[0])] = $sym_type.$prefix$Z$suffix$

        @classmethod
        def getKind(cls, c: int) -> int:
            # 0 <= c < 128? 
            return ( $action_type.tokenKind[c] if (c & 0xFFFFFF80) == 0  else  0)
        
    ./
%End

