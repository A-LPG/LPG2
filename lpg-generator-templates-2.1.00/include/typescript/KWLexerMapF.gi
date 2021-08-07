%Terminals
    DollarSign ::= '$'
    Percent ::= '%'
    _
    a b c d e f g h i j k l m n o p q r s t u v w x y z
    A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
%End

%Headers
    /.
        static   tokenKind : number[]=  new Array(128)  ; 
        static  __b_init : boolean = %action_type.init_block(%action_type.tokenKind);
        static  init_block(tokenKind : number[]) : boolean
        {
            for (let i = 0; i < tokenKind.length; ++i) {
                tokenKind[i] = i;
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

            tokenKind['A'.charCodeAt(0)] = %sym_type.%prefix%A%suffix%;
            tokenKind['B'.charCodeAt(0)] = %sym_type.%prefix%B%suffix%;
            tokenKind['C'.charCodeAt(0)] = %sym_type.%prefix%C%suffix%;
            tokenKind['D'.charCodeAt(0)] = %sym_type.%prefix%D%suffix%;
            tokenKind['E'.charCodeAt(0)] = %sym_type.%prefix%E%suffix%;
            tokenKind['F'.charCodeAt(0)] = %sym_type.%prefix%F%suffix%;
            tokenKind['G'.charCodeAt(0)] = %sym_type.%prefix%G%suffix%;
            tokenKind['H'.charCodeAt(0)] = %sym_type.%prefix%H%suffix%;
            tokenKind['I'.charCodeAt(0)] = %sym_type.%prefix%I%suffix%;
            tokenKind['J'.charCodeAt(0)] = %sym_type.%prefix%J%suffix%;
            tokenKind['K'.charCodeAt(0)] = %sym_type.%prefix%K%suffix%;
            tokenKind['L'.charCodeAt(0)] = %sym_type.%prefix%L%suffix%;
            tokenKind['M'.charCodeAt(0)] = %sym_type.%prefix%M%suffix%;
            tokenKind['N'.charCodeAt(0)] = %sym_type.%prefix%N%suffix%;
            tokenKind['O'.charCodeAt(0)] = %sym_type.%prefix%O%suffix%;
            tokenKind['P'.charCodeAt(0)] = %sym_type.%prefix%P%suffix%;
            tokenKind['Q'.charCodeAt(0)] = %sym_type.%prefix%Q%suffix%;
            tokenKind['R'.charCodeAt(0)] = %sym_type.%prefix%R%suffix%;
            tokenKind['S'.charCodeAt(0)] = %sym_type.%prefix%S%suffix%;
            tokenKind['T'.charCodeAt(0)] = %sym_type.%prefix%T%suffix%;
            tokenKind['U'.charCodeAt(0)] = %sym_type.%prefix%U%suffix%;
            tokenKind['V'.charCodeAt(0)] = %sym_type.%prefix%V%suffix%;
            tokenKind['W'.charCodeAt(0)] = %sym_type.%prefix%W%suffix%;
            tokenKind['X'.charCodeAt(0)] = %sym_type.%prefix%X%suffix%;
            tokenKind['Y'.charCodeAt(0)] = %sym_type.%prefix%Y%suffix%;
            tokenKind['Z'.charCodeAt(0)] = %sym_type.%prefix%Z%suffix%;
            return true;
        }
    
        public  static    getKind(c : number ) : number
        {
            return (((c & 0xFFFFFF80) == 0) /* 0 <= c < 128? */ ? %action_type.tokenKind[c] : 0);
        }
    ./
%End

