%Terminals
    DollarSign ::= '$'
    Percent ::= '%'
    _
    a b c d e f g h i j k l m n o p q r s t u v w x y z
    0    1    2    3    4    5    6    7    8    9
%End

%Globals
    /.
    //
    // Each upper case letter is mapped into its corresponding
    // lower case counterpart. For example, if an 'A' appears
    // in the input, it is mapped into $sym_type.$prefix$a$suffix$ just
    // like 'a'.
    //
    static $action_type$_TOKEN_KIND: [i32; 128] = {
        let mut token_kind = [0i32; 128];
        token_kind[b'$' as usize] = $sym_type::$prefix$DollarSign$suffix$;
        token_kind[b'%' as usize] = $sym_type::$prefix$Percent$suffix$;
        token_kind[b'_' as usize] = $sym_type::$prefix$_$suffix$;

        token_kind[b'0' as usize] = $sym_type::$prefix$0$suffix$;
        token_kind[b'1' as usize] = $sym_type::$prefix$1$suffix$;
        token_kind[b'2' as usize] = $sym_type::$prefix$2$suffix$;
        token_kind[b'3' as usize] = $sym_type::$prefix$3$suffix$;
        token_kind[b'4' as usize] = $sym_type::$prefix$4$suffix$;
        token_kind[b'5' as usize] = $sym_type::$prefix$5$suffix$;
        token_kind[b'6' as usize] = $sym_type::$prefix$6$suffix$;
        token_kind[b'7' as usize] = $sym_type::$prefix$7$suffix$;
        token_kind[b'8' as usize] = $sym_type::$prefix$8$suffix$;
        token_kind[b'9' as usize] = $sym_type::$prefix$9$suffix$;

        token_kind[b'a' as usize] = $sym_type::$prefix$a$suffix$;
        token_kind[b'b' as usize] = $sym_type::$prefix$b$suffix$;
        token_kind[b'c' as usize] = $sym_type::$prefix$c$suffix$;
        token_kind[b'd' as usize] = $sym_type::$prefix$d$suffix$;
        token_kind[b'e' as usize] = $sym_type::$prefix$e$suffix$;
        token_kind[b'f' as usize] = $sym_type::$prefix$f$suffix$;
        token_kind[b'g' as usize] = $sym_type::$prefix$g$suffix$;
        token_kind[b'h' as usize] = $sym_type::$prefix$h$suffix$;
        token_kind[b'i' as usize] = $sym_type::$prefix$i$suffix$;
        token_kind[b'j' as usize] = $sym_type::$prefix$j$suffix$;
        token_kind[b'k' as usize] = $sym_type::$prefix$k$suffix$;
        token_kind[b'l' as usize] = $sym_type::$prefix$l$suffix$;
        token_kind[b'm' as usize] = $sym_type::$prefix$m$suffix$;
        token_kind[b'n' as usize] = $sym_type::$prefix$n$suffix$;
        token_kind[b'o' as usize] = $sym_type::$prefix$o$suffix$;
        token_kind[b'p' as usize] = $sym_type::$prefix$p$suffix$;
        token_kind[b'q' as usize] = $sym_type::$prefix$q$suffix$;
        token_kind[b'r' as usize] = $sym_type::$prefix$r$suffix$;
        token_kind[b's' as usize] = $sym_type::$prefix$s$suffix$;
        token_kind[b't' as usize] = $sym_type::$prefix$t$suffix$;
        token_kind[b'u' as usize] = $sym_type::$prefix$u$suffix$;
        token_kind[b'v' as usize] = $sym_type::$prefix$v$suffix$;
        token_kind[b'w' as usize] = $sym_type::$prefix$w$suffix$;
        token_kind[b'x' as usize] = $sym_type::$prefix$x$suffix$;
        token_kind[b'y' as usize] = $sym_type::$prefix$y$suffix$;
        token_kind[b'z' as usize] = $sym_type::$prefix$z$suffix$;

        token_kind[b'A' as usize] = $sym_type::$prefix$a$suffix$;
        token_kind[b'B' as usize] = $sym_type::$prefix$b$suffix$;
        token_kind[b'C' as usize] = $sym_type::$prefix$c$suffix$;
        token_kind[b'D' as usize] = $sym_type::$prefix$d$suffix$;
        token_kind[b'E' as usize] = $sym_type::$prefix$e$suffix$;
        token_kind[b'F' as usize] = $sym_type::$prefix$f$suffix$;
        token_kind[b'G' as usize] = $sym_type::$prefix$g$suffix$;
        token_kind[b'H' as usize] = $sym_type::$prefix$h$suffix$;
        token_kind[b'I' as usize] = $sym_type::$prefix$i$suffix$;
        token_kind[b'J' as usize] = $sym_type::$prefix$j$suffix$;
        token_kind[b'K' as usize] = $sym_type::$prefix$k$suffix$;
        token_kind[b'L' as usize] = $sym_type::$prefix$l$suffix$;
        token_kind[b'M' as usize] = $sym_type::$prefix$m$suffix$;
        token_kind[b'N' as usize] = $sym_type::$prefix$n$suffix$;
        token_kind[b'O' as usize] = $sym_type::$prefix$o$suffix$;
        token_kind[b'P' as usize] = $sym_type::$prefix$p$suffix$;
        token_kind[b'Q' as usize] = $sym_type::$prefix$q$suffix$;
        token_kind[b'R' as usize] = $sym_type::$prefix$r$suffix$;
        token_kind[b'S' as usize] = $sym_type::$prefix$s$suffix$;
        token_kind[b'T' as usize] = $sym_type::$prefix$t$suffix$;
        token_kind[b'U' as usize] = $sym_type::$prefix$u$suffix$;
        token_kind[b'V' as usize] = $sym_type::$prefix$v$suffix$;
        token_kind[b'W' as usize] = $sym_type::$prefix$w$suffix$;
        token_kind[b'X' as usize] = $sym_type::$prefix$x$suffix$;
        token_kind[b'Y' as usize] = $sym_type::$prefix$y$suffix$;
        token_kind[b'Z' as usize] = $sym_type::$prefix$z$suffix$;
        token_kind
    };
    ./
%End

%Headers
    /.
        pub fn get_kind(&self, c: char) -> i32 {
            let code = c as u32;
            if code < 128 {
                $action_type$_TOKEN_KIND[code as usize]
            } else {
                0
            }
        }
    ./
%End

