%Terminals
    DollarSign ::= '$'
    Percent ::= '%'
    _
    
    a    b    c    d    e    f    g    h    i    j    k    l    m
    n    o    p    q    r    s    t    u    v    w    x    y    z
%End

%Headers
    /.
        static   tokenKind : number[]=  new Array(128)  ; 
         static  __b_init : boolean = %action_type.init_block(%action_type.tokenKind);
        static  init_block(tokenKind : number[]) : boolean
        {
            for (let i = 0; i < tokenKind.length; ++i) {
                tokenKind[i] = 0;
            }
            tokenKind['$'.charCodeAt(0)] = %sym_type.%prefix%DollarSign%suffix%;
            tokenKind['%'.charCodeAt(0)] = %sym_type.%prefix%Percent%suffix%;
            tokenKind['_'.charCodeAt(0)] = %sym_type.%prefix%_%suffix%;
            
            tokenKind['a'.charCodeAt(0)] = %sym_type.%prefix%a%suffix%;
            tokenKind['b'.charCodeAt(0)] = %sym_type.%prefix%b%suffix%;
            tokenKind['c'.charCodeAt(0)] = %sym_type.%prefix%c%suffix%;
            tokenKind['d'.charCodeAt(0)] = %sym_type.%prefix%d%suffix%;
            tokenKind['e'.charCodeAt(0)] = %sym_type.%prefix%e%suffix%;
            tokenKind['f'.charCodeAt(0)] = %sym_type.%prefix%f%suffix%;
            tokenKind['g'.charCodeAt(0)] = %sym_type.%prefix%g%suffix%;
            tokenKind['h'.charCodeAt(0)] = %sym_type.%prefix%h%suffix%;
            tokenKind['i'.charCodeAt(0)] = %sym_type.%prefix%i%suffix%;
            tokenKind['j'.charCodeAt(0)] = %sym_type.%prefix%j%suffix%;
            tokenKind['k'.charCodeAt(0)] = %sym_type.%prefix%k%suffix%;
            tokenKind['l'.charCodeAt(0)] = %sym_type.%prefix%l%suffix%;
            tokenKind['m'.charCodeAt(0)] = %sym_type.%prefix%m%suffix%;
            tokenKind['n'.charCodeAt(0)] = %sym_type.%prefix%n%suffix%;
            tokenKind['o'.charCodeAt(0)] = %sym_type.%prefix%o%suffix%;
            tokenKind['p'.charCodeAt(0)] = %sym_type.%prefix%p%suffix%;
            tokenKind['q'.charCodeAt(0)] = %sym_type.%prefix%q%suffix%;
            tokenKind['r'.charCodeAt(0)] = %sym_type.%prefix%r%suffix%;
            tokenKind['s'.charCodeAt(0)] = %sym_type.%prefix%s%suffix%;
            tokenKind['t'.charCodeAt(0)] = %sym_type.%prefix%t%suffix%;
            tokenKind['u'.charCodeAt(0)] = %sym_type.%prefix%u%suffix%;
            tokenKind['v'.charCodeAt(0)] = %sym_type.%prefix%v%suffix%;
            tokenKind['w'.charCodeAt(0)] = %sym_type.%prefix%w%suffix%;
            tokenKind['x'.charCodeAt(0)] = %sym_type.%prefix%x%suffix%;
            tokenKind['y'.charCodeAt(0)] = %sym_type.%prefix%y%suffix%;
            tokenKind['z'.charCodeAt(0)] = %sym_type.%prefix%z%suffix%;
            return true;
        }
    
        public  static    getKind(c :number ):number
        {
            return ((c & 0xFFFFFF80) == 0 /* 0 <= c < 128? */ ? %action_type.tokenKind[c] : 0);
        }
    ./
%End

