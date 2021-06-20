# Makefile for lpg

CSRCS = CAction.cpp CTable.cpp CppAction.cpp CppTable.cpp JavaAction.cpp \
        JavaTable.cpp LexStream.cpp MlAction.cpp MlTable.cpp PlxAction.cpp \
        PlxTable.cpp PlxasmAction.cpp PlxasmTable.cpp XmlAction.cpp XmlTable.cpp \
        Action.cpp base.cpp buffer.cpp code.cpp control.cpp \
        dfa.cpp diagnose.cpp dump.cpp generator.cpp grammar.cpp jikespg.cpp \
        jikespg_act.cpp jikespg_init.cpp jikespg_prs.cpp \
        option.cpp optionDesc.cpp optionParser.cpp optionValues.cpp options.cpp \
        parser.cpp pda.cpp produce.cpp resolve.cpp scanner.cpp \
        sp.cpp symbol.cpp tab.cpp table.cpp util.cpp

COBJS = $(CSRCS:.cpp=.o)

XCFLAGS =
XCXXFLAGS =
XLDFLAGS =

TARGET = lpg-$(ARCH)

include MakeRules.$(ARCH)

INSTALLDIR= ../

LD = $(CXX)

GEN_FILES=jikespg_sym.h jikespg_def.h jikespg_prs.h jikespg_prs.cpp jikespg_act.h jikespg_act.cpp jikespg_init.cpp

bootstrap: jikespg.g
	../bin/lpg jikespg.g
	$(RM) jikespg.l

# DO NOT DELETE THIS LINE -- make depend depends on it.

CAction.o: CAction.h Action.h tuple.h
CAction.o: option.h util.h code.h
CAction.o: blocks.h symbol.h buffer.h
CAction.o: LexStream.h jikespg_sym.h tab.h set.h
CAction.o: control.h grammar.h scanner.h
CAction.o: parser.h jikespg_prs.h jikespg_act.h Stacks.h node.h base.h pda.h
CAction.o: produce.h dfa.h
CTable.o: partition.h tuple.h
CTable.o: util.h
CTable.o: CTable.h
CTable.o: table.h set.h buffer.h control.h option.h code.h
CTable.o: blocks.h symbol.h
CTable.o: grammar.h scanner.h jikespg_sym.h
CTable.o: LexStream.h tab.h parser.h jikespg_prs.h jikespg_act.h Stacks.h
CTable.o: Action.h node.h base.h pda.h produce.h dfa.h
CppAction.o: CppAction.h tuple.h
CppAction.o: CAction.h Action.h option.h util.h code.h
CppAction.o: blocks.h symbol.h buffer.h
CppAction.o: LexStream.h
CppAction.o: jikespg_sym.h tab.h set.h
CppAction.o: control.h grammar.h scanner.h parser.h jikespg_prs.h
CppAction.o: jikespg_act.h Stacks.h node.h base.h pda.h produce.h dfa.h
CppTable.o: partition.h tuple.h
CppTable.o: util.h
CppTable.o: CppTable.h CTable.h table.h set.h buffer.h control.h option.h
CppTable.o: code.h blocks.h symbol.h
CppTable.o: grammar.h
CppTable.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
CppTable.o: jikespg_act.h Stacks.h Action.h node.h base.h pda.h produce.h
CppTable.o: dfa.h
JavaAction.o: CTC.h tuple.h
JavaAction.o: set.h
JavaAction.o: control.h option.h util.h code.h blocks.h
JavaAction.o: symbol.h buffer.h
JavaAction.o: grammar.h scanner.h jikespg_sym.h
JavaAction.o: LexStream.h tab.h parser.h jikespg_prs.h jikespg_act.h Stacks.h
JavaAction.o: Action.h node.h base.h pda.h produce.h dfa.h NTC.h JavaAction.h
JavaTable.o: partition.h tuple.h
JavaTable.o: util.h
JavaTable.o: JavaTable.h table.h set.h buffer.h control.h option.h code.h
JavaTable.o: blocks.h symbol.h
JavaTable.o: grammar.h scanner.h jikespg_sym.h
JavaTable.o: LexStream.h tab.h parser.h jikespg_prs.h jikespg_act.h Stacks.h
JavaTable.o: Action.h node.h base.h pda.h produce.h dfa.h
LexStream.o: LexStream.h jikespg_sym.h code.h
LexStream.o: option.h util.h
LexStream.o: tuple.h
LexStream.o: blocks.h symbol.h buffer.h
LexStream.o: tab.h control.h grammar.h scanner.h
LexStream.o: parser.h jikespg_prs.h jikespg_act.h Stacks.h
LexStream.o: set.h Action.h node.h base.h
LexStream.o: pda.h produce.h dfa.h
MlAction.o: MlAction.h Action.h tuple.h
MlAction.o: option.h util.h code.h
MlAction.o: blocks.h symbol.h buffer.h
MlAction.o: LexStream.h jikespg_sym.h tab.h set.h
MlAction.o: control.h grammar.h scanner.h
MlAction.o: parser.h jikespg_prs.h jikespg_act.h Stacks.h node.h base.h pda.h
MlAction.o: produce.h dfa.h
MlTable.o: partition.h tuple.h
MlTable.o: util.h
MlTable.o: MlTable.h table.h set.h buffer.h
MlTable.o: control.h option.h code.h blocks.h symbol.h
MlTable.o: grammar.h
MlTable.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
MlTable.o: jikespg_act.h Stacks.h Action.h node.h base.h pda.h produce.h
MlTable.o: dfa.h
PlxAction.o: PlxAction.h Action.h tuple.h
PlxAction.o: option.h util.h code.h
PlxAction.o: blocks.h symbol.h buffer.h
PlxAction.o: LexStream.h
PlxAction.o: jikespg_sym.h tab.h set.h
PlxAction.o: control.h grammar.h scanner.h parser.h jikespg_prs.h
PlxAction.o: jikespg_act.h Stacks.h node.h base.h pda.h produce.h dfa.h
PlxTable.o: tuple.h
PlxTable.o: PlxTable.h table.h set.h
PlxTable.o: buffer.h control.h option.h util.h
PlxTable.o: code.h blocks.h symbol.h
PlxTable.o: grammar.h
PlxTable.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
PlxTable.o: jikespg_act.h Stacks.h Action.h node.h base.h pda.h produce.h
PlxTable.o: dfa.h
PlxasmAction.o: PlxasmAction.h tuple.h
PlxasmAction.o: PlxAction.h Action.h option.h util.h code.h
PlxasmAction.o: blocks.h symbol.h buffer.h
PlxasmAction.o: LexStream.h
PlxasmAction.o: jikespg_sym.h tab.h set.h
PlxasmAction.o: control.h grammar.h scanner.h parser.h jikespg_prs.h
PlxasmAction.o: jikespg_act.h Stacks.h node.h base.h pda.h produce.h dfa.h
PlxasmTable.o: tuple.h
PlxasmTable.o: PlxasmTable.h PlxTable.h table.h set.h
PlxasmTable.o: buffer.h control.h option.h
PlxasmTable.o: util.h code.h blocks.h symbol.h
PlxasmTable.o: grammar.h
PlxasmTable.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h
PlxasmTable.o: jikespg_prs.h jikespg_act.h Stacks.h Action.h node.h base.h
PlxasmTable.o: pda.h produce.h dfa.h
XmlAction.o: XmlAction.h Action.h tuple.h
XmlAction.o: option.h util.h code.h
XmlAction.o: blocks.h symbol.h buffer.h
XmlAction.o: LexStream.h
XmlAction.o: jikespg_sym.h tab.h set.h
XmlAction.o: control.h grammar.h scanner.h parser.h jikespg_prs.h
XmlAction.o: jikespg_act.h Stacks.h node.h base.h pda.h produce.h dfa.h
XmlTable.o: XmlTable.h table.h tuple.h
XmlTable.o: set.h
XmlTable.o: buffer.h control.h option.h util.h code.h
XmlTable.o: blocks.h symbol.h
XmlTable.o: grammar.h scanner.h jikespg_sym.h
XmlTable.o: LexStream.h tab.h parser.h jikespg_prs.h jikespg_act.h Stacks.h
XmlTable.o: Action.h node.h base.h pda.h produce.h dfa.h
action.o: NTC.h tuple.h
action.o: control.h option.h util.h code.h
action.o: blocks.h symbol.h buffer.h
action.o: grammar.h
action.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
action.o: jikespg_act.h Stacks.h
action.o: set.h
action.o: Action.h node.h base.h pda.h produce.h dfa.h TTC.h CTC.h LCA.h
base.o: control.h option.h util.h tuple.h
base.o: code.h blocks.h symbol.h
base.o: buffer.h grammar.h
base.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
base.o: jikespg_act.h Stacks.h
base.o: set.h
base.o: Action.h node.h base.h pda.h produce.h dfa.h
buffer.o: util.h tuple.h
buffer.o: buffer.h
code.o: code.h
control.o: control.h option.h util.h tuple.h
control.o: code.h blocks.h
control.o: symbol.h buffer.h
control.o: grammar.h scanner.h jikespg_sym.h LexStream.h tab.h parser.h
control.o: jikespg_prs.h jikespg_act.h Stacks.h
control.o: set.h
control.o: Action.h node.h base.h pda.h produce.h dfa.h generator.h table.h
control.o: CTable.h CppTable.h JavaTable.h PlxTable.h PlxasmTable.h MlTable.h
control.o: XmlTable.h
dfa.o: control.h option.h util.h tuple.h
dfa.o: code.h blocks.h symbol.h
dfa.o: buffer.h grammar.h
dfa.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
dfa.o: jikespg_act.h Stacks.h
dfa.o: set.h
dfa.o: Action.h node.h base.h pda.h produce.h dfa.h
diagnose.o: util.h tuple.h
diagnose.o: diagnose.h jikespg_prs.h jikespg_sym.h
diagnose.o: parser.h jikespg_act.h Stacks.h LexStream.h code.h
diagnose.o: option.h blocks.h symbol.h buffer.h
diagnose.o: tab.h
generator.o: generator.h tuple.h
generator.o: control.h option.h util.h code.h
generator.o: blocks.h symbol.h buffer.h
generator.o: grammar.h
generator.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
generator.o: jikespg_act.h Stacks.h
generator.o: set.h
generator.o: Action.h node.h base.h pda.h produce.h dfa.h table.h frequency.h
generator.o: partition.h
grammar.o: control.h option.h
grammar.o: util.h tuple.h
grammar.o: code.h
grammar.o: blocks.h symbol.h buffer.h grammar.h
grammar.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
grammar.o: jikespg_act.h Stacks.h
grammar.o: set.h
grammar.o: Action.h node.h base.h pda.h produce.h dfa.h CAction.h CppAction.h
grammar.o: PlxAction.h PlxasmAction.h JavaAction.h MlAction.h XmlAction.h
jikespg.o: symbol.h tuple.h
jikespg.o: code.h
jikespg.o: util.h buffer.h
jikespg.o: option.h blocks.h scanner.h jikespg_sym.h
jikespg.o: LexStream.h tab.h control.h grammar.h parser.h jikespg_prs.h
jikespg.o: jikespg_act.h Stacks.h
jikespg.o: set.h
jikespg.o: Action.h node.h base.h pda.h produce.h dfa.h
jikespg_act.o: jikespg_act.h Stacks.h jikespg_prs.h jikespg_sym.h
jikespg_act.o: LexStream.h code.h
jikespg_act.o: option.h
jikespg_act.o: util.h tuple.h
jikespg_act.o: blocks.h symbol.h buffer.h
jikespg_act.o: tab.h
jikespg_act.o: control.h grammar.h scanner.h
jikespg_act.o: parser.h set.h Action.h node.h base.h pda.h produce.h dfa.h
jikespg_init.o: jikespg_act.h Stacks.h jikespg_prs.h jikespg_sym.h
jikespg_init.o: LexStream.h code.h
jikespg_init.o: option.h
jikespg_init.o: util.h tuple.h
jikespg_init.o: blocks.h symbol.h buffer.h
jikespg_init.o: tab.h
jikespg_init.o: control.h grammar.h scanner.h
jikespg_init.o: parser.h set.h Action.h node.h base.h pda.h produce.h dfa.h
jikespg_prs.o: jikespg_prs.h jikespg_sym.h
option.o: control.h option.h util.h tuple.h
option.o: code.h blocks.h symbol.h
option.o: buffer.h grammar.h
option.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
option.o: jikespg_act.h Stacks.h
option.o: set.h
option.o: Action.h node.h base.h pda.h produce.h dfa.h
parser.o: parser.h jikespg_prs.h jikespg_sym.h
parser.o: jikespg_act.h Stacks.h LexStream.h code.h
parser.o: option.h util.h tuple.h
parser.o: blocks.h symbol.h buffer.h
parser.o: tab.h
parser.o: control.h grammar.h scanner.h set.h
parser.o: Action.h node.h base.h pda.h produce.h dfa.h
pda.o: control.h option.h util.h tuple.h
pda.o: code.h blocks.h symbol.h
pda.o: buffer.h grammar.h
pda.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
pda.o: jikespg_act.h Stacks.h
pda.o: set.h
pda.o: Action.h node.h base.h pda.h produce.h dfa.h resolve.h sp.h
produce.o: control.h option.h util.h tuple.h
produce.o: code.h blocks.h
produce.o: symbol.h buffer.h
produce.o: grammar.h scanner.h jikespg_sym.h LexStream.h tab.h parser.h
produce.o: jikespg_prs.h jikespg_act.h Stacks.h
produce.o: set.h
produce.o: Action.h node.h base.h pda.h produce.h dfa.h partition.h
resolve.o: resolve.h tuple.h
resolve.o: control.h
resolve.o: option.h util.h code.h blocks.h symbol.h
resolve.o: buffer.h
resolve.o: grammar.h scanner.h jikespg_sym.h LexStream.h tab.h parser.h
resolve.o: jikespg_prs.h jikespg_act.h Stacks.h
resolve.o: set.h
resolve.o: Action.h node.h base.h pda.h produce.h dfa.h
scanner.o: control.h option.h
scanner.o: util.h tuple.h
scanner.o: code.h
scanner.o: blocks.h symbol.h buffer.h grammar.h
scanner.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
scanner.o: jikespg_act.h Stacks.h
scanner.o: set.h
scanner.o: Action.h node.h base.h pda.h produce.h dfa.h
sp.o: sp.h control.h option.h util.h tuple.h
sp.o: code.h blocks.h symbol.h
sp.o: buffer.h grammar.h
sp.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
sp.o: jikespg_act.h Stacks.h
sp.o: set.h
sp.o: Action.h node.h base.h pda.h produce.h dfa.h
symbol.o: symbol.h tuple.h
symbol.o: code.h util.h buffer.h
tab.o: tab.h
tab.o: code.h
table.o: table.h tuple.h
table.o: set.h
table.o: buffer.h
table.o: control.h option.h util.h code.h blocks.h
table.o: symbol.h grammar.h
table.o: scanner.h jikespg_sym.h LexStream.h tab.h parser.h jikespg_prs.h
table.o: jikespg_act.h Stacks.h Action.h node.h base.h pda.h produce.h dfa.h
util.o: util.h tuple.h
