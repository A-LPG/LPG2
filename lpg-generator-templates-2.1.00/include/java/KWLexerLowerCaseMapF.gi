%Terminals
    DollarSign ::= '$'
    Percent ::= '%'
    _
    0    1    2    3    4    5    6    7    8    9
    a    b    c    d    e    f    g    h    i    j    k    l    m
    n    o    p    q    r    s    t    u    v    w    x    y    z
%End

%Headers
    /.
        final static int tokenKind[] = new int[128];
        static
        {
            tokenKind['$'] = $sym_type.$prefix$DollarSign$suffix$;
            tokenKind['%'] = $sym_type.$prefix$Percent$suffix$;
            tokenKind['_'] = $sym_type.$prefix$_$suffix$;

            tokenKind['0'] = $sym_type.$prefix$0$suffix$;
            tokenKind['1'] = $sym_type.$prefix$1$suffix$;
            tokenKind['2'] = $sym_type.$prefix$2$suffix$;
            tokenKind['3'] = $sym_type.$prefix$3$suffix$;
            tokenKind['4'] = $sym_type.$prefix$4$suffix$;
            tokenKind['5'] = $sym_type.$prefix$5$suffix$;
            tokenKind['6'] = $sym_type.$prefix$6$suffix$;
            tokenKind['7'] = $sym_type.$prefix$7$suffix$;
            tokenKind['8'] = $sym_type.$prefix$8$suffix$;
            tokenKind['9'] = $sym_type.$prefix$9$suffix$;

            tokenKind['a'] = $sym_type.$prefix$a$suffix$;
            tokenKind['b'] = $sym_type.$prefix$b$suffix$;
            tokenKind['c'] = $sym_type.$prefix$c$suffix$;
            tokenKind['d'] = $sym_type.$prefix$d$suffix$;
            tokenKind['e'] = $sym_type.$prefix$e$suffix$;
            tokenKind['f'] = $sym_type.$prefix$f$suffix$;
            tokenKind['g'] = $sym_type.$prefix$g$suffix$;
            tokenKind['h'] = $sym_type.$prefix$h$suffix$;
            tokenKind['i'] = $sym_type.$prefix$i$suffix$;
            tokenKind['j'] = $sym_type.$prefix$j$suffix$;
            tokenKind['k'] = $sym_type.$prefix$k$suffix$;
            tokenKind['l'] = $sym_type.$prefix$l$suffix$;
            tokenKind['m'] = $sym_type.$prefix$m$suffix$;
            tokenKind['n'] = $sym_type.$prefix$n$suffix$;
            tokenKind['o'] = $sym_type.$prefix$o$suffix$;
            tokenKind['p'] = $sym_type.$prefix$p$suffix$;
            tokenKind['q'] = $sym_type.$prefix$q$suffix$;
            tokenKind['r'] = $sym_type.$prefix$r$suffix$;
            tokenKind['s'] = $sym_type.$prefix$s$suffix$;
            tokenKind['t'] = $sym_type.$prefix$t$suffix$;
            tokenKind['u'] = $sym_type.$prefix$u$suffix$;
            tokenKind['v'] = $sym_type.$prefix$v$suffix$;
            tokenKind['w'] = $sym_type.$prefix$w$suffix$;
            tokenKind['x'] = $sym_type.$prefix$x$suffix$;
            tokenKind['y'] = $sym_type.$prefix$y$suffix$;
            tokenKind['z'] = $sym_type.$prefix$z$suffix$;
        };
    
        final int getKind(int c)
        {
            return ((c & 0xFFFFFF80) == 0 /* 0 <= c < 128? */ ? tokenKind[c] : 0);
        }
    ./
%End

