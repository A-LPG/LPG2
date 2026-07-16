type __minimalsym__ struct{
   a int
   EOF_TOKEN int

   OrderedTerminalSymbols []string

   NumTokenKinds int


   IsValidForParser  bool
}
func New__minimalsym__() *__minimalsym__{
    my := new(__minimalsym__)
   my.a = 1
   my.EOF_TOKEN = 2

   my.OrderedTerminalSymbols = []string{
                 "",
                 "a",
                 "EOF_TOKEN",
             }

   my.NumTokenKinds = 2


   my.IsValidForParser = true
   return my
}
var minimalsym = New__minimalsym__()
