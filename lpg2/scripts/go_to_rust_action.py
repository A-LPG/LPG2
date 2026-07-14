#!/usr/bin/env python3
"""Convert GoAction.cpp / GoTable.cpp to Rust backends.

Only renames C++ class symbols and transforms Go patterns inside emitted string literals.
"""

from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"
INCLUDE = ROOT / "include"


def rename_cpp_symbols(text: str, go_name: str, rust_name: str) -> str:
    text = text.replace(f"{go_name}.h", f"{rust_name}.h")
    text = text.replace(f"{go_name}::", f"{rust_name}::")
    text = text.replace(f"{go_name}(", f"{rust_name}(")
    return text


def patch_action_expand_export(text: str) -> str:
    old = """void RustAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
{
    buffer -> Put(option -> exp_type);
    buffer -> Put(".");
    buffer -> Put(option -> exp_prefix);
    buffer -> Put(simple_macro -> Name() + 2); // skip initial escape and '_' characters
    buffer -> Put(option -> exp_suffix);
}"""
    new = """void RustAction::ExpandExportMacro(TextBuffer *buffer, SimpleMacroSymbol *simple_macro)
{
    buffer -> Put("crate::");
    if (*option -> exp_type != '\\0')
    {
        buffer -> Put(option -> exp_type);
        buffer -> Put("::");
    }
    buffer -> Put(option -> exp_prefix);
    buffer -> Put(simple_macro -> Name() + 2); // skip initial escape and '_' characters
    buffer -> Put(option -> exp_suffix);
}"""
    return text.replace(old, new)


def patch_action_cast_helpers(text: str) -> str:
    start = text.find("namespace \n{")
    end = text.find("}\nvoid RustAction::ProcessCodeActionEnd", start)
    if start < 0 or end < 0:
        raise RuntimeError("Could not locate AnyCast helper namespace in GoAction.cpp")

    replacement = """namespace 
{
	std::string templateAnyCastToInterface(const char* interfaceName)
	{
        char temp[2048] = {};
        sprintf(temp,
            "pub fn any_cast_to_%s(i: Option<Box<dyn std::any::Any>>) -> Option<Arc<dyn %s>> {\\n"
            "    i.and_then(|boxed| boxed.downcast::<Arc<dyn %s>>().ok().map(|a| (*a).clone()))\\n"
            "}\\n",
            interfaceName, interfaceName, interfaceName);
        return temp;
	}
    std::string templateAnyCastToStruct(const char* structName)
    {
        char temp[2048] = {};
        sprintf(temp,
            "pub fn any_cast_to_%s(i: Option<Box<dyn std::any::Any>>) -> Option<Arc<%s>> {\\n"
            "    i.and_then(|boxed| boxed.downcast::<Arc<%s>>().ok().map(|a| (*a).clone()))\\n"
            "}\\n",
            structName, structName, structName);
        return temp;
    }
}
"""
    return text[:start] + replacement + text[end + 2:]


def patch_action_package_lines(text: str) -> str:
    text = text.replace('buffer->Put("package ");', 'buffer->Put("// mod ");')
    text = text.replace("// Issue the package state", "// Issue the module comment")
    return text


# Emitted Go patterns -> Rust (applied only to quoted fragments / sprintf templates)
EMITTED_REPLACEMENTS = [
    ("interface{}", "Box<dyn std::any::Any>"),
    ("[]IToken", "Vec<Rc<dyn IToken>>"),
    ("*ArrayList", "&ArrayList"),
    (" IToken", " Rc<dyn IToken>"),
    (" IAst", " Arc<dyn IAst>"),
    ("{return nil}", "{ return None }"),
    ("return nil", "return None"),
    ("if nil == i", "if i.is_none()"),
    ("if nil !=", "if"),
    (" != nil", ".is_some()"),
    (" == nil", ".is_none()"),
    ("nil == i", "i.is_none()"),
    ("nil != my.", "my."),
    ('+"type "', '+"pub trait "'),
    ('b+"type "', 'b+"pub trait "'),
    ('b+"type "+', 'b+"pub trait "+'),
    ('b + "type " +', 'b + "pub trait " +'),
    (" struct{\\n", " {\\n"),
    (" struct{", " {"),
    ('def_prefix_holder = "func (my *"', 'def_prefix_holder = "impl "'),
    ('std::string def_prefix_holder = "func (my *"', 'std::string def_prefix_holder = "impl "'),
    ('+" interface{\\n"', '+" {\\n"'),
    (' interface{\\n', ' {\\n'),
    ("AnyCastTo", "any_cast_to_"),
    ("leftIToken", "left_i_token"),
    ("rightIToken", "right_i_token"),
    ("nextAst", "next_ast"),
]


def apply_emitted_replacements(text: str) -> str:
    for old, new in EMITTED_REPLACEMENTS:
        text = text.replace(old, new)
    return text


def fix_broken_interface_lines(text: str) -> str:
    """Fix C++ lines broken by naive interface -> trait replacement."""
    import re
    text = text.replace(' : Send + Sync {\\n"";', ' {\\n";')
    text = re.sub(
        r'b(\s*\+\s*)"pub struct "\s*\+\s*',
        r'b\1"pub trait " + ',
        text,
    )
    text = re.sub(
        r'b\+"pub struct "\s*\+\s*',
        r'b+"pub trait " + ',
        text,
    )
    # type Foo interface{ -> pub trait Foo {
    text = text.replace('+" interface{\\n"', '+" {\\n"')
    text = text.replace(' + " interface{\\n"', ' + " {\\n"')
    return text


def write_rust_action_h() -> None:
    go_h = (INCLUDE / "GoAction.h").read_text()
    rust_h = go_h.replace("GoAction", "RustAction")
    rust_h = rust_h.replace('return "IToken"', 'return "Rc<dyn IToken>"')
    rust_h = rust_h.replace('return "interface{}"', 'return "Box<dyn std::any::Any>"')
    (INCLUDE / "RustAction.h").write_text(rust_h)


def convert_action() -> None:
    text = (SRC / "GoAction.cpp").read_text()
    text = rename_cpp_symbols(text, "GoAction", "RustAction")
    text = patch_action_cast_helpers(text)
    text = patch_action_expand_export(text)
    text = patch_action_package_lines(text)
    text = apply_emitted_replacements(text)
    text = fix_broken_interface_lines(text)
    (SRC / "RustAction.cpp").write_text(text)


def patch_table_prs_prefix(text: str) -> str:
    return text.replace(
        'std::string temp = "func (my * ";\n    temp += option->prs_type;\n\ttemp+=")";',
        'std::string temp = "impl ";\n    temp += option->prs_type;\n    temp += " ";',
    )


def patch_table_emitted_strings(text: str) -> str:
    pairs = [
        ('line += "package ";', 'line += "// mod ";'),
        ('line += "type __";', 'line += "pub struct __";'),
        ('line += "__ struct{\\n";', 'line += "__ {\\n";'),
        ('line += " int\\n";', 'line += ": i32,\\n";'),
        ('fprintf(syssym, "\\n   IsValidForParser  bool\\n}\\n");',
         'fprintf(syssym, "\\n   pub is_valid_for_parser: bool,\\n}\\n");'),
        ('fprintf(syssym, "\\n   OrderedTerminalSymbols []string\\n");',
         'fprintf(syssym, "\\n   pub ordered_terminal_symbols: Vec<String>,\\n");'),
        ('fprintf(syssym, "\\n   NumTokenKinds int\\n\\n");',
         'fprintf(syssym, "\\n   pub num_token_kinds: i32,\\n\\n");'),
        ('fprintf(syssym, "\\n   OrderedRuleNames []string\\n");',
         'fprintf(syssym, "\\n   pub ordered_rule_names: Vec<String>,\\n");'),
        ('fprintf(syssym, "\\n   NumRuleNames int\\n");',
         'fprintf(syssym, "\\n   pub num_rule_names: i32,\\n");'),
        ('new_func_name = "New__";', 'new_func_name = "new___";'),
        ('line += "func ";', 'line += "pub fn ";'),
        ('line += " *__";', 'line += "() -> __";'),
        ('line += "__{\\n";', 'line += "__ {\\n";'),
        ('line += "    my := new(__";', 'line += "    let mut my = __";'),
        ('line += "__)\\n";', 'line += "__::default();\\n";'),
        ('fprintf(syssym, "\\n   my.OrderedTerminalSymbols = []string{\\n");',
         'fprintf(syssym, "\\n   my.ordered_terminal_symbols = vec![\\n");'),
        ('fprintf(syssym, "\\n   my.NumTokenKinds = %d\\n\\n", grammar->num_terminals);',
         'fprintf(syssym, "\\n   my.num_token_kinds = %d;\\n\\n", grammar->num_terminals);'),
        ('fprintf(syssym, "\\n   my.OrderedRuleNames = []string{\\n");',
         'fprintf(syssym, "\\n   my.ordered_rule_names = vec![\\n");'),
        ('fprintf(syssym, "\\n   my.NumRuleNames = %d\\n", grammar->LastRule());',
         'fprintf(syssym, "\\n   my.num_rule_names = %d;\\n", grammar->LastRule());'),
        ('fprintf(syssym, "\\n   my.IsValidForParser = true");',
         'fprintf(syssym, "\\n   my.is_valid_for_parser = true");'),
        ('fprintf(syssym, "\\n   return my\\n}\\n");',
         'fprintf(syssym, "\\n   my\\n}\\n");'),
        ('fprintf(syssym, "var %s = %s\\n", option->sym_type, new_func_name.c_str());',
         'fprintf(syssym, "pub static %s: once_cell::sync::Lazy<__%s__> = once_cell::sync::Lazy::new(|| %s());\\n", option->sym_type, option->sym_type, new_func_name.c_str());'),
        ('prs_buffer + "\\nvar  "+ option->prs_type+ "_"+ name + " []" + type + "=[]" + type + "{";',
         'prs_buffer + "\\npub const "+ option->prs_type+ "_"+ name + ": &[" + type + "] = &[";'),
        ('prs_buffer.Put("}\\n");', 'prs_buffer.Put("];\\n");'),
        ('prs_buffer + prs_def_prefix + " " + name+"(index int)";',
         'prs_buffer + "fn " + name+"(index: i32)";'),
        ('prs_buffer.Put(array_info.type_id == Table::B ? "bool" : "int");',
         'prs_buffer.Put(array_info.type_id == Table::B ? " -> bool" : " -> i32");'),
        ('prs_buffer+ "    return "+ option->prs_type+"_"+name+ "[index]";',
         'prs_buffer+ "    "+ option->prs_type+"_"+name+ "[index as usize]";'),
        ('prs_buffer+"const " + option->prs_type + "_" + variable + " int = " + value + "\\n";',
         'prs_buffer+"pub const " + option->prs_type + "_" + variable + ": i32 = " + value + ";\\n";'),
        ('prs_buffer + prs_def_prefix + " " + method + "() int {\\n";',
         'prs_buffer + "pub fn " + method + "(&self) -> i32 {\\n";'),
        ('prs_buffer + "     return " + option->prs_type + "_" + variable + "\\n}\\n";',
         'prs_buffer + "    " + option->prs_type + "_" + variable + "\\n}\\n";'),
        ('prs_buffer + "const " + option->prs_type + "_" + variable + " bool = ";',
         'prs_buffer + "pub const " + option->prs_type + "_" + variable + ": bool = ";'),
        ('prs_buffer + prs_def_prefix + " " + method + "() bool {\\n";',
         'prs_buffer + "pub fn " + method + "(&self) -> bool {\\n";'),
        ('prs_buffer.Put("type ");\n    prs_buffer.Put(option -> prs_type);\n    prs_buffer.Put(" struct{}\\n");',
         'prs_buffer.Put("pub struct ");\n    prs_buffer.Put(option -> prs_type);\n    prs_buffer.Put(" {}\\n");'),
        ('sprintf(temp, "func New%s() *%s{\\n"\n        "    return &%s{}\\n}\\n",option->prs_type,option->prs_type,option->prs_type);',
         'sprintf(temp, "impl %s {\\n    pub fn new() -> Self {\\n        Self {}\\n    }\\n}\\n", option->prs_type);'),
        ('type_name[B] = type_name[I8] = "int";', 'type_name[B] = type_name[I8] = "i32";'),
        ('type_name[I16] = "int";', 'type_name[I16] = "i32";'),
        ('type_name[U8]  = "int";', 'type_name[U8]  = "i32";'),
        ('type_name[U16] = "int";', 'type_name[U16] = "i32";'),
        ('type_name[I32] = "int";', 'type_name[I32] = "i32";'),
    ]
    for old, new in pairs:
        text = text.replace(old, new)

    # Method names only inside sprintf / prs_buffer emitted strings
    emitted_method_map = [
        ("GetErrorSymbol", "get_error_symbol"),
        ("GetScopeUbound", "get_scope_ubound"),
        ("GetScopeSize", "get_scope_size"),
        ("GetMaxNameLength", "get_max_name_length"),
        ("GetNumStates", "get_num_states"),
        ("GetNtOffset", "get_nt_offset"),
        ("GetLaStateOffset", "get_la_state_offset"),
        ("GetMaxLa", "get_max_la"),
        ("GetNumRules", "get_num_rules"),
        ("GetNumNonterminals", "get_num_nonterminals"),
        ("GetNumSymbols", "get_num_symbols"),
        ("GetStartState", "get_start_state"),
        ("getIdentifier_SYMBOL", "get_identifier_symbol"),
        ("GetEoftSymbol", "get_eoft_symbol"),
        ("GetEoltSymbol", "get_eolt_symbol"),
        ("GetAcceptAction", "get_accept_action"),
        ("GetErrorAction", "get_error_action"),
        ("GetBacktrack", "get_backtrack"),
        ("GetStartSymbol", "get_start_symbol"),
        ("IsValidForParser", "is_valid_for_parser"),
        ("OriginalState", "original_state"),
        ("IsNullable", "is_nullable"),
        ("NtAction", "nt_action"),
        ("TAction", "t_action"),
        ("LookAhead", "look_ahead"),
        ("BaseCheck", "base_check"),
        ("Rhs", "rhs"),
        ("BaseAction", "base_action"),
        ("Lhs", "lhs"),
        ("TermCheck", "term_check"),
        ("TermAction", "term_action"),
        ("Asi", "asi"),
        ("Nasi", "nasi"),
        ("InSymbol", "in_symbol"),
        ("ScopePrefix", "scope_prefix"),
        ("ScopeSuffix", "scope_suffix"),
        ("ScopeLhs", "scope_lhs"),
        ("ScopeLa", "scope_la"),
        ("ScopeStateSet", "scope_state_set"),
        ("ScopeRhs", "scope_rhs"),
        ("ScopeState", "scope_state"),
        ("ProsthesesIndex", "prostheses_index"),
        ("IsKeyword", "is_keyword"),
    ]
    for go_name, rust_name in emitted_method_map:
        text = text.replace(f'"{go_name}', f'"{rust_name}')
        text = text.replace(f" {go_name}(", f" {rust_name}(")
        text = text.replace(f"%s {go_name}(", f"%s {rust_name}(")

    return text


def convert_table() -> None:
    text = (SRC / "GoTable.cpp").read_text()
    text = rename_cpp_symbols(text, "GoTable", "RustTable")
    text = patch_table_prs_prefix(text)
    text = patch_table_emitted_strings(text)
    (SRC / "RustTable.cpp").write_text(text)


def main() -> None:
    write_rust_action_h()
    convert_action()
    convert_table()
    print("Rust backend files generated.")


if __name__ == "__main__":
    main()
