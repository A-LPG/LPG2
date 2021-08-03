%Terminals
    DollarSign ::= '$'
    Percent ::= '%'
    _
    a b c d e f g h i j k l m n o p q r s t u v w x y z
%End

%Headers
    /.
        //
        // Each upper case letter is mapped into its corresponding
        // lower case counterpart. For example, if an 'A' appears
        // in the input, it is mapped into $sym_type.$prefix$a$suffix$ just
        // like 'a'.
        //
       public  static   tokenKind : number[]=  new Array(128)  ; 
         static  __b_init : boolean = $prs_type.init_block();
        static  init_block() : boolean
        {
            for (let i = 0; i < tokenKind.length; ++i) {
                tokenKind[i] = i;
            }
            tokenKind['$'.charCodeAt(0)] = $sym_type.$prefix$DollarSign$suffix$;
            tokenKind['%'.charCodeAt(0)] = $sym_type.$prefix$Percent$suffix$;
            tokenKind['_'.charCodeAt(0)] = $sym_type.$prefix$_$suffix$;

            tokenKind['a'.charCodeAt(0)] = $sym_type.$prefix$a$suffix$;
            tokenKind['b'.charCodeAt(0)] = $sym_type.$prefix$b$suffix$;
            tokenKind['c'.charCodeAt(0)] = $sym_type.$prefix$c$suffix$;
            tokenKind['d'.charCodeAt(0)] = $sym_type.$prefix$d$suffix$;
            tokenKind['e'.charCodeAt(0)] = $sym_type.$prefix$e$suffix$;
            tokenKind['f'.charCodeAt(0)] = $sym_type.$prefix$f$suffix$;
            tokenKind['g'.charCodeAt(0)] = $sym_type.$prefix$g$suffix$;
            tokenKind['h'.charCodeAt(0)] = $sym_type.$prefix$h$suffix$;
            tokenKind['i'.charCodeAt(0)] = $sym_type.$prefix$i$suffix$;
            tokenKind['j'.charCodeAt(0)] = $sym_type.$prefix$j$suffix$;
            tokenKind['k'.charCodeAt(0)] = $sym_type.$prefix$k$suffix$;
            tokenKind['l'.charCodeAt(0)] = $sym_type.$prefix$l$suffix$;
            tokenKind['m'.charCodeAt(0)] = $sym_type.$prefix$m$suffix$;
            tokenKind['n'.charCodeAt(0)] = $sym_type.$prefix$n$suffix$;
            tokenKind['o'.charCodeAt(0)] = $sym_type.$prefix$o$suffix$;
            tokenKind['p'.charCodeAt(0)] = $sym_type.$prefix$p$suffix$;
            tokenKind['q'.charCodeAt(0)] = $sym_type.$prefix$q$suffix$;
            tokenKind['r'.charCodeAt(0)] = $sym_type.$prefix$r$suffix$;
            tokenKind['s'.charCodeAt(0)] = $sym_type.$prefix$s$suffix$;
            tokenKind['t'.charCodeAt(0)] = $sym_type.$prefix$t$suffix$;
            tokenKind['u'.charCodeAt(0)] = $sym_type.$prefix$u$suffix$;
            tokenKind['v'.charCodeAt(0)] = $sym_type.$prefix$v$suffix$;
            tokenKind['w'.charCodeAt(0)] = $sym_type.$prefix$w$suffix$;
            tokenKind['x'.charCodeAt(0)] = $sym_type.$prefix$x$suffix$;
            tokenKind['y'.charCodeAt(0)] = $sym_type.$prefix$y$suffix$;
            tokenKind['z'.charCodeAt(0)] = $sym_type.$prefix$z$suffix$;

            tokenKind['A'.charCodeAt(0)] = $sym_type.$prefix$a$suffix$;
            tokenKind['B'.charCodeAt(0)] = $sym_type.$prefix$b$suffix$;
            tokenKind['C'.charCodeAt(0)] = $sym_type.$prefix$c$suffix$;
            tokenKind['D'.charCodeAt(0)] = $sym_type.$prefix$d$suffix$;
            tokenKind['E'.charCodeAt(0)] = $sym_type.$prefix$e$suffix$;
            tokenKind['F'.charCodeAt(0)] = $sym_type.$prefix$f$suffix$;
            tokenKind['G'.charCodeAt(0)] = $sym_type.$prefix$g$suffix$;
            tokenKind['H'.charCodeAt(0)] = $sym_type.$prefix$h$suffix$;
            tokenKind['I'.charCodeAt(0)] = $sym_type.$prefix$i$suffix$;
            tokenKind['J'.charCodeAt(0)] = $sym_type.$prefix$j$suffix$;
            tokenKind['K'.charCodeAt(0)] = $sym_type.$prefix$k$suffix$;
            tokenKind['L'.charCodeAt(0)] = $sym_type.$prefix$l$suffix$;
            tokenKind['M'.charCodeAt(0)] = $sym_type.$prefix$m$suffix$;
            tokenKind['N'.charCodeAt(0)] = $sym_type.$prefix$n$suffix$;
            tokenKind['O'.charCodeAt(0)] = $sym_type.$prefix$o$suffix$;
            tokenKind['P'.charCodeAt(0)] = $sym_type.$prefix$p$suffix$;
            tokenKind['Q'.charCodeAt(0)] = $sym_type.$prefix$q$suffix$;
            tokenKind['R'.charCodeAt(0)] = $sym_type.$prefix$r$suffix$;
            tokenKind['S'.charCodeAt(0)] = $sym_type.$prefix$s$suffix$;
            tokenKind['T'.charCodeAt(0)] = $sym_type.$prefix$t$suffix$;
            tokenKind['U'.charCodeAt(0)] = $sym_type.$prefix$u$suffix$;
            tokenKind['V'.charCodeAt(0)] = $sym_type.$prefix$v$suffix$;
            tokenKind['W'.charCodeAt(0)] = $sym_type.$prefix$w$suffix$;
            tokenKind['X'.charCodeAt(0)] = $sym_type.$prefix$x$suffix$;
            tokenKind['Y'.charCodeAt(0)] = $sym_type.$prefix$y$suffix$;
            tokenKind['Z'.charCodeAt(0)] = $sym_type.$prefix$z$suffix$;
            return true;
        }
    
       public  static    getKind(c :number ):number
        {
            return (c < 128 ? tokenKind[c] : 0);
        }
    ./
%End

