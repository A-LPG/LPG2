var __create = Object.create;
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __getProtoOf = Object.getPrototypeOf;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __esm = (fn, res) => function __init() {
  return fn && (res = (0, fn[__getOwnPropNames(fn)[0]])(fn = 0)), res;
};
var __commonJS = (cb, mod) => function __require() {
  return mod || (0, cb[__getOwnPropNames(cb)[0]])((mod = { exports: {} }).exports, mod), mod.exports;
};
var __export = (target, all) => {
  for (var name in all)
    __defProp(target, name, { get: all[name], enumerable: true });
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toESM = (mod, isNodeMode, target) => (target = mod != null ? __create(__getProtoOf(mod)) : {}, __copyProps(
  // If the importer is in node compatibility mode or this is not an ESM
  // file that has been converted to a CommonJS file using a Babel-
  // compatible transform (i.e. "__esModule" has not been set), then set
  // "default" to the CommonJS "module.exports" for node compatibility.
  isNodeMode || !mod || !mod.__esModule ? __defProp(target, "default", { value: mod, enumerable: true }) : target,
  mod
));
var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

// runtime/LPG-typescript-runtime/lpg2ts/dist/AbstractToken.js
var require_AbstractToken = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/AbstractToken.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.AbstractToken = void 0;
    var AbstractToken = class {
      constructor(startOffset, endOffset, kind, iPrsStream) {
        this.kind = 0;
        this.startOffset = 0;
        this.endOffset = 0;
        this.tokenIndex = 0;
        this.adjunctIndex = 0;
        this.iPrsStream = iPrsStream;
        this.startOffset = startOffset;
        this.endOffset = endOffset;
        this.kind = kind;
      }
      getKind() {
        return this.kind;
      }
      setKind(kind) {
        this.kind = kind;
      }
      getStartOffset() {
        return this.startOffset;
      }
      setStartOffset(startOffset) {
        this.startOffset = startOffset;
      }
      getEndOffset() {
        return this.endOffset;
      }
      setEndOffset(endOffset) {
        this.endOffset = endOffset;
      }
      getTokenIndex() {
        return this.tokenIndex;
      }
      setTokenIndex(tokenIndex) {
        this.tokenIndex = tokenIndex;
      }
      setAdjunctIndex(adjunctIndex) {
        this.adjunctIndex = adjunctIndex;
      }
      getAdjunctIndex() {
        return this.adjunctIndex;
      }
      getIPrsStream() {
        return this.iPrsStream;
      }
      getILexStream() {
        var _a;
        return (_a = this.iPrsStream) === null || _a === void 0 ? void 0 : _a.getILexStream();
      }
      getLine() {
        var _a;
        let ret = (_a = this.iPrsStream) === null || _a === void 0 ? void 0 : _a.getILexStream().getLineNumberOfCharAt(this.startOffset);
        if (ret)
          return ret;
        return 0;
      }
      getColumn() {
        var _a;
        let ret = (_a = this.iPrsStream) === null || _a === void 0 ? void 0 : _a.getILexStream().getColumnOfCharAt(this.startOffset);
        if (ret)
          return ret;
        return 0;
      }
      getEndLine() {
        var _a;
        let ret = (_a = this.iPrsStream) === null || _a === void 0 ? void 0 : _a.getILexStream().getLineNumberOfCharAt(this.endOffset);
        if (ret)
          return ret;
        return 0;
      }
      getEndColumn() {
        var _a;
        let ret = (_a = this.iPrsStream) === null || _a === void 0 ? void 0 : _a.getILexStream().getColumnOfCharAt(this.endOffset);
        if (ret)
          return ret;
        return 0;
      }
      toString() {
        return this.iPrsStream == void 0 ? "<toString>" : this.iPrsStream.toString(this, this);
      }
    };
    exports.AbstractToken = AbstractToken;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/Adjunct.js
var require_Adjunct = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/Adjunct.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.Adjunct = void 0;
    var AbstractToken_1 = require_AbstractToken();
    var Adjunct = class extends AbstractToken_1.AbstractToken {
      constructor(startOffset, endOffset, kind, prsStream) {
        super(startOffset, endOffset, kind, prsStream);
      }
      getFollowingAdjuncts() {
        return [];
      }
      getPrecedingAdjuncts() {
        return [];
      }
    };
    exports.Adjunct = Adjunct;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/Utils.js
var require_Utils = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/Utils.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.Lpg = void 0;
    var Lpg2;
    (function(Lpg3) {
      let Lang;
      (function(Lang2) {
        let System;
        (function(System2) {
          function arraycopy(src, srcPos, dest, destPos, numElements) {
            if ((dest instanceof Float64Array || dest instanceof Int32Array) && (src instanceof Float64Array || src instanceof Int32Array)) {
              if (numElements == src.length) {
                dest.set(src, destPos);
              } else {
                dest.set(src.subarray(srcPos, srcPos + numElements), destPos);
              }
            } else {
              for (let i = 0; i < numElements; i++) {
                dest[destPos + i] = src[srcPos + i];
              }
            }
          }
          System2.arraycopy = arraycopy;
          class Out {
            static print(message, ...optionalParams) {
              console.log(message, optionalParams);
            }
            static println(message, ...optionalParams) {
              console.log(message, optionalParams);
              console.log("\n");
            }
          }
          System2.Out = Out;
        })(System = Lang2.System || (Lang2.System = {}));
        ;
      })(Lang = Lpg3.Lang || (Lpg3.Lang = {}));
      let Util;
      (function(Util2) {
        class Collections {
          static swap(list, i, j) {
            const l = list;
            l.set(i, l.set(j, l.get(i)));
          }
        }
        Util2.Collections = Collections;
        class Itr {
          constructor(list) {
            this.cursor = 0;
            this.lastRet = -1;
            this.list = list;
          }
          hasNext() {
            return this.cursor !== this.list.size();
          }
          next() {
            try {
              let i = this.cursor;
              let next = this.list.get(i);
              this.lastRet = i;
              this.cursor = i + 1;
              return next;
            } catch ($ex$) {
              if ($ex$ instanceof Error) {
                let e = $ex$;
                throw new Error("no such element exception");
              } else {
                throw $ex$;
              }
            }
          }
        }
        Util2.Itr = Itr;
        class AbstractList {
          constructor() {
            this.content = [];
          }
          needInterger() {
            throw Error(" AbstractList  need interger index");
          }
          clone() {
            let result = new AbstractList();
            for (let i = 0; i < this.content.length; i++) {
              result.content.push(this.content[i]);
            }
            return result;
          }
          addAll(index, vals) {
            if (typeof vals !== "undefined") {
              let tempArray = vals.toArray();
              for (let i = 0; i < tempArray.length; i++) {
                this.add(index, tempArray[i]);
              }
              return true;
            } else {
              let tempArray = index.toArray();
              for (let i = 0; i < tempArray.length; i++) {
                this.content.push(tempArray[i]);
              }
              return true;
            }
          }
          clear() {
            this.content = [];
          }
          poll() {
            return this.content.shift();
          }
          remove(indexOrElem) {
            this.content.splice(indexOrElem, 1);
            return true;
          }
          removeAll() {
            this.content = [];
            return true;
          }
          toArray() {
            let result = [];
            for (let entry of this.content) {
              result.push(entry);
            }
            return result;
          }
          size() {
            return this.content.length;
          }
          add(index, elem) {
            if (typeof elem !== "undefined") {
              if (!Number.isInteger(index)) {
                this.needInterger();
              }
              this.content.splice(index, 0, elem);
            } else {
              this.content.push(index);
            }
          }
          get(index) {
            if (!Number.isInteger(index)) {
              this.needInterger();
            }
            return this.content[index];
          }
          contains(val) {
            return this.content.indexOf(val) != -1;
          }
          isEmpty() {
            return this.content.length == 0;
          }
          set(index, element) {
            if (!Number.isInteger(index)) {
              this.needInterger();
            }
            this.content[index] = element;
            return element;
          }
          indexOf(element) {
            return this.content.indexOf(element);
          }
          lastIndexOf(element) {
            return this.content.lastIndexOf(element);
          }
          iterator() {
            return new Itr(this);
          }
        }
        Util2.AbstractList = AbstractList;
        class ArrayList extends AbstractList {
        }
        Util2.ArrayList = ArrayList;
      })(Util = Lpg3.Util || (Lpg3.Util = {}));
    })(Lpg2 = exports.Lpg || (exports.Lpg = {}));
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/Stacks.js
var require_Stacks = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/Stacks.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.Stacks = void 0;
    var Utils_1 = require_Utils();
    var Stacks = class {
      constructor() {
        this.STACK_INCREMENT = 1024;
        this.stateStackTop = 0;
        this.stateStack = new Int32Array(0);
        this.locationStack = new Int32Array(0);
        this.parseStack = new Array();
      }
      //
      // Given a rule of the form     A ::= x1 x2 ... xn     n > 0
      //
      // the function GETTOKEN(i) yields the symbol xi, if xi is a terminal
      // or ti, if xi is a nonterminal that produced a string of the form
      // xi => ti w.
      //
      getToken(i) {
        return this.locationStack[this.stateStackTop + (i - 1)];
      }
      //
      // Given a rule of the form     A ::= x1 x2 ... xn     n > 0
      //
      // The function GETSYM(i) yields the AST subtree associated with symbol
      // xi. NOTE that if xi is a terminal, GETSYM(i) is undefined ! (However,
      // see token_action below.)
      //
      // setSYM1(Object ast) is a function that allows us to assign an AST
      // tree to GETSYM(1).
      //
      getSym(i) {
        return this.parseStack[this.stateStackTop + (i - 1)];
      }
      setSym1(ast) {
        this.parseStack[this.stateStackTop] = ast;
      }
      //
      // Allocate or reallocate all the stacks. Their sizes should always be the same.
      //
      reallocateStacks() {
        let old_stack_length = this.stateStack == void 0 ? 0 : this.stateStack.length, stack_length = old_stack_length + this.STACK_INCREMENT;
        if (!this.stateStack || this.stateStack.length == 0) {
          this.stateStack = new Int32Array(stack_length);
          this.locationStack = new Int32Array(stack_length);
          this.parseStack = new Array(stack_length);
        } else {
          Utils_1.Lpg.Lang.System.arraycopy(this.stateStack, 0, this.stateStack = new Int32Array(stack_length), 0, old_stack_length);
          Utils_1.Lpg.Lang.System.arraycopy(this.locationStack, 0, this.locationStack = new Int32Array(stack_length), 0, old_stack_length);
          Utils_1.Lpg.Lang.System.arraycopy(this.parseStack, 0, this.parseStack = new Array(stack_length), 0, old_stack_length);
        }
        return;
      }
      //
      // Allocate or reallocate the state stack only.
      //
      reallocateStateStack() {
        let old_stack_length = this.stateStack == void 0 ? 0 : this.stateStack.length, stack_length = old_stack_length + this.STACK_INCREMENT;
        if (this.stateStack == void 0 || this.stateStack.length == 0) {
          this.stateStack = new Int32Array(stack_length);
        } else {
          Utils_1.Lpg.Lang.System.arraycopy(this.stateStack, 0, this.stateStack = new Int32Array(stack_length), 0, old_stack_length);
        }
        return;
      }
      //
      // Allocate location and parse stacks using the size of the state stack.
      //
      allocateOtherStacks() {
        this.locationStack = new Int32Array(this.stateStack.length);
        this.parseStack = new Array(this.stateStack.length);
        return;
      }
    };
    exports.Stacks = Stacks;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/TokenStream.js
var require_TokenStream = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/TokenStream.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.EscapeStrictPropertyInitializationTokenStream = void 0;
    var EscapeStrictPropertyInitializationTokenStream = class {
      getToken(end_token) {
        throw new Error("Method not implemented.");
      }
      getKind(i) {
        throw new Error("Method not implemented.");
      }
      getNext(i) {
        throw new Error("Method not implemented.");
      }
      getPrevious(i) {
        throw new Error("Method not implemented.");
      }
      getName(i) {
        throw new Error("Method not implemented.");
      }
      peek() {
        throw new Error("Method not implemented.");
      }
      reset(i) {
        throw new Error("Method not implemented.");
      }
      badToken() {
        throw new Error("Method not implemented.");
      }
      getLine(i) {
        throw new Error("Method not implemented.");
      }
      getColumn(i) {
        throw new Error("Method not implemented.");
      }
      getEndLine(i) {
        throw new Error("Method not implemented.");
      }
      getEndColumn(i) {
        throw new Error("Method not implemented.");
      }
      afterEol(i) {
        throw new Error("Method not implemented.");
      }
      getFileName() {
        throw new Error("Method not implemented.");
      }
      getStreamLength() {
        throw new Error("Method not implemented.");
      }
      getFirstRealToken(i) {
        throw new Error("Method not implemented.");
      }
      getLastRealToken(i) {
        throw new Error("Method not implemented.");
      }
      reportError(errorCode, leftToken, rightToken, errorInfo, errorToken) {
        throw new Error("Method not implemented.");
      }
    };
    exports.EscapeStrictPropertyInitializationTokenStream = EscapeStrictPropertyInitializationTokenStream;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/ParseTable.js
var require_ParseTable = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/ParseTable.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.EscapeStrictPropertyInitializationParseTable = void 0;
    var EscapeStrictPropertyInitializationParseTable = class {
      baseCheck(index) {
        throw new Error("Method not implemented.");
      }
      rhs(index) {
        throw new Error("Method not implemented.");
      }
      baseAction(index) {
        throw new Error("Method not implemented.");
      }
      lhs(index) {
        throw new Error("Method not implemented.");
      }
      termCheck(index) {
        throw new Error("Method not implemented.");
      }
      termAction(index) {
        throw new Error("Method not implemented.");
      }
      asb(index) {
        throw new Error("Method not implemented.");
      }
      asr(index) {
        throw new Error("Method not implemented.");
      }
      nasb(index) {
        throw new Error("Method not implemented.");
      }
      nasr(index) {
        throw new Error("Method not implemented.");
      }
      terminalIndex(index) {
        throw new Error("Method not implemented.");
      }
      nonterminalIndex(index) {
        throw new Error("Method not implemented.");
      }
      scopePrefix(index) {
        throw new Error("Method not implemented.");
      }
      scopeSuffix(index) {
        throw new Error("Method not implemented.");
      }
      scopeLhs(index) {
        throw new Error("Method not implemented.");
      }
      scopeLa(index) {
        throw new Error("Method not implemented.");
      }
      scopeStateSet(index) {
        throw new Error("Method not implemented.");
      }
      scopeRhs(index) {
        throw new Error("Method not implemented.");
      }
      scopeState(index) {
        throw new Error("Method not implemented.");
      }
      inSymb(index) {
        throw new Error("Method not implemented.");
      }
      name(index) {
        throw new Error("Method not implemented.");
      }
      originalState(state) {
        throw new Error("Method not implemented.");
      }
      asi(state) {
        throw new Error("Method not implemented.");
      }
      nasi(state) {
        throw new Error("Method not implemented.");
      }
      inSymbol(state) {
        throw new Error("Method not implemented.");
      }
      ntAction(state, sym) {
        throw new Error("Method not implemented.");
      }
      tAction(act, sym) {
        throw new Error("Method not implemented.");
      }
      lookAhead(act, sym) {
        throw new Error("Method not implemented.");
      }
      getErrorSymbol() {
        throw new Error("Method not implemented.");
      }
      getScopeUbound() {
        throw new Error("Method not implemented.");
      }
      getScopeSize() {
        throw new Error("Method not implemented.");
      }
      getMaxNameLength() {
        throw new Error("Method not implemented.");
      }
      getNumStates() {
        throw new Error("Method not implemented.");
      }
      getNtOffset() {
        throw new Error("Method not implemented.");
      }
      getLaStateOffset() {
        throw new Error("Method not implemented.");
      }
      getMaxLa() {
        throw new Error("Method not implemented.");
      }
      getNumRules() {
        throw new Error("Method not implemented.");
      }
      getNumNonterminals() {
        throw new Error("Method not implemented.");
      }
      getNumSymbols() {
        throw new Error("Method not implemented.");
      }
      getSegmentSize() {
        throw new Error("Method not implemented.");
      }
      getStartState() {
        throw new Error("Method not implemented.");
      }
      getStartSymbol() {
        throw new Error("Method not implemented.");
      }
      getEoftSymbol() {
        throw new Error("Method not implemented.");
      }
      getEoltSymbol() {
        throw new Error("Method not implemented.");
      }
      getAcceptAction() {
        throw new Error("Method not implemented.");
      }
      getErrorAction() {
        throw new Error("Method not implemented.");
      }
      isNullable(symbol) {
        throw new Error("Method not implemented.");
      }
      isValidForParser() {
        throw new Error("Method not implemented.");
      }
      getBacktrack() {
        throw new Error("Method not implemented.");
      }
    };
    exports.EscapeStrictPropertyInitializationParseTable = EscapeStrictPropertyInitializationParseTable;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/RuleAction.js
var require_RuleAction = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/RuleAction.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.EscapeStrictPropertyInitializationRuleAction = void 0;
    var EscapeStrictPropertyInitializationRuleAction = class {
      ruleAction(ruleNumber) {
        throw new Error("Method not implemented.");
      }
    };
    exports.EscapeStrictPropertyInitializationRuleAction = EscapeStrictPropertyInitializationRuleAction;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/IntSegmentedTuple.js
var require_IntSegmentedTuple = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/IntSegmentedTuple.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.IntSegmentedTuple = void 0;
    var Utils_1 = require_Utils();
    var IntSegmentedTuple = class {
      //
      // Allocate another block of storage for the dynamic array.
      //
      allocateMoreSpace() {
        let k = this._size >> this.log_blksize;
        if (k == this.base_size) {
          this.base_size *= 2;
          Utils_1.Lpg.Lang.System.arraycopy(this.base, 0, this.base = new Array(this.base_size), 0, k);
        }
        this.base[k] = new Int32Array(1 << this.log_blksize);
        this._size += 1 << this.log_blksize;
        return;
      }
      //
      // This function is invoked with an integer argument n. It ensures
      // that enough space is allocated for n elements in the dynamic array.
      // I.e., that the array will be indexable in the range  (0..n-1)
      //
      // Note that this function can be used as a garbage collector.  When
      // invoked with no argument(or 0), it frees up all dynamic space that
      // was allocated for the array.
      //
      resize(n = 0) {
        if (n > this._size) {
          do {
            this.allocateMoreSpace();
          } while (n > this._size);
        }
        this.top = n;
      }
      needInterger() {
        throw Error("IntSegmentedTuple need interger");
      }
      //
      // This function is used to reset the size of a dynamic array without
      // allocating or deallocting space. It may be invoked with an integer
      // argument n which indicates the new size or with no argument which
      // indicates that the size should be reset to 0.
      //
      reset(n = 0) {
        if (!Number.isInteger(n)) {
          this.needInterger();
        }
        this.top = n;
      }
      capacity() {
        return this.base.length;
      }
      //
      // Return size of the dynamic array.
      //
      size() {
        return this.top;
      }
      //
      // Can the tuple be indexed with i?
      //
      outOfRange(i) {
        return i < 0 || i >= this.top;
      }
      //
      // Return a reference to the ith element of the dynamic array.
      //
      // Note that no check is made here to ensure that 0 <= i < top.
      // Such a check might be useful for debugging and a range exception
      // should be thrown if it yields true.
      //
      get(i) {
        if (!Number.isInteger(i)) {
          this.needInterger();
        }
        return this.base[i >> this.log_blksize][i % (1 << this.log_blksize)];
      }
      //
      // Insert an element in the dynamic array at the location indicated.
      //
      set(i, element) {
        if (!Number.isInteger(i)) {
          this.needInterger();
        }
        this.base[i >> this.log_blksize][i % (1 << this.log_blksize)] = element;
      }
      //
      // Add an element to the dynamic array and return the top index.
      //
      NextIndex() {
        let i = this.top++;
        if (i == this._size) {
          this.allocateMoreSpace();
        }
        return i;
      }
      //
      // Add an element to the dynamic array and return a reference to
      // that new element.
      //
      add(element) {
        let i = this.NextIndex();
        this.base[i >> this.log_blksize][i % (1 << this.log_blksize)] = element;
      }
      //
      // If array is sorted, this function will find the index location
      // of a given element if it is contained in the array. Otherwise, it
      // will return the negation of the index of the element prior to
      // which the new element would be inserted in the array.
      //
      binarySearch(element) {
        if (!Number.isInteger(element)) {
          this.needInterger();
        }
        let low = 0, high = this.top;
        while (high > low) {
          let mid = Math.floor((high + low) / 2), mid_element = this.get(mid);
          if (element == mid_element) {
            return mid;
          } else {
            if (element < mid_element) {
              high = mid;
            } else {
              low = mid + 1;
            }
          }
        }
        return -low;
      }
      constructor(log_blksize_, base_size_) {
        this.top = 0;
        this._size = 0;
        this.log_blksize = 3;
        this.base_size = 4;
        if (log_blksize_)
          this.log_blksize = log_blksize_;
        if (base_size_)
          this.base_size = base_size_ <= 0 ? 4 : base_size_;
        this.base = new Array(this.base_size);
      }
    };
    exports.IntSegmentedTuple = IntSegmentedTuple;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/IntTuple.js
var require_IntTuple = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/IntTuple.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.IntTuple = void 0;
    var Utils_1 = require_Utils();
    var IntTuple = class {
      //
      // This function is used to reset the size of a dynamic array without
      // allocating or deallocting space. It may be invoked with an integer
      // argument n which indicates the new size or with no argument which
      // indicates that the size should be reset to 0.
      //
      reset(n = 0) {
        this.top = n;
      }
      //
      // Return size of the dynamic array.
      //
      size() {
        return this.top;
      }
      needInterger() {
        throw Error(" IntTuple  need interger index");
      }
      //
      // Return a reference to the ith element of the dynamic array.
      //
      // Note that no check is made here to ensure that 0 <= i < top.
      // Such a check might be useful for debugging and a range exception
      // should be thrown if it yields true.
      //
      get(i) {
        if (!Number.isInteger(i)) {
          this.needInterger();
        }
        return this.array[i];
      }
      //
      // Insert an element in the dynamic array at the location indicated.
      //
      set(i, element) {
        if (!Number.isInteger(i)) {
          this.needInterger();
        }
        this.array[i] = element;
      }
      //
      // Add an element to the dynamic array and return the top index.
      //
      nextIndex() {
        let i = this.top++;
        if (i >= this.array.length) {
          Utils_1.Lpg.Lang.System.arraycopy(this.array, 0, this.array = new Int32Array(i * 2), 0, i);
        }
        return i;
      }
      //
      // Add an element to the dynamic array and return a reference to
      // that new element.
      //
      add(element) {
        let i = this.nextIndex();
        this.array[i] = element;
      }
      capacity() {
        return this.array.length;
      }
      //
      // Constructor of a Tuple
      //
      constructor(estimate = 10) {
        this.top = 0;
        this.array = new Int32Array(estimate);
      }
    };
    exports.IntTuple = IntTuple;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/ConfigurationElement.js
var require_ConfigurationElement = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/ConfigurationElement.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.ConfigurationElement = void 0;
    var ConfigurationElement = class {
      constructor() {
        this.stack_top = 0;
        this.action_length = 0;
        this.conflict_index = 0;
        this.curtok = 0;
        this.act = 0;
      }
      retrieveStack(stack) {
        let tail = this.last_element;
        for (let i = this.stack_top; i >= 0; i--) {
          if (!tail)
            return;
          stack[i] = tail.number;
          tail = tail.parent;
        }
        return;
      }
    };
    exports.ConfigurationElement = ConfigurationElement;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/StateElement.js
var require_StateElement = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/StateElement.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.StateElement = void 0;
    var StateElement = class {
      constructor() {
        this.number = 0;
      }
    };
    exports.StateElement = StateElement;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/ObjectTuple.js
var require_ObjectTuple = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/ObjectTuple.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.ObjectTuple = void 0;
    var Utils_1 = require_Utils();
    var ObjectTuple = class {
      needInterger() {
        throw Error(" ObjectTuple  need interger index");
      }
      //
      // This function is used to reset the size of a dynamic array without
      // allocating or deallocting space. It may be invoked with an integer
      // argument n which indicates the new size or with no argument which
      // indicates that the size should be reset to 0.
      //
      reset(n = 0) {
        if (!Number.isInteger(n)) {
          this.needInterger();
        }
        this.top = n;
      }
      capacity() {
        return this.array.length;
      }
      //
      // Return size of the dynamic array.
      //
      size() {
        return this.top;
      }
      //
      // Return a reference to the ith element of the dynamic array.
      //
      // Note that no check is made here to ensure that 0 <= i < top.
      // Such a check might be useful for debugging and a range exception
      // should be thrown if it yields true.
      //
      get(i) {
        if (!Number.isInteger(i)) {
          this.needInterger();
        }
        return this.array[i];
      }
      //
      // Insert an element in the dynamic array at the location indicated.
      //
      set(i, element) {
        if (!Number.isInteger(i)) {
          this.needInterger();
        }
        this.array[i] = element;
      }
      //
      // Add an element to the dynamic array and return the top index.
      //
      nextIndex() {
        let i = this.top++;
        if (i >= this.array.length) {
          Utils_1.Lpg.Lang.System.arraycopy(this.array, 0, this.array = new Array(i * 2), 0, i);
        }
        return i;
      }
      //
      // Add an element to the dynamic array and return a reference to
      // that new element.
      //
      add(element) {
        let i = this.nextIndex();
        this.array[i] = element;
      }
      //
      // Constructor of a Tuple
      //
      constructor(estimate = 10) {
        this.top = 0;
        this.array = new Array(estimate);
      }
    };
    exports.ObjectTuple = ObjectTuple;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/ConfigurationStack.js
var require_ConfigurationStack = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/ConfigurationStack.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.ConfigurationStack = void 0;
    var ConfigurationElement_1 = require_ConfigurationElement();
    var StateElement_1 = require_StateElement();
    var ObjectTuple_1 = require_ObjectTuple();
    var ConfigurationStack = class _ConfigurationStack {
      constructor(prs) {
        this.table = new Array(_ConfigurationStack.TABLE_SIZE);
        this.configuration_stack = new ObjectTuple_1.ObjectTuple(1 << 12);
        this.max_configuration_size = 0;
        this.stacks_size = 0;
        this.state_element_size = 0;
        this.prs = prs;
        this.state_element_size++;
        this.state_root = new StateElement_1.StateElement();
        this.state_root.number = prs.getStartState();
      }
      makeStateList(parent, stack, index, stack_top) {
        for (let i = index; i <= stack_top; i++) {
          this.state_element_size++;
          let state = new StateElement_1.StateElement();
          state.number = stack[i];
          state.parent = parent;
          parent.children = state;
          parent = state;
        }
        return parent;
      }
      findOrInsertStack(root, stack, index, stack_top) {
        let state_number = stack[index];
        for (let p = root; ; p = p.siblings) {
          if (!p)
            break;
          if (p.number == state_number) {
            return index == stack_top ? p : p.children == void 0 ? this.makeStateList(p, stack, index + 1, stack_top) : this.findOrInsertStack(p.children, stack, index + 1, stack_top);
          }
        }
        this.state_element_size++;
        let node = new StateElement_1.StateElement();
        node.number = state_number;
        node.parent = root.parent;
        node.siblings = root.siblings;
        root.siblings = node;
        return index == stack_top ? node : this.makeStateList(node, stack, index + 1, stack_top);
      }
      findConfiguration(stack, stack_top, curtok) {
        let last_element = this.findOrInsertStack(this.state_root, stack, 0, stack_top);
        let hash_address = curtok % _ConfigurationStack.TABLE_SIZE;
        for (let configuration = this.table[hash_address]; ; ) {
          if (configuration) {
            if (configuration.curtok == curtok && last_element == configuration.last_element) {
              return true;
            }
            configuration = configuration.next;
          } else {
            break;
          }
        }
        return false;
      }
      push(stack, stack_top, conflict_index, curtok, action_length) {
        let configuration = new ConfigurationElement_1.ConfigurationElement();
        let hash_address = curtok % _ConfigurationStack.TABLE_SIZE;
        configuration.next = this.table[hash_address];
        this.table[hash_address] = configuration;
        this.max_configuration_size++;
        configuration.stack_top = stack_top;
        this.stacks_size += stack_top + 1;
        configuration.last_element = this.findOrInsertStack(this.state_root, stack, 0, stack_top);
        configuration.conflict_index = conflict_index;
        configuration.curtok = curtok;
        configuration.action_length = action_length;
        this.configuration_stack.add(configuration);
        return;
      }
      pop() {
        if (this.configuration_stack.size() > 0) {
          let index = this.configuration_stack.size() - 1;
          let configuration = this.configuration_stack.get(index);
          configuration.act = this.prs.baseAction(configuration.conflict_index++);
          if (this.prs.baseAction(configuration.conflict_index) == 0) {
            this.configuration_stack.reset(index);
          }
          return configuration;
        } else {
          return void 0;
        }
      }
      top() {
        if (this.configuration_stack.size() > 0) {
          let index = this.configuration_stack.size() - 1;
          let configuration = this.configuration_stack.get(index);
          configuration.act = this.prs.baseAction(configuration.conflict_index);
          return configuration;
        } else {
          return void 0;
        }
      }
      size() {
        return this.configuration_stack.size();
      }
      maxConfigurationSize() {
        return this.max_configuration_size;
      }
      numStateElements() {
        return this.state_element_size;
      }
      stacksSize() {
        return this.stacks_size;
      }
    };
    exports.ConfigurationStack = ConfigurationStack;
    ConfigurationStack.TABLE_SIZE = 1021;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/ParseErrorCodes.js
var require_ParseErrorCodes = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/ParseErrorCodes.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.errorMsgText = exports.ParseErrorCodes = void 0;
    var ParseErrorCodes;
    (function(ParseErrorCodes2) {
      ParseErrorCodes2[ParseErrorCodes2["LEX_ERROR_CODE"] = 0] = "LEX_ERROR_CODE";
      ParseErrorCodes2[ParseErrorCodes2["ERROR_CODE"] = 1] = "ERROR_CODE";
      ParseErrorCodes2[ParseErrorCodes2["BEFORE_CODE"] = 2] = "BEFORE_CODE";
      ParseErrorCodes2[ParseErrorCodes2["INSERTION_CODE"] = 3] = "INSERTION_CODE";
      ParseErrorCodes2[ParseErrorCodes2["INVALID_CODE"] = 4] = "INVALID_CODE";
      ParseErrorCodes2[ParseErrorCodes2["SUBSTITUTION_CODE"] = 5] = "SUBSTITUTION_CODE";
      ParseErrorCodes2[ParseErrorCodes2["SECONDARY_CODE"] = 5] = "SECONDARY_CODE";
      ParseErrorCodes2[ParseErrorCodes2["DELETION_CODE"] = 6] = "DELETION_CODE";
      ParseErrorCodes2[ParseErrorCodes2["MERGE_CODE"] = 7] = "MERGE_CODE";
      ParseErrorCodes2[ParseErrorCodes2["MISPLACED_CODE"] = 8] = "MISPLACED_CODE";
      ParseErrorCodes2[ParseErrorCodes2["SCOPE_CODE"] = 9] = "SCOPE_CODE";
      ParseErrorCodes2[ParseErrorCodes2["EOF_CODE"] = 10] = "EOF_CODE";
      ParseErrorCodes2[ParseErrorCodes2["INVALID_TOKEN_CODE"] = 11] = "INVALID_TOKEN_CODE";
      ParseErrorCodes2[ParseErrorCodes2["ERROR_RULE_ERROR_CODE"] = 11] = "ERROR_RULE_ERROR_CODE";
      ParseErrorCodes2[ParseErrorCodes2["ERROR_RULE_WARNING_CODE"] = 12] = "ERROR_RULE_WARNING_CODE";
      ParseErrorCodes2[ParseErrorCodes2["NO_MESSAGE_CODE"] = 13] = "NO_MESSAGE_CODE";
      ParseErrorCodes2[ParseErrorCodes2["MANUAL_CODE"] = 14] = "MANUAL_CODE";
    })(ParseErrorCodes = exports.ParseErrorCodes || (exports.ParseErrorCodes = {}));
    exports.errorMsgText = [
      /* LEX_ERROR_CODE */
      "unexpected character ignored",
      /* ERROR_CODE */
      "parsing terminated at this token",
      /* BEFORE_CODE */
      " inserted before this token",
      /* INSERTION_CODE */
      " expected after this token",
      /* INVALID_CODE */
      "unexpected input discarded",
      /* SUBSTITUTION_CODE, SECONDARY_CODE */
      " expected instead of this input",
      /* DELETION_CODE */
      " unexpected token(s) ignored",
      /* MERGE_CODE */
      " formed from merged tokens",
      /* MISPLACED_CODE */
      "misplaced construct(s)",
      /* SCOPE_CODE */
      " inserted to complete scope",
      /* EOF_CODE */
      " reached after this token",
      /* INVALID_TOKEN_CODE, ERROR_RULE_ERROR */
      " is invalid",
      /* ERROR_RULE_WARNING */
      " is ignored",
      /* NO_MESSAGE_CODE */
      ""
      //$NON-NLS-1$
    ];
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/DiagnoseParser.js
var require_DiagnoseParser = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/DiagnoseParser.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.DiagnoseParser = exports.MIN_DISTANCE = exports.MAX_DISTANCE = exports.BUFF_SIZE = exports.BUFF_UBOUND = exports.STACK_INCREMENT = exports.StateInfo = exports.SecondaryRepairInfo = exports.PrimaryRepairInfo = exports.RepairCandidate = void 0;
    var ConfigurationStack_1 = require_ConfigurationStack();
    var IntTuple_1 = require_IntTuple();
    var ParseErrorCodes_1 = require_ParseErrorCodes();
    var Utils_1 = require_Utils();
    var RepairCandidate = class {
      constructor() {
        this.symbol = 0;
        this.location = 0;
      }
    };
    exports.RepairCandidate = RepairCandidate;
    var PrimaryRepairInfo = class {
      constructor(clone) {
        this.distance = 0;
        this.misspellIndex = 0;
        this.code = 0;
        this.bufferPosition = 0;
        this.symbol = 0;
        if (clone)
          this.copy(clone);
      }
      copy(clone) {
        this.distance = clone.distance;
        this.misspellIndex = clone.misspellIndex;
        this.code = clone.code;
        this.bufferPosition = clone.bufferPosition;
        this.symbol = clone.symbol;
        return;
      }
    };
    exports.PrimaryRepairInfo = PrimaryRepairInfo;
    var SecondaryRepairInfo = class {
      constructor() {
        this.code = 0;
        this.distance = 0;
        this.bufferPosition = 0;
        this.stackPosition = 0;
        this.numDeletions = 0;
        this.symbol = 0;
        this.recoveryOnNextStack = false;
      }
    };
    exports.SecondaryRepairInfo = SecondaryRepairInfo;
    var StateInfo = class {
      constructor(state, next) {
        this.state = 0;
        this.next = 0;
        this.state = state;
        this.next = next;
      }
    };
    exports.StateInfo = StateInfo;
    exports.STACK_INCREMENT = 256;
    exports.BUFF_UBOUND = 31;
    exports.BUFF_SIZE = 32;
    exports.MAX_DISTANCE = 30;
    exports.MIN_DISTANCE = 3;
    var DiagnoseParser2 = class _DiagnoseParser {
      setMonitor(monitor) {
        this.monitor = monitor;
      }
      constructor(tokStream, prs, maxErrors = 0, maxTime = 0, monitor) {
        this.ERROR_SYMBOL = 0;
        this.SCOPE_SIZE = 0;
        this.MAX_NAME_LENGTH = 0;
        this.NT_OFFSET = 0;
        this.LA_STATE_OFFSET = 0;
        this.NUM_RULES = 0;
        this.NUM_SYMBOLS = 0;
        this.START_STATE = 0;
        this.EOFT_SYMBOL = 0;
        this.EOLT_SYMBOL = 0;
        this.ACCEPT_ACTION = 0;
        this.ERROR_ACTION = 0;
        this.maxErrors = 0;
        this.maxTime = 0;
        this.stateStackTop = -1;
        this.stateStack = new Int32Array(0);
        this.locationStack = new Int32Array(0);
        this.tempStackTop = -1;
        this.tempStack = new Int32Array(0);
        this.prevStackTop = -1;
        this.prevStack = new Int32Array(0);
        this.nextStackTop = -1;
        this.nextStack = new Int32Array(0);
        this.scopeStackTop = -1;
        this.scopeIndex = new Int32Array(0);
        this.scopePosition = new Int32Array(0);
        this.buffer = new Int32Array(exports.BUFF_SIZE);
        this.stateSeen = new Int32Array(0);
        this.statePoolTop = -1;
        this.statePool = [];
        this.monitor = monitor;
        this.maxErrors = maxErrors;
        this.maxTime = maxTime;
        this.tokStream = tokStream;
        this.prs = prs;
        this.main_configuration_stack = new ConfigurationStack_1.ConfigurationStack(prs);
        this.ERROR_SYMBOL = prs.getErrorSymbol();
        this.SCOPE_SIZE = prs.getScopeSize();
        this.MAX_NAME_LENGTH = prs.getMaxNameLength();
        this.NT_OFFSET = prs.getNtOffset();
        this.LA_STATE_OFFSET = prs.getLaStateOffset();
        this.NUM_RULES = prs.getNumRules();
        this.NUM_SYMBOLS = prs.getNumSymbols();
        this.START_STATE = prs.getStartState();
        this.EOFT_SYMBOL = prs.getEoftSymbol();
        this.EOLT_SYMBOL = prs.getEoltSymbol();
        this.ACCEPT_ACTION = prs.getAcceptAction();
        this.ERROR_ACTION = prs.getErrorAction();
        this.list = new Int32Array(this.NUM_SYMBOLS + 1);
      }
      rhs(index) {
        return this.prs.rhs(index);
      }
      baseAction(index) {
        return this.prs.baseAction(index);
      }
      baseCheck(index) {
        return this.prs.baseCheck(index);
      }
      lhs(index) {
        return this.prs.lhs(index);
      }
      termCheck(index) {
        return this.prs.termCheck(index);
      }
      termAction(index) {
        return this.prs.termAction(index);
      }
      asb(index) {
        return this.prs.asb(index);
      }
      asr(index) {
        return this.prs.asr(index);
      }
      nasb(index) {
        return this.prs.nasb(index);
      }
      nasr(index) {
        return this.prs.nasr(index);
      }
      terminalIndex(index) {
        return this.prs.terminalIndex(index);
      }
      nonterminalIndex(index) {
        return this.prs.nonterminalIndex(index);
      }
      symbolIndex(index) {
        return index > this.NT_OFFSET ? this.nonterminalIndex(index - this.NT_OFFSET) : this.terminalIndex(index);
      }
      scopePrefix(index) {
        return this.prs.scopePrefix(index);
      }
      scopeSuffix(index) {
        return this.prs.scopeSuffix(index);
      }
      scopeLhs(index) {
        return this.prs.scopeLhs(index);
      }
      scopeLa(index) {
        return this.prs.scopeLa(index);
      }
      scopeStateSet(index) {
        return this.prs.scopeStateSet(index);
      }
      scopeRhs(index) {
        return this.prs.scopeRhs(index);
      }
      scopeState(index) {
        return this.prs.scopeState(index);
      }
      inSymb(index) {
        return this.prs.inSymb(index);
      }
      name(index) {
        return this.prs.name(index);
      }
      originalState(state) {
        return this.prs.originalState(state);
      }
      asi(state) {
        return this.prs.asi(state);
      }
      nasi(state) {
        return this.prs.nasi(state);
      }
      inSymbol(state) {
        return this.prs.inSymbol(state);
      }
      ntAction(state, sym) {
        return this.prs.ntAction(state, sym);
      }
      isNullable(symbol) {
        return this.prs.isNullable(symbol);
      }
      reallocateStacks() {
        let old_stack_length = this.stateStack == void 0 ? 0 : this.stateStack.length, stack_length = old_stack_length + exports.STACK_INCREMENT;
        if (!this.stateStack || this.stateStack.length == 0) {
          this.stateStack = new Int32Array(stack_length);
          this.locationStack = new Int32Array(stack_length);
          this.tempStack = new Int32Array(stack_length);
          this.prevStack = new Int32Array(stack_length);
          this.nextStack = new Int32Array(stack_length);
          this.scopeIndex = new Int32Array(stack_length);
          this.scopePosition = new Int32Array(stack_length);
        } else {
          Utils_1.Lpg.Lang.System.arraycopy(this.stateStack, 0, this.stateStack = new Int32Array(stack_length), 0, old_stack_length);
          Utils_1.Lpg.Lang.System.arraycopy(this.locationStack, 0, this.locationStack = new Int32Array(stack_length), 0, old_stack_length);
          Utils_1.Lpg.Lang.System.arraycopy(this.tempStack, 0, this.tempStack = new Int32Array(stack_length), 0, old_stack_length);
          Utils_1.Lpg.Lang.System.arraycopy(this.prevStack, 0, this.prevStack = new Int32Array(stack_length), 0, old_stack_length);
          Utils_1.Lpg.Lang.System.arraycopy(this.nextStack, 0, this.nextStack = new Int32Array(stack_length), 0, old_stack_length);
          Utils_1.Lpg.Lang.System.arraycopy(this.scopeIndex, 0, this.scopeIndex = new Int32Array(stack_length), 0, old_stack_length);
          Utils_1.Lpg.Lang.System.arraycopy(this.scopePosition, 0, this.scopePosition = new Int32Array(stack_length), 0, old_stack_length);
        }
        return;
      }
      diagnose(error_token = 0) {
        this.diagnoseEntry(0, error_token);
      }
      diagnoseEntry(marker_kind, error_token) {
        if (error_token) {
          this.diagnoseEntry2(marker_kind, error_token);
        } else {
          this.diagnoseEntry1(marker_kind);
        }
      }
      diagnoseEntry1(marker_kind) {
        this.reallocateStacks();
        this.tempStackTop = 0;
        this.tempStack[this.tempStackTop] = this.START_STATE;
        this.tokStream.reset();
        let current_token, current_kind;
        if (marker_kind == 0) {
          current_token = this.tokStream.getToken();
          current_kind = this.tokStream.getKind(current_token);
        } else {
          current_token = this.tokStream.peek();
          current_kind = marker_kind;
        }
        let error_token = this.parseForError(current_kind);
        if (error_token !== 0) {
          this.diagnoseEntry(marker_kind, error_token);
        }
        return;
      }
      diagnoseEntry2(marker_kind, error_token) {
        let action = new IntTuple_1.IntTuple(1 << 18);
        let startTime = Date.now();
        let errorCount = 0;
        if (!this.stateStack || this.stateStack.length == 0) {
          this.reallocateStacks();
        }
        this.tempStackTop = 0;
        this.tempStack[this.tempStackTop] = this.START_STATE;
        this.tokStream.reset();
        let current_token, current_kind;
        if (marker_kind == 0) {
          current_token = this.tokStream.getToken();
          current_kind = this.tokStream.getKind(current_token);
        } else {
          current_token = this.tokStream.peek();
          current_kind = marker_kind;
        }
        this.parseUpToError(action, current_kind, error_token);
        this.stateStackTop = 0;
        this.stateStack[this.stateStackTop] = this.START_STATE;
        this.tempStackTop = this.stateStackTop;
        Utils_1.Lpg.Lang.System.arraycopy(this.tempStack, 0, this.stateStack, 0, this.tempStackTop + 1);
        this.tokStream.reset();
        if (marker_kind == 0) {
          current_token = this.tokStream.getToken();
          current_kind = this.tokStream.getKind(current_token);
        } else {
          current_token = this.tokStream.peek();
          current_kind = marker_kind;
        }
        this.locationStack[this.stateStackTop] = current_token;
        let act;
        do {
          let prev_pos = -1;
          this.prevStackTop = -1;
          let next_pos = -1;
          this.nextStackTop = -1;
          let pos = this.stateStackTop;
          this.tempStackTop = this.stateStackTop - 1;
          Utils_1.Lpg.Lang.System.arraycopy(this.stateStack, 0, this.tempStack, 0, this.stateStackTop + 1);
          let action_index = 0;
          act = action.get(action_index++);
          while (act <= this.NUM_RULES) {
            do {
              this.tempStackTop -= this.rhs(act) - 1;
              act = this.ntAction(this.tempStack[this.tempStackTop], this.lhs(act));
            } while (act <= this.NUM_RULES);
            if (this.tempStackTop + 1 >= this.stateStack.length) {
              this.reallocateStacks();
            }
            pos = pos < this.tempStackTop ? pos : this.tempStackTop;
            this.tempStack[this.tempStackTop + 1] = act;
            act = action.get(action_index++);
          }
          while (act > this.ERROR_ACTION || act < this.ACCEPT_ACTION) {
            if (this.monitor && this.monitor.isCancelled()) {
              return;
            }
            this.nextStackTop = this.tempStackTop + 1;
            for (let i = next_pos + 1; i <= this.nextStackTop; i++) {
              this.nextStack[i] = this.tempStack[i];
            }
            for (let k = pos + 1; k <= this.nextStackTop; k++) {
              this.locationStack[k] = this.locationStack[this.stateStackTop];
            }
            if (act > this.ERROR_ACTION) {
              act -= this.ERROR_ACTION;
              do {
                this.nextStackTop -= this.rhs(act) - 1;
                act = this.ntAction(this.nextStack[this.nextStackTop], this.lhs(act));
              } while (act <= this.NUM_RULES);
              pos = pos < this.nextStackTop ? pos : this.nextStackTop;
            }
            if (this.nextStackTop + 1 >= this.stateStack.length) {
              this.reallocateStacks();
            }
            this.tempStackTop = this.nextStackTop;
            this.nextStack[++this.nextStackTop] = act;
            next_pos = this.nextStackTop;
            current_token = this.tokStream.getToken();
            current_kind = this.tokStream.getKind(current_token);
            act = action.get(action_index++);
            while (act <= this.NUM_RULES) {
              do {
                let lhs_symbol = this.lhs(act);
                this.tempStackTop -= this.rhs(act) - 1;
                act = this.tempStackTop > next_pos ? this.tempStack[this.tempStackTop] : this.nextStack[this.tempStackTop];
                act = this.ntAction(act, lhs_symbol);
              } while (act <= this.NUM_RULES);
              if (this.tempStackTop + 1 >= this.stateStack.length) {
                this.reallocateStacks();
              }
              next_pos = next_pos < this.tempStackTop ? next_pos : this.tempStackTop;
              this.tempStack[this.tempStackTop + 1] = act;
              act = action.get(action_index++);
            }
            if (act !== this.ERROR_ACTION) {
              this.prevStackTop = this.stateStackTop;
              for (let i = prev_pos + 1; i <= this.prevStackTop; i++) {
                this.prevStack[i] = this.stateStack[i];
              }
              prev_pos = pos;
              this.stateStackTop = this.nextStackTop;
              for (let k = pos + 1; k <= this.stateStackTop; k++) {
                this.stateStack[k] = this.nextStack[k];
              }
              this.locationStack[this.stateStackTop] = current_token;
              pos = next_pos;
            }
          }
          if (act == this.ERROR_ACTION) {
            errorCount += 1;
            if (errorCount > 1) {
              if (this.maxErrors > 0 && errorCount > this.maxErrors) {
                break;
              }
              if (this.maxTime > 0 && Date.now() - startTime > this.maxTime) {
                break;
              }
            }
            let candidate = this.errorRecovery(current_token);
            if (this.monitor && this.monitor.isCancelled()) {
              return;
            }
            act = this.stateStack[this.stateStackTop];
            if (candidate.symbol == 0) {
              break;
            } else {
              if (candidate.symbol > this.NT_OFFSET) {
                let lhs_symbol = candidate.symbol - this.NT_OFFSET;
                act = this.ntAction(act, lhs_symbol);
                while (act <= this.NUM_RULES) {
                  this.stateStackTop -= this.rhs(act) - 1;
                  act = this.ntAction(this.stateStack[this.stateStackTop], this.lhs(act));
                }
                this.stateStack[++this.stateStackTop] = act;
                current_token = this.tokStream.getToken();
                current_kind = this.tokStream.getKind(current_token);
                this.locationStack[this.stateStackTop] = current_token;
              } else {
                current_kind = candidate.symbol;
                this.locationStack[this.stateStackTop] = candidate.location;
              }
            }
            let next_token = this.tokStream.peek();
            this.tempStackTop = this.stateStackTop;
            Utils_1.Lpg.Lang.System.arraycopy(this.stateStack, 0, this.tempStack, 0, this.stateStackTop + 1);
            error_token = this.parseForError(current_kind);
            if (error_token !== 0) {
              this.tokStream.reset(next_token);
              this.tempStackTop = this.stateStackTop;
              Utils_1.Lpg.Lang.System.arraycopy(this.stateStack, 0, this.tempStack, 0, this.stateStackTop + 1);
              this.parseUpToError(action, current_kind, error_token);
              this.tokStream.reset(next_token);
            } else {
              act = this.ACCEPT_ACTION;
            }
          }
        } while (act !== this.ACCEPT_ACTION);
        return;
      }
      //
      // Given the configuration consisting of the states in tempStack
      // and the sequence of tokens (current_kind, followed by the tokens
      // in tokStream), keep parsing until either the parse completes
      // successfully or it encounters an error. If the parse is not
      // succesful, we return the farthest token on which an error was
      // encountered. Otherwise, we return 0.
      //
      parseForError(current_kind) {
        let error_token = 0;
        let curtok = this.tokStream.getPrevious(this.tokStream.peek()), act = this.tAction(this.tempStack[this.tempStackTop], current_kind);
        let configuration_stack = new ConfigurationStack_1.ConfigurationStack(this.prs);
        for (; ; ) {
          if (act <= this.NUM_RULES) {
            this.tempStackTop--;
            do {
              this.tempStackTop -= this.rhs(act) - 1;
              act = this.ntAction(this.tempStack[this.tempStackTop], this.lhs(act));
            } while (act <= this.NUM_RULES);
          } else if (act > this.ERROR_ACTION) {
            curtok = this.tokStream.getToken();
            current_kind = this.tokStream.getKind(curtok);
            act -= this.ERROR_ACTION;
            do {
              this.tempStackTop -= this.rhs(act) - 1;
              act = this.ntAction(this.tempStack[this.tempStackTop], this.lhs(act));
            } while (act <= this.NUM_RULES);
          } else if (act < this.ACCEPT_ACTION) {
            curtok = this.tokStream.getToken();
            current_kind = this.tokStream.getKind(curtok);
          } else if (act == this.ERROR_ACTION) {
            error_token = error_token > curtok ? error_token : curtok;
            let configuration = configuration_stack.pop();
            if (configuration == void 0) {
              act = this.ERROR_ACTION;
            } else {
              this.tempStackTop = configuration.stack_top;
              configuration.retrieveStack(this.tempStack);
              act = configuration.act;
              curtok = configuration.curtok;
              current_kind = this.tokStream.getKind(curtok);
              this.tokStream.reset(this.tokStream.getNext(curtok));
              continue;
            }
            break;
          } else if (act > this.ACCEPT_ACTION) {
            if (configuration_stack.findConfiguration(this.tempStack, this.tempStackTop, curtok)) {
              act = this.ERROR_ACTION;
            } else {
              configuration_stack.push(this.tempStack, this.tempStackTop, act + 1, curtok, 0);
              act = this.baseAction(act);
            }
            continue;
          } else {
            break;
          }
          if (++this.tempStackTop >= this.tempStack.length) {
            this.reallocateStacks();
          }
          this.tempStack[this.tempStackTop] = act;
          act = this.tAction(act, current_kind);
        }
        return act == this.ERROR_ACTION ? error_token : 0;
      }
      //
      // Given the configuration consisting of the states in tempStack
      // and the sequence of tokens (current_kind, followed by the tokens
      // in tokStream), parse up to error_token in the tokStream and store
      // all the parsing actions executed in the "action" tuple.
      //
      parseUpToError(action, current_kind, error_token) {
        let curtok = this.tokStream.getPrevious(this.tokStream.peek());
        let act = this.tAction(this.tempStack[this.tempStackTop], current_kind);
        let configuration_stack = new ConfigurationStack_1.ConfigurationStack(this.prs);
        action.reset();
        for (; ; ) {
          if (act <= this.NUM_RULES) {
            action.add(act);
            this.tempStackTop--;
            do {
              this.tempStackTop -= this.rhs(act) - 1;
              act = this.ntAction(this.tempStack[this.tempStackTop], this.lhs(act));
            } while (act <= this.NUM_RULES);
          } else if (act > this.ERROR_ACTION) {
            action.add(act);
            curtok = this.tokStream.getToken();
            current_kind = this.tokStream.getKind(curtok);
            act -= this.ERROR_ACTION;
            do {
              this.tempStackTop -= this.rhs(act) - 1;
              act = this.ntAction(this.tempStack[this.tempStackTop], this.lhs(act));
            } while (act <= this.NUM_RULES);
          } else if (act < this.ACCEPT_ACTION) {
            action.add(act);
            curtok = this.tokStream.getToken();
            current_kind = this.tokStream.getKind(curtok);
          } else if (act == this.ERROR_ACTION) {
            if (curtok !== error_token) {
              let configuration = configuration_stack.pop();
              if (configuration == void 0) {
                act = this.ERROR_ACTION;
              } else {
                this.tempStackTop = configuration.stack_top;
                configuration.retrieveStack(this.tempStack);
                act = configuration.act;
                curtok = configuration.curtok;
                action.reset(configuration.action_length);
                current_kind = this.tokStream.getKind(curtok);
                this.tokStream.reset(this.tokStream.getNext(curtok));
                continue;
              }
            }
            break;
          } else if (act > this.ACCEPT_ACTION) {
            if (configuration_stack.findConfiguration(this.tempStack, this.tempStackTop, curtok)) {
              act = this.ERROR_ACTION;
            } else {
              configuration_stack.push(this.tempStack, this.tempStackTop, act + 1, curtok, action.size());
              act = this.baseAction(act);
            }
            continue;
          } else {
            break;
          }
          if (++this.tempStackTop >= this.tempStack.length) {
            this.reallocateStacks();
          }
          this.tempStack[this.tempStackTop] = act;
          act = this.tAction(act, current_kind);
        }
        action.add(this.ERROR_ACTION);
        return;
      }
      //
      // Try to parse until first_symbol and all tokens in BUFFER have
      // been consumed, or an error is encountered. Return the number
      // of tokens that were expended before the parse blocked.
      //
      parseCheck(stack, stack_top, first_symbol, buffer_position) {
        let buffer_index, current_kind;
        let local_stack = new Int32Array(stack.length);
        let local_stack_top = stack_top;
        for (let i = 0; i <= stack_top; i++) {
          local_stack[i] = stack[i];
        }
        let configuration_stack = new ConfigurationStack_1.ConfigurationStack(this.prs);
        let act = local_stack[local_stack_top];
        if (first_symbol > this.NT_OFFSET) {
          let lhs_symbol = first_symbol - this.NT_OFFSET;
          buffer_index = buffer_position;
          current_kind = this.tokStream.getKind(this.buffer[buffer_index]);
          this.tokStream.reset(this.tokStream.getNext(this.buffer[buffer_index]));
          act = this.ntAction(act, lhs_symbol);
          while (act <= this.NUM_RULES) {
            local_stack_top -= this.rhs(act) - 1;
            act = this.ntAction(local_stack[local_stack_top], this.lhs(act));
          }
        } else {
          local_stack_top--;
          buffer_index = buffer_position - 1;
          current_kind = first_symbol;
          this.tokStream.reset(this.buffer[buffer_position]);
        }
        if (++local_stack_top >= local_stack.length) {
          return buffer_index;
        }
        local_stack[local_stack_top] = act;
        act = this.tAction(act, current_kind);
        for (; ; ) {
          if (act <= this.NUM_RULES) {
            local_stack_top -= this.rhs(act);
            act = this.ntAction(local_stack[local_stack_top], this.lhs(act));
            while (act <= this.NUM_RULES) {
              local_stack_top -= this.rhs(act) - 1;
              act = this.ntAction(local_stack[local_stack_top], this.lhs(act));
            }
          } else if (act > this.ERROR_ACTION) {
            if (buffer_index++ == exports.MAX_DISTANCE) {
              break;
            }
            current_kind = this.tokStream.getKind(this.buffer[buffer_index]);
            this.tokStream.reset(this.tokStream.getNext(this.buffer[buffer_index]));
            act -= this.ERROR_ACTION;
            do {
              local_stack_top -= this.rhs(act) - 1;
              act = this.ntAction(local_stack[local_stack_top], this.lhs(act));
            } while (act <= this.NUM_RULES);
          } else if (act < this.ACCEPT_ACTION) {
            if (buffer_index++ == exports.MAX_DISTANCE) {
              break;
            }
            current_kind = this.tokStream.getKind(this.buffer[buffer_index]);
            this.tokStream.reset(this.tokStream.getNext(this.buffer[buffer_index]));
          } else if (act == this.ERROR_ACTION) {
            let configuration = configuration_stack.pop();
            if (configuration == void 0) {
              act = this.ERROR_ACTION;
            } else {
              local_stack_top = configuration.stack_top;
              configuration.retrieveStack(local_stack);
              act = configuration.act;
              buffer_index = configuration.curtok;
              current_kind = this.tokStream.getKind(this.buffer[buffer_index]);
              this.tokStream.reset(this.tokStream.getNext(this.buffer[buffer_index]));
              continue;
            }
            break;
          } else if (act > this.ACCEPT_ACTION) {
            if (configuration_stack.findConfiguration(local_stack, local_stack_top, buffer_index)) {
              act = this.ERROR_ACTION;
            } else {
              configuration_stack.push(local_stack, local_stack_top, act + 1, buffer_index, 0);
              act = this.baseAction(act);
            }
            continue;
          } else {
            break;
          }
          if (++local_stack_top >= local_stack.length) {
            break;
          }
          local_stack[local_stack_top] = act;
          act = this.tAction(act, current_kind);
        }
        return act == this.ACCEPT_ACTION ? exports.MAX_DISTANCE : buffer_index;
      }
      //
      //  This routine is invoked when an error is encountered.  It
      // tries to diagnose the error and recover from it.  If it is
      // successful, the state stack, the current token and the buffer
      // are readjusted; i.e., after a successful recovery,
      // state_stack_top points to the location in the state stack
      // that contains the state on which to recover; current_token
      // identifies the symbol on which to recover.
      //
      // Up to three configurations may be available when this routine
      // is invoked. PREV_STACK may contain the sequence of states
      // preceding any action on prevtok, STACK always contains the
      // sequence of states preceding any action on current_token, and
      // NEXT_STACK may contain the sequence of states preceding any
      // action on the successor of current_token.
      //
      errorRecovery(error_token) {
        let prevtok = this.tokStream.getPrevious(error_token);
        let candidate = this.primaryPhase(error_token);
        if (candidate.symbol !== 0) {
          return candidate;
        }
        candidate = this.secondaryPhase(error_token);
        if (candidate.symbol !== 0) {
          return candidate;
        }
        if (this.tokStream.getKind(error_token) !== this.EOFT_SYMBOL) {
          while (this.tokStream.getKind(this.buffer[exports.BUFF_UBOUND]) !== this.EOFT_SYMBOL) {
            candidate = this.secondaryPhase(this.buffer[exports.MAX_DISTANCE - exports.MIN_DISTANCE + 2]);
            if (candidate.symbol != 0) {
              return candidate;
            }
          }
        }
        let scope_repair = new PrimaryRepairInfo();
        scope_repair.bufferPosition = exports.BUFF_UBOUND;
        for (let top = this.stateStackTop; top >= 0; top--) {
          this.scopeTrial(scope_repair, this.stateStack, top);
          if (scope_repair.distance > 0) {
            break;
          }
        }
        for (let i = 0; i < this.scopeStackTop; i++) {
          this.emitError(ParseErrorCodes_1.ParseErrorCodes.SCOPE_CODE, -this.scopeIndex[i], this.locationStack[this.scopePosition[i]], this.buffer[1], this.nonterminalIndex(this.scopeLhs(this.scopeIndex[i])));
        }
        if (this.tokStream.getKind(error_token) == this.EOFT_SYMBOL) {
          this.emitError(ParseErrorCodes_1.ParseErrorCodes.EOF_CODE, this.terminalIndex(this.EOFT_SYMBOL), prevtok, prevtok);
        } else {
          let i;
          for (i = exports.BUFF_UBOUND; this.tokStream.getKind(this.buffer[i]) == this.EOFT_SYMBOL; i--) {
          }
          this.emitError(ParseErrorCodes_1.ParseErrorCodes.DELETION_CODE, this.terminalIndex(this.tokStream.getKind(error_token)), error_token, this.buffer[i]);
        }
        candidate.symbol = 0;
        candidate.location = this.buffer[exports.BUFF_UBOUND];
        return candidate;
      }
      //
      // This function tries primary and scope recovery on each
      // available configuration.  If a successful recovery is found
      // and no secondary phase recovery can do better, a diagnosis is
      // issued, the configuration is updated and the function returns
      // "true".  Otherwise, it returns "false".
      //
      primaryPhase(error_token) {
        let i = this.nextStackTop >= 0 ? 3 : 2;
        this.buffer[i] = error_token;
        for (let j = i; j > 0; j--) {
          this.buffer[j - 1] = this.tokStream.getPrevious(this.buffer[j]);
        }
        for (let k = i + 1; k < exports.BUFF_SIZE; k++) {
          this.buffer[k] = this.tokStream.getNext(this.buffer[k - 1]);
        }
        let repair = new PrimaryRepairInfo();
        if (this.nextStackTop >= 0) {
          repair.bufferPosition = 3;
          this.checkPrimaryDistance(repair, this.nextStack, this.nextStackTop);
        }
        let base_repair = new PrimaryRepairInfo(repair);
        base_repair.bufferPosition = 2;
        this.checkPrimaryDistance(base_repair, this.stateStack, this.stateStackTop);
        if (base_repair.distance > repair.distance || base_repair.misspellIndex > repair.misspellIndex) {
          repair = base_repair;
        }
        if (this.prevStackTop >= 0) {
          let prev_repair = new PrimaryRepairInfo(repair);
          prev_repair.bufferPosition = 1;
          this.checkPrimaryDistance(prev_repair, this.prevStack, this.prevStackTop);
          if (prev_repair.distance > repair.distance || prev_repair.misspellIndex > repair.misspellIndex) {
            repair = prev_repair;
          }
        }
        let candidate = new RepairCandidate();
        if (this.nextStackTop >= 0) {
          if (this.secondaryCheck(this.nextStack, this.nextStackTop, 3, repair.distance)) {
            return candidate;
          }
        } else {
          if (this.secondaryCheck(this.stateStack, this.stateStackTop, 2, repair.distance)) {
            return candidate;
          }
        }
        repair.distance = repair.distance - repair.bufferPosition + 1;
        if (repair.code == ParseErrorCodes_1.ParseErrorCodes.INVALID_CODE || repair.code == ParseErrorCodes_1.ParseErrorCodes.DELETION_CODE || repair.code == ParseErrorCodes_1.ParseErrorCodes.SUBSTITUTION_CODE || repair.code == ParseErrorCodes_1.ParseErrorCodes.MERGE_CODE) {
          repair.distance--;
        }
        if (repair.distance < exports.MIN_DISTANCE) {
          return candidate;
        }
        if (repair.code == ParseErrorCodes_1.ParseErrorCodes.INSERTION_CODE) {
          if (this.tokStream.getKind(this.buffer[repair.bufferPosition - 1]) == 0) {
            repair.code = ParseErrorCodes_1.ParseErrorCodes.BEFORE_CODE;
          }
        }
        if (repair.bufferPosition == 1) {
          this.stateStackTop = this.prevStackTop;
          Utils_1.Lpg.Lang.System.arraycopy(this.prevStack, 0, this.stateStack, 0, this.stateStackTop + 1);
        } else {
          if (this.nextStackTop >= 0 && repair.bufferPosition >= 3) {
            this.stateStackTop = this.nextStackTop;
            Utils_1.Lpg.Lang.System.arraycopy(this.nextStack, 0, this.stateStack, 0, this.stateStackTop + 1);
            this.locationStack[this.stateStackTop] = this.buffer[3];
          }
        }
        return this.primaryDiagnosis(repair);
      }
      //
      //  This function checks whether or not a given state has a
      // candidate, whose string representaion is a merging of the two
      // tokens at positions buffer_position and buffer_position+1 in
      // the buffer.  If so, it returns the candidate in question;
      // otherwise it returns 0.
      //
      mergeCandidate(state, buffer_position) {
        let str = this.tokStream.getName(this.buffer[buffer_position]) + this.tokStream.getName(this.buffer[buffer_position + 1]);
        for (let k = this.asi(state); this.asr(k) != 0; k++) {
          let i = this.terminalIndex(this.asr(k));
          if (str.length == this.name(i).length) {
            if (str.toLowerCase() == this.name(i).toLowerCase()) {
              return this.asr(k);
            }
          }
        }
        return 0;
      }
      //
      // This procedure takes as arguments a parsing configuration
      // consisting of a state stack (stack and stack_top) and a fixed
      // number of input tokens (starting at buffer_position) in the
      // input BUFFER; and some reference arguments: repair_code,
      // distance, misspell_index, candidate, and stack_position
      // which it sets based on the best possible recovery that it
      // finds in the given configuration.  The effectiveness of a
      // a repair is judged based on two criteria:
      //
      //       1) the number of tokens that can be parsed after the repair
      //              is applied: distance.
      //       2) how close to perfection is the candidate that is chosen:
      //              misspell_index.
      //
      // When this procedure is entered, distance, misspell_index and
      // repair_code are assumed to be initialized.
      //
      checkPrimaryDistance(repair, stck, stack_top) {
        let scope_repair = new PrimaryRepairInfo(repair);
        this.scopeTrial(scope_repair, stck, stack_top);
        if (scope_repair.distance > repair.distance) {
          repair.copy(scope_repair);
        }
        let symbol = this.mergeCandidate(stck[stack_top], repair.bufferPosition);
        if (symbol !== 0) {
          let j2 = this.parseCheck(stck, stack_top, symbol, repair.bufferPosition + 2);
          if (j2 > repair.distance || j2 == repair.distance && repair.misspellIndex < 10) {
            repair.misspellIndex = 10;
            repair.symbol = symbol;
            repair.distance = j2;
            repair.code = ParseErrorCodes_1.ParseErrorCodes.MERGE_CODE;
          }
        }
        let j = this.parseCheck(stck, stack_top, this.tokStream.getKind(this.buffer[repair.bufferPosition + 1]), repair.bufferPosition + 2);
        let k = this.tokStream.getKind(this.buffer[repair.bufferPosition]) == this.EOLT_SYMBOL && this.tokStream.afterEol(this.buffer[repair.bufferPosition + 1]) ? 10 : 0;
        if (j > repair.distance || j == repair.distance && k > repair.misspellIndex) {
          repair.misspellIndex = k;
          repair.code = ParseErrorCodes_1.ParseErrorCodes.DELETION_CODE;
          repair.distance = j;
        }
        let next_state = stck[stack_top], max_pos = stack_top;
        this.tempStackTop = stack_top - 1;
        this.tokStream.reset(this.buffer[repair.bufferPosition + 1]);
        let tok = this.tokStream.getKind(this.buffer[repair.bufferPosition]), act = this.tAction(next_state, tok);
        while (act <= this.NUM_RULES) {
          do {
            let lhs_symbol = this.lhs(act);
            this.tempStackTop -= this.rhs(act) - 1;
            act = this.tempStackTop > max_pos ? this.tempStack[this.tempStackTop] : stck[this.tempStackTop];
            act = this.ntAction(act, lhs_symbol);
          } while (act <= this.NUM_RULES);
          max_pos = max_pos < this.tempStackTop ? max_pos : this.tempStackTop;
          this.tempStack[this.tempStackTop + 1] = act;
          next_state = act;
          act = this.tAction(next_state, tok);
        }
        let root = 0;
        for (let i = this.asi(next_state); this.asr(i) !== 0; i++) {
          symbol = this.asr(i);
          if (symbol !== this.EOFT_SYMBOL && symbol !== this.ERROR_SYMBOL) {
            if (root == 0) {
              this.list[symbol] = symbol;
            } else {
              this.list[symbol] = this.list[root];
              this.list[root] = symbol;
            }
            root = symbol;
          }
        }
        if (stck[stack_top] !== next_state) {
          for (let i = this.asi(stck[stack_top]); this.asr(i) !== 0; i++) {
            symbol = this.asr(i);
            if (symbol !== this.EOFT_SYMBOL && symbol !== this.ERROR_SYMBOL && this.list[symbol] == 0) {
              if (root == 0) {
                this.list[symbol] = symbol;
              } else {
                this.list[symbol] = this.list[root];
                this.list[root] = symbol;
              }
              root = symbol;
            }
          }
        }
        let head = this.list[root];
        this.list[root] = 0;
        root = head;
        symbol = root;
        while (symbol !== 0) {
          let m = this.parseCheck(stck, stack_top, symbol, repair.bufferPosition), n = symbol == this.EOLT_SYMBOL && this.tokStream.afterEol(this.buffer[repair.bufferPosition]) ? 10 : 0;
          if (m > repair.distance || m == repair.distance && n > repair.misspellIndex) {
            repair.misspellIndex = n;
            repair.distance = m;
            repair.symbol = symbol;
            repair.code = ParseErrorCodes_1.ParseErrorCodes.INSERTION_CODE;
          }
          symbol = this.list[symbol];
        }
        symbol = root;
        while (symbol !== 0) {
          let m = this.parseCheck(stck, stack_top, symbol, repair.bufferPosition + 1), n = symbol == this.EOLT_SYMBOL && this.tokStream.afterEol(this.buffer[repair.bufferPosition + 1]) ? 10 : this.misspell(symbol, this.buffer[repair.bufferPosition]);
          if (m > repair.distance || m == repair.distance && n > repair.misspellIndex) {
            repair.misspellIndex = n;
            repair.distance = m;
            repair.symbol = symbol;
            repair.code = ParseErrorCodes_1.ParseErrorCodes.SUBSTITUTION_CODE;
          }
          let s = symbol;
          symbol = this.list[symbol];
          this.list[s] = 0;
        }
        for (let nt_index = this.nasi(stck[stack_top]); this.nasr(nt_index) !== 0; nt_index++) {
          symbol = this.nasr(nt_index) + this.NT_OFFSET;
          let n = this.parseCheck(stck, stack_top, symbol, repair.bufferPosition + 1);
          if (n > repair.distance) {
            repair.misspellIndex = 0;
            repair.distance = n;
            repair.symbol = symbol;
            repair.code = ParseErrorCodes_1.ParseErrorCodes.INVALID_CODE;
          }
          n = this.parseCheck(stck, stack_top, symbol, repair.bufferPosition);
          if (n > repair.distance || n == repair.distance && repair.code == ParseErrorCodes_1.ParseErrorCodes.INVALID_CODE) {
            repair.misspellIndex = 0;
            repair.distance = n;
            repair.symbol = symbol;
            repair.code = ParseErrorCodes_1.ParseErrorCodes.INSERTION_CODE;
          }
        }
        return;
      }
      //
      // This procedure is invoked to issue a diagnostic message and
      // adjust the input buffer.  The recovery in question is either
      // the insertion of one or more scopes, the merging of the error
      // token with its successor, the deletion of the error token,
      // the insertion of a single token in front of the error token
      // or the substitution of another token for the error token.
      //
      primaryDiagnosis(repair) {
        let prevtok = this.buffer[repair.bufferPosition - 1], current_token = this.buffer[repair.bufferPosition];
        switch (repair.code) {
          case ParseErrorCodes_1.ParseErrorCodes.INSERTION_CODE:
          case ParseErrorCodes_1.ParseErrorCodes.BEFORE_CODE:
            {
              let name_index = repair.symbol > this.NT_OFFSET ? this.getNtermIndex(this.stateStack[this.stateStackTop], repair.symbol, repair.bufferPosition) : this.getTermIndex(this.stateStack, this.stateStackTop, repair.symbol, repair.bufferPosition);
              let tok = repair.code == ParseErrorCodes_1.ParseErrorCodes.INSERTION_CODE ? prevtok : current_token;
              this.emitError(repair.code, name_index, tok, tok);
            }
            break;
          case ParseErrorCodes_1.ParseErrorCodes.INVALID_CODE:
            {
              let name_index = this.getNtermIndex(this.stateStack[this.stateStackTop], repair.symbol, repair.bufferPosition + 1);
              this.emitError(repair.code, name_index, current_token, current_token);
            }
            break;
          case ParseErrorCodes_1.ParseErrorCodes.SUBSTITUTION_CODE:
            {
              let name_index;
              if (repair.misspellIndex >= 6) {
                name_index = this.terminalIndex(repair.symbol);
              } else {
                name_index = this.getTermIndex(this.stateStack, this.stateStackTop, repair.symbol, repair.bufferPosition + 1);
                if (name_index != this.terminalIndex(repair.symbol)) {
                  repair.code = ParseErrorCodes_1.ParseErrorCodes.INVALID_CODE;
                }
              }
              this.emitError(repair.code, name_index, current_token, current_token);
            }
            break;
          case ParseErrorCodes_1.ParseErrorCodes.MERGE_CODE:
            this.emitError(repair.code, this.terminalIndex(repair.symbol), current_token, this.tokStream.getNext(current_token));
            break;
          case ParseErrorCodes_1.ParseErrorCodes.SCOPE_CODE: {
            for (let i = 0; i < this.scopeStackTop; i++) {
              this.emitError(repair.code, -this.scopeIndex[i], this.locationStack[this.scopePosition[i]], prevtok, this.nonterminalIndex(this.scopeLhs(this.scopeIndex[i])));
            }
            repair.symbol = this.scopeLhs(this.scopeIndex[this.scopeStackTop]) + this.NT_OFFSET;
            this.stateStackTop = this.scopePosition[this.scopeStackTop];
            this.emitError(repair.code, -this.scopeIndex[this.scopeStackTop], this.locationStack[this.scopePosition[this.scopeStackTop]], prevtok, this.getNtermIndex(this.stateStack[this.stateStackTop], repair.symbol, repair.bufferPosition));
            break;
          }
          default:
            this.emitError(repair.code, this.terminalIndex(this.ERROR_SYMBOL), current_token, current_token);
        }
        let candidate = new RepairCandidate();
        switch (repair.code) {
          case ParseErrorCodes_1.ParseErrorCodes.INSERTION_CODE:
          case ParseErrorCodes_1.ParseErrorCodes.BEFORE_CODE:
          case ParseErrorCodes_1.ParseErrorCodes.SCOPE_CODE:
            candidate.symbol = repair.symbol;
            candidate.location = this.buffer[repair.bufferPosition];
            this.tokStream.reset(this.buffer[repair.bufferPosition]);
            break;
          case ParseErrorCodes_1.ParseErrorCodes.INVALID_CODE:
          case ParseErrorCodes_1.ParseErrorCodes.SUBSTITUTION_CODE:
            candidate.symbol = repair.symbol;
            candidate.location = this.buffer[repair.bufferPosition];
            this.tokStream.reset(this.buffer[repair.bufferPosition + 1]);
            break;
          case ParseErrorCodes_1.ParseErrorCodes.MERGE_CODE:
            candidate.symbol = repair.symbol;
            candidate.location = this.buffer[repair.bufferPosition];
            this.tokStream.reset(this.buffer[repair.bufferPosition + 2]);
            break;
          default:
            candidate.location = this.buffer[repair.bufferPosition + 1];
            candidate.symbol = this.tokStream.getKind(this.buffer[repair.bufferPosition + 1]);
            this.tokStream.reset(this.buffer[repair.bufferPosition + 2]);
            break;
        }
        return candidate;
      }
      //
      // This function takes as parameter an integer STACK_TOP that
      // points to a STACK element containing the state on which a
      // primary recovery will be made; the terminal candidate on which
      // to recover; and an integer: buffer_position, which points to
      // the position of the next input token in the BUFFER.  The
      // parser is simulated until a shift (or shift-reduce) action
      // is computed on the candidate.  Then we proceed to compute the
      // the name index of the highest level nonterminal that can
      // directly or indirectly produce the candidate.
      //
      getTermIndex(stck, stack_top, tok, buffer_position) {
        let act = stck[stack_top], max_pos = stack_top, highest_symbol = tok;
        this.tempStackTop = stack_top - 1;
        this.tokStream.reset(this.buffer[buffer_position]);
        act = this.tAction(act, tok);
        while (act <= this.NUM_RULES) {
          do {
            let lhs_symbol = this.lhs(act);
            this.tempStackTop -= this.rhs(act) - 1;
            act = this.tempStackTop > max_pos ? this.tempStack[this.tempStackTop] : stck[this.tempStackTop];
            act = this.ntAction(act, lhs_symbol);
          } while (act <= this.NUM_RULES);
          max_pos = max_pos < this.tempStackTop ? max_pos : this.tempStackTop;
          this.tempStack[this.tempStackTop + 1] = act;
          act = this.tAction(act, tok);
        }
        this.tempStackTop++;
        let threshold = this.tempStackTop;
        tok = this.tokStream.getKind(this.buffer[buffer_position]);
        this.tokStream.reset(this.buffer[buffer_position + 1]);
        if (act > this.ERROR_ACTION) {
          act -= this.ERROR_ACTION;
        } else {
          if (act < this.ACCEPT_ACTION) {
            this.tempStack[this.tempStackTop + 1] = act;
            act = this.tAction(act, tok);
          }
        }
        while (act <= this.NUM_RULES) {
          do {
            let lhs_symbol = this.lhs(act);
            this.tempStackTop -= this.rhs(act) - 1;
            if (this.tempStackTop < threshold) {
              return highest_symbol > this.NT_OFFSET ? this.nonterminalIndex(highest_symbol - this.NT_OFFSET) : this.terminalIndex(highest_symbol);
            }
            if (this.tempStackTop == threshold) {
              highest_symbol = lhs_symbol + this.NT_OFFSET;
            }
            act = this.tempStackTop > max_pos ? this.tempStack[this.tempStackTop] : stck[this.tempStackTop];
            act = this.ntAction(act, lhs_symbol);
          } while (act <= this.NUM_RULES);
          this.tempStack[this.tempStackTop + 1] = act;
          act = this.tAction(act, tok);
        }
        return highest_symbol > this.NT_OFFSET ? this.nonterminalIndex(highest_symbol - this.NT_OFFSET) : this.terminalIndex(highest_symbol);
      }
      //
      // This function takes as parameter a starting state number:
      // start, a nonterminal symbol, A (candidate), and an integer,
      // buffer_position,  which points to the position of the next
      // input token in the BUFFER.
      // It returns the highest level non-terminal B such that
      // B =>*rm A.  I.e., there does not exists a nonterminal C such
      // that C =>+rm B. (Recall that for an LALR(k) grammar if
      // C =>+rm B, it cannot be the case that B =>+rm C)
      //
      getNtermIndex(start, sym, buffer_position) {
        let highest_symbol = sym - this.NT_OFFSET, tok = this.tokStream.getKind(this.buffer[buffer_position]);
        this.tokStream.reset(this.buffer[buffer_position + 1]);
        this.tempStackTop = 0;
        this.tempStack[this.tempStackTop] = start;
        let act = this.ntAction(start, highest_symbol);
        if (act > this.NUM_RULES) {
          this.tempStack[this.tempStackTop + 1] = act;
          act = this.tAction(act, tok);
        }
        while (act <= this.NUM_RULES) {
          do {
            this.tempStackTop -= this.rhs(act) - 1;
            if (this.tempStackTop < 0) {
              return this.nonterminalIndex(highest_symbol);
            }
            if (this.tempStackTop == 0) {
              highest_symbol = this.lhs(act);
            }
            act = this.ntAction(this.tempStack[this.tempStackTop], this.lhs(act));
          } while (act <= this.NUM_RULES);
          this.tempStack[this.tempStackTop + 1] = act;
          act = this.tAction(act, tok);
        }
        return this.nonterminalIndex(highest_symbol);
      }
      //
      //  Check whether or not there is a high probability that a
      // given string is a misspelling of another.
      // Certain singleton symbols (such as ":" and ";") are also
      // considered to be misspellings of each other.
      //
      misspell(sym, tok) {
        let s1 = this.name(this.terminalIndex(sym)).toLowerCase();
        let n = s1.length;
        s1 += "\0";
        let s2 = this.tokStream.getName(tok).toLowerCase();
        let m = s2.length < this.MAX_NAME_LENGTH ? s2.length : this.MAX_NAME_LENGTH;
        s2 = s2.substring(0, m) + "\0";
        if (n == 1 && m == 1) {
          if (s1.charAt(0) == ";" && s2.charAt(0) == "," || s1.charAt(0) == "," && s2.charAt(0) == ";" || s1.charAt(0) == ";" && s2.charAt(0) == ":" || s1.charAt(0) == ":" && s2.charAt(0) == ";" || s1.charAt(0) == "." && s2.charAt(0) == "," || s1.charAt(0) == "," && s2.charAt(0) == "." || s1.charAt(0) == "'" && s2.charAt(0) == '"' || s1.charAt(0) == '"' && s2.charAt(0) == "'") {
            return 3;
          }
        }
        let count = 0, prefix_length = 0, num_errors = 0;
        let i = 0, j = 0;
        while (i < n && j < m) {
          if (s1.charAt(i) == s2.charAt(j)) {
            count++;
            i++;
            j++;
            if (num_errors == 0) {
              prefix_length++;
            }
          } else if (s1.charAt(i + 1) == s2.charAt(j) && s1.charAt(i) == s2.charAt(j + 1)) {
            count += 2;
            i += 2;
            j += 2;
            num_errors++;
          } else if (s1.charAt(i + 1) == s2.charAt(j + 1)) {
            i += 2;
            j += 2;
            num_errors++;
          } else {
            if (n - i > m - j) {
              i++;
            } else if (m - j > n - i) {
              j++;
            } else {
              i++;
              j++;
            }
            num_errors++;
          }
        }
        if (i < n || j < m) {
          num_errors++;
        }
        if (num_errors > (n < m ? n : m) / 6 + 1) {
          count = prefix_length;
        }
        return Math.floor(count * 10 / ((n < s1.length ? s1.length : n) + num_errors));
      }
      scopeTrial(repair, stack, stack_top) {
        if (!this.stateSeen || this.stateSeen.length == 0 || this.stateSeen.length < this.stateStack.length) {
          this.stateSeen = new Int32Array(this.stateStack.length);
        }
        for (let i = 0; i < this.stateStack.length; i++) {
          this.stateSeen[i] = _DiagnoseParser.NIL;
        }
        this.statePoolTop = 0;
        if (!this.statePool || this.statePool.length == 0 || this.statePool.length < this.stateStack.length) {
          this.statePool = new Array(this.stateStack.length);
        }
        this.scopeTrialCheck(repair, stack, stack_top, 0);
        repair.code = ParseErrorCodes_1.ParseErrorCodes.SCOPE_CODE;
        repair.misspellIndex = 10;
        return;
      }
      scopeTrialCheck(repair, stack, stack_top, indx) {
        for (let i = this.stateSeen[stack_top]; i != _DiagnoseParser.NIL; i = this.statePool[i].next) {
          if (this.statePool[i].state == stack[stack_top]) {
            return;
          }
        }
        let old_state_pool_top = this.statePoolTop++;
        if (this.statePoolTop >= this.statePool.length) {
          Utils_1.Lpg.Lang.System.arraycopy(this.statePool, 0, this.statePool = new Array(this.statePoolTop * 2), 0, this.statePoolTop);
        }
        this.statePool[old_state_pool_top] = new StateInfo(stack[stack_top], this.stateSeen[stack_top]);
        this.stateSeen[stack_top] = old_state_pool_top;
        let action = new IntTuple_1.IntTuple(1 << 3);
        for (let i = 0; i < this.SCOPE_SIZE; i++) {
          action.reset();
          let act = this.tAction(stack[stack_top], this.scopeLa(i));
          if (act > this.ACCEPT_ACTION && act < this.ERROR_ACTION) {
            do {
              action.add(this.baseAction(act++));
            } while (this.baseAction(act) != 0);
          } else {
            action.add(act);
          }
          for (let action_index = 0; action_index < action.size(); action_index++) {
            this.tokStream.reset(this.buffer[repair.bufferPosition]);
            this.tempStackTop = stack_top - 1;
            let max_pos = stack_top;
            act = action.get(action_index);
            while (act <= this.NUM_RULES) {
              do {
                let lhs_symbol = this.lhs(act);
                this.tempStackTop -= this.rhs(act) - 1;
                act = this.tempStackTop > max_pos ? this.tempStack[this.tempStackTop] : stack[this.tempStackTop];
                act = this.ntAction(act, lhs_symbol);
              } while (act <= this.NUM_RULES);
              if (this.tempStackTop + 1 >= this.stateStack.length) {
                return;
              }
              max_pos = max_pos < this.tempStackTop ? max_pos : this.tempStackTop;
              this.tempStack[this.tempStackTop + 1] = act;
              act = this.tAction(act, this.scopeLa(i));
            }
            if (act !== this.ERROR_ACTION) {
              let j, k = this.scopePrefix(i);
              for (j = this.tempStackTop + 1; j >= max_pos + 1 && this.inSymbol(this.tempStack[j]) == this.scopeRhs(k); j--) {
                k++;
              }
              if (j == max_pos) {
                for (j = max_pos; j >= 1 && this.inSymbol(stack[j]) == this.scopeRhs(k); j--) {
                  k++;
                }
              }
              let marked_pos = max_pos < stack_top ? max_pos + 1 : stack_top;
              if (this.scopeRhs(k) == 0 && j < marked_pos) {
                let stack_position = j;
                for (j = this.scopeStateSet(i); stack[stack_position] != this.scopeState(j) && this.scopeState(j) !== 0; j++) {
                }
                if (this.scopeState(j) !== 0) {
                  let previous_distance = repair.distance, distance = this.parseCheck(stack, stack_position, this.scopeLhs(i) + this.NT_OFFSET, repair.bufferPosition);
                  if (distance - repair.bufferPosition + 1 < exports.MIN_DISTANCE) {
                    let top = stack_position;
                    act = this.ntAction(stack[top], this.scopeLhs(i));
                    while (act <= this.NUM_RULES) {
                      top -= this.rhs(act) - 1;
                      act = this.ntAction(stack[top], this.lhs(act));
                    }
                    top++;
                    j = act;
                    act = stack[top];
                    stack[top] = j;
                    this.scopeTrialCheck(repair, stack, top, indx + 1);
                    stack[top] = act;
                  } else if (distance > repair.distance) {
                    this.scopeStackTop = indx;
                    repair.distance = distance;
                  }
                  if (
                    // TODO: main_configuration_stack.size() == 0 && // no other bactracking possibilities left
                    this.tokStream.getKind(this.buffer[repair.bufferPosition]) == this.EOFT_SYMBOL && repair.distance == previous_distance
                  ) {
                    this.scopeStackTop = indx;
                    repair.distance = exports.MAX_DISTANCE;
                  }
                  if (repair.distance > previous_distance) {
                    this.scopeIndex[indx] = i;
                    this.scopePosition[indx] = stack_position;
                    return;
                  }
                }
              }
            }
          }
        }
      }
      //
      // This function computes the ParseCheck distance for the best
      // possible secondary recovery for a given configuration that
      // either deletes none or only one symbol in the forward context.
      // If the recovery found is more effective than the best primary
      // recovery previously computed, then the function returns true.
      // Only misplacement, scope and manual recoveries are attempted;
      // simple insertion or substitution of a nonterminal are tried
      // in CHECK_PRIMARY_DISTANCE as part of primary recovery.
      //
      secondaryCheck(stack, stack_top, buffer_position, distance) {
        for (let top = stack_top - 1; top >= 0; top--) {
          let j = this.parseCheck(stack, top, this.tokStream.getKind(this.buffer[buffer_position]), buffer_position + 1);
          if (j - buffer_position + 1 > exports.MIN_DISTANCE && j > distance) {
            return true;
          }
        }
        let scope_repair = new PrimaryRepairInfo();
        scope_repair.bufferPosition = buffer_position + 1;
        scope_repair.distance = distance;
        this.scopeTrial(scope_repair, stack, stack_top);
        return scope_repair.distance - buffer_position > exports.MIN_DISTANCE && scope_repair.distance > distance;
      }
      //
      // Secondary_phase is a boolean function that checks whether or
      // not some form of secondary recovery is applicable to one of
      // the error configurations. First, if "next_stack" is available,
      // misplacement and secondary recoveries are attempted on it.
      // Then, in any case, these recoveries are attempted on "stack".
      // If a successful recovery is found, a diagnosis is issued, the
      // configuration is updated and the function returns "true".
      // Otherwise, the function returns false.
      //
      secondaryPhase(error_token) {
        let repair = new SecondaryRepairInfo(), misplaced_repair = new SecondaryRepairInfo();
        let next_last_index = 0;
        if (this.nextStackTop >= 0) {
          let save_location;
          this.buffer[2] = error_token;
          this.buffer[1] = this.tokStream.getPrevious(this.buffer[2]);
          this.buffer[0] = this.tokStream.getPrevious(this.buffer[1]);
          for (let k = 3; k < exports.BUFF_UBOUND; k++) {
            this.buffer[k] = this.tokStream.getNext(this.buffer[k - 1]);
          }
          this.buffer[exports.BUFF_UBOUND] = this.tokStream.badToken();
          for (next_last_index = exports.MAX_DISTANCE - 1; next_last_index >= 1 && this.tokStream.getKind(this.buffer[next_last_index]) == this.EOFT_SYMBOL; next_last_index--) {
          }
          next_last_index = next_last_index + 1;
          save_location = this.locationStack[this.nextStackTop];
          this.locationStack[this.nextStackTop] = this.buffer[2];
          misplaced_repair.numDeletions = this.nextStackTop;
          this.misplacementRecovery(misplaced_repair, this.nextStack, this.nextStackTop, next_last_index, true);
          if (misplaced_repair.recoveryOnNextStack) {
            misplaced_repair.distance++;
          }
          repair.numDeletions = this.nextStackTop + exports.BUFF_UBOUND;
          this.secondaryRecovery(repair, this.nextStack, this.nextStackTop, next_last_index, true);
          if (repair.recoveryOnNextStack) {
            repair.distance++;
          }
          this.locationStack[this.nextStackTop] = save_location;
        } else {
          misplaced_repair.numDeletions = this.stateStackTop;
          repair.numDeletions = this.stateStackTop + exports.BUFF_UBOUND;
        }
        this.buffer[3] = error_token;
        this.buffer[2] = this.tokStream.getPrevious(this.buffer[3]);
        this.buffer[1] = this.tokStream.getPrevious(this.buffer[2]);
        this.buffer[0] = this.tokStream.getPrevious(this.buffer[1]);
        for (let k = 4; k < exports.BUFF_SIZE; k++) {
          this.buffer[k] = this.tokStream.getNext(this.buffer[k - 1]);
        }
        let last_index;
        for (last_index = exports.MAX_DISTANCE - 1; last_index >= 1 && this.tokStream.getKind(this.buffer[last_index]) == this.EOFT_SYMBOL; last_index--) {
        }
        last_index++;
        this.misplacementRecovery(misplaced_repair, this.stateStack, this.stateStackTop, last_index, false);
        this.secondaryRecovery(repair, this.stateStack, this.stateStackTop, last_index, false);
        if (misplaced_repair.distance > exports.MIN_DISTANCE) {
          if (misplaced_repair.numDeletions <= repair.numDeletions || misplaced_repair.distance - misplaced_repair.numDeletions >= repair.distance - repair.numDeletions) {
            repair.code = ParseErrorCodes_1.ParseErrorCodes.MISPLACED_CODE;
            repair.stackPosition = misplaced_repair.stackPosition;
            repair.bufferPosition = 2;
            repair.numDeletions = misplaced_repair.numDeletions;
            repair.distance = misplaced_repair.distance;
            repair.recoveryOnNextStack = misplaced_repair.recoveryOnNextStack;
          }
        }
        if (repair.recoveryOnNextStack) {
          this.stateStackTop = this.nextStackTop;
          Utils_1.Lpg.Lang.System.arraycopy(this.nextStack, 0, this.stateStack, 0, this.stateStackTop + 1);
          this.buffer[2] = error_token;
          this.buffer[1] = this.tokStream.getPrevious(this.buffer[2]);
          this.buffer[0] = this.tokStream.getPrevious(this.buffer[1]);
          for (let k = 3; k < exports.BUFF_UBOUND; k++) {
            this.buffer[k] = this.tokStream.getNext(this.buffer[k - 1]);
          }
          this.buffer[exports.BUFF_UBOUND] = this.tokStream.badToken();
          this.locationStack[this.nextStackTop] = this.buffer[2];
          last_index = next_last_index;
        }
        if (repair.code == ParseErrorCodes_1.ParseErrorCodes.SECONDARY_CODE || repair.code == ParseErrorCodes_1.ParseErrorCodes.DELETION_CODE) {
          let scope_repair = new PrimaryRepairInfo();
          for (scope_repair.bufferPosition = 2; scope_repair.bufferPosition <= repair.bufferPosition && repair.code !== ParseErrorCodes_1.ParseErrorCodes.SCOPE_CODE; scope_repair.bufferPosition++) {
            this.scopeTrial(scope_repair, this.stateStack, this.stateStackTop);
            let j = scope_repair.distance == exports.MAX_DISTANCE ? last_index : scope_repair.distance, k = scope_repair.bufferPosition - 1;
            if (scope_repair.distance - k > exports.MIN_DISTANCE && j - k > repair.distance - repair.numDeletions) {
              let i = this.scopeIndex[this.scopeStackTop];
              repair.code = ParseErrorCodes_1.ParseErrorCodes.SCOPE_CODE;
              repair.symbol = this.scopeLhs(i) + this.NT_OFFSET;
              repair.stackPosition = this.stateStackTop;
              repair.bufferPosition = scope_repair.bufferPosition;
            }
          }
        }
        let candidate = new RepairCandidate();
        if (repair.code == 0) {
          return candidate;
        }
        this.secondaryDiagnosis(repair);
        switch (repair.code) {
          case ParseErrorCodes_1.ParseErrorCodes.MISPLACED_CODE:
            candidate.location = this.buffer[2];
            candidate.symbol = this.tokStream.getKind(this.buffer[2]);
            this.tokStream.reset(this.tokStream.getNext(this.buffer[2]));
            break;
          case ParseErrorCodes_1.ParseErrorCodes.DELETION_CODE:
            candidate.location = this.buffer[repair.bufferPosition];
            candidate.symbol = this.tokStream.getKind(this.buffer[repair.bufferPosition]);
            this.tokStream.reset(this.tokStream.getNext(this.buffer[repair.bufferPosition]));
            break;
          default:
            candidate.symbol = repair.symbol;
            candidate.location = this.buffer[repair.bufferPosition];
            this.tokStream.reset(this.buffer[repair.bufferPosition]);
            break;
        }
        return candidate;
      }
      //
      // This boolean function checks whether or not a given
      // configuration yields a better misplacement recovery than
      // the best misplacement recovery computed previously.
      //
      misplacementRecovery(repair, stack, stack_top, last_index, stack_flag) {
        let previous_loc = this.buffer[2], stack_deletions = 0;
        for (let top = stack_top - 1; top >= 0; top--) {
          if (this.locationStack[top] < previous_loc) {
            stack_deletions++;
          }
          previous_loc = this.locationStack[top];
          let parse_distance = this.parseCheck(stack, top, this.tokStream.getKind(this.buffer[2]), 3), j = parse_distance == exports.MAX_DISTANCE ? last_index : parse_distance;
          if (parse_distance > exports.MIN_DISTANCE && j - stack_deletions > repair.distance - repair.numDeletions) {
            repair.stackPosition = top;
            repair.distance = j;
            repair.numDeletions = stack_deletions;
            repair.recoveryOnNextStack = stack_flag;
          }
        }
        return;
      }
      //
      // This function checks whether or not a given
      // configuration yields a better secondary recovery than the
      // best misplacement recovery computed previously.
      //
      secondaryRecovery(repair, stack, stack_top, last_index, stack_flag) {
        let previous_loc = this.buffer[2], stack_deletions = 0;
        for (let top = stack_top; top >= 0 && repair.numDeletions >= stack_deletions; top--) {
          if (this.locationStack[top] < previous_loc) {
            stack_deletions++;
          }
          previous_loc = this.locationStack[top];
          for (let i = 2; i <= last_index - exports.MIN_DISTANCE + 1 && repair.numDeletions >= stack_deletions + i - 1; i++) {
            let parse_distance = this.parseCheck(stack, top, this.tokStream.getKind(this.buffer[i]), i + 1), j = parse_distance == exports.MAX_DISTANCE ? last_index : parse_distance;
            if (parse_distance - i + 1 > exports.MIN_DISTANCE) {
              let k = stack_deletions + i - 1;
              if (k < repair.numDeletions || j - k > repair.distance - repair.numDeletions || repair.code == ParseErrorCodes_1.ParseErrorCodes.SECONDARY_CODE && j - k == repair.distance - repair.numDeletions) {
                repair.code = ParseErrorCodes_1.ParseErrorCodes.DELETION_CODE;
                repair.distance = j;
                repair.stackPosition = top;
                repair.bufferPosition = i;
                repair.numDeletions = k;
                repair.recoveryOnNextStack = stack_flag;
              }
            }
            for (let l = this.nasi(stack[top]); l >= 0 && this.nasr(l) != 0; l++) {
              let symbol = this.nasr(l) + this.NT_OFFSET;
              parse_distance = this.parseCheck(stack, top, symbol, i);
              j = parse_distance == exports.MAX_DISTANCE ? last_index : parse_distance;
              if (parse_distance - i + 1 > exports.MIN_DISTANCE) {
                let k = stack_deletions + i - 1;
                if (k < repair.numDeletions || j - k > repair.distance - repair.numDeletions) {
                  repair.code = ParseErrorCodes_1.ParseErrorCodes.SECONDARY_CODE;
                  repair.symbol = symbol;
                  repair.distance = j;
                  repair.stackPosition = top;
                  repair.bufferPosition = i;
                  repair.numDeletions = k;
                  repair.recoveryOnNextStack = stack_flag;
                }
              }
            }
          }
        }
        return;
      }
      //
      // This procedure is invoked to issue a secondary diagnosis and
      // adjust the input buffer.  The recovery in question is either
      // an automatic scope recovery, a manual scope recovery, a
      // secondary substitution or a secondary deletion.
      //
      secondaryDiagnosis(repair) {
        switch (repair.code) {
          case ParseErrorCodes_1.ParseErrorCodes.SCOPE_CODE:
            if (repair.stackPosition < this.stateStackTop) {
              this.emitError(ParseErrorCodes_1.ParseErrorCodes.DELETION_CODE, this.terminalIndex(this.ERROR_SYMBOL), this.locationStack[repair.stackPosition], this.buffer[1]);
            }
            for (let i = 0; i < this.scopeStackTop; i++) {
              this.emitError(ParseErrorCodes_1.ParseErrorCodes.SCOPE_CODE, -this.scopeIndex[i], this.locationStack[this.scopePosition[i]], this.buffer[1], this.nonterminalIndex(this.scopeLhs(this.scopeIndex[i])));
            }
            repair.symbol = this.scopeLhs(this.scopeIndex[this.scopeStackTop]) + this.NT_OFFSET;
            this.stateStackTop = this.scopePosition[this.scopeStackTop];
            this.emitError(ParseErrorCodes_1.ParseErrorCodes.SCOPE_CODE, -this.scopeIndex[this.scopeStackTop], this.locationStack[this.scopePosition[this.scopeStackTop]], this.buffer[1], this.getNtermIndex(this.stateStack[this.stateStackTop], repair.symbol, repair.bufferPosition));
            break;
          default:
            this.emitError(repair.code, repair.code == ParseErrorCodes_1.ParseErrorCodes.SECONDARY_CODE ? this.getNtermIndex(this.stateStack[repair.stackPosition], repair.symbol, repair.bufferPosition) : this.terminalIndex(this.ERROR_SYMBOL), this.locationStack[repair.stackPosition], this.buffer[repair.bufferPosition - 1]);
            this.stateStackTop = repair.stackPosition;
        }
        return;
      }
      //
      // This method is invoked by an LPG PARSER or a semantic
      // routine to process an error message.
      //
      emitError(msg_code, name_index, left_token, right_token, scope_name_index = 0) {
        let left_token_loc = left_token > right_token ? right_token : left_token, right_token_loc = right_token;
        let token_name = name_index >= 0 && !(this.name(name_index).toUpperCase() == "ERROR") ? '"' + this.name(name_index) + '"' : "";
        if (msg_code == ParseErrorCodes_1.ParseErrorCodes.INVALID_CODE) {
          msg_code = token_name.length == 0 ? ParseErrorCodes_1.ParseErrorCodes.INVALID_CODE : ParseErrorCodes_1.ParseErrorCodes.INVALID_TOKEN_CODE;
        }
        if (msg_code == ParseErrorCodes_1.ParseErrorCodes.SCOPE_CODE) {
          token_name = '"';
          for (let i = this.scopeSuffix(-name_index); this.scopeRhs(i) !== 0; i++) {
            if (!this.isNullable(this.scopeRhs(i))) {
              let symbol_index = this.scopeRhs(i) > this.NT_OFFSET ? this.nonterminalIndex(this.scopeRhs(i) - this.NT_OFFSET) : this.terminalIndex(this.scopeRhs(i));
              if (this.name(symbol_index).length > 0) {
                if (token_name.length > 1) {
                  token_name += " ";
                }
                token_name += this.name(symbol_index);
              }
            }
          }
          token_name += '"';
        }
        this.tokStream.reportError(msg_code, left_token, right_token, token_name);
        return;
      }
      //
      // keep looking ahead until we compute a valid action
      //
      lookahead(act, token) {
        act = this.prs.lookAhead(act - this.LA_STATE_OFFSET, this.tokStream.getKind(token));
        return act > this.LA_STATE_OFFSET ? this.lookahead(act, this.tokStream.getNext(token)) : act;
      }
      //
      // Compute the next action defined on act and sym. If this
      // action requires more lookahead, these lookahead symbols
      // are in the token stream beginning at the next token that
      // is yielded by peek().
      //
      tAction(act, sym) {
        act = this.prs.tAction(act, sym);
        return act > this.LA_STATE_OFFSET ? this.lookahead(act, this.tokStream.peek()) : act;
      }
    };
    exports.DiagnoseParser = DiagnoseParser2;
    DiagnoseParser2.NIL = -1;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/BadParseException.js
var require_BadParseException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/BadParseException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.BadParseException = void 0;
    var BadParseException2 = class extends Error {
      constructor(errorToken) {
        super();
        this.error_token = errorToken;
      }
    };
    exports.BadParseException = BadParseException2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/RecoveryParser.js
var require_RecoveryParser = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/RecoveryParser.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.RecoveryParser = void 0;
    var DiagnoseParser_1 = require_DiagnoseParser();
    var IntTuple_1 = require_IntTuple();
    var ParseErrorCodes_1 = require_ParseErrorCodes();
    var ConfigurationStack_1 = require_ConfigurationStack();
    var BadParseException_1 = require_BadParseException();
    var Utils_1 = require_Utils();
    var RecoveryParser = class extends DiagnoseParser_1.DiagnoseParser {
      //
      // maxErrors is the maximum number of errors to be repaired
      // maxTime is the maximum amount of time allowed for diagnosing
      // but at least one error must be diagnosed 
      //
      constructor(parser, action, tokens, tokStream, prs, maxErrors = 0, maxTime = 0, monitor) {
        super(tokStream, prs, maxErrors, maxTime, monitor);
        this.actionStack = new Int32Array(0);
        this.scope_repair = new DiagnoseParser_1.PrimaryRepairInfo();
        this.parser = parser;
        this.action = action;
        this.tokens = tokens;
      }
      reallocateStacks() {
        super.reallocateStacks();
        if (!this.actionStack || this.actionStack.length == 0) {
          this.actionStack = new Int32Array(this.stateStack.length);
        } else {
          let old_stack_length = this.actionStack.length;
          Utils_1.Lpg.Lang.System.arraycopy(this.actionStack, 0, this.actionStack = new Int32Array(this.stateStack.length), 0, old_stack_length);
        }
        return;
      }
      reportError(scope_index, error_token) {
        let text = '"';
        for (let i = this.scopeSuffix(scope_index); this.scopeRhs(i) !== 0; i++) {
          if (!this.isNullable(this.scopeRhs(i))) {
            let symbol_index = this.scopeRhs(i) > this.NT_OFFSET ? this.nonterminalIndex(this.scopeRhs(i) - this.NT_OFFSET) : this.terminalIndex(this.scopeRhs(i));
            if (this.name(symbol_index).length > 0) {
              if (text.length > 1) {
                text += " ";
              }
              text += this.name(symbol_index);
            }
          }
        }
        text += '"';
        this.tokStream.reportError(ParseErrorCodes_1.ParseErrorCodes.SCOPE_CODE, error_token, error_token, [text]);
        return;
      }
      recover(marker_token, error_token) {
        if (!this.stateStack || this.stateStack.length == 0) {
          this.reallocateStacks();
        }
        this.tokens.reset();
        this.tokStream.reset();
        this.tokens.add(this.tokStream.getPrevious(this.tokStream.peek()));
        let restart_token = marker_token !== 0 ? marker_token : this.tokStream.getToken(), old_action_size = 0;
        this.stateStackTop = 0;
        this.stateStack[this.stateStackTop] = this.START_STATE;
        do {
          this.action.reset(old_action_size);
          if (!this.fixError(restart_token, error_token)) {
            throw new BadParseException_1.BadParseException(error_token);
          }
          if (this.monitor && this.monitor.isCancelled()) {
            break;
          }
          restart_token = error_token;
          this.tokStream.reset(error_token);
          old_action_size = this.action.size();
          error_token = this.parser.backtrackParse(this.stateStack, this.stateStackTop, this.action, 0);
          this.tokStream.reset(this.tokStream.getNext(restart_token));
        } while (error_token !== 0);
        return restart_token;
      }
      //
      // Given the configuration consisting of the states in stateStack
      // and the sequence of tokens (current_kind, followed by the tokens
      // in tokStream), fixError parses up to error_token in the tokStream
      // recovers, if possible, from that error and returns the result.
      // While doing this, it also computes the location_stack information
      // and the sequence of actions that matches up with the result that
      // it returns.
      //
      fixError(start_token, error_token) {
        let curtok = start_token, current_kind = this.tokStream.getKind(curtok), first_stream_token = this.tokStream.peek();
        this.buffer[1] = error_token;
        this.buffer[0] = this.tokStream.getPrevious(this.buffer[1]);
        for (let k = 2; k < DiagnoseParser_1.BUFF_SIZE; k++) {
          this.buffer[k] = this.tokStream.getNext(this.buffer[k - 1]);
        }
        this.scope_repair.distance = 0;
        this.scope_repair.misspellIndex = 0;
        this.scope_repair.bufferPosition = 1;
        this.main_configuration_stack = new ConfigurationStack_1.ConfigurationStack(this.prs);
        this.locationStack[this.stateStackTop] = curtok;
        this.actionStack[this.stateStackTop] = this.action.size();
        let act = this.tAction(this.stateStack[this.stateStackTop], current_kind);
        for (; ; ) {
          if (this.monitor && this.monitor.isCancelled()) {
            return true;
          }
          if (act <= this.NUM_RULES) {
            this.action.add(act);
            this.stateStackTop--;
            do {
              this.stateStackTop -= this.rhs(act) - 1;
              act = this.ntAction(this.stateStack[this.stateStackTop], this.lhs(act));
            } while (act <= this.NUM_RULES);
            if (++this.stateStackTop >= this.stateStack.length) {
              this.reallocateStacks();
            }
            this.stateStack[this.stateStackTop] = act;
            this.locationStack[this.stateStackTop] = curtok;
            this.actionStack[this.stateStackTop] = this.action.size();
            act = this.tAction(act, current_kind);
            continue;
          } else if (act == this.ERROR_ACTION) {
            if (curtok !== error_token || this.main_configuration_stack.size() > 0) {
              let configuration = this.main_configuration_stack.pop();
              if (configuration == void 0) {
                act = this.ERROR_ACTION;
              } else {
                this.stateStackTop = configuration.stack_top;
                configuration.retrieveStack(this.stateStack);
                act = configuration.act;
                curtok = configuration.curtok;
                this.action.reset(configuration.action_length);
                current_kind = this.tokStream.getKind(curtok);
                this.tokStream.reset(this.tokStream.getNext(curtok));
                continue;
              }
            }
            break;
          } else if (act > this.ACCEPT_ACTION && act < this.ERROR_ACTION) {
            if (this.main_configuration_stack.findConfiguration(this.stateStack, this.stateStackTop, curtok)) {
              act = this.ERROR_ACTION;
            } else {
              this.main_configuration_stack.push(this.stateStack, this.stateStackTop, act + 1, curtok, this.action.size());
              act = this.baseAction(act);
            }
            continue;
          } else {
            if (act < this.ACCEPT_ACTION) {
              this.action.add(act);
              curtok = this.tokStream.getToken();
              current_kind = this.tokStream.getKind(curtok);
            } else if (act > this.ERROR_ACTION) {
              this.action.add(act);
              curtok = this.tokStream.getToken();
              current_kind = this.tokStream.getKind(curtok);
              act -= this.ERROR_ACTION;
              do {
                this.stateStackTop -= this.rhs(act) - 1;
                act = this.ntAction(this.stateStack[this.stateStackTop], this.lhs(act));
              } while (act <= this.NUM_RULES);
            } else {
              break;
            }
            if (++this.stateStackTop >= this.stateStack.length) {
              this.reallocateStacks();
            }
            this.stateStack[this.stateStackTop] = act;
            if (curtok == error_token) {
              this.scopeTrial(this.scope_repair, this.stateStack, this.stateStackTop);
              if (this.scope_repair.distance >= DiagnoseParser_1.MIN_DISTANCE) {
                this.tokens.add(start_token);
                for (let token = first_stream_token; token !== error_token; token = this.tokStream.getNext(token)) {
                  this.tokens.add(token);
                }
                this.acceptRecovery(error_token);
                break;
              }
            }
            this.locationStack[this.stateStackTop] = curtok;
            this.actionStack[this.stateStackTop] = this.action.size();
            act = this.tAction(act, current_kind);
          }
        }
        return act !== this.ERROR_ACTION;
      }
      acceptRecovery(error_token) {
        let recovery_action = new IntTuple_1.IntTuple();
        for (let k = 0; k <= this.scopeStackTop; k++) {
          let scope_index = this.scopeIndex[k], la = this.scopeLa(scope_index);
          recovery_action.reset();
          let act = this.tAction(this.stateStack[this.stateStackTop], la);
          if (act > this.ACCEPT_ACTION && act < this.ERROR_ACTION) {
            do {
              recovery_action.add(this.baseAction(act++));
            } while (this.baseAction(act) != 0);
          } else {
            recovery_action.add(act);
          }
          let start_action_size = this.action.size();
          let index;
          for (index = 0; index < recovery_action.size(); index++) {
            this.action.reset(start_action_size);
            this.tokStream.reset(error_token);
            this.tempStackTop = this.stateStackTop - 1;
            let max_pos = this.stateStackTop;
            act = recovery_action.get(index);
            while (act <= this.NUM_RULES) {
              this.action.add(act);
              do {
                let lhs_symbol = this.lhs(act);
                this.tempStackTop -= this.rhs(act) - 1;
                act = this.tempStackTop > max_pos ? this.tempStack[this.tempStackTop] : this.stateStack[this.tempStackTop];
                act = this.ntAction(act, lhs_symbol);
              } while (act <= this.NUM_RULES);
              if (this.tempStackTop + 1 >= this.stateStack.length) {
                this.reallocateStacks();
              }
              max_pos = max_pos < this.tempStackTop ? max_pos : this.tempStackTop;
              this.tempStack[this.tempStackTop + 1] = act;
              act = this.tAction(act, la);
            }
            if (act !== this.ERROR_ACTION) {
              this.nextStackTop = ++this.tempStackTop;
              for (let i = 0; i <= max_pos; i++) {
                this.nextStack[i] = this.stateStack[i];
              }
              for (let i = max_pos + 1; i <= this.tempStackTop; i++) {
                this.nextStack[i] = this.tempStack[i];
              }
              if (this.completeScope(this.action, this.scopeSuffix(scope_index))) {
                for (let i = this.scopeSuffix(this.scopeIndex[k]); this.scopeRhs(i) !== 0; i++) {
                  this.tokens.add(this.tokStream.makeErrorToken(error_token, this.tokStream.getPrevious(error_token), error_token, this.scopeRhs(i)));
                }
                this.reportError(this.scopeIndex[k], this.tokStream.getPrevious(error_token));
                break;
              }
            }
          }
          this.stateStackTop = this.nextStackTop;
          Utils_1.Lpg.Lang.System.arraycopy(this.nextStack, 0, this.stateStack, 0, this.stateStackTop + 1);
        }
        return;
      }
      completeScope(action, scope_rhs_index) {
        let kind = this.scopeRhs(scope_rhs_index);
        if (kind == 0) {
          return true;
        }
        let act = this.nextStack[this.nextStackTop];
        if (kind > this.NT_OFFSET) {
          let lhs_symbol = kind - this.NT_OFFSET;
          if (this.baseCheck(act + lhs_symbol) !== lhs_symbol) {
            return false;
          }
          act = this.ntAction(act, lhs_symbol);
          action.add(act <= this.NUM_RULES ? act + this.ERROR_ACTION : act);
          while (act <= this.NUM_RULES) {
            this.nextStackTop -= this.rhs(act) - 1;
            act = this.ntAction(this.nextStack[this.nextStackTop], this.lhs(act));
          }
          this.nextStackTop++;
          this.nextStack[this.nextStackTop] = act;
          return this.completeScope(action, scope_rhs_index + 1);
        }
        act = this.tAction(act, kind);
        action.add(act);
        if (act < this.ACCEPT_ACTION) {
          this.nextStackTop++;
          this.nextStack[this.nextStackTop] = act;
          return this.completeScope(action, scope_rhs_index + 1);
        } else if (act > this.ERROR_ACTION) {
          act -= this.ERROR_ACTION;
          do {
            this.nextStackTop -= this.rhs(act) - 1;
            act = this.ntAction(this.nextStack[this.nextStackTop], this.lhs(act));
          } while (act <= this.NUM_RULES);
          this.nextStackTop++;
          this.nextStack[this.nextStackTop] = act;
          return true;
        } else if (act > this.ACCEPT_ACTION && act < this.ERROR_ACTION) {
          let save_action_size = action.size();
          for (let i = act; this.baseAction(i) !== 0; i++) {
            action.reset(save_action_size);
            act = this.baseAction(i);
            action.add(act);
            if (act <= this.NUM_RULES) {
            } else if (act < this.ACCEPT_ACTION) {
              this.nextStackTop++;
              this.nextStack[this.nextStackTop] = act;
              if (this.completeScope(action, scope_rhs_index + 1)) {
                return true;
              }
            } else if (act > this.ERROR_ACTION) {
              act -= this.ERROR_ACTION;
              do {
                this.nextStackTop -= this.rhs(act) - 1;
                act = this.ntAction(this.nextStack[this.nextStackTop], this.lhs(act));
              } while (act <= this.NUM_RULES);
              this.nextStackTop++;
              this.nextStack[this.nextStackTop] = act;
              return true;
            }
          }
        }
        return false;
      }
    };
    exports.RecoveryParser = RecoveryParser;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/Protocol.js
var require_Protocol = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/Protocol.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.instanceOfIPrsStream = exports.instanceOfILexStream = exports.EscapeStrictPropertyInitializationLexStream = exports.IToken = void 0;
    var IToken2;
    (function(IToken3) {
      IToken3.EOF = 65535;
    })(IToken2 = exports.IToken || (exports.IToken = {}));
    var EscapeStrictPropertyInitializationLexStream = class {
      constructor() {
        this.m77Ac341Feebeb7C0A7Ff8F9C6540531500693Bac = 0;
      }
      getIPrsStream() {
        throw new Error("Method not implemented.");
      }
      getPrsStream() {
        throw new Error("Method not implemented.");
      }
      setPrsStream(stream) {
        throw new Error("Method not implemented.");
      }
      getLineCount() {
        throw new Error("Method not implemented.");
      }
      orderedExportedSymbols() {
        throw new Error("Method not implemented.");
      }
      getLineOffset(i) {
        throw new Error("Method not implemented.");
      }
      getLineNumberOfCharAt(i) {
        throw new Error("Method not implemented.");
      }
      getColumnOfCharAt(i) {
        throw new Error("Method not implemented.");
      }
      getCharValue(i) {
        throw new Error("Method not implemented.");
      }
      getIntValue(i) {
        throw new Error("Method not implemented.");
      }
      makeToken(startLoc, endLoc, kind) {
        throw new Error("Method not implemented.");
      }
      setMessageHandler(errMsg) {
        throw new Error("Method not implemented.");
      }
      getMessageHandler() {
        throw new Error("Method not implemented.");
      }
      getLocation(left_loc, right_loc) {
        throw new Error("Method not implemented.");
      }
      reportLexicalError(left_loc, right_loc, errorCode, error_left_loc, error_right_loc, errorInfo) {
        throw new Error("Method not implemented.");
      }
      toString(startOffset, endOffset) {
        throw new Error("Method not implemented.");
      }
      getToken(end_token) {
        throw new Error("Method not implemented.");
      }
      getKind(i) {
        throw new Error("Method not implemented.");
      }
      getNext(i) {
        throw new Error("Method not implemented.");
      }
      getPrevious(i) {
        throw new Error("Method not implemented.");
      }
      getName(i) {
        throw new Error("Method not implemented.");
      }
      peek() {
        throw new Error("Method not implemented.");
      }
      reset(i) {
        throw new Error("Method not implemented.");
      }
      badToken() {
        throw new Error("Method not implemented.");
      }
      getLine(i) {
        throw new Error("Method not implemented.");
      }
      getColumn(i) {
        throw new Error("Method not implemented.");
      }
      getEndLine(i) {
        throw new Error("Method not implemented.");
      }
      getEndColumn(i) {
        throw new Error("Method not implemented.");
      }
      afterEol(i) {
        throw new Error("Method not implemented.");
      }
      getFileName() {
        throw new Error("Method not implemented.");
      }
      getStreamLength() {
        throw new Error("Method not implemented.");
      }
      getFirstRealToken(i) {
        throw new Error("Method not implemented.");
      }
      getLastRealToken(i) {
        throw new Error("Method not implemented.");
      }
      reportError(errorCode, leftToken, rightToken, errorInfo, errorToken) {
        throw new Error("Method not implemented.");
      }
    };
    exports.EscapeStrictPropertyInitializationLexStream = EscapeStrictPropertyInitializationLexStream;
    function instanceOfILexStream(object) {
      return "m77Ac341Feebeb7C0A7Ff8F9C6540531500693Bac" in object;
    }
    exports.instanceOfILexStream = instanceOfILexStream;
    function instanceOfIPrsStream(object) {
      return "m3C89586D99F2567D21410F29B1B2606574892Aa7" in object;
    }
    exports.instanceOfIPrsStream = instanceOfIPrsStream;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/TokenStreamNotIPrsStreamException.js
var require_TokenStreamNotIPrsStreamException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/TokenStreamNotIPrsStreamException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.TokenStreamNotIPrsStreamException = void 0;
    var TokenStreamNotIPrsStreamException = class extends Error {
      constructor(str) {
        super();
        if (!str) {
          this.str = "TokenStreamNotIPrsStreamException";
        } else {
          this.str = str;
        }
      }
      toString() {
        return this.str;
      }
    };
    exports.TokenStreamNotIPrsStreamException = TokenStreamNotIPrsStreamException;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/BadParseSymFileException.js
var require_BadParseSymFileException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/BadParseSymFileException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.BadParseSymFileException = void 0;
    var BadParseSymFileException2 = class extends Error {
      constructor(str) {
        super();
        if (!str) {
          this.str = "BadParseSymFileException";
        } else {
          this.str = str;
        }
      }
      toString() {
        return this.str;
      }
    };
    exports.BadParseSymFileException = BadParseSymFileException2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/NotBacktrackParseTableException.js
var require_NotBacktrackParseTableException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/NotBacktrackParseTableException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.NotBacktrackParseTableException = void 0;
    var NotBacktrackParseTableException = class extends Error {
      constructor(str) {
        super();
        if (!str) {
          this.str = "NotBacktrackParseTableException";
        } else {
          this.str = str;
        }
      }
      toString() {
        return this.str;
      }
    };
    exports.NotBacktrackParseTableException = NotBacktrackParseTableException;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/BacktrackingParser.js
var require_BacktrackingParser = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/BacktrackingParser.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.BacktrackingParser = void 0;
    var Stacks_1 = require_Stacks();
    var TokenStream_1 = require_TokenStream();
    var ParseTable_1 = require_ParseTable();
    var RuleAction_1 = require_RuleAction();
    var IntSegmentedTuple_1 = require_IntSegmentedTuple();
    var IntTuple_1 = require_IntTuple();
    var RecoveryParser_1 = require_RecoveryParser();
    var ConfigurationStack_1 = require_ConfigurationStack();
    var Protocol_1 = require_Protocol();
    var BadParseException_1 = require_BadParseException();
    var Utils_1 = require_Utils();
    var TokenStreamNotIPrsStreamException_1 = require_TokenStreamNotIPrsStreamException();
    var BadParseSymFileException_1 = require_BadParseSymFileException();
    var NotBacktrackParseTableException_1 = require_NotBacktrackParseTableException();
    var BacktrackingParser2 = class extends Stacks_1.Stacks {
      getMarkerToken(marker_kind, start_token_index) {
        if (marker_kind == 0) {
          return 0;
        } else {
          if (this.markerTokenIndex == 0) {
            if (!(0, Protocol_1.instanceOfIPrsStream)(this.tokStream)) {
              throw new TokenStreamNotIPrsStreamException_1.TokenStreamNotIPrsStreamException();
            }
            this.markerTokenIndex = this.tokStream.makeErrorToken(this.tokStream.getPrevious(start_token_index), this.tokStream.getPrevious(start_token_index), this.tokStream.getPrevious(start_token_index), marker_kind);
          } else {
            this.tokStream.getIToken(this.markerTokenIndex).setKind(marker_kind);
          }
        }
        return this.markerTokenIndex;
      }
      //
      // Override the getToken function in Stacks.
      //
      getToken(i) {
        return this.tokens.get(this.locationStack[this.stateStackTop + (i - 1)]);
      }
      getCurrentRule() {
        return this.currentAction;
      }
      getFirstToken2() {
        return this.tokStream.getFirstRealToken(this.getToken(1));
      }
      getFirstToken(i) {
        if (!i) {
          return this.getFirstToken2();
        }
        return this.tokStream.getFirstRealToken(this.getToken(i));
      }
      getLastToken2() {
        return this.tokStream.getLastRealToken(this.lastToken);
      }
      getLastToken(i) {
        if (!i) {
          return this.getLastToken2();
        }
        let l = i >= this.prs.rhs(this.currentAction) ? this.lastToken : this.tokens.get(this.locationStack[this.stateStackTop + i] - 1);
        return this.tokStream.getLastRealToken(l);
      }
      setMonitor(monitor) {
        this.monitor = monitor;
      }
      reset1() {
        this.action.reset();
        this.skipTokens = false;
        this.markerTokenIndex = 0;
      }
      reset2(tokStream, monitor) {
        this.monitor = monitor;
        this.tokStream = tokStream;
        this.reset1();
      }
      reset(tokStream, prs, ra, monitor) {
        if (prs) {
          this.prs = prs;
          this.START_STATE = prs.getStartState();
          this.NUM_RULES = prs.getNumRules();
          this.NT_OFFSET = prs.getNtOffset();
          this.LA_STATE_OFFSET = prs.getLaStateOffset();
          this.EOFT_SYMBOL = prs.getEoftSymbol();
          this.ERROR_SYMBOL = prs.getErrorSymbol();
          this.ACCEPT_ACTION = prs.getAcceptAction();
          this.ERROR_ACTION = prs.getErrorAction();
          if (!prs.isValidForParser())
            throw new BadParseSymFileException_1.BadParseSymFileException();
          if (!prs.getBacktrack())
            throw new NotBacktrackParseTableException_1.NotBacktrackParseTableException();
        }
        if (ra)
          this.ra = ra;
        if (!tokStream) {
          this.reset1();
          return;
        }
        this.reset2(tokStream, monitor);
      }
      reset3(tokStream, prs, ra) {
        this.reset(tokStream, prs, ra);
      }
      constructor(tokStream, prs, ra, monitor) {
        super();
        this.START_STATE = 0;
        this.NUM_RULES = 0;
        this.NT_OFFSET = 0;
        this.LA_STATE_OFFSET = 0;
        this.EOFT_SYMBOL = 0;
        this.ERROR_SYMBOL = 0;
        this.ACCEPT_ACTION = 0;
        this.ERROR_ACTION = 0;
        this.lastToken = 0;
        this.currentAction = 0;
        this.tokStream = new TokenStream_1.EscapeStrictPropertyInitializationTokenStream();
        this.prs = new ParseTable_1.EscapeStrictPropertyInitializationParseTable();
        this.ra = new RuleAction_1.EscapeStrictPropertyInitializationRuleAction();
        this.action = new IntSegmentedTuple_1.IntSegmentedTuple(10, 1024);
        this.tokens = new IntTuple_1.IntTuple(0);
        this.actionStack = new Int32Array(0);
        this.skipTokens = false;
        this.markerTokenIndex = 0;
        this.reset(tokStream, prs, ra, monitor);
      }
      //
      // Allocate or reallocate all the stacks. Their sizes should always be the same.
      //
      reallocateOtherStacks(start_token_index) {
        if (!this.actionStack || this.actionStack.length == 0) {
          this.actionStack = new Int32Array(this.stateStack.length);
          this.locationStack = new Int32Array(this.stateStack.length);
          this.parseStack = new Array(this.stateStack.length);
          this.actionStack[0] = 0;
          this.locationStack[0] = start_token_index;
        } else {
          if (this.actionStack.length < this.stateStack.length) {
            let old_length = this.actionStack.length;
            Utils_1.Lpg.Lang.System.arraycopy(this.actionStack, 0, this.actionStack = new Int32Array(this.stateStack.length), 0, old_length);
            Utils_1.Lpg.Lang.System.arraycopy(this.locationStack, 0, this.locationStack = new Int32Array(this.stateStack.length), 0, old_length);
            Utils_1.Lpg.Lang.System.arraycopy(this.parseStack, 0, this.parseStack = new Array(this.stateStack.length), 0, old_length);
          }
        }
        return;
      }
      //public fuzzyParse(): any {
      //    return this.fuzzyParseEntry(0, lpg.lang.Integer.MAX_VALUE);
      //}
      fuzzyParse(max_error_count) {
        if (!max_error_count) {
          max_error_count = Number.MAX_VALUE;
        }
        return this.fuzzyParseEntry(0, max_error_count);
      }
      //public fuzzyParseEntry(marker_kind: number): any {
      //    return this.fuzzyParseEntry(marker_kind, lpg.lang.Integer.MAX_VALUE);
      //}
      fuzzyParseEntry(marker_kind, max_error_count) {
        if (!max_error_count) {
          max_error_count = Number.MAX_VALUE;
        }
        this.action.reset();
        this.tokStream.reset();
        this.reallocateStateStack();
        this.stateStackTop = 0;
        this.stateStack[0] = this.START_STATE;
        let first_token = this.tokStream.peek(), start_token = first_token, marker_token = this.getMarkerToken(marker_kind, first_token);
        this.tokens = new IntTuple_1.IntTuple(this.tokStream.getStreamLength());
        this.tokens.add(this.tokStream.getPrevious(first_token));
        let error_token = this.backtrackParseInternal(this.action, marker_token);
        if (error_token !== 0) {
          if (!(0, Protocol_1.instanceOfIPrsStream)(this.tokStream)) {
            throw new TokenStreamNotIPrsStreamException_1.TokenStreamNotIPrsStreamException();
          }
          let rp = new RecoveryParser_1.RecoveryParser(this, this.action, this.tokens, this.tokStream, this.prs, max_error_count, 0, this.monitor);
          start_token = rp.recover(marker_token, error_token);
        }
        if (marker_token !== 0 && start_token == first_token) {
          this.tokens.add(marker_token);
        }
        let t;
        for (t = start_token; this.tokStream.getKind(t) != this.EOFT_SYMBOL; t = this.tokStream.getNext(t)) {
          this.tokens.add(t);
        }
        this.tokens.add(t);
        return this.parseActions(marker_kind);
      }
      parse(max_error_count = 0) {
        return this.parseEntry(0, max_error_count);
      }
      //
      // Parse input allowing up to max_error_count Error token recoveries.
      // When max_error_count is 0, no Error token recoveries occur.
      // When max_error is > 0, it limits the number of Error token recoveries.
      // When max_error is < 0, the number of error token recoveries is unlimited.
      // Also, such recoveries only require one token to be parsed beyond the recovery point.
      // (normally two tokens beyond the recovery point must be parsed)
      // Thus, a negative max_error_count should be used when error productions are used to 
      // skip tokens.
      //
      parseEntry(marker_kind = 0, max_error_count = 0) {
        this.action.reset();
        this.tokStream.reset();
        this.reallocateStateStack();
        this.stateStackTop = 0;
        this.stateStack[0] = this.START_STATE;
        this.skipTokens = max_error_count < 0;
        if (max_error_count > 0 && (0, Protocol_1.instanceOfIPrsStream)(this.tokStream)) {
          max_error_count = 0;
        }
        this.tokens = new IntTuple_1.IntTuple(this.tokStream.getStreamLength());
        this.tokens.add(this.tokStream.getPrevious(this.tokStream.peek()));
        let start_token_index = this.tokStream.peek(), repair_token = this.getMarkerToken(marker_kind, start_token_index), start_action_index = this.action.size(), temp_stack = new Int32Array(this.stateStackTop + 1);
        Utils_1.Lpg.Lang.System.arraycopy(this.stateStack, 0, temp_stack, 0, temp_stack.length);
        let initial_error_token = this.backtrackParseInternal(this.action, repair_token);
        for (let error_token = initial_error_token, count = 0; error_token !== 0; error_token = this.backtrackParseInternal(this.action, repair_token), count++) {
          if (count == max_error_count) {
            throw new BadParseException_1.BadParseException(initial_error_token);
          }
          this.action.reset(start_action_index);
          this.tokStream.reset(start_token_index);
          this.stateStackTop = temp_stack.length - 1;
          Utils_1.Lpg.Lang.System.arraycopy(temp_stack, 0, this.stateStack, 0, temp_stack.length);
          this.reallocateOtherStacks(start_token_index);
          this.backtrackParseUpToError(repair_token, error_token);
          for (this.stateStackTop = this.findRecoveryStateIndex(this.stateStackTop); this.stateStackTop >= 0; this.stateStackTop = this.findRecoveryStateIndex(this.stateStackTop - 1)) {
            let recovery_token = this.tokens.get(this.locationStack[this.stateStackTop] - 1);
            repair_token = this.errorRepair(this.tokStream, recovery_token >= start_token_index ? recovery_token : error_token, error_token);
            if (repair_token !== 0) {
              break;
            }
          }
          if (this.stateStackTop < 0) {
            throw new BadParseException_1.BadParseException(initial_error_token);
          }
          temp_stack = new Int32Array(this.stateStackTop + 1);
          Utils_1.Lpg.Lang.System.arraycopy(this.stateStack, 0, temp_stack, 0, temp_stack.length);
          start_action_index = this.action.size();
          start_token_index = this.tokStream.peek();
        }
        if (repair_token !== 0) {
          this.tokens.add(repair_token);
        }
        let t;
        for (t = start_token_index; this.tokStream.getKind(t) !== this.EOFT_SYMBOL; t = this.tokStream.getNext(t)) {
          this.tokens.add(t);
        }
        this.tokens.add(t);
        return this.parseActions(marker_kind);
      }
      //
      // Process reductions and continue...
      //
      process_reductions() {
        do {
          this.stateStackTop -= this.prs.rhs(this.currentAction) - 1;
          this.ra.ruleAction(this.currentAction);
          this.currentAction = this.prs.ntAction(this.stateStack[this.stateStackTop], this.prs.lhs(this.currentAction));
        } while (this.currentAction <= this.NUM_RULES);
        return;
      }
      //
      // Now do the final parse of the input based on the actions in
      // the list "action" and the sequence of tokens in list "tokens".
      //
      parseActions(marker_kind) {
        let ti = -1, curtok;
        this.lastToken = this.tokens.get(++ti);
        curtok = this.tokens.get(++ti);
        this.allocateOtherStacks();
        this.stateStackTop = -1;
        this.currentAction = this.START_STATE;
        for (let i = 0; i < this.action.size(); i++) {
          if (this.monitor && this.monitor.isCancelled()) {
            return null;
          }
          this.stateStack[++this.stateStackTop] = this.currentAction;
          this.locationStack[this.stateStackTop] = ti;
          this.currentAction = this.action.get(i);
          if (this.currentAction <= this.NUM_RULES) {
            this.stateStackTop--;
            this.process_reductions();
          } else {
            if (this.tokStream.getKind(curtok) > this.NT_OFFSET) {
              let synthesized = null;
              let factories = this.ra.getProstheticAst ? this.ra.getProstheticAst() : null;
              if (factories && this.prs.getProsthesisIndex) {
                let slot = this.prs.getProsthesisIndex(this.tokStream.getKind(curtok));
                if (slot >= 0 && slot < factories.length) {
                  let factory = factories[slot];
                  if (factory) {
                    synthesized = factory(this.tokStream.getIToken(curtok));
                  }
                }
              }
              if (synthesized === null) {
                let badtok = this.tokStream.getIToken(curtok);
                throw new BadParseException_1.BadParseException(badtok.getErrorToken().getTokenIndex());
              }
              this.parseStack[this.stateStackTop] = synthesized;
            }
            this.lastToken = curtok;
            curtok = this.tokens.get(++ti);
            if (this.currentAction > this.ERROR_ACTION) {
              this.currentAction -= this.ERROR_ACTION;
              this.process_reductions();
            }
          }
        }
        return this.parseStack[marker_kind == 0 ? 0 : 1];
      }
      //
      // Process reductions and continue...
      //
      process_backtrack_reductions(act) {
        do {
          this.stateStackTop -= this.prs.rhs(act) - 1;
          act = this.prs.ntAction(this.stateStack[this.stateStackTop], this.prs.lhs(act));
        } while (act <= this.NUM_RULES);
        return act;
      }
      //
      // This method is intended to be used by the type RecoveryParser.
      // Note that the action tuple passed here must be the same action
      // tuple that was passed down to RecoveryParser. It is passed back
      // to this method as documention.
      //
      backtrackParse(stack, stack_top, action, initial_token) {
        this.stateStackTop = stack_top;
        Utils_1.Lpg.Lang.System.arraycopy(stack, 0, this.stateStack, 0, this.stateStackTop + 1);
        return this.backtrackParseInternal(action, initial_token);
      }
      //
      // Parse the input until either the parse completes successfully or
      // an error is encountered. This function returns an integer that
      // represents the last action that was executed by the parser. If
      // the parse was succesful, then the tuple "action" contains the
      // successful sequence of actions that was executed.
      //
      backtrackParseInternal(action, initial_token) {
        let configuration_stack = new ConfigurationStack_1.ConfigurationStack(this.prs);
        let error_token = 0, maxStackTop = this.stateStackTop, start_token = this.tokStream.peek(), curtok = initial_token > 0 ? initial_token : this.tokStream.getToken(), current_kind = this.tokStream.getKind(curtok), act = this.tAction(this.stateStack[this.stateStackTop], current_kind);
        for (; ; ) {
          if (this.monitor && this.monitor.isCancelled()) {
            return 0;
          }
          if (act <= this.NUM_RULES) {
            action.add(act);
            this.stateStackTop--;
            act = this.process_backtrack_reductions(act);
          } else if (act > this.ERROR_ACTION) {
            action.add(act);
            curtok = this.tokStream.getToken();
            current_kind = this.tokStream.getKind(curtok);
            act = this.process_backtrack_reductions(act - this.ERROR_ACTION);
          } else if (act < this.ACCEPT_ACTION) {
            action.add(act);
            curtok = this.tokStream.getToken();
            current_kind = this.tokStream.getKind(curtok);
          } else if (act == this.ERROR_ACTION) {
            error_token = error_token > curtok ? error_token : curtok;
            let configuration = configuration_stack.pop();
            if (configuration == void 0) {
              act = this.ERROR_ACTION;
            } else {
              action.reset(configuration.action_length);
              act = configuration.act;
              curtok = configuration.curtok;
              current_kind = this.tokStream.getKind(curtok);
              this.tokStream.reset(curtok == initial_token ? start_token : this.tokStream.getNext(curtok));
              this.stateStackTop = configuration.stack_top;
              configuration.retrieveStack(this.stateStack);
              continue;
            }
            break;
          } else if (act > this.ACCEPT_ACTION) {
            if (configuration_stack.findConfiguration(this.stateStack, this.stateStackTop, curtok)) {
              act = this.ERROR_ACTION;
            } else {
              configuration_stack.push(this.stateStack, this.stateStackTop, act + 1, curtok, action.size());
              act = this.prs.baseAction(act);
              maxStackTop = this.stateStackTop > maxStackTop ? this.stateStackTop : maxStackTop;
            }
            continue;
          } else {
            break;
          }
          if (++this.stateStackTop >= this.stateStack.length) {
            this.reallocateStateStack();
          }
          this.stateStack[this.stateStackTop] = act;
          act = this.tAction(act, current_kind);
        }
        return act == this.ERROR_ACTION ? error_token : 0;
      }
      backtrackParseUpToError(initial_token, error_token) {
        let configuration_stack = new ConfigurationStack_1.ConfigurationStack(this.prs);
        let start_token = this.tokStream.peek(), curtok = initial_token > 0 ? initial_token : this.tokStream.getToken(), current_kind = this.tokStream.getKind(curtok), act = this.tAction(this.stateStack[this.stateStackTop], current_kind);
        this.tokens.add(curtok);
        this.locationStack[this.stateStackTop] = this.tokens.size();
        this.actionStack[this.stateStackTop] = this.action.size();
        for (; ; ) {
          if (this.monitor && this.monitor.isCancelled()) {
            return;
          }
          if (act <= this.NUM_RULES) {
            this.action.add(act);
            this.stateStackTop--;
            act = this.process_backtrack_reductions(act);
          } else if (act > this.ERROR_ACTION) {
            this.action.add(act);
            curtok = this.tokStream.getToken();
            current_kind = this.tokStream.getKind(curtok);
            this.tokens.add(curtok);
            act = this.process_backtrack_reductions(act - this.ERROR_ACTION);
          } else if (act < this.ACCEPT_ACTION) {
            this.action.add(act);
            curtok = this.tokStream.getToken();
            current_kind = this.tokStream.getKind(curtok);
            this.tokens.add(curtok);
          } else if (act == this.ERROR_ACTION) {
            if (curtok !== error_token) {
              let configuration = configuration_stack.pop();
              if (configuration == void 0) {
                act = this.ERROR_ACTION;
              } else {
                this.action.reset(configuration.action_length);
                act = configuration.act;
                let next_token_index = configuration.curtok;
                this.tokens.reset(next_token_index);
                curtok = this.tokens.get(next_token_index - 1);
                current_kind = this.tokStream.getKind(curtok);
                this.tokStream.reset(curtok == initial_token ? start_token : this.tokStream.getNext(curtok));
                this.stateStackTop = configuration.stack_top;
                configuration.retrieveStack(this.stateStack);
                this.locationStack[this.stateStackTop] = this.tokens.size();
                this.actionStack[this.stateStackTop] = this.action.size();
                continue;
              }
            }
            break;
          } else if (act > this.ACCEPT_ACTION) {
            if (configuration_stack.findConfiguration(this.stateStack, this.stateStackTop, this.tokens.size())) {
              act = this.ERROR_ACTION;
            } else {
              configuration_stack.push(this.stateStack, this.stateStackTop, act + 1, this.tokens.size(), this.action.size());
              act = this.prs.baseAction(act);
            }
            continue;
          } else {
            break;
          }
          this.stateStack[++this.stateStackTop] = act;
          this.locationStack[this.stateStackTop] = this.tokens.size();
          this.actionStack[this.stateStackTop] = this.action.size();
          act = this.tAction(act, current_kind);
        }
        return;
      }
      repairable(error_token) {
        let configuration_stack = new ConfigurationStack_1.ConfigurationStack(this.prs);
        let start_token = this.tokStream.peek(), final_token = this.tokStream.getStreamLength(), curtok = 0, current_kind = this.ERROR_SYMBOL, act = this.tAction(this.stateStack[this.stateStackTop], current_kind);
        for (; ; ) {
          if (act <= this.NUM_RULES) {
            this.stateStackTop--;
            act = this.process_backtrack_reductions(act);
          } else if (act > this.ERROR_ACTION) {
            curtok = this.tokStream.getToken();
            if (curtok > final_token) {
              return true;
            }
            current_kind = this.tokStream.getKind(curtok);
            act = this.process_backtrack_reductions(act - this.ERROR_ACTION);
          } else if (act < this.ACCEPT_ACTION) {
            curtok = this.tokStream.getToken();
            if (curtok > final_token) {
              return true;
            }
            current_kind = this.tokStream.getKind(curtok);
          } else if (act == this.ERROR_ACTION) {
            let configuration = configuration_stack.pop();
            if (configuration == void 0) {
              act = this.ERROR_ACTION;
            } else {
              this.stateStackTop = configuration.stack_top;
              configuration.retrieveStack(this.stateStack);
              act = configuration.act;
              curtok = configuration.curtok;
              if (curtok == 0) {
                current_kind = this.ERROR_SYMBOL;
                this.tokStream.reset(start_token);
              } else {
                current_kind = this.tokStream.getKind(curtok);
                this.tokStream.reset(this.tokStream.getNext(curtok));
              }
              continue;
            }
            break;
          } else if (act > this.ACCEPT_ACTION) {
            if (configuration_stack.findConfiguration(this.stateStack, this.stateStackTop, curtok)) {
              act = this.ERROR_ACTION;
            } else {
              configuration_stack.push(this.stateStack, this.stateStackTop, act + 1, curtok, 0);
              act = this.prs.baseAction(act);
            }
            continue;
          } else {
            break;
          }
          if (curtok > error_token && final_token == this.tokStream.getStreamLength()) {
            if (this.recoverableState(act)) {
              final_token = this.skipTokens ? curtok : this.tokStream.getNext(curtok);
            }
          }
          if (++this.stateStackTop >= this.stateStack.length) {
            this.reallocateStateStack();
          }
          this.stateStack[this.stateStackTop] = act;
          act = this.tAction(act, current_kind);
        }
        return act == this.ACCEPT_ACTION;
      }
      recoverableState(state) {
        for (let k = this.prs.asi(state); this.prs.asr(k) !== 0; k++) {
          if (this.prs.asr(k) == this.ERROR_SYMBOL) {
            return true;
          }
        }
        return false;
      }
      findRecoveryStateIndex(start_index) {
        let i;
        for (i = start_index; i >= 0; i--) {
          if (this.recoverableState(this.stateStack[i])) {
            break;
          }
        }
        if (i >= 0) {
          let k;
          for (k = i - 1; k >= 0; k--) {
            if (this.locationStack[k] != this.locationStack[i]) {
              break;
            }
          }
          i = k + 1;
        }
        return i;
      }
      errorRepair(stream, recovery_token, error_token) {
        let temp_stack = new Int32Array(this.stateStackTop + 1);
        Utils_1.Lpg.Lang.System.arraycopy(this.stateStack, 0, temp_stack, 0, temp_stack.length);
        for (; stream.getKind(recovery_token) !== this.EOFT_SYMBOL; recovery_token = stream.getNext(recovery_token)) {
          stream.reset(recovery_token);
          if (this.repairable(error_token)) {
            break;
          }
          this.stateStackTop = temp_stack.length - 1;
          Utils_1.Lpg.Lang.System.arraycopy(temp_stack, 0, this.stateStack, 0, temp_stack.length);
        }
        if (stream.getKind(recovery_token) == this.EOFT_SYMBOL) {
          stream.reset(recovery_token);
          if (!this.repairable(error_token)) {
            this.stateStackTop = temp_stack.length - 1;
            Utils_1.Lpg.Lang.System.arraycopy(temp_stack, 0, this.stateStack, 0, temp_stack.length);
            return 0;
          }
        }
        this.stateStackTop = temp_stack.length - 1;
        Utils_1.Lpg.Lang.System.arraycopy(temp_stack, 0, this.stateStack, 0, temp_stack.length);
        stream.reset(recovery_token);
        this.tokens.reset(this.locationStack[this.stateStackTop] - 1);
        this.action.reset(this.actionStack[this.stateStackTop]);
        return stream.makeErrorToken(this.tokens.get(this.locationStack[this.stateStackTop] - 1), stream.getPrevious(recovery_token), error_token, this.ERROR_SYMBOL);
      }
      //
      // keep looking ahead until we compute a valid action
      //
      lookahead(act, token) {
        act = this.prs.lookAhead(act - this.LA_STATE_OFFSET, this.tokStream.getKind(token));
        return act > this.LA_STATE_OFFSET ? this.lookahead(act, this.tokStream.getNext(token)) : act;
      }
      //
      // Compute the next action defined on act and sym. If this
      // action requires more lookahead, these lookahead symbols
      // are in the token stream beginning at the next token that
      // is yielded by peek().
      //
      tAction(act, sym) {
        act = this.prs.tAction(act, sym);
        return act > this.LA_STATE_OFFSET ? this.lookahead(act, this.tokStream.peek()) : act;
      }
    };
    exports.BacktrackingParser = BacktrackingParser2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/NotGLRParseTableException.js
var require_NotGLRParseTableException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/NotGLRParseTableException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.NotGLRParseTableException = void 0;
    var NotGLRParseTableException2 = class extends Error {
      constructor(str) {
        super();
        if (!str) {
          this.str = "NotGLRParseTableException";
        } else {
          this.str = str;
        }
      }
      toString() {
        return this.str;
      }
    };
    exports.NotGLRParseTableException = NotGLRParseTableException2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/UnavailableParserInformationException.js
var require_UnavailableParserInformationException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/UnavailableParserInformationException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.UnavailableParserInformationException = void 0;
    var UnavailableParserInformationException = class extends Error {
      constructor(str) {
        super();
        if (!str) {
          this.str = "Unavailable parser Information Exception";
        } else {
          this.str = str;
        }
      }
      toString() {
        return this.str;
      }
    };
    exports.UnavailableParserInformationException = UnavailableParserInformationException;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/GssNode.js
var require_GssNode = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/GssNode.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.GssNode = void 0;
    var GssNode = class {
      constructor(state, index) {
        this.edges = [];
        this.state = state;
        this.index = index;
      }
      getState() {
        return this.state;
      }
      getIndex() {
        return this.index;
      }
      getEdges() {
        return this.edges;
      }
    };
    exports.GssNode = GssNode;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/GssEdge.js
var require_GssEdge = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/GssEdge.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.GssEdge = void 0;
    var GssEdge = class {
      constructor(predecessor, symbol, location, semantic, sppf) {
        this.predecessor = predecessor;
        this.symbol = symbol;
        this.location = location;
        this.semantic = semantic;
        this.sppf = sppf;
      }
      getPredecessor() {
        return this.predecessor;
      }
      getSymbol() {
        return this.symbol;
      }
      getLocation() {
        return this.location;
      }
      getSemantic() {
        return this.semantic;
      }
      getSppf() {
        return this.sppf;
      }
    };
    exports.GssEdge = GssEdge;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/SppfNode.js
var require_SppfNode = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/SppfNode.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.SppfNode = void 0;
    var SppfNode = class {
      constructor(grammarSymbol, leftExtent, rightExtent) {
        this.packs = [];
        this.grammarSymbol = grammarSymbol;
        this.leftExtent = leftExtent;
        this.rightExtent = rightExtent;
      }
      getGrammarSymbol() {
        return this.grammarSymbol;
      }
      getLeftExtent() {
        return this.leftExtent;
      }
      getRightExtent() {
        return this.rightExtent;
      }
      getPacks() {
        return this.packs;
      }
      getAstForest() {
        return this.astForest;
      }
    };
    exports.SppfNode = SppfNode;
    SppfNode.Packed = class Packed {
      constructor(rule, children, semantic) {
        this.rule = rule;
        this.children = children == null ? [] : children;
        this.semantic = semantic;
      }
      getRule() {
        return this.rule;
      }
      getChildren() {
        let out = [];
        for (let c of this.children) {
          if (c != null)
            out.push(c);
        }
        return out;
      }
      getSemantic() {
        return this.semantic;
      }
    };
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/GLRParser.js
var require_GLRParser = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/GLRParser.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.GLRParser = void 0;
    var Stacks_1 = require_Stacks();
    var TokenStream_1 = require_TokenStream();
    var ParseTable_1 = require_ParseTable();
    var RuleAction_1 = require_RuleAction();
    var BadParseException_1 = require_BadParseException();
    var BadParseSymFileException_1 = require_BadParseSymFileException();
    var NotGLRParseTableException_1 = require_NotGLRParseTableException();
    var NotBacktrackParseTableException_1 = require_NotBacktrackParseTableException();
    var UnavailableParserInformationException_1 = require_UnavailableParserInformationException();
    var BacktrackingParser_1 = require_BacktrackingParser();
    var GssNode_1 = require_GssNode();
    var GssEdge_1 = require_GssEdge();
    var SppfNode_1 = require_SppfNode();
    var GLRParser3 = class _GLRParser extends Stacks_1.Stacks {
      lookahead(act, token) {
        act = this.prs.lookAhead(act - this.LA_STATE_OFFSET, this.tokStream.getKind(token));
        return act > this.LA_STATE_OFFSET ? this.lookahead(act, this.tokStream.getNext(token)) : act;
      }
      /** Act on sym in state, with lookahead past curtok. */
      tAction(state, sym, curtok) {
        let act = this.prs.tAction(state, sym);
        return act > this.LA_STATE_OFFSET ? this.lookahead(act, this.tokStream.getNext(curtok)) : act;
      }
      expandConflict(act, out) {
        for (let i = act; ; i++) {
          let cand = this.prs.baseAction(i);
          if (cand === 0)
            break;
          out.push(cand);
        }
      }
      getCurrentRule() {
        if (this.taking_actions)
          return this.currentAction;
        throw new UnavailableParserInformationException_1.UnavailableParserInformationException();
      }
      getToken(i) {
        if (this.taking_actions)
          return this.frameLocation[this.frameTop + (i - 1)];
        return super.getToken(i);
      }
      getSym(i) {
        if (this.taking_actions)
          return this.frameParse[this.frameTop + (i - 1)];
        return super.getSym(i);
      }
      setSym1(ast) {
        if (this.taking_actions)
          this.frameParse[this.frameTop] = ast;
        else
          super.setSym1(ast);
      }
      getFirstToken(i) {
        if (!this.taking_actions)
          throw new UnavailableParserInformationException_1.UnavailableParserInformationException();
        if (i === void 0)
          return this.getToken(1);
        return this.getToken(i);
      }
      getLastToken(i) {
        if (!this.taking_actions)
          throw new UnavailableParserInformationException_1.UnavailableParserInformationException();
        if (i === void 0)
          return this.lastToken;
        return i >= this.prs.rhs(this.currentAction) ? this.lastToken : this.tokStream.getPrevious(this.getToken(i + 1));
      }
      /** Root SPPF symbol node from the last successful error-free parse, or null. */
      getSppfRoot() {
        return this.sppfRoot;
      }
      /** Number of distinct SPPF symbol nodes created in the last parse. */
      getSppfSymbolCount() {
        return this.sppfSymbolCount;
      }
      setMonitor(monitor) {
        this.monitor = monitor;
      }
      reset1() {
        this.taking_actions = false;
        this.sppfRoot = null;
        this.sppfSymbolCount = 0;
      }
      reset2(tokStream, monitor) {
        this.monitor = monitor;
        this.tokStream = tokStream;
        this.reset1();
      }
      reset(tokStream, prs, ra, monitor) {
        if (prs) {
          this.prs = prs;
          this.START_STATE = prs.getStartState();
          this.NUM_RULES = prs.getNumRules();
          this.NT_OFFSET = prs.getNtOffset();
          this.LA_STATE_OFFSET = prs.getLaStateOffset();
          this.ACCEPT_ACTION = prs.getAcceptAction();
          this.ERROR_ACTION = prs.getErrorAction();
          if (!prs.isValidForParser())
            throw new BadParseSymFileException_1.BadParseSymFileException();
          if (!prs.isGLR || !prs.isGLR())
            throw new NotGLRParseTableException_1.NotGLRParseTableException();
        }
        if (ra)
          this.ra = ra;
        if (!tokStream) {
          this.reset1();
          return;
        }
        this.reset2(tokStream, monitor);
      }
      constructor(tokStream, prs, ra, monitor) {
        super();
        this.START_STATE = 0;
        this.NUM_RULES = 0;
        this.NT_OFFSET = 0;
        this.LA_STATE_OFFSET = 0;
        this.ACCEPT_ACTION = 0;
        this.ERROR_ACTION = 0;
        this.tokStream = new TokenStream_1.EscapeStrictPropertyInitializationTokenStream();
        this.prs = new ParseTable_1.EscapeStrictPropertyInitializationParseTable();
        this.ra = new RuleAction_1.EscapeStrictPropertyInitializationRuleAction();
        this.taking_actions = false;
        this.currentAction = 0;
        this.lastToken = 0;
        this.parseStackRoot = 0;
        this.frameTop = 0;
        this.frameLocation = new Int32Array(0);
        this.frameParse = [];
        this.familyCache = new EqMap();
        this.forestCache = new EqMap();
        this.gssNodes = /* @__PURE__ */ new Map();
        this.sppfNodes = /* @__PURE__ */ new Map();
        this.sppfRoot = null;
        this.sppfSymbolCount = 0;
        this.reset(tokStream, prs, ra, monitor);
      }
      parse(max_error_count = 0) {
        return this.parseEntry(0, max_error_count);
      }
      /** Error-free GLR parse when max_error_count omitted/0; else repair fallback. */
      parseEntry(marker_kind = 0, max_error_count = 0) {
        try {
          return this.parseEntryNoRepair(marker_kind);
        } catch (e) {
          if (!(e instanceof BadParseException_1.BadParseException) || max_error_count <= 0)
            throw e;
          try {
            let bt = new BacktrackingParser_1.BacktrackingParser(this.tokStream, this.prs, this.ra, this.monitor);
            if (this.ra.setRecoverParser)
              this.ra.setRecoverParser(bt);
            try {
              return bt.fuzzyParseEntry(marker_kind, max_error_count);
            } finally {
              if (this.ra.setRecoverParser)
                this.ra.setRecoverParser(null);
            }
          } catch (ex) {
            if (ex instanceof BadParseSymFileException_1.BadParseSymFileException || ex instanceof NotBacktrackParseTableException_1.NotBacktrackParseTableException)
              throw new Error(String(ex));
            throw ex;
          }
        }
      }
      parseEntryNoRepair(marker_kind) {
        this.tokStream.reset();
        this.familyCache = new EqMap();
        this.forestCache = new EqMap();
        this.gssNodes = /* @__PURE__ */ new Map();
        this.sppfNodes = /* @__PURE__ */ new Map();
        this.sppfRoot = null;
        let firstTok = this.tokStream.getToken();
        let prev = this.tokStream.getPrevious(firstTok);
        let startTok = marker_kind === 0 ? firstTok : prev;
        let startKind = marker_kind === 0 ? this.tokStream.getKind(firstTok) : marker_kind;
        this.parseStackRoot = marker_kind === 0 ? 0 : 1;
        let start = new Config();
        start.stateStackTop = -1;
        start.currentAction = this.START_STATE;
        start.curtok = startTok;
        start.lastToken = prev;
        start.currentKind = startKind;
        start.gssTip = null;
        this.ensureCapacity(start, 16);
        let live = [start];
        let accepts = [];
        let errorTok = startTok;
        let outerGuard = this.prs.getNumStates() * 64 + this.tokStream.getStreamLength() * 8 + 256;
        while (live.length > 0) {
          if (this.monitor && this.monitor.isCancelled())
            return null;
          if (--outerGuard < 0)
            throw new Error("cyclic/\u03B5-loop grammar not supported by GLR v2");
          let next = [];
          let packed = new EqMap();
          for (let cfg of live) {
            if (cfg.curtok > errorTok)
              errorTok = cfg.curtok;
            let stepResults = [];
            let stepAccepts = [];
            this.stepConfig(cfg, stepResults, stepAccepts);
            for (let a of stepAccepts)
              this.packAccept(accepts, a);
            for (let r of stepResults) {
              let k = r.key();
              let bucket = packed.get(k);
              if (bucket == null) {
                bucket = [r];
                packed.set(k, bucket);
                next.push(r);
              } else {
                let merged = false;
                for (let existing of bucket) {
                  if (this.canPackParseStacks(existing, r)) {
                    this.packParseStacks(existing, r);
                    merged = true;
                    break;
                  }
                }
                if (!merged) {
                  bucket.push(r);
                  next.push(r);
                }
              }
            }
          }
          if (accepts.length > 0 && next.length === 0)
            break;
          live = next;
          if (live.length === 0 && accepts.length === 0)
            throw new BadParseException_1.BadParseException(errorTok);
        }
        if (accepts.length === 0)
          throw new BadParseException_1.BadParseException(errorTok);
        let root = accepts[0].ast;
        let rootSymbol = accepts[0].grammarSymbol;
        this.sppfRoot = accepts[0].sppf;
        for (let i = 1; i < accepts.length; i++) {
          let other = accepts[i];
          if (other.grammarSymbol !== rootSymbol)
            throw new Error("GLR accepted distinct start symbols");
          if (this.sppfRoot == null)
            this.sppfRoot = other.sppf;
          if (!_GLRParser.appendNextAst(root, other.ast))
            throw new Error("overlapping GLR accept forests");
        }
        this.sppfSymbolCount = this.sppfNodes.size;
        return root === _GLRParser.NULL_RESULT ? null : root;
      }
      stepConfig(cfg, out, accepts) {
        let work = [cfg.copy()];
        let guard = this.prs.getNumStates() * 4 + 8;
        while (work.length > 0) {
          if (--guard < 0)
            throw new Error("cyclic/\u03B5-loop grammar not supported by GLR v2");
          let c = work.pop();
          this.ensureCapacity(c, c.stateStackTop + 2);
          c.stateStack[++c.stateStackTop] = c.currentAction;
          c.locationStack[c.stateStackTop] = c.curtok;
          c.symbolStack[c.stateStackTop] = 0;
          c.sppfStack[c.stateStackTop] = null;
          if (c.stateStackTop !== this.parseStackRoot)
            c.parseStack[c.stateStackTop] = null;
          c.gssTip = this.gssPush(c.gssTip, c.currentAction, c.curtok, 0, null, null);
          let act = this.tAction(c.currentAction, c.currentKind, c.curtok);
          let candidates = [];
          if (act > this.ACCEPT_ACTION && act < this.ERROR_ACTION)
            this.expandConflict(act, candidates);
          else
            candidates.push(act);
          for (let ci = 0; ci < candidates.length; ci++) {
            let cand = candidates[ci];
            let fork = candidates.length === 1 ? c : c.copy();
            this.applyConcreteAction(fork, cand, work, out, accepts);
          }
        }
      }
      applyConcreteAction(fork, cand, work, out, accepts) {
        if (cand <= this.NUM_RULES) {
          fork.stateStackTop--;
          fork.gssTip = _GLRParser.gssPop(fork.gssTip);
          this.applyReduceClosure(fork, cand, work);
        } else if (cand > this.ERROR_ACTION) {
          fork.symbolStack[fork.stateStackTop] = fork.currentKind;
          let term = this.terminalSppf(fork.currentKind, fork.curtok);
          fork.sppfStack[fork.stateStackTop] = term;
          fork.gssTip = _GLRParser.gssRelabel(fork.gssTip, fork.currentKind, fork.curtok, null, term);
          fork.lastToken = fork.curtok;
          fork.curtok = this.tokStream.getNext(fork.curtok);
          fork.currentKind = this.tokStream.getKind(fork.curtok);
          this.applyReduceClosure(fork, cand - this.ERROR_ACTION, work);
        } else if (cand < this.ACCEPT_ACTION) {
          fork.symbolStack[fork.stateStackTop] = fork.currentKind;
          let term = this.terminalSppf(fork.currentKind, fork.curtok);
          fork.sppfStack[fork.stateStackTop] = term;
          fork.gssTip = _GLRParser.gssRelabel(fork.gssTip, fork.currentKind, fork.curtok, null, term);
          fork.lastToken = fork.curtok;
          fork.curtok = this.tokStream.getNext(fork.curtok);
          fork.currentKind = this.tokStream.getKind(fork.curtok);
          fork.currentAction = cand;
          out.push(fork);
        } else if (cand === this.ACCEPT_ACTION) {
          let root = null;
          let rootSymbol = 0;
          if (fork.parseStack != null && this.parseStackRoot < fork.parseStack.length)
            root = fork.parseStack[this.parseStackRoot];
          if (fork.symbolStack != null && this.parseStackRoot <= fork.stateStackTop)
            rootSymbol = fork.symbolStack[this.parseStackRoot];
          let rootSppf = null;
          if (fork.sppfStack != null && this.parseStackRoot < fork.sppfStack.length)
            rootSppf = fork.sppfStack[this.parseStackRoot];
          accepts.push(new AcceptCandidate(root == null ? _GLRParser.NULL_RESULT : root, rootSymbol, rootSppf));
        }
      }
      applyReduceClosure(fork, rule, work) {
        let action = rule;
        do {
          let rhs = this.prs.rhs(action);
          if (fork.stateStackTop - (rhs - 1) < 0)
            throw new Error("GLR reduce stack underflow");
          let kids = new Array(rhs);
          if (rhs > 0) {
            for (let i = 0; i < rhs; i++)
              kids[i] = fork.sppfStack[fork.stateStackTop - rhs + 1 + i];
          }
          fork.stateStackTop -= rhs - 1;
          if (rhs > 0) {
            for (let i = 0; i < rhs - 1; i++)
              fork.gssTip = _GLRParser.gssPop(fork.gssTip);
          } else {
            this.ensureCapacity(fork, fork.stateStackTop + 1);
            fork.gssTip = this.gssPush(fork.gssTip, fork.stateStack[fork.stateStackTop], fork.locationStack[fork.stateStackTop], 0, null, null);
          }
          let reductionKey = new ReductionKey(action, fork.lastToken, rhs, fork.stateStackTop, fork.locationStack, fork.symbolStack, fork.parseStack);
          this.currentAction = action;
          this.lastToken = fork.lastToken;
          this.frameTop = fork.stateStackTop;
          this.frameLocation = fork.locationStack;
          this.frameParse = fork.parseStack;
          this.taking_actions = true;
          try {
            this.ra.ruleAction(action);
          } finally {
            this.taking_actions = false;
          }
          let lhs = this.prs.lhs(action);
          let lhsSymbol = this.NT_OFFSET + lhs;
          let result = fork.parseStack[fork.stateStackTop];
          if (isIAst(result)) {
            let canonical = this.familyCache.get(reductionKey);
            if (canonical == null) {
              let ast = result;
              let forestKey = new ForestKey(lhsSymbol, ast);
              canonical = forestKey.isPackable() ? this.forestCache.get(forestKey) : void 0;
              if (canonical == null) {
                canonical = ast;
                if (forestKey.isPackable())
                  this.forestCache.set(forestKey, canonical);
              } else if (canonical !== ast && !_GLRParser.appendNextAst(canonical, ast)) {
                throw new Error("cannot merge GLR production family");
              }
              this.familyCache.set(reductionKey, canonical);
            }
            fork.parseStack[fork.stateStackTop] = canonical;
            result = canonical;
          }
          let leftExt = fork.locationStack[fork.stateStackTop];
          let rightExt = fork.lastToken;
          if (isIAst(result)) {
            let lt = result.getLeftIToken();
            let rt = result.getRightIToken();
            if (lt != null && rt != null) {
              leftExt = lt.getTokenIndex();
              rightExt = rt.getTokenIndex();
            }
          }
          let symNode = this.sppfSymbol(lhsSymbol, leftExt, rightExt);
          this.addPacked(symNode, action, kids, result);
          if (isIAst(result))
            symNode.astForest = result;
          fork.sppfStack[fork.stateStackTop] = symNode;
          fork.symbolStack[fork.stateStackTop] = lhsSymbol;
          fork.gssTip = _GLRParser.gssRelabel(fork.gssTip, lhsSymbol, leftExt, result, symNode);
          action = this.prs.ntAction(fork.stateStack[fork.stateStackTop], lhs);
        } while (action <= this.NUM_RULES);
        fork.currentAction = action;
        work.push(fork);
      }
      ensureCapacity(c, need) {
        let len = c.stateStack == null ? 0 : c.stateStack.length;
        if (need < len)
          return;
        let neu = Math.max(need + 8, len + this.STACK_INCREMENT);
        if (c.stateStack == null) {
          c.stateStack = new Int32Array(neu);
          c.symbolStack = new Int32Array(neu);
          c.parseStack = new Array(neu);
          c.locationStack = new Int32Array(neu);
          c.sppfStack = new Array(neu);
        } else {
          c.stateStack = copyInt32(c.stateStack, neu);
          c.symbolStack = copyInt32(c.symbolStack, neu);
          c.parseStack = copyAny(c.parseStack, neu);
          c.locationStack = copyInt32(c.locationStack, neu);
          c.sppfStack = copyAny(c.sppfStack, neu);
        }
      }
      static sppfKey(symbol, left, right) {
        return symbol + ":" + left + ":" + right;
      }
      sppfSymbol(grammarSymbol, leftExtent, rightExtent) {
        let key = _GLRParser.sppfKey(grammarSymbol, leftExtent, rightExtent);
        let n = this.sppfNodes.get(key);
        if (n == null) {
          n = new SppfNode_1.SppfNode(grammarSymbol, leftExtent, rightExtent);
          this.sppfNodes.set(key, n);
        }
        return n;
      }
      terminalSppf(kind, tok) {
        let term = this.sppfSymbol(kind, tok, tok);
        if (term.packs.length === 0)
          term.packs.push(new SppfNode_1.SppfNode.Packed(-kind, null, null));
        return term;
      }
      addPacked(symNode, rule, children, semantic) {
        let n = children == null ? 0 : children.length;
        for (let i = 0; i < symNode.packs.length; i++) {
          let p = symNode.packs[i];
          if (p.rule !== rule || p.children.length !== n)
            continue;
          let same = true;
          for (let c = 0; c < n; c++) {
            if (p.children[c] !== children[c]) {
              same = false;
              break;
            }
          }
          if (same)
            return;
        }
        symNode.packs.push(new SppfNode_1.SppfNode.Packed(rule, children, semantic));
      }
      gssPush(tip, state, index, symbol, semantic, sppf) {
        let n = new GssNode_1.GssNode(state, index);
        let pred = tip == null ? new GssNode_1.GssNode(Number.MIN_SAFE_INTEGER, -1) : tip;
        n.edges.push(new GssEdge_1.GssEdge(pred, symbol, index, semantic, sppf));
        let key = state + ":" + (index >>> 0);
        let canon = this.gssNodes.get(key);
        if (canon == null) {
          canon = new GssNode_1.GssNode(state, index);
          this.gssNodes.set(key, canon);
        }
        canon.edges.push(new GssEdge_1.GssEdge(pred, symbol, index, semantic, sppf));
        return n;
      }
      static gssPop(tip) {
        if (tip == null || tip.edges.length === 0)
          return null;
        let pred = tip.edges[0].predecessor;
        return pred.state === Number.MIN_SAFE_INTEGER ? null : pred;
      }
      static gssRelabel(tip, symbol, location, semantic, sppf) {
        if (tip == null || tip.edges.length === 0)
          return tip;
        let pred = tip.edges[0].predecessor;
        let n = new GssNode_1.GssNode(tip.state, tip.index);
        n.edges.push(new GssEdge_1.GssEdge(pred, symbol, location, semantic, sppf));
        return n;
      }
      packAccept(accepts, cand) {
        let ast = cand.ast;
        let grammarSymbol = cand.grammarSymbol;
        if (ast === _GLRParser.NULL_RESULT) {
          for (let i = 0; i < accepts.length; i++) {
            if (accepts[i].ast === _GLRParser.NULL_RESULT)
              return;
          }
          accepts.push(cand);
          return;
        }
        if (ast == null)
          return;
        for (let i = 0; i < accepts.length; i++) {
          let existing = accepts[i];
          let a = existing.ast;
          if (a === _GLRParser.NULL_RESULT)
            continue;
          if (existing.grammarSymbol === grammarSymbol && _GLRParser.sameSpan(a, ast) && _GLRParser.appendNextAst(a, ast)) {
            return;
          }
        }
        accepts.push(cand);
      }
      canPackParseStacks(existing, incoming) {
        if (existing.stateStackTop !== incoming.stateStackTop)
          return false;
        for (let i = 0; i <= existing.stateStackTop; i++) {
          let a = existing.parseStack[i];
          let b = incoming.parseStack[i];
          if (a === b)
            continue;
          if (!isIAst(a) || !isIAst(b))
            return false;
          if (!_GLRParser.sameSpan(a, b))
            return false;
          if (!_GLRParser.canAppendNextAst(a, b))
            return false;
        }
        return true;
      }
      static canAppendNextAst(root, alt) {
        return _GLRParser.appendNextAst(root, alt, false);
      }
      packParseStacks(existing, incoming) {
        for (let i = 0; i <= existing.stateStackTop; i++) {
          let a = existing.parseStack[i];
          let b = incoming.parseStack[i];
          if (a === b || a == null || b == null)
            continue;
          if (!_GLRParser.canAppendNextAst(a, b))
            throw new Error("overlapping GLR semantic forests");
        }
        for (let i = 0; i <= existing.stateStackTop; i++) {
          existing.parseStack[i] = this.packSym(existing.parseStack[i], incoming.parseStack[i]);
          if (existing.sppfStack[i] == null)
            existing.sppfStack[i] = incoming.sppfStack[i];
          else if (incoming.sppfStack[i] != null && existing.sppfStack[i] !== incoming.sppfStack[i] && existing.sppfStack[i].grammarSymbol === incoming.sppfStack[i].grammarSymbol && existing.sppfStack[i].leftExtent === incoming.sppfStack[i].leftExtent && existing.sppfStack[i].rightExtent === incoming.sppfStack[i].rightExtent) {
            let canon = existing.sppfStack[i];
            let other = incoming.sppfStack[i];
            for (let p = 0; p < other.packs.length; p++) {
              let pk = other.packs[p];
              this.addPacked(canon, pk.rule, pk.children, pk.semantic);
            }
            if (isIAst(existing.parseStack[i]))
              canon.astForest = existing.parseStack[i];
          }
        }
        if (incoming.gssTip != null)
          existing.gssTip = incoming.gssTip;
      }
      packSym(a, b) {
        if (a == null)
          return b;
        if (b == null || a === b)
          return a;
        if (!_GLRParser.appendNextAst(a, b))
          throw new Error("overlapping GLR semantic forests");
        return a;
      }
      static sameSpan(a, b) {
        if (!isIAst(a) || !isIAst(b))
          return false;
        let la = a.getLeftIToken(), ra = a.getRightIToken();
        let lb = b.getLeftIToken(), rb = b.getRightIToken();
        if (la == null || ra == null || lb == null || rb == null)
          return false;
        return la.getILexStream() === lb.getILexStream() && ra.getILexStream() === rb.getILexStream() && la.getTokenIndex() === lb.getTokenIndex() && ra.getTokenIndex() === rb.getTokenIndex();
      }
      static appendNextAst(root, alt, commit = true) {
        if (!isIAst(root) || !isIAst(alt))
          return false;
        let cur = root;
        let neu = alt;
        if (cur === neu)
          return true;
        let seen = /* @__PURE__ */ new Set();
        let tail = null;
        for (let p = cur; p != null; p = p.getNextAst()) {
          if (seen.has(p))
            return false;
          seen.add(p);
          tail = p;
        }
        let incoming = /* @__PURE__ */ new Set();
        for (let p = neu; p != null; ) {
          if (incoming.has(p))
            return false;
          incoming.add(p);
          if (seen.has(p)) {
            p = p.getNextAst();
            continue;
          }
          for (let q = p.getNextAst(); q != null; q = q.getNextAst()) {
            if (incoming.has(q))
              return false;
            incoming.add(q);
            if (seen.has(q))
              return false;
          }
          if (commit) {
            if (tail != null && tail.setNextAst)
              tail.setNextAst(p);
            for (let q = p; q != null; q = q.getNextAst()) {
              seen.add(q);
              tail = q;
            }
          }
          return true;
        }
        return true;
      }
    };
    exports.GLRParser = GLRParser3;
    GLRParser3.NULL_RESULT = {};
    var AcceptCandidate = class {
      constructor(ast, grammarSymbol, sppf) {
        this.ast = ast;
        this.grammarSymbol = grammarSymbol;
        this.sppf = sppf;
      }
    };
    var Config = class _Config {
      constructor() {
        this.stateStack = new Int32Array(0);
        this.symbolStack = new Int32Array(0);
        this.parseStack = [];
        this.locationStack = new Int32Array(0);
        this.sppfStack = [];
        this.gssTip = null;
        this.stateStackTop = 0;
        this.currentAction = 0;
        this.curtok = 0;
        this.lastToken = 0;
        this.currentKind = 0;
      }
      copy() {
        let c = new _Config();
        c.stateStackTop = this.stateStackTop;
        c.currentAction = this.currentAction;
        c.curtok = this.curtok;
        c.lastToken = this.lastToken;
        c.currentKind = this.currentKind;
        c.gssTip = this.gssTip;
        if (this.stateStack != null && this.stateStack.length > 0) {
          c.stateStack = copyInt32(this.stateStack, this.stateStack.length);
          c.symbolStack = copyInt32(this.symbolStack, this.symbolStack.length);
          c.parseStack = copyAny(this.parseStack, this.parseStack.length);
          c.locationStack = copyInt32(this.locationStack, this.locationStack.length);
          if (this.sppfStack != null)
            c.sppfStack = copyAny(this.sppfStack, this.sppfStack.length);
        }
        return c;
      }
      key() {
        return new ConfigKey(this);
      }
    };
    var ConfigKey = class {
      constructor(config) {
        this.config = config;
        let h = 31 * config.curtok + config.currentKind;
        h = 31 * h + config.lastToken;
        h = 31 * h + config.currentAction;
        for (let i = 0; i <= config.stateStackTop; i++) {
          h = 31 * h + config.stateStack[i];
          h = 31 * h + config.locationStack[i];
          h = 31 * h + config.symbolStack[i];
        }
        this.hash = h | 0;
      }
      hashCode() {
        return this.hash;
      }
      equals(other) {
        if (this === other)
          return true;
        let a = this.config;
        let b = other.config;
        if (a.curtok !== b.curtok || a.currentKind !== b.currentKind || a.lastToken !== b.lastToken || a.currentAction !== b.currentAction || a.stateStackTop !== b.stateStackTop)
          return false;
        for (let i = 0; i <= a.stateStackTop; i++) {
          if (a.stateStack[i] !== b.stateStack[i] || a.locationStack[i] !== b.locationStack[i] || a.symbolStack[i] !== b.symbolStack[i])
            return false;
        }
        return true;
      }
    };
    var ReductionKey = class {
      constructor(rule, lastToken, rhs, frameTop, locationStack, symbolStack, parseStack) {
        this.rule = rule;
        this.lastToken = lastToken;
        this.locations = new Int32Array(rhs);
        this.grammarSymbols = new Int32Array(rhs);
        this.semanticValues = new Array(rhs);
        let h = 31 * rule + lastToken;
        for (let i = 0; i < rhs; i++) {
          let index = frameTop + i;
          this.locations[i] = locationStack[index];
          this.grammarSymbols[i] = symbolStack[index];
          this.semanticValues[i] = parseStack[index];
          h = 31 * h + this.locations[i];
          h = 31 * h + this.grammarSymbols[i];
          h = 31 * h + identityHash(this.semanticValues[i]);
        }
        this.hash = h | 0;
      }
      hashCode() {
        return this.hash;
      }
      equals(other) {
        if (this === other)
          return true;
        if (this.rule !== other.rule || this.lastToken !== other.lastToken || this.locations.length !== other.locations.length)
          return false;
        for (let i = 0; i < this.locations.length; i++) {
          if (this.locations[i] !== other.locations[i] || this.grammarSymbols[i] !== other.grammarSymbols[i] || this.semanticValues[i] !== other.semanticValues[i])
            return false;
        }
        return true;
      }
    };
    var ForestKey = class {
      constructor(grammarSymbol, ast) {
        let left = ast.getLeftIToken();
        let right = ast.getRightIToken();
        this.grammarSymbol = grammarSymbol;
        this.lexStream = left == null ? null : left.getILexStream();
        this.leftToken = left == null ? -1 : left.getTokenIndex();
        this.rightToken = right == null ? -1 : right.getTokenIndex();
        let h = 31 * grammarSymbol + identityHash(this.lexStream);
        h = 31 * h + this.leftToken;
        this.hash = 31 * h + this.rightToken | 0;
      }
      isPackable() {
        return this.leftToken >= 0 && this.rightToken >= 0;
      }
      hashCode() {
        return this.hash;
      }
      equals(other) {
        if (this === other)
          return true;
        return this.grammarSymbol === other.grammarSymbol && this.lexStream === other.lexStream && this.leftToken === other.leftToken && this.rightToken === other.rightToken;
      }
    };
    var EqMap = class {
      constructor() {
        this.buckets = /* @__PURE__ */ new Map();
        this._size = 0;
      }
      get(key) {
        let bucket = this.buckets.get(key.hashCode());
        if (bucket == null)
          return void 0;
        for (let e of bucket) {
          if (key.equals(e.k))
            return e.v;
        }
        return void 0;
      }
      set(key, value) {
        let h = key.hashCode();
        let bucket = this.buckets.get(h);
        if (bucket == null) {
          bucket = [];
          this.buckets.set(h, bucket);
        }
        for (let i = 0; i < bucket.length; i++) {
          if (key.equals(bucket[i].k)) {
            bucket[i] = { k: key, v: value };
            return;
          }
        }
        bucket.push({ k: key, v: value });
        this._size++;
      }
      get size() {
        return this._size;
      }
    };
    function isIAst(o) {
      return o != null && typeof o.getNextAst === "function" && typeof o.getLeftIToken === "function" && typeof o.getRightIToken === "function";
    }
    var nextIdentityId = 1;
    var identityIds = /* @__PURE__ */ new WeakMap();
    function identityHash(o) {
      if (o == null || typeof o !== "object" && typeof o !== "function")
        return 0;
      let id = identityIds.get(o);
      if (id === void 0) {
        id = nextIdentityId++;
        identityIds.set(o, id);
      }
      return id;
    }
    function copyInt32(src, neu) {
      let dst = new Int32Array(neu);
      dst.set(src.subarray(0, Math.min(src.length, neu)));
      return dst;
    }
    function copyAny(src, neu) {
      let dst = new Array(neu);
      for (let i = 0; i < Math.min(src.length, neu); i++)
        dst[i] = src[i];
      return dst;
    }
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/NotDeterministicParseTableException.js
var require_NotDeterministicParseTableException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/NotDeterministicParseTableException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.NotDeterministicParseTableException = void 0;
    var NotDeterministicParseTableException2 = class extends Error {
      constructor(str) {
        super();
        if (!str) {
          this.str = "NotDeterministicParseTableException";
        } else {
          this.str = str;
        }
      }
      toString() {
        return this.str;
      }
    };
    exports.NotDeterministicParseTableException = NotDeterministicParseTableException2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/DeterministicParser.js
var require_DeterministicParser = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/DeterministicParser.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.DeterministicParser = void 0;
    var Stacks_1 = require_Stacks();
    var IntTuple_1 = require_IntTuple();
    var TokenStream_1 = require_TokenStream();
    var ParseTable_1 = require_ParseTable();
    var RuleAction_1 = require_RuleAction();
    var BadParseException_1 = require_BadParseException();
    var UnavailableParserInformationException_1 = require_UnavailableParserInformationException();
    var BadParseSymFileException_1 = require_BadParseSymFileException();
    var NotDeterministicParseTableException_1 = require_NotDeterministicParseTableException();
    var DeterministicParser2 = class extends Stacks_1.Stacks {
      //
      // keep looking ahead until we compute a valid action
      //
      lookahead(act, token) {
        act = this.prs.lookAhead(act - this.LA_STATE_OFFSET, this.tokStream.getKind(token));
        return act > this.LA_STATE_OFFSET ? this.lookahead(act, this.tokStream.getNext(token)) : act;
      }
      //
      // Compute the next action defined on act and sym. If this
      // action requires more lookahead, these lookahead symbols
      // are in the token stream beginning at the next token that
      // is yielded by peek().
      //
      tAction1(act, sym) {
        act = this.prs.tAction(act, sym);
        return act > this.LA_STATE_OFFSET ? this.lookahead(act, this.tokStream.peek()) : act;
      }
      //
      // Compute the next action defined on act and the next k tokens
      // whose types are stored in the array sym starting at location
      // index. The array sym is a circular buffer. If we reach the last
      // element of sym and we need more lookahead, we proceed to the
      // first element.
      // 
      // assert(sym.length == prs.getMaxLa());
      //
      tAction(act, sym, index) {
        act = this.prs.tAction(act, sym[index]);
        while (act > this.LA_STATE_OFFSET) {
          index = (index + 1) % sym.length;
          act = this.prs.lookAhead(act - this.LA_STATE_OFFSET, sym[index]);
        }
        return act;
      }
      //
      // Process reductions and continue...
      //
      processReductions() {
        do {
          this.stateStackTop -= this.prs.rhs(this.currentAction) - 1;
          this.ra.ruleAction(this.currentAction);
          this.currentAction = this.prs.ntAction(this.stateStack[this.stateStackTop], this.prs.lhs(this.currentAction));
        } while (this.currentAction <= this.NUM_RULES);
        return;
      }
      //
      // The following functions can be invoked only when the parser is
      // processing actions. Thus, they can be invoked when the parser
      // was entered via the main entry point (parse()). When using
      // the incremental parser (via the entry point parse(int [], int)),
      // an Exception is thrown if any of these functions is invoked?
      // However, note that when parseActions() is invoked after successfully
      // parsing an input with the incremental parser, then they can be invoked.
      //
      getCurrentRule() {
        if (this.taking_actions) {
          return this.currentAction;
        }
        throw new UnavailableParserInformationException_1.UnavailableParserInformationException();
      }
      getFirstToken1() {
        if (this.taking_actions) {
          return this.getToken(1);
        }
        throw new UnavailableParserInformationException_1.UnavailableParserInformationException();
      }
      getFirstToken(i) {
        if (!i) {
          return this.getFirstToken1();
        }
        if (this.taking_actions) {
          return this.getToken(i);
        }
        throw new UnavailableParserInformationException_1.UnavailableParserInformationException();
      }
      getLastToken1() {
        if (this.taking_actions) {
          return this.lastToken;
        }
        throw new UnavailableParserInformationException_1.UnavailableParserInformationException();
      }
      getLastToken(i) {
        if (!i) {
          return this.getLastToken1();
        }
        if (this.taking_actions) {
          return i >= this.prs.rhs(this.currentAction) ? this.lastToken : this.tokStream.getPrevious(this.getToken(i + 1));
        }
        throw new UnavailableParserInformationException_1.UnavailableParserInformationException();
      }
      setMonitor(monitor) {
        this.monitor = monitor;
      }
      reset1() {
        this.taking_actions = false;
        this.markerKind = 0;
        if (this.action.capacity() !== 0) {
          this.action.reset();
        }
      }
      reset2(tokStream, monitor) {
        this.monitor = monitor;
        this.tokStream = tokStream;
        this.reset1();
      }
      reset(tokStream, prs, ra, monitor) {
        if (ra)
          this.ra = ra;
        if (prs) {
          this.prs = prs;
          this.START_STATE = prs.getStartState();
          this.NUM_RULES = prs.getNumRules();
          this.NT_OFFSET = prs.getNtOffset();
          this.LA_STATE_OFFSET = prs.getLaStateOffset();
          this.EOFT_SYMBOL = prs.getEoftSymbol();
          this.ERROR_SYMBOL = prs.getErrorSymbol();
          this.ACCEPT_ACTION = prs.getAcceptAction();
          this.ERROR_ACTION = prs.getErrorAction();
          if (!prs.isValidForParser())
            throw new BadParseSymFileException_1.BadParseSymFileException();
          if (prs.getBacktrack())
            throw new NotDeterministicParseTableException_1.NotDeterministicParseTableException();
        }
        if (!tokStream) {
          this.reset1();
          return;
        }
        this.reset2(tokStream, monitor);
      }
      constructor(tokStream, prs, ra, monitor) {
        super();
        this.taking_actions = false;
        this.markerKind = 0;
        this.START_STATE = 0;
        this.NUM_RULES = 0;
        this.NT_OFFSET = 0;
        this.LA_STATE_OFFSET = 0;
        this.EOFT_SYMBOL = 0;
        this.ACCEPT_ACTION = 0;
        this.ERROR_ACTION = 0;
        this.ERROR_SYMBOL = 0;
        this.lastToken = 0;
        this.currentAction = 0;
        this.action = new IntTuple_1.IntTuple(0);
        this.tokStream = new TokenStream_1.EscapeStrictPropertyInitializationTokenStream();
        this.prs = new ParseTable_1.EscapeStrictPropertyInitializationParseTable();
        this.ra = new RuleAction_1.EscapeStrictPropertyInitializationRuleAction();
        this.reset(tokStream, prs, ra, monitor);
      }
      parseEntry(marker_kind = 0) {
        this.taking_actions = true;
        this.tokStream.reset();
        this.lastToken = this.tokStream.getPrevious(this.tokStream.peek());
        let curtok, current_kind;
        if (marker_kind == 0) {
          curtok = this.tokStream.getToken();
          current_kind = this.tokStream.getKind(curtok);
        } else {
          curtok = this.lastToken;
          current_kind = marker_kind;
        }
        this.reallocateStacks();
        this.stateStackTop = -1;
        this.currentAction = this.START_STATE;
        processTerminals: for (; ; ) {
          if (this.monitor != null && this.monitor.isCancelled()) {
            this.taking_actions = false;
            return null;
          }
          if (++this.stateStackTop >= this.stateStack.length) {
            this.reallocateStacks();
          }
          this.stateStack[this.stateStackTop] = this.currentAction;
          this.locationStack[this.stateStackTop] = curtok;
          this.currentAction = this.tAction1(this.currentAction, current_kind);
          if (this.currentAction <= this.NUM_RULES) {
            this.stateStackTop--;
            this.processReductions();
          } else if (this.currentAction > this.ERROR_ACTION) {
            this.lastToken = curtok;
            curtok = this.tokStream.getToken();
            current_kind = this.tokStream.getKind(curtok);
            this.currentAction -= this.ERROR_ACTION;
            this.processReductions();
          } else if (this.currentAction < this.ACCEPT_ACTION) {
            this.lastToken = curtok;
            curtok = this.tokStream.getToken();
            current_kind = this.tokStream.getKind(curtok);
          } else
            break processTerminals;
        }
        this.taking_actions = false;
        if (this.currentAction == this.ERROR_ACTION)
          throw new BadParseException_1.BadParseException(curtok);
        return this.parseStack[marker_kind == 0 ? 0 : 1];
      }
      //
      // This method is invoked when using the parser in an incremental mode
      // using the entry point parse(int [], int).
      //
      resetParser() {
        this.resetParserEntry(0);
      }
      //
      // This method is invoked when using the parser in an incremental mode
      // using the entry point parse(int [], int).
      //
      resetParserEntry(marker_kind) {
        this.markerKind = marker_kind;
        if (this.stateStack == void 0 || this.stateStack.length == 0) {
          this.reallocateStacks();
        }
        this.stateStackTop = 0;
        this.stateStack[this.stateStackTop] = this.START_STATE;
        if (this.action.capacity() == 0) {
          this.action = new IntTuple_1.IntTuple(1 << 20);
        } else {
          this.action.reset();
        }
        this.taking_actions = false;
        if (marker_kind !== 0) {
          let sym = new Int32Array(1);
          sym[0] = this.markerKind;
          this.parse(sym, 0);
        }
      }
      //
      // Find a state in the state stack that has a valid action on ERROR token
      //
      recoverableState(state) {
        for (let k = this.prs.asi(state); this.prs.asr(k) !== 0; k++) {
          if (this.prs.asr(k) == this.ERROR_SYMBOL) {
            return true;
          }
        }
        return false;
      }
      //
      // Reset the parser at a point where it can legally process
      // the error token. If we can't do that, reset it to the beginning.
      //
      errorReset() {
        let gate = this.markerKind == 0 ? 0 : 1;
        for (; this.stateStackTop >= gate; this.stateStackTop--) {
          if (this.recoverableState(this.stateStack[this.stateStackTop])) {
            break;
          }
        }
        if (this.stateStackTop < gate) {
          this.resetParserEntry(this.markerKind);
        }
        return;
      }
      //
      // This is an incremental LALR(k) parser that takes as argument
      // the next k tokens in the input. If these k tokens are valid for
      // the current configuration, it advances past the first of the k
      // tokens and returns either:
      //
      //    . the last transition induced by that token 
      //    . the Accept action
      //
      // If the tokens are not valid, the initial configuration remains
      // unchanged and the Error action is returned.
      //
      // Note that it is the user's responsibility to start the parser in a
      // proper configuration by initially invoking the method resetParser
      // prior to invoking this function.
      //
      parse(sym, index) {
        let save_action_length = this.action.size(), pos = this.stateStackTop, location_top = this.stateStackTop - 1;
        for (this.currentAction = this.tAction(this.stateStack[this.stateStackTop], sym, index); this.currentAction <= this.NUM_RULES; this.currentAction = this.tAction(this.currentAction, sym, index)) {
          this.action.add(this.currentAction);
          do {
            location_top -= this.prs.rhs(this.currentAction) - 1;
            let state = location_top > pos ? this.locationStack[location_top] : this.stateStack[location_top];
            this.currentAction = this.prs.ntAction(state, this.prs.lhs(this.currentAction));
          } while (this.currentAction <= this.NUM_RULES);
          pos = pos < location_top ? pos : location_top;
          if (location_top + 1 >= this.locationStack.length) {
            this.reallocateStacks();
          }
          this.locationStack[location_top + 1] = this.currentAction;
        }
        if (this.currentAction > this.ERROR_ACTION || // SHIFT-REDUCE action ?
        this.currentAction < this.ACCEPT_ACTION) {
          this.action.add(this.currentAction);
          this.stateStackTop = location_top + 1;
          for (let i = pos + 1; i <= this.stateStackTop; i++) {
            this.stateStack[i] = this.locationStack[i];
          }
          if (this.currentAction > this.ERROR_ACTION) {
            this.currentAction -= this.ERROR_ACTION;
            do {
              this.stateStackTop -= this.prs.rhs(this.currentAction) - 1;
              this.currentAction = this.prs.ntAction(this.stateStack[this.stateStackTop], this.prs.lhs(this.currentAction));
            } while (this.currentAction <= this.NUM_RULES);
          }
          if (++this.stateStackTop >= this.stateStack.length) {
            this.reallocateStacks();
          }
          this.stateStack[this.stateStackTop] = this.currentAction;
        } else if (this.currentAction == this.ERROR_ACTION) {
          this.action.reset(save_action_length);
        }
        return this.currentAction;
      }
      //
      // Now do the final parse of the input based on the actions in
      // the list "action" and the sequence of tokens in the token stream.
      //
      parseActions() {
        this.taking_actions = true;
        this.tokStream.reset();
        this.lastToken = this.tokStream.getPrevious(this.tokStream.peek());
        let curtok = this.markerKind == 0 ? this.tokStream.getToken() : this.lastToken;
        try {
          this.stateStackTop = -1;
          this.currentAction = this.START_STATE;
          for (let i = 0; i < this.action.size(); i++) {
            if (this.monitor && this.monitor.isCancelled()) {
              this.taking_actions = false;
              return void 0;
            }
            this.stateStack[++this.stateStackTop] = this.currentAction;
            this.locationStack[this.stateStackTop] = curtok;
            this.currentAction = this.action.get(i);
            if (this.currentAction <= this.NUM_RULES) {
              this.stateStackTop--;
              this.processReductions();
            } else {
              this.lastToken = curtok;
              curtok = this.tokStream.getToken();
              if (this.currentAction > this.ERROR_ACTION) {
                this.currentAction -= this.ERROR_ACTION;
                this.processReductions();
              }
            }
          }
        } catch ($ex$) {
          this.taking_actions = false;
          throw new BadParseException_1.BadParseException(curtok);
        }
        this.taking_actions = false;
        this.action = new IntTuple_1.IntTuple(0);
        return this.parseStack[this.markerKind == 0 ? 0 : 1];
      }
    };
    exports.DeterministicParser = DeterministicParser2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/Token.js
var require_Token = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/Token.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.Token = void 0;
    var AbstractToken_1 = require_AbstractToken();
    var Token = class extends AbstractToken_1.AbstractToken {
      constructor(startOffset, endOffset, kind, iPrsStream) {
        super(startOffset, endOffset, kind, iPrsStream);
      }
      getFollowingAdjuncts() {
        var _a;
        let result = (_a = this.getIPrsStream()) === null || _a === void 0 ? void 0 : _a.getFollowingAdjuncts(this.getTokenIndex());
        return result ? result : [];
      }
      getPrecedingAdjuncts() {
        var _a;
        let result = (_a = this.getIPrsStream()) === null || _a === void 0 ? void 0 : _a.getPrecedingAdjuncts(this.getTokenIndex());
        return result ? result : [];
      }
    };
    exports.Token = Token;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/ErrorToken.js
var require_ErrorToken = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/ErrorToken.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.ErrorToken = void 0;
    var Token_1 = require_Token();
    var ErrorToken2 = class extends Token_1.Token {
      constructor(firstToken, lastToken, errorToken, startOffset, endOffset, kind) {
        super(startOffset, endOffset, kind, firstToken.getIPrsStream());
        this.firstToken = firstToken;
        this.lastToken = lastToken;
        this.errorToken = errorToken;
      }
      getFirstToken() {
        return this.getFirstRealToken();
      }
      getFirstRealToken() {
        return this.firstToken;
      }
      getLastToken() {
        return this.getLastRealToken();
      }
      getLastRealToken() {
        return this.lastToken;
      }
      getErrorToken() {
        return this.errorToken;
      }
      getPrecedingAdjuncts() {
        return this.firstToken.getPrecedingAdjuncts();
      }
      getFollowingAdjuncts() {
        return this.lastToken.getFollowingAdjuncts();
      }
    };
    exports.ErrorToken = ErrorToken2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/ExpectedTokens.js
var require_ExpectedTokens = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/ExpectedTokens.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.expectedTerminalNames = void 0;
    function expectedTerminalNames(prs, state) {
      if (!prs) {
        return [];
      }
      const errorAction = prs.getErrorAction();
      const ntOffset = prs.getNtOffset();
      const unique = /* @__PURE__ */ new Set();
      for (let sym = 1; sym < ntOffset; sym++) {
        const act = prs.tAction(state, sym);
        if (act === errorAction) {
          continue;
        }
        const n = prs.name(prs.terminalIndex(sym));
        if (n) {
          unique.add(n);
        }
      }
      return Array.from(unique).sort();
    }
    exports.expectedTerminalNames = expectedTerminalNames;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/IAbstractArrayList.js
var require_IAbstractArrayList = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/IAbstractArrayList.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/IAst.js
var require_IAst = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/IAst.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/IAstVisitor.js
var require_IAstVisitor = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/IAstVisitor.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/IMessageHandler.js
var require_IMessageHandler = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/IMessageHandler.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.IMessageHandler = void 0;
    var IMessageHandler = class {
    };
    exports.IMessageHandler = IMessageHandler;
    (function(IMessageHandler2) {
      IMessageHandler2.OFFSET_INDEX = 0;
      IMessageHandler2.LENGTH_INDEX = 1;
      IMessageHandler2.START_LINE_INDEX = 2;
      IMessageHandler2.START_COLUMN_INDEX = 3;
      IMessageHandler2.END_LINE_INDEX = 4;
      IMessageHandler2.END_COLUMN_INDEX = 5;
    })(IMessageHandler = exports.IMessageHandler || (exports.IMessageHandler = {}));
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/IncrementalParse.js
var require_IncrementalParse = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/IncrementalParse.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.incrementalResetAtDamage = exports.incrementalReparseStep = exports.resetIncrementalParser = exports.incrementalRelexAfterDamage = exports.INCREMENTAL_PARSING_POSITIONING = void 0;
    var Adjunct_1 = require_Adjunct();
    exports.INCREMENTAL_PARSING_POSITIONING = "Token-level re-lex + statement-level re-parse \u2014 not tree-sitter subtree reuse.";
    function isAdjunct(token) {
      return token instanceof Adjunct_1.Adjunct;
    }
    function incrementalRelexAfterDamage(ctx) {
      const { lexStream, lexParser, prsStream, inputChars, damage } = ctx;
      const start_change_offset = damage.startOffset;
      const end_change_offset = damage.endOffset;
      const offset_adjustment = inputChars.length - lexStream.getStreamLength();
      const affected_tokens = prsStream.incrementalResetAtCharacterOffset(start_change_offset);
      let affected_index = 0;
      let repair_offset = start_change_offset;
      if (affected_tokens.length > 0) {
        const token0 = affected_tokens[0];
        if (token0.getEndOffset() + 1 < start_change_offset) {
          repair_offset = token0.getEndOffset() + 1;
          if (isAdjunct(token0)) {
            prsStream.makeAdjunct(token0.getStartOffset(), token0.getEndOffset(), token0.getKind());
          } else {
            prsStream.makeToken(token0.getStartOffset(), token0.getEndOffset(), token0.getKind());
          }
          affected_index++;
        } else {
          repair_offset = token0.getStartOffset();
        }
      }
      lexStream.setInputChars(inputChars);
      lexStream.setStreamLength(inputChars.length);
      lexStream.computeLineOffsets();
      lexParser.resetTokenStream(repair_offset);
      let next_offset;
      do {
        next_offset = lexParser.incrementalParseCharacters();
        while (affected_index < affected_tokens.length && affected_tokens[affected_index].getStartOffset() + offset_adjustment < next_offset) {
          affected_index++;
        }
      } while (next_offset <= end_change_offset && affected_index < affected_tokens.length && affected_tokens[affected_index].getStartOffset() + offset_adjustment !== next_offset);
      return {
        affectedTokens: affected_tokens,
        repairOffset: repair_offset,
        lastScannedOffset: next_offset
      };
    }
    exports.incrementalRelexAfterDamage = incrementalRelexAfterDamage;
    function resetIncrementalParser(parser, markerKind = 0) {
      parser.resetParserEntry(markerKind);
    }
    exports.resetIncrementalParser = resetIncrementalParser;
    function incrementalReparseStep(parser, lookahead, index = 0) {
      return parser.parse(lookahead, index);
    }
    exports.incrementalReparseStep = incrementalReparseStep;
    function incrementalResetAtDamage(stream, damageOffset) {
      return stream.incrementalResetAtCharacterOffset(damageOffset);
    }
    exports.incrementalResetAtDamage = incrementalResetAtDamage;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/LexParser.js
var require_LexParser = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/LexParser.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.LexParser = void 0;
    var ParseTable_1 = require_ParseTable();
    var RuleAction_1 = require_RuleAction();
    var IntTuple_1 = require_IntTuple();
    var Utils_1 = require_Utils();
    var Protocol_1 = require_Protocol();
    var UnavailableParserInformationException_1 = require_UnavailableParserInformationException();
    var LexParser = class {
      reset(tokStream, prs, ra) {
        this.tokStream = tokStream;
        this.prs = prs;
        this.ra = ra;
        this.START_STATE = prs.getStartState();
        this.LA_STATE_OFFSET = prs.getLaStateOffset();
        this.EOFT_SYMBOL = prs.getEoftSymbol();
        this.ACCEPT_ACTION = prs.getAcceptAction();
        this.ERROR_ACTION = prs.getErrorAction();
        this.START_SYMBOL = prs.getStartSymbol();
        this.NUM_RULES = prs.getNumRules();
      }
      constructor(tokStream, prs, ra) {
        this.taking_actions = false;
        this.START_STATE = 0;
        this.LA_STATE_OFFSET = 0;
        this.EOFT_SYMBOL = 0;
        this.ACCEPT_ACTION = 0;
        this.ERROR_ACTION = 0;
        this.START_SYMBOL = 0;
        this.NUM_RULES = 0;
        this.tokStream = new Protocol_1.EscapeStrictPropertyInitializationLexStream();
        this.prs = new ParseTable_1.EscapeStrictPropertyInitializationParseTable();
        this.ra = new RuleAction_1.EscapeStrictPropertyInitializationRuleAction();
        this.action = new IntTuple_1.IntTuple(0);
        this.STACK_INCREMENT = 1024;
        this.stateStackTop = 0;
        this.stackLength = 0;
        this.stack = new Int32Array(0);
        this.locationStack = new Int32Array(0);
        this.tempStack = new Int32Array(0);
        this.lastToken = 0;
        this.currentAction = 0;
        this.curtok = 0;
        this.starttok = 0;
        this.current_kind = 0;
        if (tokStream && prs && ra)
          this.reset(tokStream, prs, ra);
      }
      reallocateStacks() {
        let old_stack_length = this.stack.length == 0 ? 0 : this.stackLength;
        this.stackLength += this.STACK_INCREMENT;
        if (old_stack_length == 0) {
          this.stack = new Int32Array(this.stackLength);
          this.locationStack = new Int32Array(this.stackLength);
          this.tempStack = new Int32Array(this.stackLength);
        } else {
          Utils_1.Lpg.Lang.System.arraycopy(this.stack, 0, this.stack = new Int32Array(this.stackLength), 0, old_stack_length);
          Utils_1.Lpg.Lang.System.arraycopy(this.locationStack, 0, this.locationStack = new Int32Array(this.stackLength), 0, old_stack_length);
          Utils_1.Lpg.Lang.System.arraycopy(this.tempStack, 0, this.tempStack = new Int32Array(this.stackLength), 0, old_stack_length);
        }
        return;
      }
      //
      // The following functions can be invoked only when the parser is
      // processing actions. Thus, they can be invoked when the parser
      // was entered via the main entry point (parseCharacters()). When using
      // the incremental parser (via the entry point scanNextToken(int [], int)),
      // they always return 0 when invoked. // TODO: Should we throw an Exception instead?
      // However, note that when parseActions() is invoked after successfully
      // parsing an input with the incremental parser, then they can be invoked.
      //
      getFirstToken(i) {
        if (i) {
          return this.getToken(i);
        }
        return this.starttok;
      }
      getLastToken(i) {
        if (!i) {
          return this.lastToken;
        }
        if (this.taking_actions) {
          return i >= this.prs.rhs(this.currentAction) ? this.lastToken : this.tokStream.getPrevious(this.getToken(i + 1));
        }
        throw new UnavailableParserInformationException_1.UnavailableParserInformationException();
      }
      getCurrentRule() {
        if (this.taking_actions) {
          return this.currentAction;
        }
        throw new UnavailableParserInformationException_1.UnavailableParserInformationException();
      }
      //
      // Given a rule of the form     A ::= x1 x2 ... xn     n > 0
      //
      // the function getToken(i) yields the symbol xi, if xi is a terminal
      // or ti, if xi is a nonterminal that produced a string of the form
      // xi => ti w. If xi is a nullable nonterminal, then ti is the first
      //  symbol that immediately follows xi in the input (the lookahead).
      //
      getToken(i) {
        if (this.taking_actions) {
          return this.locationStack[this.stateStackTop + (i - 1)];
        }
        throw new UnavailableParserInformationException_1.UnavailableParserInformationException();
      }
      setSym1(i) {
      }
      getSym(i) {
        return this.getLastToken(i);
      }
      resetTokenStream(i) {
        this.tokStream.reset(i > this.tokStream.getStreamLength() ? this.tokStream.getStreamLength() : i);
        this.curtok = this.tokStream.getToken();
        this.current_kind = this.tokStream.getKind(this.curtok);
        if (!this.stack || this.stack.length == 0) {
          this.reallocateStacks();
        }
        if (this.action.capacity() == 0) {
          this.action = new IntTuple_1.IntTuple(1 << 10);
        }
      }
      //
      // Parse the input and create a stream of tokens.
      //
      parseCharacters(start_offset, end_offset, monitor) {
        this.resetTokenStream(start_offset);
        while (this.curtok <= end_offset) {
          if (monitor && monitor.isCancelled()) {
            return;
          }
          this.lexNextToken(end_offset);
        }
      }
      //
      // Parse the input and create a stream of tokens.
      //
      parseCharactersWhitMonitor(monitor) {
        this.taking_actions = true;
        this.resetTokenStream(0);
        this.lastToken = this.tokStream.getPrevious(this.curtok);
        ProcessTokens: while (this.current_kind != this.EOFT_SYMBOL) {
          if (monitor != null && monitor.isCancelled())
            break ProcessTokens;
          this.stateStackTop = -1;
          this.currentAction = this.START_STATE;
          this.starttok = this.curtok;
          ScanToken: for (; ; ) {
            if (++this.stateStackTop >= this.stack.length) {
              this.reallocateStacks();
            }
            this.stack[this.stateStackTop] = this.currentAction;
            this.locationStack[this.stateStackTop] = this.curtok;
            this.parseNextCharacter(this.curtok, this.current_kind);
            if (this.currentAction == this.ERROR_ACTION && this.current_kind != this.EOFT_SYMBOL) {
              let save_next_token = this.tokStream.peek();
              this.tokStream.reset(this.tokStream.getStreamLength() - 1);
              this.parseNextCharacter(this.curtok, this.EOFT_SYMBOL);
              this.tokStream.reset(save_next_token);
            }
            if (this.currentAction > this.ERROR_ACTION) {
              this.lastToken = this.curtok;
              this.curtok = this.tokStream.getToken();
              this.current_kind = this.tokStream.getKind(this.curtok);
              this.currentAction -= this.ERROR_ACTION;
              do {
                this.stateStackTop -= this.prs.rhs(this.currentAction) - 1;
                this.ra.ruleAction(this.currentAction);
                let lhs_symbol = this.prs.lhs(this.currentAction);
                if (lhs_symbol == this.START_SYMBOL)
                  continue ProcessTokens;
                this.currentAction = this.prs.ntAction(this.stack[this.stateStackTop], lhs_symbol);
              } while (this.currentAction <= this.NUM_RULES);
            } else if (this.currentAction < this.ACCEPT_ACTION) {
              this.lastToken = this.curtok;
              this.curtok = this.tokStream.getToken();
              this.current_kind = this.tokStream.getKind(this.curtok);
            } else if (this.currentAction == this.ACCEPT_ACTION)
              continue ProcessTokens;
            else
              break ScanToken;
          }
          if (this.starttok == this.curtok) {
            if (this.current_kind == this.EOFT_SYMBOL)
              break ProcessTokens;
            this.tokStream.reportLexicalError(this.starttok, this.curtok);
            this.lastToken = this.curtok;
            this.curtok = this.tokStream.getToken();
            this.current_kind = this.tokStream.getKind(this.curtok);
          } else
            this.tokStream.reportLexicalError(this.starttok, this.lastToken);
        }
        this.taking_actions = false;
        return;
      }
      //
      // This function takes as argument a configuration ([stack, stackTop], [tokStream, curtok])
      // and determines whether or not curtok can be validly parsed in this configuration. If so,
      // it parses curtok and returns the final shift or shift-reduce action on it. Otherwise, it
      // leaves the configuration unchanged and returns ERROR_ACTION.
      //
      parseNextCharacter(token, kind) {
        let start_action = this.stack[this.stateStackTop], pos = this.stateStackTop, tempStackTop = this.stateStackTop - 1;
        Scan: for (this.currentAction = this.tAction(start_action, kind); this.currentAction <= this.NUM_RULES; this.currentAction = this.tAction(this.currentAction, kind)) {
          do {
            let lhs_symbol = this.prs.lhs(this.currentAction);
            if (lhs_symbol == this.START_SYMBOL)
              break Scan;
            tempStackTop -= this.prs.rhs(this.currentAction) - 1;
            let state = tempStackTop > pos ? this.tempStack[tempStackTop] : this.stack[tempStackTop];
            this.currentAction = this.prs.ntAction(state, lhs_symbol);
          } while (this.currentAction <= this.NUM_RULES);
          if (tempStackTop + 1 >= this.stack.length)
            this.reallocateStacks();
          pos = pos < tempStackTop ? pos : tempStackTop;
          this.tempStack[tempStackTop + 1] = this.currentAction;
        }
        if (this.currentAction != this.ERROR_ACTION) {
          Replay: for (this.currentAction = this.tAction(start_action, kind); this.currentAction <= this.NUM_RULES; this.currentAction = this.tAction(this.currentAction, kind)) {
            this.stateStackTop--;
            do {
              this.stateStackTop -= this.prs.rhs(this.currentAction) - 1;
              this.ra.ruleAction(this.currentAction);
              let lhs_symbol = this.prs.lhs(this.currentAction);
              if (lhs_symbol == this.START_SYMBOL) {
                this.currentAction = this.starttok == token ? this.ERROR_ACTION : this.ACCEPT_ACTION;
                break Replay;
              }
              this.currentAction = this.prs.ntAction(this.stack[this.stateStackTop], lhs_symbol);
            } while (this.currentAction <= this.NUM_RULES);
            if (++this.stateStackTop >= this.stack.length) {
              this.reallocateStacks();
            }
            this.stack[this.stateStackTop] = this.currentAction;
            this.locationStack[this.stateStackTop] = token;
          }
        }
        return;
      }
      //
      // keep looking ahead until we compute a valid action
      //
      lookahead(act, token) {
        act = this.prs.lookAhead(act - this.LA_STATE_OFFSET, this.tokStream.getKind(token));
        return act > this.LA_STATE_OFFSET ? this.lookahead(act, this.tokStream.getNext(token)) : act;
      }
      //
      // Compute the next action defined on act and sym. If this
      // action requires more lookahead, these lookahead symbols
      // are in the token stream beginning at the next token that
      // is yielded by peek().
      //
      tAction(act, sym) {
        act = this.prs.tAction(act, sym);
        return act > this.LA_STATE_OFFSET ? this.lookahead(act, this.tokStream.peek()) : act;
      }
      scanNextToken2() {
        return this.lexNextToken(this.tokStream.getStreamLength());
      }
      scanNextToken(start_offset) {
        if (!start_offset) {
          return this.scanNextToken2();
        }
        this.resetTokenStream(start_offset);
        return this.lexNextToken(this.tokStream.getStreamLength());
      }
      /**
       * Scan the next token during incremental re-lex. Returns the parser stream
       * index of the next unscanned character (curtok).
       */
      incrementalParseCharacters() {
        this.scanNextToken();
        return this.curtok;
      }
      lexNextToken(end_offset) {
        this.taking_actions = false;
        this.stateStackTop = -1;
        this.currentAction = this.START_STATE;
        this.starttok = this.curtok;
        this.action.reset();
        ScanToken: for (; ; ) {
          if (++this.stateStackTop >= this.stack.length) {
            this.reallocateStacks();
          }
          this.stack[this.stateStackTop] = this.currentAction;
          this.currentAction = this.lexNextCharacter(this.currentAction, this.current_kind);
          if (this.currentAction == this.ERROR_ACTION && this.current_kind != this.EOFT_SYMBOL) {
            let save_next_token = this.tokStream.peek();
            this.tokStream.reset(this.tokStream.getStreamLength() - 1);
            this.currentAction = this.lexNextCharacter(this.stack[this.stateStackTop], this.EOFT_SYMBOL);
            this.tokStream.reset(save_next_token);
          }
          this.action.add(this.currentAction);
          if (this.currentAction > this.ERROR_ACTION) {
            this.curtok = this.tokStream.getToken();
            if (this.curtok > end_offset)
              this.curtok = this.tokStream.getStreamLength();
            this.current_kind = this.tokStream.getKind(this.curtok);
            this.currentAction -= this.ERROR_ACTION;
            do {
              let lhs_symbol = this.prs.lhs(this.currentAction);
              if (lhs_symbol == this.START_SYMBOL) {
                this.parseActions();
                return true;
              }
              this.stateStackTop -= this.prs.rhs(this.currentAction) - 1;
              this.currentAction = this.prs.ntAction(this.stack[this.stateStackTop], lhs_symbol);
            } while (this.currentAction <= this.NUM_RULES);
          } else if (this.currentAction < this.ACCEPT_ACTION) {
            this.curtok = this.tokStream.getToken();
            if (this.curtok > end_offset)
              this.curtok = this.tokStream.getStreamLength();
            this.current_kind = this.tokStream.getKind(this.curtok);
          } else if (this.currentAction == this.ACCEPT_ACTION)
            return true;
          else
            break ScanToken;
        }
        if (this.starttok == this.curtok) {
          if (this.current_kind == this.EOFT_SYMBOL) {
            this.action = new IntTuple_1.IntTuple(0);
            return false;
          }
          this.lastToken = this.curtok;
          this.tokStream.reportLexicalError(this.starttok, this.curtok);
          this.curtok = this.tokStream.getToken();
          if (this.curtok > end_offset)
            this.curtok = this.tokStream.getStreamLength();
          this.current_kind = this.tokStream.getKind(this.curtok);
        } else {
          this.lastToken = this.tokStream.getPrevious(this.curtok);
          this.tokStream.reportLexicalError(this.starttok, this.lastToken);
        }
        return true;
      }
      //
      // This function takes as argument a configuration ([this.stack, stackTop], [this.tokStream, this.curtok])
      // and determines whether or not the reduce this.action the this.curtok can be validly parsed in this
      // configuration.
      //
      lexNextCharacter(act, kind) {
        let action_save = this.action.size(), pos = this.stateStackTop, tempStackTop = this.stateStackTop - 1;
        act = this.tAction(act, kind);
        Scan: while (act <= this.NUM_RULES) {
          this.action.add(act);
          do {
            let lhs_symbol = this.prs.lhs(act);
            if (lhs_symbol == this.START_SYMBOL) {
              if (this.starttok == this.curtok) {
                act = this.ERROR_ACTION;
                break Scan;
              } else {
                this.parseActions();
                return this.ACCEPT_ACTION;
              }
            }
            tempStackTop -= this.prs.rhs(act) - 1;
            let state = tempStackTop > pos ? this.tempStack[tempStackTop] : this.stack[tempStackTop];
            act = this.prs.ntAction(state, lhs_symbol);
          } while (act <= this.NUM_RULES);
          if (tempStackTop + 1 >= this.stack.length)
            this.reallocateStacks();
          pos = pos < tempStackTop ? pos : tempStackTop;
          this.tempStack[tempStackTop + 1] = act;
          act = this.tAction(act, kind);
        }
        if (act == this.ERROR_ACTION)
          this.action.reset(action_save);
        else {
          this.stateStackTop = tempStackTop + 1;
          for (let i = pos + 1; i <= this.stateStackTop; i++)
            this.stack[i] = this.tempStack[i];
        }
        return act;
      }
      //
      // Now do the final parse of the input based on the actions in
      // the list "this.action" and the sequence of tokens in the token stream.
      //
      parseActions() {
        this.taking_actions = true;
        this.curtok = this.starttok;
        this.lastToken = this.tokStream.getPrevious(this.curtok);
        this.stateStackTop = -1;
        this.currentAction = this.START_STATE;
        process_actions: for (let i = 0; i < this.action.size(); i++) {
          this.stack[++this.stateStackTop] = this.currentAction;
          this.locationStack[this.stateStackTop] = this.curtok;
          this.currentAction = this.action.get(i);
          if (this.currentAction <= this.NUM_RULES) {
            this.stateStackTop--;
            do {
              this.stateStackTop -= this.prs.rhs(this.currentAction) - 1;
              this.ra.ruleAction(this.currentAction);
              let lhs_symbol = this.prs.lhs(this.currentAction);
              if (lhs_symbol == this.START_SYMBOL) {
                break process_actions;
              }
              this.currentAction = this.prs.ntAction(this.stack[this.stateStackTop], lhs_symbol);
            } while (this.currentAction <= this.NUM_RULES);
          } else {
            this.lastToken = this.curtok;
            this.curtok = this.tokStream.getNext(this.curtok);
            if (this.currentAction > this.ERROR_ACTION) {
              this.current_kind = this.tokStream.getKind(this.curtok);
              this.currentAction -= this.ERROR_ACTION;
              do {
                this.stateStackTop -= this.prs.rhs(this.currentAction) - 1;
                this.ra.ruleAction(this.currentAction);
                let lhs_symbol = this.prs.lhs(this.currentAction);
                if (lhs_symbol == this.START_SYMBOL)
                  break process_actions;
                this.currentAction = this.prs.ntAction(this.stack[this.stateStackTop], lhs_symbol);
              } while (this.currentAction <= this.NUM_RULES);
            }
          }
        }
        this.taking_actions = false;
        return;
      }
    };
    exports.LexParser = LexParser;
  }
});

// playground/fs-stub.js
var fs_stub_exports = {};
__export(fs_stub_exports, {
  default: () => fs_stub_default,
  readFileSync: () => readFileSync
});
function readFileSync() {
  throw new Error("fs-extra.readFileSync is not available in the browser playground");
}
var fs_stub_default;
var init_fs_stub = __esm({
  "playground/fs-stub.js"() {
    fs_stub_default = { readFileSync };
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/LexStream.js
var require_LexStream = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/LexStream.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.LexStream = void 0;
    var fs = (init_fs_stub(), __toCommonJS(fs_stub_exports));
    var ParseErrorCodes_1 = require_ParseErrorCodes();
    var IntSegmentedTuple_1 = require_IntSegmentedTuple();
    var LexStream2 = class _LexStream {
      constructor(fileName, inputChars, tab = _LexStream.DEFAULT_TAB, lineOffsets) {
        this.m77Ac341Feebeb7C0A7Ff8F9C6540531500693Bac = 0;
        this.index = -1;
        this.streamLength = 0;
        this.inputChars = "";
        this.fileName = "";
        this.tab = _LexStream.DEFAULT_TAB;
        this.lineOffsets = new IntSegmentedTuple_1.IntSegmentedTuple(12);
        this.setLineOffset(-1);
        this.tab = tab;
        this.initialize(fileName, inputChars, lineOffsets);
      }
      initialize(fileName, inputChars, lineOffsets) {
        if (!inputChars) {
          try {
            inputChars = fs.readFileSync(fileName, "ascii");
          } catch ($ex$) {
            if ($ex$ instanceof Error) {
              let e = $ex$;
              let io = new Error();
              console.error(e.message);
              console.error(e);
              throw io;
            } else {
              throw $ex$;
            }
          }
        }
        if (!inputChars)
          return;
        this.setInputChars(inputChars);
        this.setStreamLength(inputChars.length);
        this.setFileName(fileName);
        if (lineOffsets) {
          this.lineOffsets = lineOffsets;
        } else {
          this.computeLineOffsets();
        }
      }
      computeLineOffsets() {
        this.lineOffsets.reset();
        this.setLineOffset(-1);
        for (let i = 0; i < this.inputChars.length; i++) {
          if (this.inputChars.charCodeAt(i) == 10) {
            this.setLineOffset(i);
          }
        }
      }
      setInputChars(inputChars) {
        this.inputChars = inputChars;
        this.index = -1;
      }
      getInputChars() {
        return this.inputChars;
      }
      setFileName(fileName) {
        this.fileName = fileName;
      }
      getFileName() {
        return this.fileName;
      }
      setLineOffsets(lineOffsets) {
        this.lineOffsets = lineOffsets;
      }
      getLineOffsets() {
        return this.lineOffsets;
      }
      setTab(tab) {
        this.tab = tab;
      }
      getTab() {
        return this.tab;
      }
      setStreamIndex(index) {
        this.index = index;
      }
      getStreamIndex() {
        return this.index;
      }
      setStreamLength(streamLength) {
        this.streamLength = streamLength;
      }
      getStreamLength() {
        return this.streamLength;
      }
      setLineOffset(i) {
        this.lineOffsets.add(i);
      }
      getLineOffset(i) {
        return this.lineOffsets.get(i);
      }
      setPrsStream(prsStream) {
        prsStream.setLexStream(this);
        this.prsStream = prsStream;
      }
      getIPrsStream() {
        return this.prsStream;
      }
      orderedExportedSymbols() {
        return [];
      }
      getCharValue(i) {
        return this.inputChars[i];
      }
      getIntValue(i) {
        return this.inputChars.charCodeAt(i);
      }
      getLineCount() {
        return this.lineOffsets.size() - 1;
      }
      getLineNumberOfCharAt(i) {
        let index = this.lineOffsets.binarySearch(i);
        return index < 0 ? -index : index == 0 ? 1 : index;
      }
      getColumnOfCharAt(i) {
        let lineNo = this.getLineNumberOfCharAt(i), start = this.lineOffsets.get(lineNo - 1);
        if (start + 1 >= this.streamLength) {
          return 1;
        }
        for (let k = start + 1; k < i; k++) {
          if (this.inputChars[k] == "	") {
            let offset = k - start - 1;
            start -= this.tab - 1 - offset % this.tab;
          }
        }
        return i - start;
      }
      getToken2() {
        return this.index = this.getNext(this.index);
      }
      getToken(end_token) {
        if (!end_token) {
          return this.getToken2();
        }
        return this.index = this.index < end_token ? this.getNext(this.index) : this.streamLength;
      }
      getKind(i) {
        return 0;
      }
      next(i) {
        return this.getNext(i);
      }
      getNext(i) {
        return ++i < this.streamLength ? i : this.streamLength;
      }
      previous(i) {
        return this.getPrevious(i);
      }
      getPrevious(i) {
        return i <= 0 ? 0 : i - 1;
      }
      getName(i) {
        return i >= this.getStreamLength() ? "" : "" + this.getCharValue(i);
      }
      peek() {
        return this.getNext(this.index);
      }
      reset(i) {
        if (i)
          this.index = i - 1;
        else {
          this.index = -1;
        }
      }
      badToken() {
        return 0;
      }
      getLine(i) {
        if (!i) {
          return this.getLine2();
        }
        return this.getLineNumberOfCharAt(i);
      }
      getLine2() {
        return this.getLineCount();
      }
      getColumn(i) {
        return this.getColumnOfCharAt(i);
      }
      getEndLine(i) {
        return this.getLine(i);
      }
      getEndColumn(i) {
        return this.getColumnOfCharAt(i);
      }
      afterEol(i) {
        return i < 1 ? true : this.getLineNumberOfCharAt(i - 1) < this.getLineNumberOfCharAt(i);
      }
      getFirstErrorToken(i) {
        return this.getFirstRealToken(i);
      }
      getFirstRealToken(i) {
        return i;
      }
      getLastErrorToken(i) {
        return this.getLastRealToken(i);
      }
      getLastRealToken(i) {
        return i;
      }
      setMessageHandler(errMsg) {
        this.errMsg = errMsg;
      }
      getMessageHandler() {
        return this.errMsg;
      }
      makeToken(startLoc, endLoc, kind) {
        if (this.prsStream) {
          this.prsStream.makeToken(startLoc, endLoc, kind);
        } else {
          this.reportLexicalError(startLoc, endLoc);
        }
      }
      getLocation(left_loc, right_loc) {
        let length = (right_loc < this.streamLength ? right_loc : this.streamLength - 1) - left_loc + 1;
        return new Int32Array([left_loc, length, this.getLineNumberOfCharAt(left_loc), this.getColumnOfCharAt(left_loc), this.getLineNumberOfCharAt(right_loc), this.getColumnOfCharAt(right_loc)]);
      }
      reportLexicalError(left_loc, right_loc, errorCode, error_left_loc_arg, error_right_loc_arg, errorInfoArg) {
        let error_left_loc = 0;
        if (error_left_loc_arg) {
          error_left_loc = error_left_loc_arg;
        }
        let error_right_loc = 0;
        if (error_right_loc_arg) {
          error_right_loc = error_right_loc_arg;
        }
        let errorInfo = [];
        if (errorInfoArg) {
          errorInfo = errorInfoArg;
        }
        if (!errorCode) {
          errorCode = right_loc >= this.streamLength ? ParseErrorCodes_1.ParseErrorCodes.EOF_CODE : left_loc == right_loc ? ParseErrorCodes_1.ParseErrorCodes.LEX_ERROR_CODE : ParseErrorCodes_1.ParseErrorCodes.INVALID_TOKEN_CODE;
          let tokenText = errorCode == ParseErrorCodes_1.ParseErrorCodes.EOF_CODE ? "End-of-file " : errorCode == ParseErrorCodes_1.ParseErrorCodes.INVALID_TOKEN_CODE ? '"' + this.inputChars.slice(left_loc, left_loc + right_loc - left_loc + 1) + '" ' : '"' + this.getCharValue(left_loc) + '" ';
          error_left_loc = 0;
          error_right_loc = 0;
          errorInfo = [tokenText];
        }
        if (!this.errMsg) {
          let locationInfo = this.getFileName() + ":" + this.getLineNumberOfCharAt(left_loc) + ":" + this.getColumnOfCharAt(left_loc) + ":" + this.getLineNumberOfCharAt(right_loc) + ":" + this.getColumnOfCharAt(right_loc) + ":" + error_left_loc + ":" + error_right_loc + ":" + errorCode + ": ";
          console.log("****Error: " + locationInfo);
          if (errorInfo) {
            for (let i = 0; i < errorInfo.length; i++) {
              console.log(errorInfo[i] + " ");
            }
          }
          console.log(ParseErrorCodes_1.errorMsgText[errorCode]);
        } else {
          this.errMsg.handleMessage(errorCode, this.getLocation(left_loc, right_loc), this.getLocation(error_left_loc, error_right_loc), this.getFileName(), errorInfo);
        }
      }
      reportError(errorCode, leftToken, rightToken, errorInfo, errorToken = 0) {
        let tempInfo;
        if (typeof errorInfo == "string") {
          tempInfo = [errorInfo];
        } else if (Array.isArray(errorInfo)) {
          tempInfo = errorInfo;
        } else {
          tempInfo = [];
        }
        this.reportLexicalError(leftToken, rightToken, errorCode, errorToken, errorToken, tempInfo);
      }
      toString(startOffset, endOffset) {
        let length = endOffset - startOffset + 1;
        return endOffset >= this.inputChars.length ? "$EOF" : length <= 0 ? "" : this.inputChars.slice(startOffset, startOffset + length);
      }
    };
    exports.LexStream = LexStream2;
    LexStream2.DEFAULT_TAB = 1;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/LpgLexStream.js
var require_LpgLexStream = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/LpgLexStream.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.LpgLexStream = void 0;
    var LexStream_1 = require_LexStream();
    var LpgLexStream = class extends LexStream_1.LexStream {
      constructor(fileName, inputChars, tab = LexStream_1.LexStream.DEFAULT_TAB, lineOffsets) {
        super(fileName, inputChars, tab, lineOffsets);
      }
    };
    exports.LpgLexStream = LpgLexStream;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/MismatchedInputCharsException.js
var require_MismatchedInputCharsException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/MismatchedInputCharsException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.MismatchedInputCharsException = void 0;
    var MismatchedInputCharsException = class extends Error {
      constructor(str) {
        super();
        if (!str) {
          this.str = "MismatchedInputCharsException";
        } else {
          this.str = str;
        }
      }
      toString() {
        return this.str;
      }
    };
    exports.MismatchedInputCharsException = MismatchedInputCharsException;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/Monitor.js
var require_Monitor = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/Monitor.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/NullExportedSymbolsException.js
var require_NullExportedSymbolsException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/NullExportedSymbolsException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.NullExportedSymbolsException = void 0;
    var NullExportedSymbolsException2 = class extends Error {
      constructor(str) {
        super();
        if (!str) {
          this.str = "NullExportedSymbolsException";
        } else {
          this.str = str;
        }
      }
      toString() {
        return this.str;
      }
    };
    exports.NullExportedSymbolsException = NullExportedSymbolsException2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/NullTerminalSymbolsException.js
var require_NullTerminalSymbolsException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/NullTerminalSymbolsException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.NullTerminalSymbolsException = void 0;
    var NullTerminalSymbolsException2 = class extends Error {
      constructor(str) {
        super();
        if (!str) {
          this.str = "NullTerminalSymbolsException";
        } else {
          this.str = str;
        }
      }
      toString() {
        return this.str;
      }
    };
    exports.NullTerminalSymbolsException = NullTerminalSymbolsException2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/ProstheticAst.js
var require_ProstheticAst = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/ProstheticAst.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/UndefinedEofSymbolException.js
var require_UndefinedEofSymbolException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/UndefinedEofSymbolException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.UndefinedEofSymbolException = void 0;
    var UndefinedEofSymbolException2 = class extends Error {
      constructor(str) {
        super();
        if (!str) {
          this.str = "UndefinedEofSymbolException";
        } else {
          this.str = str;
        }
      }
      toString() {
        return this.str;
      }
    };
    exports.UndefinedEofSymbolException = UndefinedEofSymbolException2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/UnimplementedTerminalsException.js
var require_UnimplementedTerminalsException = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/UnimplementedTerminalsException.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.UnimplementedTerminalsException = void 0;
    var UnimplementedTerminalsException2 = class extends Error {
      constructor(symbols) {
        super();
        this.symbols = symbols;
      }
      getSymbols() {
        return this.symbols;
      }
    };
    exports.UnimplementedTerminalsException = UnimplementedTerminalsException2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/PrsStream.js
var require_PrsStream = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/PrsStream.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.PrsStream = void 0;
    var LexStream_1 = require_LexStream();
    var Token_1 = require_Token();
    var ErrorToken_1 = require_ErrorToken();
    var Adjunct_1 = require_Adjunct();
    var Utils_1 = require_Utils();
    var NullTerminalSymbolsException_1 = require_NullTerminalSymbolsException();
    var UndefinedEofSymbolException_1 = require_UndefinedEofSymbolException();
    var UnimplementedTerminalsException_1 = require_UnimplementedTerminalsException();
    var Protocol_1 = require_Protocol();
    var PrsStream2 = class {
      constructor(iLexStream) {
        this.m3C89586D99F2567D21410F29B1B2606574892Aa7 = 0;
        this.iLexStream = new Protocol_1.EscapeStrictPropertyInitializationLexStream();
        this.kindMap = new Int32Array(0);
        this.tokens = new Utils_1.Lpg.Util.ArrayList();
        this.adjuncts = new Utils_1.Lpg.Util.ArrayList();
        this.index = 0;
        this.len = 0;
        if (iLexStream) {
          this.iLexStream = iLexStream;
          iLexStream.setPrsStream(this);
          this.resetTokenStream();
        }
      }
      orderedExportedSymbols() {
        return [];
      }
      remapTerminalSymbols(ordered_parser_symbols, eof_symbol) {
        if (!this.iLexStream || this.iLexStream instanceof Protocol_1.EscapeStrictPropertyInitializationLexStream) {
          throw new ReferenceError("PrsStream.remapTerminalSymbols(..):  lexStream is undefined");
        }
        let ordered_lexer_symbols = this.iLexStream.orderedExportedSymbols();
        if (ordered_lexer_symbols == void 0) {
          throw new NullTerminalSymbolsException_1.NullTerminalSymbolsException();
        }
        if (ordered_parser_symbols == void 0) {
          throw new NullTerminalSymbolsException_1.NullTerminalSymbolsException();
        }
        let unimplemented_symbols = new Utils_1.Lpg.Util.ArrayList();
        if (ordered_lexer_symbols != ordered_parser_symbols) {
          this.kindMap = new Int32Array(ordered_lexer_symbols.length);
          let terminal_map = /* @__PURE__ */ new Map();
          for (let i = 0; i < ordered_lexer_symbols.length; i++) {
            terminal_map.set(ordered_lexer_symbols[i], i);
          }
          for (let i = 0; i < ordered_parser_symbols.length; i++) {
            let k = terminal_map.get(ordered_parser_symbols[i]);
            if (k != void 0) {
              this.kindMap[k] = i;
            } else {
              if (i == eof_symbol) {
                throw new UndefinedEofSymbolException_1.UndefinedEofSymbolException();
              }
              unimplemented_symbols.add(i);
            }
          }
        }
        if (unimplemented_symbols.size() > 0) {
          throw new UnimplementedTerminalsException_1.UnimplementedTerminalsException(unimplemented_symbols);
        }
      }
      mapKind(kind) {
        return this.kindMap.length == 0 ? kind : this.kindMap[kind];
      }
      resetTokenStream() {
        this.tokens = new Utils_1.Lpg.Util.ArrayList();
        this.index = 0;
        this.adjuncts = new Utils_1.Lpg.Util.ArrayList();
      }
      setLexStream(lexStream) {
        this.iLexStream = lexStream;
        this.resetTokenStream();
      }
      resetLexStream(lexStream) {
        if (lexStream) {
          lexStream.setPrsStream(this);
          this.iLexStream = lexStream;
        }
      }
      makeToken(startLoc, endLoc, kind) {
        let token = new Token_1.Token(startLoc, endLoc, this.mapKind(kind), this);
        token.setTokenIndex(this.tokens.size());
        this.tokens.add(token);
        token.setAdjunctIndex(this.adjuncts.size());
      }
      removeLastToken() {
        let last_index = this.tokens.size() - 1;
        let token = this.tokens.get(last_index);
        let adjuncts_size = this.adjuncts.size();
        while (adjuncts_size > token.getAdjunctIndex()) {
          this.adjuncts.remove(--adjuncts_size);
        }
        this.tokens.remove(last_index);
      }
      makeErrorToken(firsttok, lasttok, errortok, kind) {
        let index = this.tokens.size();
        let token = new ErrorToken_1.ErrorToken(this.getIToken(firsttok), this.getIToken(lasttok), this.getIToken(errortok), this.getStartOffset(firsttok), this.getEndOffset(lasttok), kind);
        token.setTokenIndex(this.tokens.size());
        this.tokens.add(token);
        token.setAdjunctIndex(this.adjuncts.size());
        return index;
      }
      addToken(token) {
        token.setTokenIndex(this.tokens.size());
        this.tokens.add(token);
        token.setAdjunctIndex(this.adjuncts.size());
      }
      makeAdjunct(startLoc, endLoc, kind) {
        let token_index = this.tokens.size() - 1;
        let adjunct = new Adjunct_1.Adjunct(startLoc, endLoc, this.mapKind(kind), this);
        adjunct.setAdjunctIndex(this.adjuncts.size());
        adjunct.setTokenIndex(token_index);
        this.adjuncts.add(adjunct);
      }
      addAdjunct(adjunct) {
        let token_index = this.tokens.size() - 1;
        adjunct.setTokenIndex(token_index);
        adjunct.setAdjunctIndex(this.adjuncts.size());
        this.adjuncts.add(adjunct);
      }
      getTokenText(i) {
        let t = this.tokens.get(i);
        return t.toString();
      }
      getStartOffset(i) {
        let t = this.tokens.get(i);
        return t.getStartOffset();
      }
      getEndOffset(i) {
        let t = this.tokens.get(i);
        return t.getEndOffset();
      }
      getTokenLength(i) {
        let t = this.tokens.get(i);
        return t.getEndOffset() - t.getStartOffset() + 1;
      }
      getLineNumberOfTokenAt(i) {
        var _a;
        if (!this.iLexStream)
          return 0;
        let t = this.tokens.get(i);
        return (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.getLineNumberOfCharAt(t.getStartOffset());
      }
      getEndLineNumberOfTokenAt(i) {
        var _a;
        if (!this.iLexStream)
          return 0;
        let t = this.tokens.get(i);
        return (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.getLineNumberOfCharAt(t.getEndOffset());
      }
      getColumnOfTokenAt(i) {
        var _a;
        if (!this.iLexStream)
          return 0;
        let t = this.tokens.get(i);
        return (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.getColumnOfCharAt(t.getStartOffset());
      }
      getEndColumnOfTokenAt(i) {
        var _a;
        if (!this.iLexStream)
          return 0;
        let t = this.tokens.get(i);
        return (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.getColumnOfCharAt(t.getEndOffset());
      }
      orderedTerminalSymbols() {
        return [];
      }
      getLineOffset(i) {
        var _a;
        if (!this.iLexStream)
          return 0;
        return (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.getLineOffset(i);
      }
      getLineCount() {
        var _a;
        if (!this.iLexStream)
          return 0;
        return (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.getLineCount();
      }
      getLineNumberOfCharAt(i) {
        var _a;
        if (!this.iLexStream)
          return 0;
        return (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.getLineNumberOfCharAt(i);
      }
      getColumnOfCharAt(i) {
        var _a;
        return (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.getColumnOfCharAt(i);
      }
      getFirstErrorToken(i) {
        return this.getFirstRealToken(i);
      }
      getFirstRealToken(i) {
        while (i >= this.len) {
          i = this.tokens.get(i).getFirstRealToken().getTokenIndex();
        }
        return i;
      }
      getLastErrorToken(i) {
        return this.getLastRealToken(i);
      }
      getLastRealToken(i) {
        while (i >= this.len) {
          i = this.tokens.get(i).getLastRealToken().getTokenIndex();
        }
        return i;
      }
      getInputChars() {
        return this.iLexStream instanceof LexStream_1.LexStream ? this.iLexStream.getInputChars() : "";
      }
      getInputBytes() {
        return new Int8Array(0);
      }
      toStringFromIndex(first_token, last_token) {
        return this.toString(this.tokens.get(first_token), this.tokens.get(last_token));
      }
      toString(t1, t2) {
        var _a;
        if (!this.iLexStream)
          return "";
        return (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.toString(t1.getStartOffset(), t2.getEndOffset());
      }
      getSize() {
        return this.tokens.size();
      }
      setSize() {
        this.len = this.tokens.size();
      }
      getTokenIndexAtCharacter(offset) {
        let low = 0, high = this.tokens.size();
        while (high > low) {
          let mid = Math.floor((high + low) / 2);
          let mid_element = this.tokens.get(mid);
          if (offset >= mid_element.getStartOffset() && offset <= mid_element.getEndOffset()) {
            return mid;
          } else {
            if (offset < mid_element.getStartOffset()) {
              high = mid;
            } else {
              low = mid + 1;
            }
          }
        }
        return -(low - 1);
      }
      getTokenAtCharacter(offset) {
        let tokenIndex = this.getTokenIndexAtCharacter(offset);
        return tokenIndex < 0 ? void 0 : this.getTokenAt(tokenIndex);
      }
      getTokenAt(i) {
        return this.tokens.get(i);
      }
      getIToken(i) {
        return this.tokens.get(i);
      }
      getTokens() {
        return this.tokens;
      }
      getStreamIndex() {
        return this.index;
      }
      getStreamLength() {
        return this.len;
      }
      setStreamIndex(index) {
        this.index = index;
      }
      setStreamLength2() {
        this.len = this.tokens.size();
      }
      setStreamLength(len) {
        if (typeof len == "undefined") {
          this.setStreamLength2();
          return;
        }
        this.len = len;
      }
      getILexStream() {
        return this.iLexStream;
      }
      getLexStream() {
        return this.iLexStream;
      }
      dumpTokens() {
        if (this.getSize() <= 2) {
          return;
        }
        Utils_1.Lpg.Lang.System.Out.println(" Kind 	Offset 	Len 	Line 	Col 	Text\n");
        for (let i = 1; i < this.getSize() - 1; i++) {
          this.dumpToken(i);
        }
      }
      dumpToken(i) {
        console.log(" (" + this.getKind(i) + ")");
        console.log(" 	" + this.getStartOffset(i));
        console.log(" 	" + this.getTokenLength(i));
        console.log(" 	" + this.getLineNumberOfTokenAt(i));
        console.log(" 	" + this.getColumnOfTokenAt(i));
        console.log(" 	" + this.getTokenText(i));
        console.log("\n");
      }
      getAdjunctsFromIndex(i) {
        let start_index = this.tokens.get(i).getAdjunctIndex(), end_index = i + 1 == this.tokens.size() ? this.adjuncts.size() : this.tokens.get(this.getNext(i)).getAdjunctIndex(), size = end_index - start_index;
        let slice = new Array(size);
        for (let j = start_index, k = 0; j < end_index; j++, k++) {
          slice[k] = this.adjuncts.get(j);
        }
        return slice;
      }
      getFollowingAdjuncts(i) {
        return this.getAdjunctsFromIndex(i);
      }
      getPrecedingAdjuncts(i) {
        return this.getAdjunctsFromIndex(this.getPrevious(i));
      }
      getAdjuncts() {
        return this.adjuncts;
      }
      getToken2() {
        this.index = this.getNext(this.index);
        return this.index;
      }
      getToken(end_token) {
        if (!end_token) {
          return this.getToken2();
        }
        return this.index = this.index < end_token ? this.getNext(this.index) : this.len - 1;
      }
      getKind(i) {
        let t = this.tokens.get(i);
        return t.getKind();
      }
      getNext(i) {
        return ++i < this.len ? i : this.len - 1;
      }
      getPrevious(i) {
        return i <= 0 ? 0 : i - 1;
      }
      getName(i) {
        return this.getTokenText(i);
      }
      peek() {
        return this.getNext(this.index);
      }
      reset1() {
        this.index = 0;
      }
      reset2(i) {
        this.index = this.getPrevious(i);
      }
      reset(i) {
        if (!i) {
          this.reset1();
        } else {
          this.reset2(i);
        }
      }
      badToken() {
        return 0;
      }
      getLine(i) {
        return this.getLineNumberOfTokenAt(i);
      }
      getColumn(i) {
        return this.getColumnOfTokenAt(i);
      }
      getEndLine(i) {
        return this.getEndLineNumberOfTokenAt(i);
      }
      getEndColumn(i) {
        return this.getEndColumnOfTokenAt(i);
      }
      afterEol(i) {
        return i < 1 ? true : this.getEndLineNumberOfTokenAt(i - 1) < this.getLineNumberOfTokenAt(i);
      }
      getFileName() {
        var _a;
        if (!this.iLexStream)
          return "";
        return (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.getFileName();
      }
      //
      // Here is where we report errors.  The default method is simply to print the error message to the console.
      // However, the user may supply an error message handler to process error messages.  To support that
      // a message handler interface is provided that has a single method handleMessage().  The user has his
      // error message handler class implement the IMessageHandler interface and provides an object of this type
      // to the runtime using the setMessageHandler(errorMsg) method. If the message handler object is set,
      // the reportError methods will invoke its handleMessage() method.
      //
      // IMessageHandler errMsg = null; // the error message handler object is declared in LexStream
      //
      setMessageHandler(errMsg) {
        var _a;
        (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.setMessageHandler(errMsg);
      }
      getMessageHandler() {
        var _a;
        return (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.getMessageHandler();
      }
      reportError(errorCode, leftToken, rightToken, errorInfo, errorToken = 0) {
        var _a;
        let tempInfo;
        if (typeof errorInfo == "string") {
          tempInfo = [errorInfo];
        } else if (Array.isArray(errorInfo)) {
          tempInfo = errorInfo;
        } else {
          tempInfo = [];
        }
        (_a = this.iLexStream) === null || _a === void 0 ? void 0 : _a.reportLexicalError(this.getStartOffset(leftToken), this.getEndOffset(rightToken), errorCode, this.getStartOffset(errorToken), this.getEndOffset(errorToken), tempInfo);
      }
      truncateList(list, newSize) {
        while (list.size() > newSize) {
          list.remove(list.size() - 1);
        }
      }
      /**
       * Reset the token stream at a character damage offset for incremental re-lex.
       * Prefix tokens before the damage point are retained; the suffix is discarded
       * (returned as affected tokens — not tree-sitter subtree reuse).
       */
      incrementalResetAtCharacterOffset(damage_offset) {
        let token_index = this.getTokenIndexAtCharacter(damage_offset);
        let adjunct_index = -1;
        token_index = token_index < 0 ? -token_index : token_index;
        if (this.getTokenAt(token_index).getEndOffset() + 1 < damage_offset) {
          for (let i = this.getTokenAt(token_index).getAdjunctIndex(); i < this.adjuncts.size() && this.adjuncts.get(i).getTokenIndex() == token_index; i++) {
            if (this.adjuncts.get(i).getStartOffset() < damage_offset) {
              adjunct_index = i;
            } else {
              break;
            }
          }
        }
        const affected_tokens = [];
        if (adjunct_index >= 0) {
          token_index++;
          for (let i = adjunct_index; i < this.getTokenAt(token_index).getAdjunctIndex(); i++) {
            affected_tokens.push(this.adjuncts.get(i));
          }
          this.truncateList(this.adjuncts, adjunct_index);
        } else {
          this.truncateList(this.adjuncts, this.getTokenAt(token_index).getAdjunctIndex());
        }
        for (let i = token_index; i < this.tokens.size() - 1; i++) {
          affected_tokens.push(this.getTokenAt(i));
          for (let k = this.getTokenAt(i).getAdjunctIndex(); k < this.getTokenAt(i + 1).getAdjunctIndex(); k++) {
            affected_tokens.push(this.adjuncts.get(k));
          }
        }
        affected_tokens.push(this.getTokenAt(this.tokens.size() - 1));
        this.truncateList(this.tokens, token_index);
        return affected_tokens;
      }
    };
    exports.PrsStream = PrsStream2;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/UnknownStreamType.js
var require_UnknownStreamType = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/UnknownStreamType.js"(exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.UnknownStreamType = void 0;
    var UnknownStreamType = class extends Error {
      constructor(str) {
        super();
        if (!str) {
          this.str = "UnknownStreamType";
        } else {
          this.str = str;
        }
      }
      toString() {
        return this.str;
      }
    };
    exports.UnknownStreamType = UnknownStreamType;
  }
});

// runtime/LPG-typescript-runtime/lpg2ts/dist/index.js
var require_dist = __commonJS({
  "runtime/LPG-typescript-runtime/lpg2ts/dist/index.js"(exports) {
    "use strict";
    var __createBinding = exports && exports.__createBinding || (Object.create ? function(o, m, k, k2) {
      if (k2 === void 0) k2 = k;
      var desc = Object.getOwnPropertyDescriptor(m, k);
      if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
        desc = { enumerable: true, get: function() {
          return m[k];
        } };
      }
      Object.defineProperty(o, k2, desc);
    } : function(o, m, k, k2) {
      if (k2 === void 0) k2 = k;
      o[k2] = m[k];
    });
    var __exportStar = exports && exports.__exportStar || function(m, exports2) {
      for (var p in m) if (p !== "default" && !Object.prototype.hasOwnProperty.call(exports2, p)) __createBinding(exports2, m, p);
    };
    Object.defineProperty(exports, "__esModule", { value: true });
    __exportStar(require_AbstractToken(), exports);
    __exportStar(require_Adjunct(), exports);
    __exportStar(require_BacktrackingParser(), exports);
    __exportStar(require_BadParseException(), exports);
    __exportStar(require_BadParseSymFileException(), exports);
    __exportStar(require_GLRParser(), exports);
    __exportStar(require_GssEdge(), exports);
    __exportStar(require_GssNode(), exports);
    __exportStar(require_ConfigurationElement(), exports);
    __exportStar(require_ConfigurationStack(), exports);
    __exportStar(require_DeterministicParser(), exports);
    __exportStar(require_DiagnoseParser(), exports);
    __exportStar(require_ErrorToken(), exports);
    __exportStar(require_ExpectedTokens(), exports);
    __exportStar(require_IAbstractArrayList(), exports);
    __exportStar(require_IAst(), exports);
    __exportStar(require_IAstVisitor(), exports);
    __exportStar(require_IMessageHandler(), exports);
    __exportStar(require_IncrementalParse(), exports);
    __exportStar(require_IntSegmentedTuple(), exports);
    __exportStar(require_IntTuple(), exports);
    __exportStar(require_LexParser(), exports);
    __exportStar(require_LexStream(), exports);
    __exportStar(require_LpgLexStream(), exports);
    __exportStar(require_MismatchedInputCharsException(), exports);
    __exportStar(require_Monitor(), exports);
    __exportStar(require_NotBacktrackParseTableException(), exports);
    __exportStar(require_NotDeterministicParseTableException(), exports);
    __exportStar(require_NotGLRParseTableException(), exports);
    __exportStar(require_NullExportedSymbolsException(), exports);
    __exportStar(require_NullTerminalSymbolsException(), exports);
    __exportStar(require_ObjectTuple(), exports);
    __exportStar(require_ParseErrorCodes(), exports);
    __exportStar(require_ParseTable(), exports);
    __exportStar(require_ProstheticAst(), exports);
    __exportStar(require_Protocol(), exports);
    __exportStar(require_PrsStream(), exports);
    __exportStar(require_RecoveryParser(), exports);
    __exportStar(require_RuleAction(), exports);
    __exportStar(require_Stacks(), exports);
    __exportStar(require_SppfNode(), exports);
    __exportStar(require_StateElement(), exports);
    __exportStar(require_Token(), exports);
    __exportStar(require_TokenStream(), exports);
    __exportStar(require_TokenStreamNotIPrsStreamException(), exports);
    __exportStar(require_UnavailableParserInformationException(), exports);
    __exportStar(require_UndefinedEofSymbolException(), exports);
    __exportStar(require_UnimplementedTerminalsException(), exports);
    __exportStar(require_UnknownStreamType(), exports);
    __exportStar(require_Utils(), exports);
  }
});

// playground/glr-demo/main.ts
var import_lpg2ts2 = __toESM(require_dist());

// playground/glr-demo/glr_expr.ts
var import_lpg2ts = __toESM(require_dist());

// playground/glr-demo/glr_exprsym.ts
var glr_exprsym;
((glr_exprsym2) => {
  glr_exprsym2.TK_NUMBER = 2;
  glr_exprsym2.TK_PLUS = 1;
  glr_exprsym2.TK_EOF_TOKEN = 3;
  glr_exprsym2.TK_ERROR_TOKEN = 4;
  glr_exprsym2.orderedTerminalSymbols = [
    "",
    "PLUS",
    "NUMBER",
    "EOF_TOKEN",
    "ERROR_TOKEN"
  ];
  glr_exprsym2.numTokenKinds = 5;
  glr_exprsym2.RULE_E = 1;
  glr_exprsym2.orderedRuleNames = [
    "",
    "E",
    "E"
  ];
  glr_exprsym2.numRuleNames = 2;
  glr_exprsym2.isValidForParser = true;
})(glr_exprsym || (glr_exprsym = {}));

// playground/glr-demo/glr_exprprs.ts
var glr_exprprs = class _glr_exprprs {
  ERROR_SYMBOL = 4;
  getErrorSymbol() {
    return this.ERROR_SYMBOL;
  }
  SCOPE_UBOUND = -1;
  getScopeUbound() {
    return this.SCOPE_UBOUND;
  }
  SCOPE_SIZE = 0;
  getScopeSize() {
    return this.SCOPE_SIZE;
  }
  MAX_NAME_LENGTH = 11;
  getMaxNameLength() {
    return this.MAX_NAME_LENGTH;
  }
  NUM_STATES = 4;
  getNumStates() {
    return this.NUM_STATES;
  }
  NT_OFFSET = 4;
  getNtOffset() {
    return this.NT_OFFSET;
  }
  LA_STATE_OFFSET = 16;
  getLaStateOffset() {
    return this.LA_STATE_OFFSET;
  }
  MAX_LA = 1;
  getMaxLa() {
    return this.MAX_LA;
  }
  NUM_RULES = 2;
  getNumRules() {
    return this.NUM_RULES;
  }
  NUM_NONTERMINALS = 2;
  getNumNonterminals() {
    return this.NUM_NONTERMINALS;
  }
  NUM_SYMBOLS = 6;
  getNumSymbols() {
    return this.NUM_SYMBOLS;
  }
  START_STATE = 3;
  getStartState() {
    return this.START_STATE;
  }
  IDENTIFIER_SYMBOL = 0;
  getIdentifier_SYMBOL() {
    return this.IDENTIFIER_SYMBOL;
  }
  EOFT_SYMBOL = 3;
  getEoftSymbol() {
    return this.EOFT_SYMBOL;
  }
  EOLT_SYMBOL = 3;
  getEoltSymbol() {
    return this.EOLT_SYMBOL;
  }
  ACCEPT_ACTION = 10;
  getAcceptAction() {
    return this.ACCEPT_ACTION;
  }
  ERROR_ACTION = 14;
  getErrorAction() {
    return this.ERROR_ACTION;
  }
  BACKTRACK = true;
  getBacktrack() {
    return this.BACKTRACK;
  }
  GLR = true;
  isGLR() {
    return this.GLR;
  }
  getStartSymbol() {
    return this.lhs(0);
  }
  isValidForParser() {
    return glr_exprsym.isValidForParser;
  }
  static _isNullable = [
    0,
    0,
    0,
    0,
    0,
    0,
    0
  ];
  isNullable(index) {
    return _glr_exprprs._isNullable[index] != 0;
  }
  static _prosthesesIndex = [
    0,
    2,
    1
  ];
  prosthesesIndex(index) {
    return _glr_exprprs._prosthesesIndex[index];
  }
  static _isKeyword = [
    0,
    0,
    0,
    0,
    0
  ];
  isKeyword(index) {
    return _glr_exprprs._isKeyword[index] != 0;
  }
  static _baseCheck = [
    0,
    3,
    1,
    -1,
    1,
    -3,
    1,
    -2,
    -4,
    0,
    0
  ];
  baseCheck(index) {
    return _glr_exprprs._baseCheck[index];
  }
  static _rhs = _glr_exprprs._baseCheck;
  rhs(index) {
    return _glr_exprprs._rhs[index];
  }
  static _baseAction = [
    1,
    1,
    1,
    3,
    7,
    3,
    8,
    1,
    6,
    14,
    0,
    5,
    1,
    0
  ];
  baseAction(index) {
    return _glr_exprprs._baseAction[index];
  }
  static _lhs = _glr_exprprs._baseAction;
  lhs(index) {
    return _glr_exprprs._lhs[index];
  }
  static _termCheck = [
    0,
    0,
    1,
    0,
    3,
    2,
    0,
    1,
    0,
    0,
    0
  ];
  termCheck(index) {
    return _glr_exprprs._termCheck[index];
  }
  static _termAction = [
    0,
    14,
    5,
    14,
    10,
    16,
    1,
    11
  ];
  termAction(index) {
    return _glr_exprprs._termAction[index];
  }
  static _asb = [
    0,
    1,
    3,
    1,
    3
  ];
  asb(index) {
    return _glr_exprprs._asb[index];
  }
  static _asr = [
    0,
    2,
    0,
    3,
    1,
    0
  ];
  asr(index) {
    return _glr_exprprs._asr[index];
  }
  static _nasb = [
    0,
    1,
    2,
    1,
    2
  ];
  nasb(index) {
    return _glr_exprprs._nasb[index];
  }
  static _nasr = [
    0,
    1,
    0
  ];
  nasr(index) {
    return _glr_exprprs._nasr[index];
  }
  static _terminalIndex = [
    0,
    3,
    2,
    4,
    5
  ];
  terminalIndex(index) {
    return _glr_exprprs._terminalIndex[index];
  }
  static _nonterminalIndex = [
    0,
    6,
    0
  ];
  nonterminalIndex(index) {
    return _glr_exprprs._nonterminalIndex[index];
  }
  static _scopePrefix;
  scopePrefix(index) {
    return 0;
  }
  static _scopeSuffix;
  scopeSuffix(index) {
    return 0;
  }
  static _scopeLhs;
  scopeLhs(index) {
    return 0;
  }
  static _scopeLa;
  scopeLa(index) {
    return 0;
  }
  static _scopeStateSet;
  scopeStateSet(index) {
    return 0;
  }
  static _scopeRhs;
  scopeRhs(index) {
    return 0;
  }
  static _scopeState;
  scopeState(index) {
    return 0;
  }
  static _inSymb;
  inSymb(index) {
    return 0;
  }
  static _name = [
    "",
    "%empty",
    "NUMBER",
    "PLUS",
    "EOF_TOKEN",
    "ERROR_TOKEN",
    "E"
  ];
  name(index) {
    return _glr_exprprs._name[index];
  }
  originalState(state) {
    return -_glr_exprprs._baseCheck[state];
  }
  asi(state) {
    return _glr_exprprs._asb[this.originalState(state)];
  }
  nasi(state) {
    return _glr_exprprs._nasb[this.originalState(state)];
  }
  inSymbol(state) {
    return _glr_exprprs._inSymb[this.originalState(state)];
  }
  /**
   * assert(! goto_default);
   */
  ntAction(state, sym) {
    return _glr_exprprs._baseAction[state + sym];
  }
  /**
   * assert(! shift_default);
   */
  tAction(state, sym) {
    let i = _glr_exprprs._baseAction[state], k = i + sym;
    return _glr_exprprs._termAction[_glr_exprprs._termCheck[k] == sym ? k : i];
  }
  lookAhead(la_state, sym) {
    let k = la_state + sym;
    return _glr_exprprs._termAction[_glr_exprprs._termCheck[k] == sym ? k : la_state];
  }
};

// playground/glr-demo/glr_expr.ts
var glr_expr = class _glr_expr extends Object {
  prsStream = new import_lpg2ts.PrsStream();
  unimplementedSymbolsWarning = false;
  static prsTable = new glr_exprprs();
  getParseTable() {
    return _glr_expr.prsTable;
  }
  glrParser;
  getParser() {
    return this.glrParser;
  }
  // During GLR→BT recover fallback, rule actions must read BT stacks.
  recoverParser = null;
  setRecoverParser(parser) {
    this.recoverParser = parser;
  }
  getRecoverParser() {
    return this.recoverParser;
  }
  setResult(object1) {
    if (this.recoverParser != null) this.recoverParser.setSym1(object1);
    else this.glrParser.setSym1(object1);
  }
  getRhsSym(i) {
    return this.recoverParser != null ? this.recoverParser.getSym(i) : this.glrParser.getSym(i);
  }
  getRhsTokenIndex(i) {
    return this.recoverParser != null ? this.recoverParser.getToken(i) : this.glrParser.getToken(i);
  }
  getRhsIToken(i) {
    return this.prsStream.getIToken(this.getRhsTokenIndex(i));
  }
  getRhsFirstTokenIndex(i) {
    return this.recoverParser != null ? this.recoverParser.getFirstToken(i) : this.glrParser.getFirstToken(i);
  }
  getRhsFirstIToken(i) {
    return this.prsStream.getIToken(this.getRhsFirstTokenIndex(i));
  }
  getRhsLastTokenIndex(i) {
    return this.recoverParser != null ? this.recoverParser.getLastToken(i) : this.glrParser.getLastToken(i);
  }
  getRhsLastIToken(i) {
    return this.prsStream.getIToken(this.getRhsLastTokenIndex(i));
  }
  getLeftSpan() {
    return this.recoverParser != null ? this.recoverParser.getFirstToken() : this.glrParser.getFirstToken();
  }
  getLeftIToken() {
    return this.prsStream.getIToken(this.getLeftSpan());
  }
  getRightSpan() {
    return this.recoverParser != null ? this.recoverParser.getLastToken() : this.glrParser.getLastToken();
  }
  getRightIToken() {
    return this.prsStream.getIToken(this.getRightSpan());
  }
  getRhsErrorTokenIndex(i) {
    let index = this.getRhsTokenIndex(i);
    let err = this.prsStream.getIToken(index);
    return err instanceof import_lpg2ts.ErrorToken ? index : 0;
  }
  getRhsErrorIToken(i) {
    let index = this.getRhsTokenIndex(i);
    let err = this.prsStream.getIToken(index);
    return err instanceof import_lpg2ts.ErrorToken ? err : null;
  }
  reset(lexStream) {
    this.prsStream.resetLexStream(lexStream);
    this.glrParser.reset(this.prsStream);
    try {
      this.prsStream.remapTerminalSymbols(this.orderedTerminalSymbols(), _glr_expr.prsTable.getEoftSymbol());
    } catch (e) {
      if (e instanceof import_lpg2ts.NullExportedSymbolsException) {
      } else if (e instanceof import_lpg2ts.NullTerminalSymbolsException) {
      } else if (e instanceof import_lpg2ts.UnimplementedTerminalsException) {
        if (this.unimplementedSymbolsWarning) {
          let unimplemented_symbols = e.getSymbols();
          import_lpg2ts.Lpg.Lang.System.Out.println("The Lexer will not scan the following token(s):");
          for (let i = 0; i < unimplemented_symbols.size(); i++) {
            let id = unimplemented_symbols.get(i);
            import_lpg2ts.Lpg.Lang.System.Out.println("    " + glr_exprsym.orderedTerminalSymbols[id]);
          }
          import_lpg2ts.Lpg.Lang.System.Out.println();
        }
      } else if (e instanceof import_lpg2ts.UndefinedEofSymbolException) {
        throw new import_lpg2ts.UndefinedEofSymbolException("The Lexer does not implement the Eof symbol " + glr_exprsym.orderedTerminalSymbols[_glr_expr.prsTable.getEoftSymbol()]);
      }
    }
  }
  constructor(lexStream) {
    super();
    try {
      this.glrParser = new import_lpg2ts.GLRParser(null, _glr_expr.prsTable, this);
    } catch (e) {
      if (e instanceof import_lpg2ts.NotGLRParseTableException)
        throw new import_lpg2ts.NotGLRParseTableException("Regenerate glr_exprprs.ts with -GLR option");
      else if (e instanceof import_lpg2ts.BadParseSymFileException) {
        throw new import_lpg2ts.BadParseSymFileException("Bad Parser Symbol File -- glr_exprsym.ts");
      } else {
        throw e;
      }
    }
    if (lexStream) {
      this.reset(lexStream);
    }
  }
  numTokenKinds() {
    return glr_exprsym.numTokenKinds;
  }
  orderedTerminalSymbols() {
    return glr_exprsym.orderedTerminalSymbols;
  }
  getTokenKindName(kind) {
    return glr_exprsym.orderedTerminalSymbols[kind];
  }
  getEOFTokenKind() {
    return _glr_expr.prsTable.getEoftSymbol();
  }
  getIPrsStream() {
    return this.prsStream;
  }
  /**
   * @deprecated replaced by {@link #getIPrsStream()}
   *
   */
  getPrsStream() {
    return this.prsStream;
  }
  /**
   * @deprecated replaced by {@link #getIPrsStream()}
   *
   */
  getParseStream() {
    return this.prsStream;
  }
  // error_repair_count>0: GLR failure falls back to BacktrackingParser
  // fuzzyParse (Recover prosthesis); Diagnose is last resort.
  parser(error_repair_count = 0, monitor) {
    this.glrParser.setMonitor(monitor);
    try {
      return this.glrParser.parse(error_repair_count);
    } catch (ex) {
      if (ex instanceof import_lpg2ts.BadParseException) {
        let e = ex;
        this.prsStream.reset(e.error_token);
        let diagnoseParser = new import_lpg2ts.DiagnoseParser(this.prsStream, _glr_expr.prsTable);
        diagnoseParser.diagnose(e.error_token);
      } else {
        throw ex;
      }
    }
    return null;
  }
  //
  // Additional entry points, if any
  //
  //#line 334 "glrParserTemplateF.gi
  ruleAction(ruleNumber) {
    switch (ruleNumber) {
      //
      // Rule 1:  E ::= E PLUS E
      //
      case 1: {
        this.setResult(
          //#line 18 glr_expr.g
          new E(
            this.getLeftIToken(),
            this.getRightIToken(),
            this.getRhsSym(1),
            this.getRhsSym(3)
          )
          //#line 18 glr_expr.g
        );
        break;
      }
      //
      // Rule 2:  E ::= NUMBER
      //
      case 2: {
        this.setResult(
          //#line 19 glr_expr.g
          new E(
            this.getLeftIToken(),
            this.getRightIToken(),
            //#line 19 glr_expr.g
            null,
            //#line 19 glr_expr.g
            null
          )
          //#line 19 glr_expr.g
        );
        break;
      }
      //#line 338 "glrParserTemplateF.gi
      default:
        break;
    }
    return;
  }
};
var Ast = class {
  nextAst = null;
  getNextAst() {
    return this.nextAst;
  }
  setNextAst(n) {
    this.nextAst = n;
  }
  resetNextAst() {
    this.nextAst = null;
  }
  leftIToken;
  rightIToken;
  getParent() {
    throw new Error("noparent-saved option in effect");
  }
  getRuleIndex() {
    return 0;
  }
  getLeftIToken() {
    return this.leftIToken;
  }
  getRightIToken() {
    return this.rightIToken;
  }
  getPrecedingAdjuncts() {
    return this.leftIToken.getPrecedingAdjuncts();
  }
  getFollowingAdjuncts() {
    return this.rightIToken.getFollowingAdjuncts();
  }
  toString() {
    let str = this.leftIToken.getILexStream()?.toString(this.leftIToken.getStartOffset(), this.rightIToken.getEndOffset());
    return str ? str : "";
  }
  constructor(leftIToken, rightIToken) {
    this.leftIToken = leftIToken;
    if (rightIToken) this.rightIToken = rightIToken;
    else this.rightIToken = leftIToken;
  }
  initialize() {
  }
  getChildren() {
    throw new Error("noparent-saved option in effect");
  }
  getAllChildren() {
    return this.getChildren();
  }
  accept(v) {
  }
};
var E = class extends Ast {
  _E;
  _E3;
  /**
   * The value returned by <b>getE</b> may be <b>null</b>
   */
  getE() {
    return this._E;
  }
  /**
   * The value returned by <b>getE3</b> may be <b>null</b>
   */
  getE3() {
    return this._E3;
  }
  constructor(leftIToken, rightIToken, _E2, _E3) {
    super(leftIToken, rightIToken);
    this._E = _E2;
    this._E3 = _E3;
    this.initialize();
  }
  acceptWithVisitor(v) {
    v.visitE(this);
  }
  acceptWithArg(v, o) {
    v.visitE(this, o);
  }
  acceptWithResult(v) {
    return v.visitE(this);
  }
  acceptWithResultArgument(v, o) {
    return v.visitE(this, o);
  }
  getRuleIndex() {
    return 2;
  }
};

// playground/glr-demo/main.ts
var StubLex = class extends import_lpg2ts2.LexStream {
  constructor() {
    super("glr-playground", "n+n+n+n+n+n+n+n ");
  }
  orderedExportedSymbols() {
    return glr_exprsym.orderedTerminalSymbols;
  }
};
function seed(stream, kinds) {
  stream.makeToken(0, 0, 0);
  for (let i = 0; i < kinds.length; i++) stream.makeToken(i + 1, i + 1, kinds[i]);
  stream.setStreamLength(stream.getSize());
}
function countAlts(n) {
  let c = 0;
  for (let p = n; p != null; p = p.getNextAst()) c++;
  return c;
}
function expressionKinds(operands) {
  const kinds = [];
  for (let n = 0; n < operands; n++) {
    kinds.push(glr_exprsym.TK_NUMBER);
    if (n + 1 < operands) kinds.push(glr_exprsym.TK_PLUS);
  }
  kinds.push(glr_exprsym.TK_EOF_TOKEN);
  return kinds;
}
function runGlrDemo(operands) {
  try {
    const lex = new StubLex();
    const parser = new glr_expr(lex);
    seed(parser.getIPrsStream(), expressionKinds(operands));
    const root = parser.parser();
    if (!root) return { operands, rootAlts: 0, sppfSymbols: 0, ok: false, error: "null root" };
    const glr = parser.getParser();
    return {
      operands,
      rootAlts: countAlts(root),
      sppfSymbols: glr.getSppfSymbolCount(),
      ok: true
    };
  } catch (e) {
    return { operands, rootAlts: 0, sppfSymbols: 0, ok: false, error: String(e && e.message ? e.message : e) };
  }
}
function runGlrDemoSuite() {
  return [1, 2, 3, 4, 5, 6, 7, 8].map(runGlrDemo);
}
export {
  runGlrDemo,
  runGlrDemoSuite
};
