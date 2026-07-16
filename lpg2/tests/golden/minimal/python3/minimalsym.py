class minimalsym(object):
   a: int  = 1
   EOF_TOKEN: int  = 2

   orderedTerminalSymbols: list = [
                 "",
                 "a",
                 "EOF_TOKEN"
             ]

   numTokenKinds: int = 2

   isValidForParser : bool = True

