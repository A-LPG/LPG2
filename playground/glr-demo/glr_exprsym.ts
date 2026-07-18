export namespace  glr_exprsym {
     export const TK_NUMBER :number  = 2;
  export const TK_PLUS :number  = 1;
  export const TK_EOF_TOKEN :number  = 3;
  export const TK_ERROR_TOKEN :number  = 4;

  export const orderedTerminalSymbols  : string[]= [
  '',
  'PLUS',
  'NUMBER',
  'EOF_TOKEN',
  'ERROR_TOKEN'
     ];

  export const numTokenKinds : number  = 5;
  export const RULE_E :number  = 1;

  export const orderedRuleNames : string[]= [
  '',
  'E',
  'E'
     ];

  export const numRuleNames : number  = 2;

  export const isValidForParser : boolean = true;
}
