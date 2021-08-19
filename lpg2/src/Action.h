#ifndef action_INCLUDED
#define action_INCLUDED

#include "tuple.h"
#include "option.h"
#include "LexStream.h"
#include "symbol.h"
#include "set.h"

class Grammar;
class LCA;
class CTC;
class NTC;
class TTC;
class ClassnameElement;
class SpecialArrayElement;
class ProcessedRuleElement;
class RuleAllocationElement;
class ActionBlockElement;

class Control;
class Blocks;


class Action
{
public:

    Action(Control *, Blocks *, Grammar *, MacroLookupTable *);
    virtual ~Action() { delete [] abstract_ast_list_classname; }
    std::string current_line_input_file_info;
    std::string rule_info_holder;
    std::string line_header_holder;
    int return_code; 
   
protected:
    std::string ast_member_prefix;
    Control *control;
    Blocks *action_blocks;
    Grammar *grammar;
    Option *option;
    LexStream *lex_stream;

    char *abstract_ast_list_classname;
    int first_locally_exported_macro,
        locally_exported_macro_gate;

    Stack<const char *> file_location_stack,
                        cursor_location_stack,
                        end_cursor_location_stack;

    MacroLookupTable *macro_table;

    SimpleMacroLookupTable local_macro_table,
                           rule_macro_table,
                           filter_macro_table,
                           export_macro_table,
                           undeclared_macro_table;

    SimpleMacroSymbol *rule_number_macro,
                      *rule_text_macro,
                      *rule_size_macro,
                      *input_file_macro,
                      *current_line_macro,
                      *next_line_macro,
                      *identifier_macro,
                      *symbol_declarations_macro,
                      *entry_name_macro,
                      *entry_marker_macro;

    MacroSymbol *entry_declarations_macro;

    SimpleMacroSymbol *InsertLocalMacro(const char *str, const char *value = NULL)
    {
        int length = strlen(str) + 1;
        char *macro_name = new char[length + 1];
        macro_name[0] = option -> macro_prefix;
       
        strcpy(&(macro_name[1]), str);

        SimpleMacroSymbol *macro_symbol = local_macro_table.InsertName(macro_name, length);
        if (value)
            macro_symbol -> SetValue(value);

        delete [] macro_name;

        return macro_symbol;
    }

    SimpleMacroSymbol *InsertLocalMacro(const char *macro_name, int value)
    {
        IntToString num(value);
        return InsertLocalMacro(macro_name, num.String());
    }

    SimpleMacroSymbol *InsertExportMacro(int export_token)
    {
        const char *str = lex_stream -> NameString(export_token);
        int length = lex_stream -> NameStringLength(export_token);

        char *macro_name = new char[length + 3];
        macro_name[0] = option -> macro_prefix;
        macro_name[1] = '_';
        strncpy(&(macro_name[2]), str, length);
        macro_name[length + 2] = '\0';

        SimpleMacroSymbol *macro_symbol = export_macro_table.FindOrInsertName(macro_name, length + 2);
        macro_symbol -> SetLocation(lex_stream -> GetTokenReference(export_token)) ;

        delete [] macro_name;

        return macro_symbol;
    }

    SimpleMacroSymbol *InsertFilterMacro(const char *name, char *value)
    {
        int length = strlen(name);
        char *macro_name = new char[length + 2];
        macro_name[0] = option -> macro_prefix;
        strncpy(&(macro_name[1]), name, length);
        macro_name[length + 1] = '\0';

        SimpleMacroSymbol *macro_symbol = filter_macro_table.FindOrInsertName(macro_name, length + 1);
        macro_symbol -> SetValue(value);

        delete [] macro_name;

        return macro_symbol;
    }

    SimpleMacroSymbol *InsertRuleMacro(const char *macro_name, char *value)
    {
        int length = strlen(macro_name);
        SimpleMacroSymbol *macro_symbol = rule_macro_table.InsertName(macro_name, length);
        assert(value);
        macro_symbol -> SetValue(value);

        return macro_symbol;
    }

    inline SimpleMacroSymbol *InsertRuleMacro(const char *macro_name, int value)
    {
        IntToString num(value);
        return InsertRuleMacro(macro_name, num.String());
    }

    SimpleMacroSymbol *InsertUndeclaredMacro(const char *macro_name)
    {
        SimpleMacroSymbol *macro_symbol = undeclared_macro_table.InsertName(macro_name, strlen(macro_name));
        return macro_symbol;
    }

    SimpleMacroSymbol *FindLocalMacro(const char *str, size_t length)
    {
        char *macro_name = new char[length + 1];
        for (size_t i = 0; i < length; i++)
            macro_name[i] = Code::ToLower(str[i]);
        macro_name[length] = '\0';

        SimpleMacroSymbol *macro_symbol = local_macro_table.FindName(macro_name, length);

        delete [] macro_name;

        return macro_symbol;
    }

    inline SimpleMacroSymbol *FindRuleMacro(const char *str, size_t length)
    {
        return rule_macro_table.FindName(str, length);
    }

    inline SimpleMacroSymbol *FindFilterMacro(const char *str, size_t length)
    {
        return filter_macro_table.FindName(str, length);
    }

    inline SimpleMacroSymbol *FindExportMacro(const char *str, size_t length)
    {
        return export_macro_table.FindName(str, length);
    }

    inline MacroSymbol *FindUserDefinedMacro(const char *str, size_t length)
    {
        return macro_table -> FindName(str, length);
    }

    inline SimpleMacroSymbol *FindUndeclaredMacro(const char *str, size_t length)
    {
        return undeclared_macro_table.FindName(str, length);
    }

    const char *getFileSuffix(const char *filename)
    {
        //
        // First check whether or not the directory_prefix is an initial prefix of the filename in question.
        //
        int length = strlen(option -> directory_prefix);
        if (length > 0 && strncmp(option -> directory_prefix, filename, length) == 0)
            return  &(filename[length]);
        //
        // Next, check whether or not one of the template_search directories is an initial prefix of the filename in question.
        //
        for (int i = 0; i < option -> template_search_directory.Length(); i++)
        {
            length = strlen(option -> template_search_directory[i]);
            if (length > 0 && strncmp(option -> template_search_directory[i], filename, length) == 0)
                return  &(filename[length]);
        }
        //
        // Next, check whether or not one of the include_search directories is an initial prefix of the filename in question.
        //
        for (int i = 0; i < option -> include_search_directory.Length(); i++)
        {
            length = strlen(option -> include_search_directory[i]);
            if (length > 0 && strncmp(option -> include_search_directory[i], filename, length) == 0)
                return  &(filename[length]);
        }

        return filename;
    }

    //
    // Remove the portion of the directory prefix that matches the filename.
    //
    std::string FileWithoutPrefix(const char *filename)
    {
        std::string result;
        const char *file_without_prefix = getFileSuffix(filename);

        if (file_without_prefix == filename)
        {
            //
            // Make a last ditch effort to express the filename in terms of the directory_prefix;
            //
            int cutoff_index = 0;
            for (const char *p = option -> directory_prefix, *q = filename; *q && *p; q++, p++)
            {
                if (*p != *q) // mismatched character.
                    break;
                if (*p == '/') // a slash?
                    cutoff_index = p - option -> directory_prefix;
            }
            //
            // Only match the prefix that corresponds to a complete directory name (partial name match of the final name is discarded...)
            //
            const char* p;
            if (cutoff_index != 0) // if we are not still at the beginning, skip slash!
            {
                file_without_prefix = &(filename[cutoff_index + 1]);
                p = &(option -> directory_prefix[cutoff_index + 1]);
            }
            else
            {
                file_without_prefix = filename;
                p = option -> directory_prefix;
            }

            //
            // Count how many subdirectories were not matched and replace each of them by "../"
            // Recall that the directory_prefix contains no trailng slashes.
            //
            if (p != option -> directory_prefix)
            {
                while (*p)
                {
                    result += "../";
                    while (*p && *p == '/') p++; // skip extraneous slashes, if any!
                    while (*p && *p != '/') p++; // skip subdirectory name
                    if (*p) p++; // skip the main slash
                }
            }
        }
        if (file_without_prefix > filename) 
        {
            while(*file_without_prefix == '/')
                file_without_prefix++;
        }

        result += std::string(file_without_prefix);

        return result;
    }

    /**
     * The local functions below are language-independent functions that
     * can be shared by all output languages.
     */
public:
    int LocalMacroTableSize() { return local_macro_table.Size(); }
    SimpleMacroSymbol *GetLocalMacro(int i) { return local_macro_table[i]; }

    int FilterMacroTableSize() { return filter_macro_table.Size(); }
    SimpleMacroSymbol *GetFilterMacro(int i) { return filter_macro_table[i]; }

    void InsertExportMacros();
    void CheckExportMacros();
    void InsertImportedFilterMacros();
    void CheckMacrosForConsistency();
    void SetupBuiltinMacros();

    const char *SkipMargin(TextBuffer *, const char *, const char *);
    void ProcessAstRule(ClassnameElement &, int, Tuple<ProcessedRuleElement> &);
    void ProcessAstMergedRules(LCA &, ClassnameElement &, Tuple<int> &, Tuple< Tuple<ProcessedRuleElement> > &);
    void ProcessCodeActions(Tuple<ActionBlockElement> &, Array<const char *> &, Tuple< Tuple<ProcessedRuleElement> > &);
	
    void CompleteClassnameInfo(LCA &,
                               TTC &,
                               BoundedArray< Tuple<int> > &,
                               Array<const char *> &,
                               Tuple< Tuple<ProcessedRuleElement> > &,
                               SymbolLookupTable &,
                               Tuple<ClassnameElement> &,
                               Array<RuleAllocationElement> &);
   virtual  void ProcessAstActions(Tuple<ActionBlockElement> &,
                           Tuple<ActionBlockElement> &,
                           Tuple<ActionBlockElement> &,
                           Array<const char *> &,
                           Tuple< Tuple<ProcessedRuleElement> > &,
                           SymbolLookupTable &,
                           Tuple<ClassnameElement> &);
    void ProcessActionBlock(ActionBlockElement &, bool add_location_directive = false);
    void ProcessMacroBlock(int, MacroSymbol *, TextBuffer *, int, const char *, int);
    void ProcessMacro(TextBuffer *, const char *, int);
    void GetCallingMacroLocations(Tuple<Token *> &);
    Token *GetMacroErrorToken(const char *, const char *, const char *);
    void EmitMacroError(const char *, const char *, const char *, Tuple<const char *> &);
    void EmitMacroWarning(const char *, const char *, const char *, Tuple<const char *> &);
    Symbol *FindClosestMatchForMacro(const char *, const char *, const char *, const char *, const char *);
    void ProcessActionLine(BlockSymbol* ,int, TextBuffer *, const char *, const char *, const char *, int, const char *, int, const char * = NULL, const char * = NULL);
    void GenerateCode(TextBuffer *, const char *, int);

protected:
    void ComputeInterfaces(ClassnameElement &, Array<const char *> &, Tuple<int> &);
    void CheckRecursivenessAndUpdate(Tuple<int> &, BoundedArray< Tuple<int> > &, Array<RuleAllocationElement> &);
    bool IsNullClassname(ClassnameElement &);

    /**
     * The virtual functions below are language-dependent functions that
     * must be implemented for each output language. Note that they are
     * declared here as asbstract (= 0;) functions.
     */
public:
	virtual void ProcessCodeActionEnd()
	{
		
	}
    virtual void ProcessRuleActionBlock(ActionBlockElement&);
    virtual const char *GetDefaultTerminalType() = 0;
    virtual const char *GetDefaultNonterminalType() = 0;
    virtual void GenerateDefaultTitle(Tuple<ActionBlockElement> &) = 0;
protected:
    virtual void ExpandExportMacro(TextBuffer *, SimpleMacroSymbol *) = 0;

    virtual ActionFileSymbol *GenerateTitle(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) = 0;
    virtual ActionFileSymbol *GenerateTitleAndGlobals(ActionFileLookupTable &, Tuple<ActionBlockElement> &, const char *, bool) = 0;

    virtual void GenerateVisitorHeaders(TextBuffer &, const char *, const char *) = 0;
    virtual void GenerateVisitorMethods(NTC &, TextBuffer &, const char *, ClassnameElement &, BitSet &) = 0;
    virtual void GenerateGetAllChildrenMethod(TextBuffer &, const char *, ClassnameElement &) = 0;

    virtual void GenerateSimpleVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) = 0;
    virtual void GenerateArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) = 0;
    virtual void GenerateResultVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) = 0;
    virtual void GenerateResultArgumentVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) = 0;

    virtual void GeneratePreorderVisitorInterface(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) = 0;
    virtual void GeneratePreorderVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) = 0;

    virtual void GenerateNoResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) = 0;
    virtual void GenerateResultVisitorAbstractClass(ActionFileSymbol*, const char *, const char *, SymbolLookupTable &) = 0;

    virtual void GenerateAstType(ActionFileSymbol* , const char *, const char *) = 0;
    virtual void GenerateAbstractAstListType(ActionFileSymbol* , const char *, const char *) = 0;
    virtual void GenerateAstTokenType(NTC &, ActionFileSymbol*, const char *, const char *) = 0;
    virtual void GenerateInterface(bool,
                                   ActionFileSymbol*,
                                   const char *,
                                   const char *,
                                   Tuple<int> &,
                                   Tuple<int> &,
                                   Tuple<ClassnameElement> &) = 0;
    virtual void GenerateCommentHeader(TextBuffer &, const char *, Tuple<int> &, Tuple<int> &) = 0;
    virtual void GenerateListExtensionClass(CTC &, NTC &, ActionFileSymbol*, const char *, SpecialArrayElement &, ClassnameElement &, Array<const char *> &) = 0;
    virtual void GenerateListClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) = 0;
    virtual void GenerateRuleClass(CTC &, NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) = 0;
    virtual void GenerateMergedClass(CTC &,
                                     NTC &,
                                     ActionFileSymbol*,
                                     const char *,
                                     ClassnameElement &,
                                     Tuple< Tuple<ProcessedRuleElement> > &,
                                     Array<const char *> &) = 0;
    virtual void GenerateTerminalMergedClass(NTC &, ActionFileSymbol*, const char *, ClassnameElement &, Array<const char *> &) = 0;
    virtual void GenerateNullAstAllocation(TextBuffer &, int rule_no) = 0;
    virtual void GenerateEnvironmentDeclaration(TextBuffer &, const char *) = 0;
    virtual void GenerateListAllocation(CTC &ctc, TextBuffer &, int, RuleAllocationElement &) = 0;
    virtual void GenerateAstAllocation(CTC &ctc,
                                       TextBuffer &,
                                       RuleAllocationElement &,
                                       Tuple<ProcessedRuleElement> &,
                                       Array<const char *> &, int) = 0;
};
#endif /* action_INCLUDED */
