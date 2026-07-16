public interface minimalsym {
    public const int
      a = 1,
      EOF_TOKEN = 2;

    public static string[] orderedTerminalSymbols = {
                 "",
                 "a",
                 "EOF_TOKEN"
             };

    public const int numTokenKinds = 2;


    public const bool isValidForParser = true;
}
