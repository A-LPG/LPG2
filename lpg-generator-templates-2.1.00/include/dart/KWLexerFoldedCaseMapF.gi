%Terminals
    DollarSign ::= '$'
    Percent ::= '%'
    _
    a b c d e f g h i j k l m n o p q r s t u v w x y z
    0    1    2    3    4    5    6    7    8    9
%End

%Headers
    /.
        //
        // Each upper case letter is mapped into its corresponding
        // lower case counterpart. For example, if an 'A' appears
        // in the input, it is mapped into %sym_type.%prefix%a%suffix% just
        // like 'a'.
        //
        static  List<int> init_tokenKind() 
        {
            List<int> tokenKind =  List<int>.filled(128,0);
            tokenKind['\$'.codeUnitAt(0)] = %sym_type.%prefix%DollarSign%suffix%;
            tokenKind['%'.codeUnitAt(0)] = %sym_type.%prefix%Percent%suffix%;
            tokenKind['_'.codeUnitAt(0)] = %sym_type.%prefix%_%suffix%;

            tokenKind['0'.codeUnitAt(0)] = %sym_type.%prefix%0%suffix%;
            tokenKind['1'.codeUnitAt(0)] = %sym_type.%prefix%1%suffix%;
            tokenKind['2'.codeUnitAt(0)] = %sym_type.%prefix%2%suffix%;
            tokenKind['3'.codeUnitAt(0)] = %sym_type.%prefix%3%suffix%;
            tokenKind['4'.codeUnitAt(0)] = %sym_type.%prefix%4%suffix%;
            tokenKind['5'.codeUnitAt(0)] = %sym_type.%prefix%5%suffix%;
            tokenKind['6'.codeUnitAt(0)] = %sym_type.%prefix%6%suffix%;
            tokenKind['7'.codeUnitAt(0)] = %sym_type.%prefix%7%suffix%;
            tokenKind['8'.codeUnitAt(0)] = %sym_type.%prefix%8%suffix%;
            tokenKind['9'.codeUnitAt(0)] = %sym_type.%prefix%9%suffix%;



            tokenKind['a'.codeUnitAt(0)] = %sym_type.%prefix%a%suffix%;
            tokenKind['b'.codeUnitAt(0)] = %sym_type.%prefix%b%suffix%;
            tokenKind['c'.codeUnitAt(0)] = %sym_type.%prefix%c%suffix%;
            tokenKind['d'.codeUnitAt(0)] = %sym_type.%prefix%d%suffix%;
            tokenKind['e'.codeUnitAt(0)] = %sym_type.%prefix%e%suffix%;
            tokenKind['f'.codeUnitAt(0)] = %sym_type.%prefix%f%suffix%;
            tokenKind['g'.codeUnitAt(0)] = %sym_type.%prefix%g%suffix%;
            tokenKind['h'.codeUnitAt(0)] = %sym_type.%prefix%h%suffix%;
            tokenKind['i'.codeUnitAt(0)] = %sym_type.%prefix%i%suffix%;
            tokenKind['j'.codeUnitAt(0)] = %sym_type.%prefix%j%suffix%;
            tokenKind['k'.codeUnitAt(0)] = %sym_type.%prefix%k%suffix%;
            tokenKind['l'.codeUnitAt(0)] = %sym_type.%prefix%l%suffix%;
            tokenKind['m'.codeUnitAt(0)] = %sym_type.%prefix%m%suffix%;
            tokenKind['n'.codeUnitAt(0)] = %sym_type.%prefix%n%suffix%;
            tokenKind['o'.codeUnitAt(0)] = %sym_type.%prefix%o%suffix%;
            tokenKind['p'.codeUnitAt(0)] = %sym_type.%prefix%p%suffix%;
            tokenKind['q'.codeUnitAt(0)] = %sym_type.%prefix%q%suffix%;
            tokenKind['r'.codeUnitAt(0)] = %sym_type.%prefix%r%suffix%;
            tokenKind['s'.codeUnitAt(0)] = %sym_type.%prefix%s%suffix%;
            tokenKind['t'.codeUnitAt(0)] = %sym_type.%prefix%t%suffix%;
            tokenKind['u'.codeUnitAt(0)] = %sym_type.%prefix%u%suffix%;
            tokenKind['v'.codeUnitAt(0)] = %sym_type.%prefix%v%suffix%;
            tokenKind['w'.codeUnitAt(0)] = %sym_type.%prefix%w%suffix%;
            tokenKind['x'.codeUnitAt(0)] = %sym_type.%prefix%x%suffix%;
            tokenKind['y'.codeUnitAt(0)] = %sym_type.%prefix%y%suffix%;
            tokenKind['z'.codeUnitAt(0)] = %sym_type.%prefix%z%suffix%;

            tokenKind['A'.codeUnitAt(0)] = %sym_type.%prefix%a%suffix%;
            tokenKind['B'.codeUnitAt(0)] = %sym_type.%prefix%b%suffix%;
            tokenKind['C'.codeUnitAt(0)] = %sym_type.%prefix%c%suffix%;
            tokenKind['D'.codeUnitAt(0)] = %sym_type.%prefix%d%suffix%;
            tokenKind['E'.codeUnitAt(0)] = %sym_type.%prefix%e%suffix%;
            tokenKind['F'.codeUnitAt(0)] = %sym_type.%prefix%f%suffix%;
            tokenKind['G'.codeUnitAt(0)] = %sym_type.%prefix%g%suffix%;
            tokenKind['H'.codeUnitAt(0)] = %sym_type.%prefix%h%suffix%;
            tokenKind['I'.codeUnitAt(0)] = %sym_type.%prefix%i%suffix%;
            tokenKind['J'.codeUnitAt(0)] = %sym_type.%prefix%j%suffix%;
            tokenKind['K'.codeUnitAt(0)] = %sym_type.%prefix%k%suffix%;
            tokenKind['L'.codeUnitAt(0)] = %sym_type.%prefix%l%suffix%;
            tokenKind['M'.codeUnitAt(0)] = %sym_type.%prefix%m%suffix%;
            tokenKind['N'.codeUnitAt(0)] = %sym_type.%prefix%n%suffix%;
            tokenKind['O'.codeUnitAt(0)] = %sym_type.%prefix%o%suffix%;
            tokenKind['P'.codeUnitAt(0)] = %sym_type.%prefix%p%suffix%;
            tokenKind['Q'.codeUnitAt(0)] = %sym_type.%prefix%q%suffix%;
            tokenKind['R'.codeUnitAt(0)] = %sym_type.%prefix%r%suffix%;
            tokenKind['S'.codeUnitAt(0)] = %sym_type.%prefix%s%suffix%;
            tokenKind['T'.codeUnitAt(0)] = %sym_type.%prefix%t%suffix%;
            tokenKind['U'.codeUnitAt(0)] = %sym_type.%prefix%u%suffix%;
            tokenKind['V'.codeUnitAt(0)] = %sym_type.%prefix%v%suffix%;
            tokenKind['W'.codeUnitAt(0)] = %sym_type.%prefix%w%suffix%;
            tokenKind['X'.codeUnitAt(0)] = %sym_type.%prefix%x%suffix%;
            tokenKind['Y'.codeUnitAt(0)] = %sym_type.%prefix%y%suffix%;
            tokenKind['Z'.codeUnitAt(0)] = %sym_type.%prefix%z%suffix%;
            return tokenKind;
        }
        
        static  final List<int> tokenKind =  init_tokenKind(); 

        static  int getKind(int c )
        {
            return (c < 128 ? %action_type.tokenKind[c] : 0);
        }
    ./
%End

