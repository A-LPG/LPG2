#pragma once
 #include <vector>
#include<string>
  struct minimalsym {
     typedef  unsigned char byte;
      static constexpr int
      a = 1,
      EOF_TOKEN = 2;

      inline const static std::vector<std::wstring> orderedTerminalSymbols = {
                 L"",
                 L"a",
                 L"EOF_TOKEN"
             };

     static constexpr  int numTokenKinds = 2;

     static constexpr  bool isValidForParser = true;
};
