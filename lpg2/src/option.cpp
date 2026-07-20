#include "control.h"
#include "option.h"
#include "options.h"
#include "LexStream.h"
#include "lpg_error.h"
#include "output_transaction.h"
#include "resource_paths.h"
#include "util.h"

#include <errno.h>
#include <cctype>
#include <cstring>
#include <string>

#include <iostream>
#include <filesystem>
using namespace std;

//
// Change the following static to "true" to enable the new options-processing code
//
static bool NEW_OPTIONS_CODE = false;

const char *Option::default_block_begin = "/.",
           *Option::default_block_end = "./";

Option::Option(int argc_, const char **argv_)
: argc(argc_), argv(argv_)
{
    diagnostics = HUMAN_DIAGNOSTICS;
    write = true;
    json_report_emitted = false;
    for_parser = true;
    syslis = NULL;
    ast_block = NULL;
    dat_directory_location = NULL;
    out_directory_location = NULL;
    ast_directory_location = NULL;
    escape_location = NULL;
    or_marker_location = NULL;
    
    lex_stream = NULL;
    
    return_code = 0;
    glr_template_hint_needed = false;

    optionParser = new OptionParser(OptionDescriptor::getAllDescriptors());
    optionProcessor = new OptionProcessor(this);
    OptionDescriptor::initializeAll(optionProcessor);

    // All of the initializations below are to fields that have cmd-line options
    // >>>
    if (!NEW_OPTIONS_CODE) {
        quiet = false;
        automatic_ast = NONE;
        attributes = false;
        backtrack = false;
        ebnf = false;
        legacy = true;
        list = false;
        glr = false;
        slr = false;
        verbose = false;
        first = false;
        follow = false;
        priority = true;
        edit = false;
        states = false;
        xref = false;
        nt_check = false;
        conflicts = true;
        fail_on_conflicts = false;
        read_reduce = true;
        remap_terminals = true;
        goto_default = false;
        shift_default = false;
        byte = true;
        warnings = true;
        single_productions = false;
        error_maps = false;
        debug = false;
        parent_saved = false;
        precedence = false;
        scopes = false;
        serialize = false;
        soft_keywords = false;
        table = false;
        variables = NONE;
        visitor = NONE;
        lalr_level = 1;
        margin = 0;
        max_cases = 1024;
        names = OPTIMIZED;
        rule_classnames = SEQUENTIAL;
        trace = CONFLICTS;
        programming_language = JAVA;
        escape = ' ';
        or_marker = '|';
        factory = NULL;
        file_prefix = NULL;
        dat_directory = NULL;
        dat_file = NULL;
        dcl_file = NULL;
        def_file = NULL;
        directory_prefix = NULL;
        imp_file = NULL;
        out_directory = NULL;
        ast_directory = NULL;
        ast_type = NULL;
        visitor_type = NULL;
        include_directory = NULL;
        template_name = NULL;
        extends_parsetable = NULL;
        parsetable_interfaces = NULL;
        package = NULL;
        prefix = NULL;
        suffix = NULL;
    }
// <<<

    // Command-line output modes must be known before the listing stream is
    // opened. OptionDescriptor::initializeAll() above resets their defaults.
    const auto equals_ignoring_case = [](const char *left, const char *right)
    {
        while (*left != '\0' && *right != '\0')
        {
            if (std::tolower(static_cast<unsigned char>(*left)) !=
                std::tolower(static_cast<unsigned char>(*right)))
                return false;
            left++;
            right++;
        }
        return *left == *right;
    };
    for (int i = 1; i < argc - 1; i++)
    {
        const char *argument = argv[i];
        while (*argument == '-')
            argument++;
        if (equals_ignoring_case(argument, "nowrite") ||
            equals_ignoring_case(argument, "dry-run") ||
            equals_ignoring_case(argument, "dry_run"))
            write = false;
        else if (equals_ignoring_case(argument, "diagnostics=json"))
            diagnostics = JSON_DIAGNOSTICS;
    }

    // The following fields have option descriptors, but use a "handler" rather
    // than a direct field member ptr, and so don't get auto initialization.
    filter = NULL;
    import_terminals = NULL;

    // The remaining fields have no associated cmd-line options
    ast_package = NULL;
    grm_file = NULL;
    lis_file = NULL;
    tab_file = NULL;
    prs_file = NULL;
    sym_file = NULL;
    top_level_ast_file = NULL;
    top_level_ast_file_prefix = NULL;
    exp_file = NULL;
    exp_prefix = NULL;
    exp_suffix = NULL;
    exp_type = NULL;
    prs_type = NULL;
    sym_type = NULL;
    dcl_type = NULL;
    imp_type = NULL;
    def_type = NULL;
    action_type = NULL;
    default_action_prefix = NULL;
    default_action_file = NULL;
    int len = strlen(default_block_begin);
    auto temp = NewString(len+1);

    memcpy(temp, default_block_begin, len);
    temp[len] = 0x00;
    default_ast_block_begin = temp;
	
    len = strlen(default_block_end);
    temp = NewString(len+1);
    memcpy(temp, default_block_end, len);
    temp[len] = 0x00;
    default_ast_block_end = temp;

	
    for (int c = 0; c < 128; c++)
        classify_option[c] = &Option::ClassifyBadOption;
    
    classify_option[(int) 'a'] = &Option::ClassifyA;
    classify_option[(int) 'A'] = &Option::ClassifyA;
    
    classify_option[(int) 'b'] = &Option::ClassifyB;
    classify_option[(int) 'B'] = &Option::ClassifyB;
    
    classify_option[(int) 'c'] = &Option::ClassifyC;
    classify_option[(int) 'C'] = &Option::ClassifyC;
    
    classify_option[(int) 'd'] = &Option::ClassifyD;
    classify_option[(int) 'D'] = &Option::ClassifyD;
    
    classify_option[(int) 'e'] = &Option::ClassifyE;
    classify_option[(int) 'E'] = &Option::ClassifyE;
    
    classify_option[(int) 'f'] = &Option::ClassifyF;
    classify_option[(int) 'F'] = &Option::ClassifyF;
    
    classify_option[(int) 'g'] = &Option::ClassifyG;
    classify_option[(int) 'G'] = &Option::ClassifyG;
    
    classify_option[(int) 'h'] = &Option::ClassifyH;
    classify_option[(int) 'H'] = &Option::ClassifyH;
    
    classify_option[(int) 'i'] = &Option::ClassifyI;
    classify_option[(int) 'I'] = &Option::ClassifyI;
    
    classify_option[(int) 'l'] = &Option::ClassifyL;
    classify_option[(int) 'L'] = &Option::ClassifyL;
    
    classify_option[(int) 'm'] = &Option::ClassifyM;
    classify_option[(int) 'M'] = &Option::ClassifyM;
    
    classify_option[(int) 'n'] = &Option::ClassifyN;
    classify_option[(int) 'N'] = &Option::ClassifyN;
    
    classify_option[(int) 'o'] = &Option::ClassifyO;
    classify_option[(int) 'O'] = &Option::ClassifyO;
    
    classify_option[(int) 'p'] = &Option::ClassifyP;
    classify_option[(int) 'P'] = &Option::ClassifyP;
    
    classify_option[(int) 'q'] = &Option::ClassifyQ;
    classify_option[(int) 'Q'] = &Option::ClassifyQ;
    
    classify_option[(int) 'r'] = &Option::ClassifyR;
    classify_option[(int) 'R'] = &Option::ClassifyR;
    
    classify_option[(int) 's'] = &Option::ClassifyS;
    classify_option[(int) 'S'] = &Option::ClassifyS;
    
    classify_option[(int) 't'] = &Option::ClassifyT;
    classify_option[(int) 'T'] = &Option::ClassifyT;
    
    classify_option[(int) 'v'] = &Option::ClassifyV;
    classify_option[(int) 'V'] = &Option::ClassifyV;
    
    classify_option[(int) 'w'] = &Option::ClassifyW;
    classify_option[(int) 'W'] = &Option::ClassifyW;
    
    classify_option[(int) 'x'] = &Option::ClassifyX;
    classify_option[(int) 'X'] = &Option::ClassifyX;
    
    //
    //
    //
    if (argc > 1)
    {
        //
        // Process the LPG_TEMPLATE and LPG_INCLUDE environment variables, if they
        // were specified.
        //
        const char *main_input_file = argv[argc - 1],
        *lpg_template = getenv("LPG_TEMPLATE"),
        *lpg_include = getenv("LPG_INCLUDE");
        
        this -> home_directory = GetPrefix(main_input_file);
        {
            std::string path = home_directory;
            if (lpg_template != NULL)
            {
                path.push_back(';');
                path.append(lpg_template);
            }
            char *temp = NewString(static_cast<int>(path.size() + 1));
            LpgString::CopyBounded(temp, path.size() + 1, path.c_str());
            template_directory = temp;
            ProcessPath(template_search_directory, template_directory);
        }
        {
            std::string path = home_directory;
            if (lpg_include != NULL)
            {
                path.push_back(';');
                path.append(lpg_include);
            }
            char *temp = NewString(static_cast<int>(path.size() + 1));
            LpgString::CopyBounded(temp, path.size() + 1, path.c_str());
            include_directory = temp;
            ProcessPath(include_search_directory, temp);
        }
        
        int length = strlen(main_input_file);
        char *temp_file_prefix = NewString(length + 3);
        file_prefix = temp_file_prefix;
        
        char *temp_lis_file = NewString(length + 3);
        lis_file = temp_lis_file;
        
        char *temp_tab_file = NewString(length + 3);
        tab_file = temp_tab_file;
        
        char *grm_file_ptr = NewString(length + 3);
        LpgString::CopyBounded(grm_file_ptr, static_cast<size_t>(length) + 3, main_input_file);
        NormalizeSlashes(grm_file_ptr); // Turn all (windows) backslashes into forward slashes in filename.
        grm_file = grm_file_ptr;
        
        int slash_index,
        dot_index = -1;
        for (slash_index = length - 1;
             slash_index >= 0 /* && grm_file[slash_index] != '\\' */ && grm_file[slash_index] != '/';
             slash_index--)
        {
            if (grm_file[slash_index] == '.')
                dot_index = slash_index;
        }
        
        const char *slash = (slash_index >= 0 ? &grm_file[slash_index] : NULL),
        *dot = (dot_index >= 0 ? &grm_file[dot_index] : NULL),
        *start = (slash ? slash + 1 : grm_file);
        size_t prefix_cap = static_cast<size_t>(length) + 3;
        if (dot == NULL) // if filename has no extension, copy it.
        {
            LpgString::CopyBounded(temp_file_prefix, prefix_cap, start);
            LpgString::CopyBounded(temp_lis_file, prefix_cap, start);
            LpgString::CopyBounded(temp_tab_file, prefix_cap, start);
            
            LpgString::AppendBounded(grm_file_ptr, static_cast<size_t>(length) + 3, ".g");
        }
        else // if file name contains an extension copy up to the dot
        {
            size_t stem = static_cast<size_t>(dot - start);
            memcpy(temp_file_prefix, start, stem);
            memcpy(temp_lis_file, start, stem);
            memcpy(temp_tab_file, start, stem);
            temp_lis_file[stem] = '\0';
            temp_tab_file[stem] = '\0';
            temp_file_prefix[stem] = '\0';
        }
        
        LpgString::AppendBounded(temp_lis_file, prefix_cap, ".l");
        LpgString::AppendBounded(temp_tab_file, prefix_cap, ".t");
        
        syslis = write ? fopen(lis_file, "w") : tmpfile();
        if (syslis  == (FILE *) NULL)
        {
            fprintf(stderr, "***ERROR: Listing output cannot be opened.\n");
            throw LpgError(12);
        }
    }
}

Option::~Option()
{
    //
    // Release all temporary strings now to save space.
    //
    for (int i = 0; i < temp_string.Length(); i++)
        delete [] temp_string[i];

    delete optionParser;
    delete optionProcessor;

    FlushReport();
    EmitJsonReport(stdout);
    if (syslis != NULL)
        fclose(syslis); // close listing file
}

const char* Option::GetFileTypeWithLanguage()
{
    switch (programming_language)
    {
    case  CPP:
    case  CPP2:
        return ".h";
    case  JAVA:
        return ".java";
    case  CSHARP:
        return ".cs";
    case  TSC:
        return ".ts";
    case  PYTHON3:
        return ".py";
    case  DART:
        return ".dart";
    case  GO:
        return ".go";
    case  RUST:
        return ".rs";
    default:
        return ".java";
    }
}

Token *Option::GetTokenLocation(const char *p, int length)
{
    Token *error_token = NULL;

    if (input_file_symbol != NULL)
    {
        assert(buffer_ptr != NULL);
        assert(parm_ptr != NULL);
        int error_location = (buffer_ptr + (p - parm_ptr)) - input_file_symbol -> Buffer();
        // RMF The following statement used to declare a shadowing var named 'error_token'...
        error_token = lex_stream -> GetErrorToken(input_file_symbol, error_location);
        error_token -> SetEndLocation(error_location + length - 1);
        error_token -> SetKind(0);
    }

    return error_token;
}

void Option::EmitHeader(Token *startToken, Token *endToken, const char *header)
{
    startToken = (startToken != NULL ? startToken : lex_stream -> GetTokenReference(0));
    endToken = (endToken != NULL ? endToken : lex_stream -> GetTokenReference(0));

    report.Put(startToken -> FileName());
    report.Put(":");
    report.Put(startToken -> Line());
    report.Put(":");
    report.Put(startToken -> Column());
    report.Put(":");
    report.Put(endToken -> EndLine());
    report.Put(":");
    report.Put(endToken -> EndColumn());
    report.Put(":");
    report.Put(startToken -> StartLocation());
    report.PutChar(':');
    report.Put(endToken -> EndLocation());
    report.Put(": ");

    if (*header != '\0')
        report.Put(header);

    return;
}

void Option::EmitHeader(Token *token, const char *header)
{
    EmitHeader(token, token, header);
}

void Option::EmitError(int index, const char *msg)
{
    return_code = 12;
    Emit(lex_stream -> GetTokenReference(index), "Error: ", msg);
}
void Option::EmitError(int index, Tuple<const char *> &msg)
{
    return_code = 12;
    Emit(lex_stream -> GetTokenReference(index), "Error: ", msg);
}
void Option::EmitWarning(int index, const char *msg)              { Emit(lex_stream -> GetTokenReference(index), "Warning: ", msg); }
void Option::EmitWarning(int index, Tuple<const char *> &msg)     { Emit(lex_stream -> GetTokenReference(index), "Warning: ", msg); }
void Option::EmitInformative(int index, const char *msg)          { Emit(lex_stream -> GetTokenReference(index), "Informative: ", msg); }
void Option::EmitInformative(int index, Tuple<const char *> &msg) { Emit(lex_stream -> GetTokenReference(index), "Informative: ", msg); }

void Option::Emit(Token *token, const char *header, const char *msg)
{
    Emit(token, token, header, msg);

    return;
}


void Option::Emit(Token *startToken, Token *endToken, const char *header, const char *msg)
{
    RecordDiagnostic(startToken, endToken, header, msg);
    EmitHeader(startToken, endToken, header);
    report.Put(msg);
    report.PutChar('\n');
    EmitSourceContext(startToken, endToken, msg);

    FlushReport(strcmp(header, "Error: ") == 0 ? stderr : stdout);

    return;
}


void Option::Emit(Token *token, const char *header, Tuple<const char *> &msg)
{
    Emit(token, token, header, msg);

    return;
}

void Option::Emit(Token *startToken, Token *endToken, const char *header, Tuple<const char *> &msg)
{
    std::string flat;
    for (int i = 0; i < msg.Length(); i++)
    {
        if (msg[i] != NULL)
            flat += msg[i];
    }
    RecordDiagnostic(startToken, endToken, header, flat.c_str());
    EmitHeader(startToken, endToken, header);
    for (int i = 0; i < msg.Length(); i++)
        report.Put(msg[i]);
    report.PutChar('\n');
    EmitSourceContext(startToken, endToken, flat.c_str());

    FlushReport(strcmp(header, "Error: ") == 0 ? stderr : stdout);

    return;
}

const char *Option::SuggestFix(const char *msg)
{
    if (msg == NULL)
        return NULL;
    if (strstr(msg, "Block not properly terminated") != NULL)
        return "Close the action block with the matching end marker (default is \"./\"; alternate styles use \"/)\").";
    if (strstr(msg, "invalid value for option") != NULL)
        return "Check -help for valid option values (e.g. programming_language=cpp|java|rust|...).";
    if (strstr(msg, "Shift/reduce conflict") != NULL)
        return "Disambiguate with %Left/%Right/%Priority, rewrite the rule, use -fail_on_conflicts to reject it, or -glr to retain intended ambiguity.";
    if (strstr(msg, "Reduce/reduce conflict") != NULL)
        return "Make the competing rules more specific, assign priorities, or use -glr to retain intended ambiguity.";
    if (strstr(msg, "not LALR") != NULL)
        return "Inspect the shift/reduce or reduce/reduce warnings above for the conflicting productions.";
    return NULL;
}

namespace
{
std::string JsonEscape(const std::string &value)
{
    static const char hex[] = "0123456789abcdef";
    std::string escaped;
    escaped.reserve(value.size() + 8);
    for (unsigned char c : value)
    {
        switch (c)
        {
        case '"': escaped += "\\\""; break;
        case '\\': escaped += "\\\\"; break;
        case '\b': escaped += "\\b"; break;
        case '\f': escaped += "\\f"; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        default:
            if (c < 0x20)
            {
                escaped += "\\u00";
                escaped += hex[c >> 4];
                escaped += hex[c & 0x0f];
            }
            else escaped += static_cast<char>(c);
            break;
        }
    }
    return escaped;
}
}

const char *Option::DiagnosticCode(const char *header, const char *message)
{
    if (message != NULL)
    {
        if (strstr(message, "Block not properly terminated") != NULL)
            return "LPG1001";
        if (strstr(message, "invalid option") != NULL ||
            strstr(message, "invalid value for option") != NULL)
            return "LPG1002";
        if (strstr(message, "Shift/reduce conflict") != NULL)
            return "LPG2001";
        if (strstr(message, "Reduce/reduce conflict") != NULL)
            return "LPG2002";
        if (strstr(message, "fail_on_conflicts") != NULL)
            return "LPG2003";
    }
    if (strcmp(header, "Warning: ") == 0)
        return "LPG0002";
    if (strcmp(header, "Informative: ") == 0)
        return "LPG0003";
    return "LPG0001";
}

void Option::RecordDiagnostic(Token *start_token,
                              Token *end_token,
                              const char *header,
                              const char *message)
{
    if (! DiagnosticsJson())
        return;

    if (start_token == NULL && lex_stream != NULL)
        start_token = lex_stream -> GetTokenReference(0);
    if (end_token == NULL)
        end_token = start_token;

    DiagnosticRecord record;
    record.code = DiagnosticCode(header, message);
    record.severity = strcmp(header, "Warning: ") == 0
                          ? "warning"
                          : strcmp(header, "Informative: ") == 0
                                ? "information"
                                : "error";
    record.message = message != NULL ? message : "";
    record.help = SuggestFix(message) != NULL ? SuggestFix(message) : "";

    if (start_token != NULL)
    {
        record.file = start_token -> FileName() != NULL
                          ? start_token -> FileName()
                          : (grm_file != NULL ? grm_file : "");
        record.start_line = start_token -> Line();
        record.start_column = start_token -> Column();
        record.start_offset = start_token -> StartLocation();
        record.end_line = end_token -> EndLine();
        record.end_column = end_token -> EndColumn();
        record.end_offset = end_token -> EndLocation();
    }
    else if (grm_file != NULL)
        record.file = grm_file;

    if (strstr(record.message.c_str(), "Shift/reduce conflict") != NULL)
        record.conflict_kind = "shift/reduce";
    else if (strstr(record.message.c_str(), "Reduce/reduce conflict") != NULL)
        record.conflict_kind = "reduce/reduce";

    const std::string marker = "example lookahead: ";
    const std::string::size_type marker_pos = record.message.find(marker);
    if (marker_pos != std::string::npos)
    {
        const std::string::size_type value_start = marker_pos + marker.size();
        const std::string::size_type value_end =
            record.message.find(')', value_start);
        record.example_lookahead =
            record.message.substr(value_start, value_end - value_start);
    }

    diagnostic_records.push_back(record);
}

void Option::SetGrammarHealth(
    int shift_reduce_conflicts,
    int reduce_reduce_conflicts,
    int soft_shift_conflicts,
    int soft_shift_reduce_conflicts,
    int soft_reduce_reduce_conflicts,
    const std::vector<std::string> &recover_symbols)
{
    grammar_health.available = true;
    grammar_health.shift_reduce_conflicts = shift_reduce_conflicts;
    grammar_health.reduce_reduce_conflicts = reduce_reduce_conflicts;
    grammar_health.soft_shift_conflicts = soft_shift_conflicts;
    grammar_health.soft_shift_reduce_conflicts = soft_shift_reduce_conflicts;
    grammar_health.soft_reduce_reduce_conflicts = soft_reduce_reduce_conflicts;
    grammar_health.recover_symbols = recover_symbols;
}

void Option::EmitFatal(const char *msg)
{
    return_code = 12;
    if (DiagnosticsJson())
    {
        DiagnosticRecord record;
        record.code = "LPG0001";
        record.severity = "error";
        record.message = msg != NULL ? msg : "";
        record.file = grm_file != NULL ? grm_file : "";
        diagnostic_records.push_back(record);
        return;
    }
    report.Put("***Error: ");
    report.Put(msg != NULL ? msg : "");
    report.PutChar('\n');
    FlushReport(stderr);
}

void Option::EmitJsonReport(FILE *output)
{
    if (! DiagnosticsJson() || json_report_emitted)
        return;
    json_report_emitted = true;

    int errors = 0;
    int warnings_count = 0;
    int information = 0;
    for (const DiagnosticRecord &record : diagnostic_records)
    {
        if (record.severity == "error")
            errors++;
        else if (record.severity == "warning")
            warnings_count++;
        else information++;
    }

    fprintf(output, "{\"schema_version\":1,\"diagnostics\":[");
    for (size_t i = 0; i < diagnostic_records.size(); i++)
    {
        const DiagnosticRecord &record = diagnostic_records[i];
        if (i != 0)
            fputc(',', output);
        fprintf(output,
                "{\"file\":\"%s\",\"span\":{\"start\":{\"line\":%d,"
                "\"column\":%d,\"offset\":%d},\"end\":{\"line\":%d,"
                "\"column\":%d,\"offset\":%d}},\"code\":\"%s\","
                "\"severity\":\"%s\",\"message\":\"%s\",\"help\":",
                JsonEscape(record.file).c_str(),
                record.start_line,
                record.start_column,
                record.start_offset,
                record.end_line,
                record.end_column,
                record.end_offset,
                JsonEscape(record.code).c_str(),
                JsonEscape(record.severity).c_str(),
                JsonEscape(record.message).c_str());
        if (record.help.empty())
            fputs("null", output);
        else fprintf(output, "\"%s\"", JsonEscape(record.help).c_str());
        if (! record.conflict_kind.empty())
            fprintf(output, ",\"conflict_kind\":\"%s\"",
                    JsonEscape(record.conflict_kind).c_str());
        if (! record.example_lookahead.empty())
            fprintf(output, ",\"example_lookahead\":\"%s\"",
                    JsonEscape(record.example_lookahead).c_str());
        fputc('}', output);
    }

    const int conflict_count =
        grammar_health.shift_reduce_conflicts +
        grammar_health.reduce_reduce_conflicts;
    fprintf(output,
            "],\"health\":{\"available\":%s,\"healthy\":%s,"
            "\"conflict_count\":%d,\"shift_reduce_conflicts\":%d,"
            "\"reduce_reduce_conflicts\":%d,\"backtrack\":%s,"
            "\"glr\":%s,\"glr_template_hint\":%s,\"soft_keywords\":%s,\"soft_conflicts\":{"
            "\"shift_shift\":%d,\"shift_reduce\":%d,\"reduce_reduce\":%d},"
            "\"recover_symbols\":[",
            grammar_health.available ? "true" : "false",
            (grammar_health.available && return_code == 0 &&
             errors == 0 && (conflict_count == 0 || glr))
                ? "true"
                : "false",
            conflict_count,
            grammar_health.shift_reduce_conflicts,
            grammar_health.reduce_reduce_conflicts,
            backtrack ? "true" : "false",
            glr ? "true" : "false",
            glr_template_hint_needed
                ? "\"use -template=.../glrParserTemplateF.gi\""
                : "null",
            soft_keywords ? "true" : "false",
            grammar_health.soft_shift_conflicts,
            grammar_health.soft_shift_reduce_conflicts,
            grammar_health.soft_reduce_reduce_conflicts);
    for (size_t i = 0; i < grammar_health.recover_symbols.size(); i++)
    {
        if (i != 0)
            fputc(',', output);
        fprintf(output, "\"%s\"",
                JsonEscape(grammar_health.recover_symbols[i]).c_str());
    }
    fprintf(output,
            "],\"programming_language\":\"%s\",\"write_enabled\":%s,"
            "\"warning_summary\":{\"errors\":%d,\"warnings\":%d,"
            "\"information\":%d}}}\n",
            JsonEscape(get_programing_language_str()).c_str(),
            write ? "true" : "false",
            errors,
            warnings_count,
            information);
    fflush(output);
}

void Option::EmitSourceContext(Token *startToken, Token *endToken, const char *msg)
{
    const char *hint = SuggestFix(msg);

    if (startToken == NULL || startToken -> FileSymbol() == NULL)
    {
        if (hint != NULL)
        {
            report.Put("  = help: ");
            report.Put(hint);
            report.PutChar('\n');
        }
        return;
    }

    InputFileSymbol *file = startToken -> FileSymbol();
    char *buffer = file -> Buffer();
    unsigned line_no = startToken -> Line();
    bool printed_excerpt = false;

    if (buffer != NULL &&
        line_no > 0 &&
        line_no < (unsigned) file -> LineLocation().Length())
    {
        unsigned line_start = file -> LineLocation(line_no);
        unsigned next_line = (line_no + 1 < (unsigned) file -> LineLocation().Length())
                                 ? file -> LineLocation(line_no + 1)
                                 : (unsigned) file -> BufferLength();
        if (next_line > line_start)
        {
            report.Put("  | ");
            for (unsigned i = line_start; i < next_line; i++)
            {
                char c = buffer[i];
                if (c == '\n' || c == '\r')
                    break;
                report.PutChar(c);
            }
            report.PutChar('\n');

            unsigned col = startToken -> Column();
            unsigned end_col = (endToken != NULL ? endToken -> EndColumn() : startToken -> EndColumn());
            if (col == 0)
                col = 1;
            if (end_col < col)
                end_col = col;

            report.Put("  | ");
            for (unsigned i = 1; i < col; i++)
                report.PutChar(' ');
            report.PutChar('^');
            for (unsigned i = col + 1; i <= end_col && i < col + 80; i++)
                report.PutChar('~');
            report.PutChar('\n');
            printed_excerpt = true;
        }
    }

    if (hint != NULL)
    {
        report.Put("  = help: ");
        report.Put(hint);
        report.PutChar('\n');
    }
    (void) printed_excerpt;
}


void Option::InvalidValueError(const char *start, const char *value, int length)
{
    char *str = NewString(start, length);

    Tuple<const char *> msg;
    msg.Next() = "\"";
    msg.Next() = value;
    msg.Next() = "\" is an invalid value for option \"";
    msg.Next() = str;
    msg.Next() = "\"";
    EmitError(GetTokenLocation(start, length), msg);
}


void Option::InvalidTripletValueError(const char *start, int length, const char *type, const char *format)
{
    char *str = NewString(start, length);

    Tuple<const char *> msg;
    msg.Next() = "Illegal ";
    msg.Next() = type;
    msg.Next() = " option specified: ";
    msg.Next() = str;
    msg.Next() = ". A value of the form \"";
    msg.Next() = format;
    msg.Next() = "\" was expected.";
    EmitError(GetTokenLocation(start, length), msg);
}


//
// Allocate a new copy of the string parameter str_ and return it.
// The deallocation will take place later.
//
const char *Option::AllocateString(const char *str_)
{
    size_t n = strlen(str_);
    char *str = new char[n + 1];
    memcpy(str, str_, n + 1);
    return str;
}

//
// Allocate a new string option and return it.
// The deallocation will take place later.
//
const char *Option::AllocateString(const char *str_, char c)
{
    size_t base = strlen(str_);
    size_t length = base + 4;
    char *str = new char[length + 1];
    memcpy(str, str_, base);
    str[base] = '=';
    str[base + 1] = '\'';
    str[base + 2] = c;
    str[base + 3] = '\'';
    str[base + 4] = '\0';
    return str;
}

//
// Allocate a new string option and return it.
// The deallocation will take place later.
//
const char *Option::AllocateString(const char *str_, const char *str2)
{
    size_t n1 = strlen(str_);
    size_t n2 = strlen(str2);
    bool quote = (*str2 != '(');
    size_t total = n1 + 1 + (quote ? 2 : 0) + n2;
    char *str = new char[total + 1];
    char *p = str;
    memcpy(p, str_, n1); p += n1;
    *p++ = '=';
    if (quote)
        *p++ = '"';
    memcpy(p, str2, n2); p += n2;
    if (quote)
        *p++ = '"';
    *p = '\0';
    return str;
}

const char *Option::AllocateString(const char *str_, int i)
{
    IntToString num(i);
    size_t n1 = strlen(str_);
    size_t n2 = strlen(num.String());
    char *str = new char[n1 + 1 + n2 + 1];
    char *p = str;
    memcpy(p, str_, n1); p += n1;
    *p++ = '=';
    memcpy(p, num.String(), n2 + 1);
    return str;
}

//
// action_block
// ast_directory
// ast_type
// attributes
// automatic_ast
//
const char *Option::ClassifyA(const char *start, bool flag)
{
    if (flag)
    {
        int i = OptionMatch(start, "ast", "block");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
            {
                return (i > 0
                          ? ReportMissingValue(start, "AST_BLOCK")
                          : ReportAmbiguousOption(start, "AST_BLOCK,ACTION_BLOCK, AST_DIRECTORY, AST_TYPE, ATTRIBUTES, AUTOMATIC_AST"));
            }
            else
            {
                if (*q == '(')
                {
                 
                    const char *block_begin;
                    p = GetStringValue(q+1 , block_begin);
                    const char *block_end;
                    p = GetStringValue(CleanUp(p), block_end);

                    //
                    // Problem: When a quoted symbol is specified on the command line, the Windows OS
                    // remove the surrounding quotes. Without the quote to delimit the block_end, GetStringValue
                    // will stop after the ")". In this case, we remove the closing paren here and reset
                    // the pointer so that it can be processed properly.
                    //
                    char *tail = (char *) &(block_end[strlen(block_end) - 1]);
                    if (*tail == ')')
                    {
                        *tail = '\0'; // remove the trailing ")" from the block_end
                        p--; // move the pointer back to process the ")"
                    }
                    p = CleanUp(p);
                    if (*p == ')')
                    {
                        p++;
                        if (*block_end != '\0') // the block-end symbol cannot be the null string
                        {
                            default_ast_block_begin = block_begin;
                            default_ast_block_end = block_end;
                            return p;
                        }
                    }

                    InvalidTripletValueError(start, p - start, "AST_BLOCK", "(begin_block_marker,end_block_marker)");

                    return p;
                }
            }
        }
        i = OptionMatch(start, "action", "block");  
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char* p = start + i + 1,
                * q = ValuedOption(p);
            if (q == NULL)
            {
                return (i > 0
                    ? ReportMissingValue(start, "ACTION_BLOCK")
                    : ReportAmbiguousOption(start, "ACTION_BLOCK, AST_DIRECTORY, AST_TYPE, ATTRIBUTES, AUTOMATIC_AST"));
            }
            else
            {
                if (*q == '(')
                {
                    const char* filename;
                    p = GetStringValue(q + 1, filename);
                    const char* block_begin;
                    p = GetStringValue(CleanUp(p), block_begin);
                    const char* block_end;
                    p = GetStringValue(CleanUp(p), block_end);

                    //
                    // Problem: When a quoted symbol is specified on the command line, the Windows OS
                    // remove the surrounding quotes. Without the quote to delimit the block_end, GetStringValue
                    // will stop after the ")". In this case, we remove the closing paren here and reset
                    // the pointer so that it can be processed properly.
                    //
                    char* tail = (char*)&(block_end[strlen(block_end) - 1]);
                    if (*tail == ')')
                    {
                        *tail = '\0'; // remove the trailing ")" from the block_end
                        p--; // move the pointer back to process the ")"
                    }
                    p = CleanUp(p);
                    if (*p == ')')
                    {
                        p++;
                        if (*block_end != '\0') // the block-end symbol cannot be the null string
                        {
                           action_options.Next().Set(GetTokenLocation(start, p - start), filename, block_begin, block_end);
                            return p;
                        }
                    }

                    InvalidTripletValueError(start, p - start, "ACTION_BLOCK", "(filename,begin_block_marker,end_block_marker)");

                    return p;
                }
            }
        }

        i = OptionMatch(start, "ast", "directory");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
            {
                return (i > 3
                          ? ReportMissingValue(start, "AST_DIRECTORY")
                          : ReportAmbiguousOption(start, "AST_DIRECTORY, AST_TYPE"));
            }
            else p = GetStringValue(q, ast_directory);
            ast_directory_location = GetTokenLocation(start, p - start);

            return (i < 4
                      ? ReportAmbiguousOption(start, "AST_DIRECTORY, AST_TYPE")
                      : p);
        }

        i = OptionMatch(start, "ast", "type");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
                 return ReportMissingValue(start, "AST_TYPE");
            else p = GetStringValue(q, ast_type);

            return p;
        }
    }

    int i = strxsub(start + 1, "ttributes");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        attributes = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "ATTRIBUTES") : p);
    }

    i = OptionMatch(start, "automatic", "ast");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        const char *p = start + i + 1,
                   *q = ValuedOption(p);

        if (q == NULL)
        {
            automatic_ast = (flag ? NESTED : NONE);
            return p;
        }
        else if (flag) // Cannot assign a value to "noautomatic_ast"
        {
            const char *value;
            p = GetStringValue(q, value);
            int length = strlen(value);
            if (strxsub(value, "none") == length)
                 automatic_ast = NONE;
            else if (strxsub(value, "nested") == length)
                 automatic_ast = NESTED;
            else if (strxsub(value, "toplevel") == length)
                 automatic_ast = TOPLEVEL;
            else InvalidValueError(start, value, i + 1);

            return p;
        }
    }

    return ClassifyBadOption(start, flag);
}

//
// backtrack
// byte
//
const char *Option::ClassifyB(const char *start, bool flag)
{
    int i = strxsub(start + 1, "acktrack");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        backtrack = flag;
        const char *p = start + i + 1;
        return (i < 1 ? ReportAmbiguousOption(start, "BACKTRACK, BYTE")
                      : ValuedOption(p) ? ReportValueNotRequired(start, "BACKTRACK") : p);
    }

    i = strxsub(start + 1, "yte");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        byte = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "BYTE") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// conflicts
//
const char *Option::ClassifyC(const char *start, bool flag)
{
    int i = strxsub(start + 1, "onflicts");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        conflicts = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "CONFLICTS") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// dat_directory
// dat_file
// dcl_file
// def_file
// debug
// directory_prefix
//
const char *Option::ClassifyD(const char *start, bool flag)
{
    if (flag)
    {
        int i = strxsub(start + 1, "iagnostics");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p),
                       *value;
            if (q == NULL)
                return ReportMissingValue(start, "DIAGNOSTICS");

            p = GetStringValue(q, value);
            int length = strlen(value);
            if (strxsub(value, "json") == length)
                diagnostics = JSON_DIAGNOSTICS;
            else if (strxsub(value, "human") == length)
                diagnostics = HUMAN_DIAGNOSTICS;
            else InvalidValueError(start, value, i + 1);
            return p;
        }

        i = OptionMatch(start, "dat", "directory");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
            {
                return (i > 0
                          ? ReportMissingValue(start, "DAT_DIRECTORY")
                          : ReportAmbiguousOption(start, "DAT_DIRECTORY, DAT_FILE, DCL_FILE, DEF_FILE, DEBUG, DIRECTORY_PREFIX"));
            }
            else p = GetStringValue(q, dat_directory);

            dat_directory_location = GetTokenLocation(start, p - start);

            return (i < 1
                      ? ReportAmbiguousOption(start, "DAT_DIRECTORY, DAT_FILE, DCL_FILE, DEF_FILE, DIRECTORY_PREFIX")
                      : i < 4
                          ? ReportAmbiguousOption(start, "DAT_DIRECTORY, DAT_FILE")
                          : p);
        }

        i = OptionMatch(start, "dat", "file");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
            {
                if (i == 1 && (start[1] == 'f' || start[1] == 'F'))
                     return ReportMissingValue(start, "DAT_FILE, DCL_FILE, DEF_FILE, DIRECTORY_PREFIX");
                else if (i > 0)
                     return ReportMissingValue(start, "DAT_FILE");
            }
            else
            {
                p = GetStringValue(q, dat_file);
                return ((i == 1 && (start[1] == 'f' || start[1] == 'F'))
                                ? ReportAmbiguousOption(start, "DAT_FILE, DCL_FILE, DEF_FILE, DIRECTORY_PREFIX")
                                : p);
            }
        }

        i = OptionMatch(start, "dcl", "file");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
            {
                if (i > 0)
                     return ReportMissingValue(start, "DCL_FILE");
            }
            else
            {
                p = GetStringValue(q, dcl_file);
                return p;
            }
        }

        i = OptionMatch(start, "def", "file");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
                return ReportMissingValue(start, "DEF_FILE");
            else
            {
                p = GetStringValue(q, def_file);
                return p;
            }
        }

        i = OptionMatch(start, "directory", "prefix");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
                return ReportMissingValue(start, "DIRECTORY_PREFIX");
            else
            {
                p = GetStringValue(q, directory_prefix);
                return p;
            }
        }
    }

    int i = strxsub(start + 1, "ebug");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        debug = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "DEBUG") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// ebnf
// edit
// error_maps
// escape
// export_terminals
// extends_parsetable
//
const char *Option::ClassifyE(const char *start, bool flag)
{
    int i = strxsub(start + 1, "bnf");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        ebnf = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "EBNF") : p);
    }

    i = OptionMatch(start, "extends", "parsetable");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        const char *p = start + i + 1,
                   *q = ValuedOption(p);
        if (i < 1)
             return ReportAmbiguousOption(start, "EBNF, EDIT, ERROR-MAPS, ESCAPE, EXPORT-TERMINALS, EXTENDS_PARSETABLE");
        else if (i < 2)
             return ReportAmbiguousOption(start, "EXPORT-TERMINALS, EXTENDS_PARSETABLE");
        else if (q == NULL && flag)
        {
            extends_parsetable = NewString("");
            return p;
        }
        else if (flag) // Cannot assign a value to "noextends_parsetable"
        {
            p = GetStringValue(q, extends_parsetable);
            return p;
        }
    }

    if (flag)
    {
        int i = strxsub(start + 1, "scape");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (i < 1)
                return ReportAmbiguousOption(start, "ESCAPE, EXPORT_TERMINALS");
            else if (q == NULL) // Must have a value
                return ReportMissingValue(start, "ESCAPE");
            else
            {
                const char *escape;
                p = GetStringValue(q, escape);
                if (escape[1] != NULL_CHAR) // more than a single character?
                    InvalidValueError(start, escape, i + 1);
                this -> escape = *escape;
                escape_location = GetTokenLocation(start, p - start);

                return p;
            }
        }

        i = OptionMatch(start, "export", "terminals");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
            {
                if (i > 0)
                     return ReportMissingValue(start, "EXPORT_TERMINALS");
            }
            else
            {
                if (*q != '(') // just the file name is provided.
                {
                    p = GetStringValue(q, exp_file);
                    return p;
                }
                else // a parenthesized list of 1, 2 or three arguments: (filename [, prefix] [, suffix])
                {
                    p = GetStringValue(q + 1, exp_file);
                    p = CleanUp(p);
                    if (*p == ')')
                        return ++p;

                    p = GetStringValue(CleanUp(p), exp_prefix);
                    p = CleanUp(p);
                    if (*p == ')')
                        return ++p;

                    p = GetStringValue(CleanUp(p), exp_suffix);
                    p = CleanUp(p);
                    if (*p == ')')
                        return ++p;

                    InvalidTripletValueError(start, p - start, "EXPORT_TERMINALS", "(filename [,prefix] [,suffix])");

                    return p;
                }
            }
        }
    }

    i = OptionMatch(start, "error", "maps");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        error_maps = flag;
        const char *p = start + i + 1;
        return (i < 1 ? ReportAmbiguousOption(start, "EDIT, ERROR_MAPS")
                      : ValuedOption(p) ? ReportValueNotRequired(start, "ERROR_MAPS") : p);
    }

    i = strxsub(start + 1, "dit");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        edit = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "EDIT") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// fail_on_conflicts
// factory
// file_prefix
// filter
// first
// follow
//
const char *Option::ClassifyF(const char *start, bool flag)
{
    int i = OptionMatch(start, "fail_on", "conflicts");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        fail_on_conflicts = flag;
        const char *p = start + i + 1;
        return (i < 1
                  ? ReportAmbiguousOption(start, "FAIL_ON_CONFLICTS, FACTORY, FILE_PREFIX, FILTER, FIRST, FOLLOW")
                  : ValuedOption(p) ? ReportValueNotRequired(start, "FAIL_ON_CONFLICTS") : p);
    }

    if (flag)
    {
        i = strxsub(start + 1, "actory");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {

            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
            {
                return (i < 1
                          ? ReportAmbiguousOption(start, "FAIL_ON_CONFLICTS, FACTORY, FILE_PREFIX, FILTER, FIRST, FOLLOW")
                          : (i == 1 && (start[1] == 'i' || start[1] == 'I'))
                                ? ReportAmbiguousOption(start, "FILE_PREFIX, FILTER, FIRST")
                                : ReportMissingValue(start, "FILE_PREFIX"));
            }
            else
            {
                p = GetStringValue(q, factory);
                return p;
            }
        }

        i = OptionMatch(start, "file", "prefix");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
            {
                if (i > 0)
                     return (i == 1 && (start[1] == 'i' || start[1] == 'I'))
                                ? ReportAmbiguousOption(start, "FILE_PREFIX, FILTER, FIRST")
                                : ReportMissingValue(start, "FILE_PREFIX");
            }
            else
            {
                p = GetStringValue(q, file_prefix);
                return p;
            }
        }

        i = strxsub(start + 1, "ilter");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {

            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
                return ReportMissingValue(start, "FILTER");
            else
            {
                p = GetStringValue(q, filter);
                ProcessPath(filter_file, filter);
                return p;
            }
        }
    }

    i = strxsub(start + 1, "irst");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        first = flag;
        const char *p = start + i + 1;
        return (i < 1 ? ReportAmbiguousOption(start, "FIRST, FOLLOW")
                      : ValuedOption(p) ? ReportValueNotRequired(start, "FIRST") : p);
    }

    i = strxsub(start + 1, "ollow");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        follow = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "FOLLOW") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// glr
// goto_default
//
const char *Option::ClassifyG(const char *start, bool flag)
{
    int i = strxsub(start + 1, "lr");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        glr = flag;
        const char *p = start + i + 1;
        return (i < 1 ? ReportAmbiguousOption(start, "GLR, GOTO_DEFAULT")
                      : ValuedOption(p) ? ReportValueNotRequired(start, "GLR") : p);
    }

    i = OptionMatch(start, "goto", "default");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        goto_default = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "GOTO_DEFAULT") : p);
    }

    return ClassifyBadOption(start, flag);
}


//
// Headers
//
const char *Option::ClassifyH(const char *start, bool flag)
{
    if (flag)
    {
        int i = strxsub(start + 1, "eaders");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
            {
                if (i > 0)
                    return ReportMissingValue(start, "HEADERS");
            }
            else
            {
                if (*q == '(')
                {
                    const char *filename;
                    p = GetStringValue(q + 1, filename);
                    const char *block_begin;
                    p = GetStringValue(CleanUp(p), block_begin);
                    const char *block_end;
                    p = GetStringValue(CleanUp(p), block_end);
                    p = CleanUp(p);

                    if (*p == ')')
                    {
                        header_options.Next().Set(GetTokenLocation(start, p - start), filename, block_begin, block_end);
                        return ++p;
                    }

                    InvalidTripletValueError(start, p - start, "HEADERS", "(filename,begin_block_marker,end_block_marker)");

                    return p;
                }
            }
        }
    }

    return ClassifyBadOption(start, flag);
}

//
// ignore_block
// imp_file
// import_terminals
// include_directory/include_directories
//
const char *Option::ClassifyI(const char *start, bool flag)
{
    if (flag)
    {
        int i = OptionMatch(start, "ignore", "block");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (i < 1)
                 return ReportAmbiguousOption(start, "IGNORE_BLOCK, IMP_FILE, IMPORT_TERMINALS, INCLUDE_DIRECTORY");
            else if (q == NULL) // Must have a value
                 return ReportMissingValue(start, "IGNORE_BLOCK");
            else
            {
                const char *ignore_block;
                p = GetStringValue(q, ignore_block);
                action_blocks.FindOrInsertIgnoredBlock(ignore_block, strlen(ignore_block));
                return p;
            }
        }

        i = OptionMatch(start, "imp", "file");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (i < 3)
                 return ReportAmbiguousOption(start, "IMP_FILE, IMPORT_TERMINALS");
            else if (q == NULL) // Must have a value
                 return ReportMissingValue(start, "IMP_FILE");
            else
            {
                p = GetStringValue(q, imp_file);
                return p;
            }
        }

        i = OptionMatch(start, "import", "terminals");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
                return ReportMissingValue(start, "IMPORT_TERMINALS");
            else
            {
                p = GetStringValue(q, import_terminals);
                ProcessPath(import_file, import_terminals);
                return p;
            }
        }

        i = OptionMatch(start, "include", "directory");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
                 return ReportMissingValue(start, "INCLUDE_DIRECTORY");
            else
            {
                p = GetStringValue(q, include_directory);
                ProcessPath(include_search_directory, include_directory , home_directory);
                // Skip index 0 (home_directory); validate user-specified components.
                for (int di = 1; di < include_search_directory.Length(); di++)
                {
                    const char *dir = include_search_directory[di];
                    if (dir == NULL || dir[0] == '\0')
                        continue;
                    if (! PathIsDirectory(dir))
                    {
                        Tuple<const char *> msg;
                        msg.Next() = "Include directory does not exist: \"";
                        msg.Next() = dir;
                        msg.Next() = "\"";
                        EmitError(GetTokenLocation(start, p - start), msg);
                        break;
                    }
                }
                return p;
            }
        }

        i = OptionMatch(start, "include", "directories"); // accept "directories" as a substitute for "directory".
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
                 return ReportMissingValue(start, "INCLUDE_DIRECTORIES");
            else
            {
                p = GetStringValue(q, include_directory);
                return p;
            }
        }
    }

    return ClassifyBadOption(start, flag);
}

//
// lalr
// legacy
// list
//
const char *Option::ClassifyL(const char *start, bool flag)
{
    int i = strxsub(start + 1, "alr");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        const char *p = start + i + 1,
                   *q = ValuedOption(p);
        if (q == NULL)
        {
            if (i > 0)
            {
                slr = ! flag;
                lalr_level = 1;
            }
            else ReportAmbiguousOption((flag ? start : start - 2), "LALR, LEGACY, LIST");
        }
        else if (flag) // Cannot assign a value to "nolalr"
        {
            const char *value;
            p = GetValue(q, value);

            slr = false;
            if (verify(value))
                 lalr_level = atoi(value);
            else InvalidValueError(start, value, i + 1);
        }

        return p;
    }

    i = strxsub(start + 1, "egacy");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        legacy = flag;
        const char *p = start + i + 1;
        return (i < 1 ? ReportAmbiguousOption(start, "LEGACY, LIST")
                      : ValuedOption(p) ? ReportValueNotRequired(start, "LEGACY") : p);
    }

    i = strxsub(start + 1, "ist");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        list = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "LIST") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// margin
// max_cases
//
const char *Option::ClassifyM(const char *start, bool flag)
{
    if (flag)
    {
        int i = strxsub(start + 1, "argin");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
            {
                if (i > 0)
                     return ReportMissingValue(start, "MARGIN");
            }
            else
            {
                const char *value;
                p = GetValue(q, value);

                if (verify(value))
                     margin = atoi(value);
                else InvalidValueError(start, value, i + 1);

                return p;
            }
        }

        i = OptionMatch(start, "max", "cases");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
            {
                if (i > 0)
                     return ReportMissingValue(start, "MAX_CASES");
            }
            else
            {
                const char *value;
                p = GetValue(q, value);

                if (verify(value))
                     max_cases = atoi(value);
                else InvalidValueError(start, value, i + 1);

                return p;
            }
        }
    }

    return ClassifyBadOption(start, flag);
}

//
// names
// nt_check
//
const char *Option::ClassifyN(const char *start, bool flag)
{
    if (flag)
    {
        int i = strxsub(start + 1, "ames");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
            {
                if (i > 0)
                    return ReportMissingValue(start, "NAMES");
            }
            else
            {
                const char *value;
                p = GetStringValue(q, value);
                int length = strlen(value);

                if (strxsub(value, "optimized") == length)
                    names = OPTIMIZED;
                else
                {
                    if (length == 1)
                         return ReportAmbiguousOption(start, "MAXIMUM, MINIMUM");
                    else if (strxsub(value, "maximum") == length)
                         names = MAXIMUM;
                    else if (strxsub(value, "minimum") == length)
                         names = MINIMUM;
                    else InvalidValueError(start, value, i + 1);
                }

                return p;
            }
        }
    }

    int i = OptionMatch(start, "nt", "check");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        nt_check = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "NT_CHECK") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// or_marker
// out_directory
//
const char *Option::ClassifyO(const char *start, bool flag)
{
    if (flag)
    {
        int i = OptionMatch(start, "or", "marker");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (i == 0)
                return ReportAmbiguousOption(start, "OR_MARKER, OUT_DIRECTORY");
            else if (q == NULL) // Must have a value
                return ReportMissingValue(start, "OR_MARKER");
            else
            {
                const char *or_marker;
                p = GetStringValue(q, or_marker);
                if (or_marker[1] != NULL_CHAR) // more than a single character?
                    InvalidValueError(start, or_marker, i + 1);
                this -> or_marker = *or_marker;
                or_marker_location = GetTokenLocation(start, p - start);

                return p;
            }
        }

        i = OptionMatch(start, "out", "directory");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
                 return ReportMissingValue(start, "OUT_DIRECTORY");
            else p = GetStringValue(q, out_directory);

            out_directory_location = GetTokenLocation(start, p - start);

            return (i < 1
                      ? ReportAmbiguousOption(start, "OUT_DIRECTORY, OR_MARKER")
                      : p);
        }
    }

    return ClassifyBadOption(start, flag);
}

//
// package
// parent_saved
// parsetable_interfaces
// prefix
// priority
// programming_language
// prs_file
//
const char *Option::ClassifyP(const char *start, bool flag)
{
    // int i = OptionMatch(start, "parsetable", "interfaces");
    // if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    // {
    //     const char *p = start + i + 1,
    //                *q = ValuedOption(p);
    //     if (i < 1)
    //          return ReportAmbiguousOption(start, "PACKAGE, PARSETABLE-INTERFACES, PREFIX, PRIORITY, PROGRAMMING_LANGUAGE, PRS_FILE");
    //     else if (i < 2)
    //          return ReportAmbiguousOption(start, "PACKAGE, PARSETABLE-INTERFACES");
    //     else if (q == NULL && flag)
    //     {
    //         parsetable_interfaces = NewString("");
    //         return p;
    //     }
    //     else if (flag) // Cannot assign a value to "noparsetable_interfaces"
    //     {
    //         p = GetStringValue(q, parsetable_interfaces);
    //         return p;
    //     }
    // }

    if (flag)
    {
        int i = strxsub(start + 1, "ackage");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (i < 1)
                 return ReportAmbiguousOption(start, "PACKAGE, PARENT_SAVED, PARSETABLE-INTERFACES, PREFIX, PRIORITY, PROGRAMMING_LANGUAGE, PRS_FILE");
            else if (i < 2)
                 return ReportAmbiguousOption(start, "PACKAGE, PARENT_SAVED, PARSETABLE-INTERFACES");
            else if (q == NULL) // Must have a value
                 return ReportMissingValue(start, "PACKAGE");
            else
            {
                p = GetStringValue(q, package);
                return p;
            }
        }

        i = OptionMatch(start, "parsetable", "interfaces");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (i < 3)
                 return ReportAmbiguousOption(start, "PARENT_SAVED, PARSETABLE-INTERFACES");
            if (q == NULL && flag)
            {
                parsetable_interfaces = NewString("");
                return p;
            }
            else if (flag) // Cannot assign a value to "noparsetable_interfaces"
            {
                p = GetStringValue(q, parsetable_interfaces);
                return p;
            }
        }

        i = strxsub(start + 1, "refix");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (i < 2)
                 return ReportAmbiguousOption(start, "PREFIX, PRIORITY, PROGRAMMING_LANGUAGE, PRS_FILE");
            else if (q == NULL) // Must have a value
                 return ReportMissingValue(start, "PREFIX");
            else
            {
                p = GetStringValue(q, prefix);
                return p;
            }
        }

        i = strxsub(start + 1, "riority");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            priority = flag;
            const char *p = start + i + 1;
            return (ValuedOption(p) ? ReportValueNotRequired(start, "PRIORITY") : p);
        }

        i = OptionMatch(start, "programming", "language");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p),
                       *value;
            if (q == NULL) // Must have a value
                return ReportMissingValue(start, "PROGRAMMING_LANGUAGE");
            else
            {
                p = GetStringValue(q, value);
                int length = strlen(value);
                if (strxsub(value, "none") == length)
                     programming_language = NONE;
                else if (length == 10 && strxsub(value, "cpp_legacy") == 10)
                    programming_language = CPP;
                // Reject removed values before cpp/csharp: strxsub("c","cpp")==1
                // (and strxsub("c","csharp")==1) would accept "c" as a live language.
                else if (strxsub(value, "python2") == length ||
                         strxsub(value, "xml") == length ||
                         (length == 1 && (value[0] == 'c' || value[0] == 'C')) ||
                         strxsub(value, "ml") == length ||
                         strxsub(value, "plx") == length ||
                         strxsub(value, "plxasm") == length)
                {
                    Tuple<const char *> msg;
                    msg.Next() = "programming_language value \"";
                    msg.Next() = value;
                    msg.Next() = "\" was removed. "
                                 "Use java, cpp, rt_cpp, csharp, typescript, "
                                 "python3, dart, go, or rust.";
                    EmitError(GetTokenLocation(start, i + 1), msg);
                    // Fatal: do not continue and generate with a stale language.
                    throw LpgError(12);
                }
                else if ((length == 6 && strxsub(value, "rt_cpp") == 6) ||
                         (length == 3 && strxsub(value, "cpp") == 3) ||
                         (length == 3 && strxsub(value, "c++") == 3))
                    programming_language = CPP2;
                else if ((length == 6 && strxsub(value, "csharp") == 6) ||
                         (length == 2 && strxsub(value, "c#") == 2))
                    programming_language = CSHARP;
                else if (strxsub(value, "typescript") == length)
                    programming_language = TSC;
                else if (strxsub(value, "python3") == length)
                    programming_language = PYTHON3;
                else if (strxsub(value, "dart") == length)
                    programming_language = DART;
                else if (strxsub(value, "go") == length)
                    programming_language = GO;
                else if (strxsub(value, "rust") == length)
                    programming_language = RUST;
                else if (strxsub(value, "java") == length)
                     programming_language = JAVA;
                else InvalidValueError(start, value, i + 1);

                return p;
            }
        }

        i = OptionMatch(start, "prs", "file");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
                return ReportMissingValue(start, "PRS_FILE");
            else
            {
                p = GetStringValue(q, prs_file);
                return p;
            }
        }
    }

    int i = OptionMatch(start, "parent", "saved");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        parent_saved = flag;
        const char *p = start + i + 1;
        if (i < 1)
             return ReportAmbiguousOption(start, "PACKAGE, PARENT_SAVED, PARSETABLE-INTERFACES, PREFIX, PRIORITY, PROGRAMMING_LANGUAGE, PRS_FILE");
        else if (i < 2)
             return ReportAmbiguousOption(start, "PACKAGE, PARENT_SAVED, PARSETABLE-INTERFACES");
        return (ValuedOption(p) ? ReportValueNotRequired(start, "PARENT_SAVED") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// quiet
//
const char *Option::ClassifyQ(const char *start, bool flag)
{
    int i = strxsub(start + 1, "uiet");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        quiet = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "QUIET") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// read_reduce
// remap_terminals
// rule_classnames
//
const char *Option::ClassifyR(const char *start, bool flag)
{
    if (flag)
    {
        int i = OptionMatch(start, "rule", "classnames");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
            {
                if (i > 0)
                     return ReportMissingValue(start, "RULE_CLASSNAMES");
            }
            else
            {
                const char *value;
                p = GetStringValue(q, value);
                int length = strlen(value);
                if (length == 1)
                    return ReportAmbiguousOption(start, "SEQUENTIAL, STABLE");
                else if (strxsub(value, "sequential") == length)
                     rule_classnames = SEQUENTIAL;
                else if (strxsub(value, "stable") == length)
                     rule_classnames = STABLE;
                else InvalidValueError(start, value, i + 1);
            }
            return p;
        }
    }

    int i = OptionMatch(start, "read", "reduce");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        read_reduce = flag;
        const char *p = start + i + 1;
        return (i < 2 ? ReportAmbiguousOption(start, "READ_REDUCE, REMAP_TERMINALS")
                      : ValuedOption(p) ? ReportValueNotRequired(start, "READ_REDUCE") : p);
    }

    i = OptionMatch(start, "remap", "terminals");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        remap_terminals = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "REMAP_TERMINALS") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// // save_parent
// scopes
// serialize
// shift_default
// single_productions
// slr
// soft_keywords
// states
// suffix
// sym_file
//
const char *Option::ClassifyS(const char *start, bool flag)
{
    if (flag)
    {
        int i = strxsub(start + 1, "uffix");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);

            if (q == NULL)
            {
                if (i > 0)
                     return ReportMissingValue(start, "SUFFIX");
            }
            else
            {
                p = GetStringValue(q, suffix);
                return p;
            }
        }

        i = OptionMatch(start, "sym", "file");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL) // Must have a value
            {
                if (i > 0)
                     return ReportMissingValue(start, "SYM_FILE");
            }
            else
            {
                p = GetStringValue(q, sym_file);
                return p;
            }
        }
    }

    // int i = OptionMatch(start, "save", "parent");
    // if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    // {
    //     save_parent = flag;
    //     const char *p = start + i + 1;
    //     return (i < 1
    //               ? ReportAmbiguousOption(start, "SAVE_PARENT, SCOPES, SERIALIZE, SOFT_KEYWORDS, SHIFT_DEFAULT, SINGLE_PRODUCTIONS, SLR, STATES")
    //               : ValuedOption(p) ? ReportValueNotRequired(start, "SAVE_PARENT") : p);
    // }
    //
    // i = strxsub(start + 1, "copes");
    // if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    // {
    //     scopes = flag;
    //     const char *p = start + i + 1;
    //     return (ValuedOption(p) ? ReportValueNotRequired(start, "SCOPES") : p);
    // }

    int i = strxsub(start + 1, "copes");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        scopes = flag;
        const char *p = start + i + 1;
        return (i < 1 ? ReportAmbiguousOption(start, "SCOPES, SERIALIZE, SOFT_KEYWORDS, SHIFT_DEFAULT, SINGLE_PRODUCTIONS, SLR, STATES")
                      : ValuedOption(p) ? ReportValueNotRequired(start, "SCOPES") : p);
    }

    i = strxsub(start + 1, "erialize");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        serialize = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "SERIALIZE") : p);
    }

    i = OptionMatch(start, "soft", "keywords");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        soft_keywords = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "SOFT_KEYWORDS") : p);
    }

    i = OptionMatch(start, "shift", "default");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        shift_default = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "SHIFT_DEFAULT") : p);
    }

    i = OptionMatch(start, "single", "productions");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        single_productions = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "SINGLE_PRODUCTIONS") : p);
    }

    i = strxsub(start + 1, "lr");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        slr = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "SLR") : p);
    }

    i = strxsub(start + 1, "tates");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        states = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "STATES") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// tab_file
// table
// template
// trace
// trailers
//
const char *Option::ClassifyT(const char *start, bool flag)
{
    if (flag)
    {
        int i = OptionMatch(start, "tab", "file");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
            {
                return (i < 3 && strxsub(start, "tab") == i + 1)
                           ? ReportAmbiguousOption(start, "TABLE, TAB_FILE")
                           : ReportMissingValue(start, "TAB_FILE");
            }
            else
            {
                p = GetStringValue(q, tab_file);
                return (i == 0 ? ReportAmbiguousOption(start, "TAB_FILE, TABLE, TEMPLATE, TRACE")
                               : i < 3 && (! (i == 1 && (start[1] == 'f' || start[1] == 'F')))
                                        ? ReportAmbiguousOption(start, "TAB_FILE, TABLE")
                                        : p);
            }
        }

        i = strxsub(start + 1, "emplate");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);

            if (q == NULL)
            {
                if (i > 0)
                     return ReportMissingValue(start, "TEMPLATE");
            }
            else
            {
                p = GetStringValue(q, template_name);
                return p;
            }
        }

        i = strxsub(start + 1, "railers");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
            {
                if (i > 0)
                    return ReportMissingValue(start, "TRAILERS");
            }
            else
            {
                if (*q == '(')
                {
                    const char *filename;
                    p = GetStringValue(q + 1, filename);
                    const char *block_begin;
                    p = GetStringValue(CleanUp(p), block_begin);
                    const char *block_end;
                    p = GetStringValue(CleanUp(p), block_end);
                    p = CleanUp(p);

                    if (*p == ')')
                    {
                        trailer_options.Next().Set(GetTokenLocation(start, p - start), filename, block_begin, block_end);
                        return ++p;
                    }

                    InvalidTripletValueError(start, p - start, "TRAILERS", "(filename,begin_block_marker,end_block_marker)");

                    return p;
                }
            }
        }
    }

    int i = strxsub(start + 1, "able");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        const char *p = start + i + 1,
                   *q = ValuedOption(p);
        table = flag;
        if (i == 0)
             return ReportAmbiguousOption(start, "TABLE, TRACE");
        else if (q == NULL)
             return p;
        else if (flag) // Cannot assign a value to "notable"
        {
            const char *value;
            p = GetStringValue(q, value);
            int length = strlen(value);
            if (strxsub(value, "none") == length)
            {
                 table = false;
                 programming_language = NONE;
            }
            else if (length == 10 && strxsub(value, "cpp_legacy") == 10)
                programming_language = CPP;
            // Same prefix trap as ClassifyP: "c" must not match "cpp"/"csharp".
            else if (strxsub(value, "python2") == length ||
                     strxsub(value, "xml") == length ||
                     (length == 1 && (value[0] == 'c' || value[0] == 'C')) ||
                     strxsub(value, "ml") == length ||
                     strxsub(value, "plx") == length ||
                     strxsub(value, "plxasm") == length)
            {
                Tuple<const char *> msg;
                msg.Next() = "table language value \"";
                msg.Next() = value;
                msg.Next() = "\" was removed. "
                             "Use java, cpp, rt_cpp, csharp, typescript, "
                             "python3, dart, go, or rust.";
                EmitError(GetTokenLocation(start, i + 1), msg);
                throw LpgError(12);
            }
            else if ((length == 6 && strxsub(value, "rt_cpp") == 6) ||
                     (length == 3 && strxsub(value, "cpp") == 3) ||
                     (length == 3 && strxsub(value, "c++") == 3))
                programming_language = CPP2;
            else if ((length == 6 && strxsub(value, "csharp") == 6) ||
                     (length == 2 && strxsub(value, "c#") == 2))
                programming_language = CSHARP;
            else if (strxsub(value, "typescript") == length)
                 programming_language = TSC;
            else if (strxsub(value, "python3") == length)
                programming_language = PYTHON3;
            else if (strxsub(value, "dart") == length)
                programming_language = DART;
            else if (strxsub(value, "go") == length)
                programming_language = GO;
            else if (strxsub(value, "rust") == length)
                programming_language = RUST;
            else if (strxsub(value, "java") == length)
                programming_language = JAVA;
            else InvalidValueError(start, value, i + 1);

            return p;
        }
    }

    i = strxsub(start + 1, "race");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        const char *p = start + i + 1,
                   *q = ValuedOption(p);
        if (q == NULL)
        {
            trace = (flag ? CONFLICTS : NONE);
            return p;
        }
        else if (flag) // Cannot assign a value to "notrace"
        {
            const char *value;
            p = GetStringValue(q, value);
            int length = strlen(value);
            if (strxsub(value, "none") == length)
                 trace = NONE;
            else if (strxsub(value, "conflicts") == length)
                 trace = CONFLICTS;
            else if (strxsub(value, "full") == length)
                 trace = FULL;
            else InvalidValueError(start, value, i + 1);

            return p;
        }
    }

    return ClassifyBadOption(start, flag);
}

//
// variables
// verbose
// visitor
// visitor_type
//
const char *Option::ClassifyV(const char *start, bool flag)
{
    int i = strxsub(start + 1, "ariables");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        const char *p = start + i + 1,
                   *q = ValuedOption(p);
        if (i < 1)
            ReportAmbiguousOption(start, "VARIABLES, VERBOSE, VISITOR, VISITOR_TYPE");

        if (q == NULL)
        {
            variables = (flag ? BOTH : NONE);
            return p;
        }
        else if (flag) // Cannot assign a value to "novariables"
        {
            const char *value;
            p = GetStringValue(q, value);
            int length = strlen(value);
            if (strxsub(value, "none") == length)
                 variables = NONE;
            else if (strxsub(value, "both") == length)
                 variables = BOTH;
            else if (strxsub(value, "terminals") == length)
                 variables = TERMINALS;
            else if (strxsub(value, "nt") == length ||
                     strxsub(value, "nonterminals") == length ||
                     strxsub(value, "non_terminals") == length)
                 variables = NON_TERMINALS;
            else InvalidValueError(start, value, i + 1);

            return p;
        }
    }

    i = strxsub(start + 1, "erbose");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        verbose = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "VERBOSE") : p);
    }

    i = strxsub(start + 1, "isitor");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        const char *p = start + i + 1,
                   *q = ValuedOption(p);

        if (q == NULL)
        {
            visitor |= (flag ? DEFAULT : NONE);
            return p;
        }
        else if (flag) // Cannot assign a value to "novisitor"
        {
            const char *value;
            p = GetStringValue(q, value);
            int length = strlen(value);
            if (strxsub(value, "none") == length)
                 visitor = NONE;
            else if (strxsub(value, "default") == length)
                 visitor = DEFAULT;
            else if (strxsub(value, "preorder") == length)
                 visitor = PREORDER;
            else if (strxsub(value, "all") == length)
                visitor = DEFAULT|PREORDER;
            else InvalidValueError(start, value, i + 1);

            return p;
        }
    }

    if (flag)
    {
        i = OptionMatch(start, "visitor", "type");
        if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
        {
            const char *p = start + i + 1,
                       *q = ValuedOption(p);
            if (q == NULL)
                return ReportMissingValue(start, "VISITOR_TYPE");
            else
            {
                p = GetStringValue(q, visitor_type);
                return p;
            }
        }
    }

    return ClassifyBadOption(start, flag);
}

//
// warnings
// write
//
const char *Option::ClassifyW(const char *start, bool flag)
{
    int i = strxsub(start + 1, "arnings");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        warnings = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "WARNINGS") : p);
    }

    i = strxsub(start + 1, "rite");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        write = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "WRITE") : p);
    }

    return ClassifyBadOption(start, flag);
}

//
// xreference
//
const char *Option::ClassifyX(const char *start, bool flag)
{
    int i = strxsub(start + 1, "reference");
    if (start[i + 1] == '=' || IsDelimiter(start[i + 1]))
    {
        xref = flag;
        const char *p = start + i + 1;
        return (ValuedOption(p) ? ReportValueNotRequired(start, "REFERENCE") : p);
    }

    return ClassifyBadOption(start, flag);
}


const char *Option::AdvancePastOption(const char *p)
{
    while (! IsDelimiter(*p))
        p++;
    while (IsSpace(*p))
        p++;
    if (*p == '=')
    {
        while (IsSpace(*p))
            p++;
        while (!IsDelimiter(*p))
            p++;
    }

    return p;
}


//
//
//
const char *Option::ClassifyBadOption(const char *start, bool flag)
{
    const char *tail = AdvancePastOption(start);
    start = (flag ? start : start - 2); // restore the "no" prefix if needed

    int length = tail - start;
    char *str = NewString(start, length);

    Tuple<const char *> msg;
    msg.Next() = "\"";
    msg.Next() = str;
    msg.Next() = "\" is an invalid option";
    EmitError(GetTokenLocation(start, length), msg);

    return tail;
}


//
//
//
const char *Option::ReportAmbiguousOption(const char *start, const char *choice_msg)
{
    const char *tail = AdvancePastOption(start);

    int length = tail - start;
    char *str = NewString(start, length);

    Tuple<const char *> msg;
    msg.Next() = "The option \"";
    msg.Next() = str;
    msg.Next() = "\" is ambiguous: ";
    msg.Next() = choice_msg;
    EmitError(GetTokenLocation(start, length), msg);

    return tail;
}


//
//
//
const char *Option::ReportMissingValue(const char *start, const char *)
{
    const char *tail = AdvancePastOption(start);

    int length = tail - start;
    char *str = NewString(start, length);

    Tuple<const char *> msg;
    msg.Next() = "A value is required for this option: \"";
    msg.Next() = str;
    msg.Next() = "\"";
    EmitError(GetTokenLocation(start, length), msg);

    return tail;
}

//
//
//
const char *Option::ReportValueNotRequired(const char *start, const char *)
{
    const char *tail = AdvancePastOption(start);

    int length = tail - start;
    char *str = NewString(start, length);

    Tuple<const char *> msg;
    msg.Next() = "An illegal value was specified for this option: \"";
    msg.Next() = str;
    msg.Next() = "\"";
    EmitError(GetTokenLocation(start, length), msg);

    return tail;
}

//
//
//
bool Option::IsDelimiter(char c)
{
    return (c == NULL_CHAR || c == ',' || Code::IsSpace(c));
}

//
// Clean front of string
//
const char *Option::CleanUp(const char *parm)
{
    while (*parm != NULL_CHAR && IsDelimiter(*parm))
        parm++;
    return parm;
}

//
//
//
const char *Option::ValuedOption(const char *p)
{
    while (IsSpace(*p))
        p++;
    if (*p == '=')
    {
        for (p++; IsSpace(*p); p++)
            ;
        return p;
    }

    return NULL;
}

//
//
//
const char *Option::GetValue(const char *p, const char *&value)
{
    const char *tail = p;

    while (! IsDelimiter(*tail))
        tail++;
    value =  NewString(p, tail - p);

    return tail;
}


//
// Starting at location p in a string pick up a value (which may be
// quoted) and assign it to the reference parameter value. Return
// the ending location of the value in the string.
//
const char *Option::GetStringValue(const char *start, const char *&value)
{
    const char *tail = start;
    char quote = ((*tail == '\'' || *tail == '\"') ? *tail : NULL_CHAR);
    int length;

    //
    //
    //
    if (quote != NULL_CHAR)
    {
        for (tail++; (*tail != quote) && (*tail != NULL_CHAR); tail++)
            ;

        if (*tail == quote)
        {
            start++; // move past the leading quote
            length = tail - start;
            tail++;  // move past trailing quote
        }
        else // an unterminated string
        {
            length = tail - start;
            char *str = NewString(start, length);

            Tuple<const char *> msg;
            msg.Next() = "The string ";
            msg.Next() = str;
            msg.Next() = "\" was not properly terminated";
            EmitError(GetTokenLocation(start, length), msg);
        }
    }
    else
    {
        while (! IsDelimiter(*tail))
            tail++;
        length = tail - start;
    }

    value = NewString(start, length);

    return tail;
}


//
//
//
int Option::OptionMatch(const char *p, const char *word1, const char *word2)
{
    if ((p[0] == *word1 || p[0] == ToUpper(*word1)) &&
        (p[1] == *word2 || p[1] == ToUpper(*word2)) &&
        (p[2] == '=' || IsDelimiter(p[2])))
        return 1;

    int length1 = strlen(word1);
    int length = length1 + strlen(word2) + 1; // +1 for separator
    char *name = new char[length + 1];

    strcpy(name, word1);
    strcat(name, word2);
    int i = strxsub(p, name) - 1;

    strcpy(name, word1);
    strcat(name, "_");
    strcat(name, word2);

    i = Max(i, strxsub(p, name) - 1);

    name[length1] = '-';

    i = Max(i, strxsub(p, name) - 1);

    delete [] name;

    return i;
}


//
//
//
void Option::ProcessBlock(BlockInfo &block_info)
{
    int block_begin_length = strlen(block_info.block_begin);
    Tuple<const char *> msg;

    const char *filename = ExpandFilename(block_info.filename);
    ActionFileSymbol *filename_symbol = action_blocks.FindFilename(filename, strlen(filename));
    if (filename_symbol != NULL)
    {
        msg.Next() = "The action filename \"";
        msg.Next() = filename;
        msg.Next() = "\" was previously associated with the begin block marker \"";
        msg.Next() = filename_symbol -> Block() -> Name();
        msg.Next() = "\" and cannot be reassociated with the block marker \"";
        msg.Next() = block_info.block_begin;
        msg.Next() = "\"";
        EmitError(block_info.location, msg);
    }
    else if (action_blocks.FindBlockname(block_info.block_begin, block_begin_length) != NULL)
    {
        msg.Next() = "The action block begin string \"";
        msg.Next() = block_info.block_begin;
        msg.Next() = "\" was used in a previous block definition";
        EmitError(block_info.location, msg);
    }
    else
    {
        (void) action_blocks.InsertBlock(block_info.location,
                                         BlockSymbol::MAIN_BLOCK,
                                         action_blocks.FindOrInsertFilename(filename, strlen(filename)),
                                         block_info.block_begin,
                                         block_begin_length,
                                         block_info.block_end,
                                         strlen(block_info.block_end));
    }
}


//
//
//
void Option::ProcessHeader(BlockInfo &block_info)
{
    Tuple<const char *> msg;

    const char *filename = ExpandFilename(block_info.filename);
    ActionFileSymbol *filename_symbol = action_blocks.FindFilename(filename, strlen(filename));
    if (filename_symbol != NULL)
    {
        if (action_blocks.FindBlockname(block_info.block_begin, strlen(block_info.block_begin)) != NULL)
        {
            msg.Next() = "The action block begin string \"";
            msg.Next() = block_info.block_begin;
            msg.Next() = "\" was used in a previous definition";
            EmitError(block_info.location, msg);
        }
        else
        {
            action_blocks.InsertBlock(block_info.location,
                                      BlockSymbol::HEADER_BLOCK,
                                      filename_symbol,
                                      block_info.block_begin,
                                      strlen(block_info.block_begin),
                                      block_info.block_end,
                                      strlen(block_info.block_end));
        }
    }
    else
    {
        msg.Next() = "The action filename \"";
        msg.Next() = filename;
        msg.Next() = "\" must be associated with an action block before being used here";
        EmitError(block_info.location, msg);
    }

    return;
}


//
//
//
void Option::ProcessTrailer(BlockInfo &block_info)
{
    Tuple<const char *> msg;

    const char *filename = ExpandFilename(block_info.filename);
    ActionFileSymbol *filename_symbol = action_blocks.FindFilename(filename, strlen(filename));
    if (filename_symbol != NULL)
    {
        if (action_blocks.FindBlockname(block_info.block_begin, strlen(block_info.block_begin)) != NULL)
        {
            msg.Next() = "The action block begin string \"";
            msg.Next() = block_info.block_begin;
            msg.Next() = "\" was used in a previous definition";
            EmitError(block_info.location, msg);
        }
        else
        {
            action_blocks.InsertBlock(block_info.location,
                                      BlockSymbol::TRAILER_BLOCK,
                                      filename_symbol,
                                      block_info.block_begin,
                                      strlen(block_info.block_begin),
                                      block_info.block_end,
                                      strlen(block_info.block_end));
        }
    }
    else
    {
        msg.Next() = "The action filename \"";
        msg.Next() = filename;
        msg.Next() = "\" must be associated with an action block before being used here";
        EmitError(block_info.location, msg);
    }

    return;
}


//
//
//
void Option::CheckBlockMarker(Token *marker_location, const char *block_marker)
{
    Tuple<const char *> msg;
    int block_marker_length = strlen(block_marker);

    if (block_marker_length == 1)
    {
        if (*block_marker == escape)
        {
            msg.Next() = "The escape symbol, \"";
            msg.Next() = block_marker;
            msg.Next() = "\", cannot be used as a block marker.";
            EmitError(marker_location, msg);
        }
        else if (*block_marker == or_marker)
        {
            msg.Next() = "The OR_MARKER symbol, \"";
            msg.Next() = block_marker;
            msg.Next() = "\", cannot be used as a block marker.";
            EmitError(marker_location, msg);
        }
    }
    else if (strcmp(block_marker, "::=") == 0)
         EmitError(marker_location, "\"::=\" cannot be used as a block marker");
    else if (strcmp(block_marker, "->") == 0)
         EmitError(marker_location, "\"->\" cannot be used as a block marker");

    return;
}


//
//
//
void Option::CheckGlobalOptionsConsistency()
{
    Tuple<const char *> msg;
    or_marker_location = or_marker_location == NULL ? lex_stream -> GetTokenReference(0) : or_marker_location;
    escape_location = escape_location == NULL ? lex_stream -> GetTokenReference(0) : escape_location;

    if (or_marker == escape)
    {
         const char str[2] = { escape, '\0'};
         msg.Reset();
         msg.Next() = "The escape symbol, \"";
         msg.Next() = str;
         msg.Next() = "\", cannot be used as the OR_MARKER";
         EmitError(or_marker_location, msg);
    }
    else if (or_marker == ':')
         EmitError(or_marker_location, "\":\" cannot be used as the OR_MARKER");
    else if (or_marker == '<')
         EmitError(or_marker_location, "\"<\" cannot be used as the OR_MARKER");
    else if (or_marker == '-')
         EmitError(or_marker_location, "\"-\" cannot be used as the OR_MARKER");
    else if (or_marker == '\'')
         EmitError(or_marker_location, "\"'\" cannot be used as the OR_MARKER");
    else if (or_marker == '\"')
         EmitError(or_marker_location, "\" cannot be used as the OR_MARKER");

    if (escape == or_marker)
    {
         const char str[2] = { or_marker, '\0'};
         msg.Reset();
         msg.Next() = "The OR_MARKER symbol, \"";
         msg.Next() = str;
         msg.Next() = "\", cannot be used as the OR_MARKER";
         EmitError(escape_location, msg);
    }
    else if (escape == ':')
         EmitError(escape_location, "\":\" cannot be used as ESCAPE");
    else if (escape == '<')
         EmitError(escape_location, "\"<\" cannot be used as ESCAPE");
    else if (escape == '-')
         EmitError(escape_location, "\"-\" cannot be used as ESCAPE");
    else if (escape == '\'')
         EmitError(escape_location, "\"'\" cannot be used as ESCAPE");
    else if (escape == '\"')
         EmitError(escape_location, "\" cannot be used as ESCAPE");

    //
    // Make sure that the block markers are compatible with all other options
    //
    for (int i = 0; i < action_blocks.NumActionBlocks(); i++)
    {
        BlockSymbol *block = action_blocks[i];

        CheckBlockMarker(block -> Location(), block -> BlockBegin());
        CheckBlockMarker(block -> Location(), block -> BlockEnd());
    }

    //
    // When more than one action block with the same starting letter is specified,
    // we check to make sure that there is no overlap in their block begin markers.
    //
    for (int c = 0; c < 256; c++)
    {
        Tuple<BlockSymbol *> &blocks = action_blocks.ActionBlocks(c);
        for (int i = 0; i < blocks.Length(); i++)
        {
            BlockSymbol *block1 = blocks[i];

            for (int k = i + 1; k < blocks.Length(); k++)
            {
                BlockSymbol *block2 = blocks[k];
                if (block1 -> BlockBeginLength() < block2 -> BlockBeginLength())
                {
                    if (strncmp(block1 -> BlockBegin(), block2 -> BlockBegin(), block1 -> BlockBeginLength()) == 0)
                    {
                        msg.Reset();
                        msg.Next() = "The block marker \"";
                        msg.Next() = block1 -> BlockBegin();
                        msg.Next() = "\" is a substring of the block marker \"";
                        msg.Next() = block2 -> BlockBegin();
                        msg.Next() = "\"";
                        EmitError(block1 -> Location(), msg);
                    }
                }
                else if (block2 -> BlockBeginLength() < block1 -> BlockBeginLength())
                {
                    if (strncmp(block2 -> BlockBegin(), block1 -> BlockBegin(), block2 -> BlockBeginLength()) == 0)
                    {
                        msg.Reset();
                        msg.Next() = "The block marker \"";
                        msg.Next() = block2 -> BlockBegin();
                        msg.Next() = "\" is a substring of the block marker \"";
                        msg.Next() = block1 -> BlockBegin();
                        msg.Next() = "\"";
                        EmitError(block2 -> Location(), msg);
                    }
                }
            }
        }
    }

    return;
}


//
//
//
void Option::CheckAutomaticAst()
{
    assert(package);       // package should always be processed first 
    assert(ast_directory); // ast_directory should always be processed first 

    if (*ast_directory == '\0')
    {
        ast_package = package;

        ast_directory = NewString("");

        int num_dots = 0,
            ast_package_length = strlen(ast_package);
        for (int i = 0; i < ast_package_length; i++)
        {
            if (ast_package[i] == '.')
                num_dots++;
        }
        const char *prefix = "../";
        int prefix_length = strlen(prefix) * (num_dots + 1),
            directory_length = prefix_length + ast_package_length + 1;
        char *temp = NewString(directory_length + 1);
        ast_directory_prefix = temp;
        if (strlen(ast_package) == 0)
            strcpy(temp, prefix + 1);
        else
        {
            strcpy(temp, prefix);
            for (int i = 0; i < num_dots; i++)
                strcat(temp, prefix);
            for (int k = 0, j = prefix_length; k < ast_package_length; k++, j++)
                 temp[j] = (ast_package[k] == '.' ? '/' : ast_package[k]);
            strcpy(&temp[prefix_length + ast_package_length], "/");
        }
        error_code ec;
       
        std::string path = ast_directory_prefix;
        if(path[path.size()-1]== '/')
        {
            path = path.substr(0, path.size() - 1);
        }
        if(!std::filesystem::exists(path, ec))
        {
	        if (!std::filesystem::create_directories(path, ec))
            {
                Tuple<const char*> msg;
                msg.Reset();
                msg.Next() = "Unable to create ast directory: \"";
                msg.Next() = path.c_str();
                msg.Next() = "\"";
                EmitError(ast_directory_location, msg);
            }
            else
            {
                Tuple<const char*> msg;
                msg.Reset();
                msg.Next() = "Create ast directory: \"";
                msg.Next() = path.c_str();
                msg.Next() = "\"";
                EmitInformative(ast_directory_location, msg);
            }
        }
    }
    else
    {
        Tuple<const char *> msg;
        ast_directory_location = ast_directory_location == NULL ? lex_stream -> GetTokenReference(0) : ast_directory_location;

        //
        // Now construct the ast_directory_prefix
        //
        int directory_length = strlen(ast_directory);
        char *temp = NewString(directory_length + 2);
        ast_directory_prefix = temp;
        strcpy(temp, ast_directory);
        if (temp[directory_length - 1] != '/' &&
            temp[directory_length - 1] != '\\')
        {
            strcat(temp, "/");
            directory_length++;
        }
    
        int start_index = (Code::IsAlpha(temp[0]) && temp[1] == ':'
                                ? 2 // possible for WIN32 only
                                : 0);
        if (temp[start_index] == '/' || temp[start_index] == '\\')
        {
            temp[start_index] = '/';  // make sure it's a forward slash
            start_index++;
        }

        int i = start_index;
        while (i < directory_length)
        {
            while (temp[i] == '.')
                i++;
            if (temp[i] == '/' || temp[i] == '\\')
            {
                temp[i] = '/';  // make sure it's a forward slash
                i++;
                continue;
            }
            break;
        }

        char *start_ast_package = &temp[i];

        while (i < directory_length)
        {
            char *name = &temp[i];
            while (temp[i] != '/' && temp[i] != '\\')
                i++;
            temp[i] = '\0'; // test this prefix
            if (! Code::IsValidVariableName(name))
            {
                msg.Reset();
                msg.Next() = "The subpackage \"";
                msg.Next() = name;
                msg.Next() = "\" specified in the ast_directory option is not a valid  variable name";
                EmitError(ast_directory_location, msg);

                break;
            }
            int rc = SystemMkdir(temp);
            if (rc == ENOENT)
            {
                msg.Reset();
                msg.Next() = "Unable to create ast directory: \"";
                msg.Next() = temp;
                msg.Next() = "\"";
                EmitError(ast_directory_location, msg);
    
                break;
            }
            else if (rc == ENOTDIR)
            {
                msg.Reset();
                msg.Next() = "The file \"";
                msg.Next() = temp;
                msg.Next() = "\" is not a directory";
                EmitError(ast_directory_location, msg);

                break;
            }
            temp[i] = '/';  // restore slash
            i++;
        }

        //
        // Remove all leading "./"
        //
        for (i = 0; temp[i] == '.' && temp[i + 1] == '/'; i += 2)
            ;

        //
        // Check whether or not the ast package is a subpackage of the
        // of the main package. If so, make sure that the main package
        // is not the unnamed package.
        //
        char *temp_ast_package;
        if (start_ast_package == &temp[i])
        {
            if (*package == NULL_CHAR)
            {
                temp_ast_package = NewString("");
                if(programming_language == JAVA )
                {
                   
                    EmitError(ast_directory_location,
                        "The ast package cannot be a subpackage of the unnamed package."
                        " Please specify a package name using the package option");
                }
                else if(CSHARP == programming_language)
                {
                    EmitError(ast_directory_location,
                        "The namespace cannot be a subpackage of the unnamed namespace."
                        " Please specify a namesapce name using the package option");
                }
            }
            else
            {
                int package_length = strlen(package),
                    ast_package_length = strlen(start_ast_package) - 1,
                    length = package_length + 1 + ast_package_length;
                temp_ast_package = NewString(length + 1);
                strcpy(temp_ast_package, package);
                strcat(temp_ast_package, ".");
                strncat(temp_ast_package, start_ast_package, ast_package_length);
                temp_ast_package[length] = '\0';
            }
        }
        else
        {
            temp_ast_package = NewString(start_ast_package, strlen(start_ast_package) - 1);
        }

        //
        // Replace all slashes (/) by dots (.) in package name.
        //
        for (char *s = temp_ast_package; *s != '\0'; s++)
        {
            if (*s == '/')
                *s = '.';
        }

        ast_package = temp_ast_package;
    }

    return;
}


void Option::ProcessOptions(const char *parm)
{
    for (parm = CleanUp(parm); *parm != NULL_CHAR; parm = CleanUp(parm))
    {
        bool flag = ! ((parm[0] == 'n' || parm[0] == 'N') &&
                       (parm[1] == 'o' || parm[1] == 'O'));

        if (!NEW_OPTIONS_CODE) {
            if (! flag)
                parm += 2; // skip the "NO" prefix
        }

        try {
            OptionValue *value = NULL;

            if (NEW_OPTIONS_CODE) {
                value = optionParser->parse(parm);
            }
            
            if (value != NULL) {
                value->processSetting(optionProcessor);
            } else {
                unsigned char c = (unsigned char) (*parm);
                parm = (this ->* classify_option[c < 128 ? c : 0])(parm, flag);
            }
        } catch (ValueFormatException& vfe) {
            cerr << "Improper value '" << vfe.value() << "' for option '" << vfe.optionDescriptor()->getName() << "': " << vfe.message() << endl;
        }
    }

    return;
}

//
//
//
void Option::ProcessUserOptions(InputFileSymbol *input_file_symbol_, char *line, int length)
{
    char *parm = new char[length + 1];
    strncpy(parm, line, length);
    parm[length] = NULL_CHAR;

    //
    // Save location info in case an error is detected.
    //
    this -> input_file_symbol = input_file_symbol_;
    this -> buffer_ptr = line;
    this -> parm_ptr = parm;

    //
    // Process parameters
    //
    try
    {
        ProcessOptions(parm);
    }
    catch (...)
    {
        this -> input_file_symbol = NULL;
        this -> buffer_ptr = NULL;
        this -> parm_ptr = NULL;
        delete [] parm;
        throw;
    }

    delete [] parm;
    // Avoid dangling pointers into the freed parm buffer (Release UAF risk
    // if a later GetTokenLocation still sees input_file_symbol set).
    this -> input_file_symbol = NULL;
    this -> buffer_ptr = NULL;
    this -> parm_ptr = NULL;

    return;
}


void Option::ProcessCommandOptions()
{
    //
    // No location info exists for command line options.
    //
    this -> input_file_symbol = NULL;
    this -> buffer_ptr = NULL;
    this -> parm_ptr = NULL;

    int parm_length = 0;
    for (int k = 1; k < argc - 1; k++)
        parm_length += strlen(argv[k]);
    parm_length++;

    char *parm = new char[parm_length + 1];
    *parm = NULL_CHAR;

    //
    // Note that in specifying options on the command line, the command processor
    // will always break up options when it sees a blank. Thus, if an option contains
    // blanks, it must be quoted (at least in windows. Furthermore, to make sure that
    // LPG processes such an option properly, the value assigned to the option must
    // also be quoted. For example, to specify:
    //
    //    -include-directory=c:\program files\templates
    //
    // the user would write:
    //
    //    "-include-directory='c:\program files\templates'"
    //
    // Observe that in this example, we used single quotes for the value
    // 'c:\program files\templates'. To use double quote, we would have
    // had to escape them with a backslash (\").
    //
    Tuple<const char *> msg;
    auto equals_ignoring_case = [](const char *left, const char *right)
    {
        while (*left != '\0' && *right != '\0')
        {
            if (std::tolower(static_cast<unsigned char>(*left)) !=
                std::tolower(static_cast<unsigned char>(*right)))
                return false;
            left++;
            right++;
        }
        return *left == *right;
    };
    for (int m = 1; m < argc - 1; m++)
    {
        if (*(argv[m]) == '-')
        {
            const char *option = argv[m];
            while (*option == '-')
                option++;
            // --dry-run / -dry_run are aliases for -nowrite (analysis only).
            if (equals_ignoring_case(option, "dry-run") ||
                equals_ignoring_case(option, "dry_run"))
                option = "nowrite";
            strcat(parm, option);
        }
        else
        {
            strcat(parm, argv[m]);

            msg.Reset();
            msg.Next() = "Option \"";
            msg.Next() = argv[m];
            msg.Next() = "\" is missing preceding '-'";
            EmitError(0, msg);
        }
        strcat(parm, ",");
    }

    try
    {
        ProcessOptions(parm);
        AddDefaultResourcePaths();
    }
    catch (...)
    {
        delete [] parm;
        throw;
    }

    delete [] parm;

    return;
}


//
//
//
void Option::ProcessPath(Tuple<const char *> &list, const char *path, const char *start_directory)
{
    int allocation_length = strlen(path) + (start_directory == NULL ? 0 : strlen(start_directory));
    if (allocation_length == 0)
        return;

    char *str = NewString(allocation_length + 3); // 2 semicolons 1 for terminating \0
    if (start_directory != NULL)
    {
        strcpy(str, start_directory);
        strcat(str, ";");
    }
    else *str = '\0';
    strcat(str, path);
    NormalizeSlashes(str);

    int length;
    for (length = strlen(str) - 1; length >= 0 && IsSpace(str[length]); length--)
        ; // remove trailing spaces
    if (length < 0)
        length = 0;

    if (str[length] != ';') // make sure that the path ends with a terminating semicolon
        str[++length] = ';';
    str[++length] = NULL_CHAR;

    list.Reset();
    list.Next() = str;
    for (int i = 0; i < length; i++)
    {
        if (str[i] == ';')
        {
            str[i] = '\0';
            if (str[i + 1] != '\0')
                list.Next() = &(str[i + 1]);
        }
    }

    return;
}

void Option::AddDefaultResourcePaths()
{
    const char *language_directory = NULL;
    switch (programming_language)
    {
    case CPP:
    case CPP2:
        language_directory = "rt_cpp";
        break;
    case JAVA:
        language_directory = "java";
        break;
    case CSHARP:
        language_directory = "csharp";
        break;
    case DART:
        language_directory = "dart";
        break;
    case GO:
        language_directory = "go";
        break;
    case RUST:
        language_directory = "rust";
        break;
    case PYTHON3:
        language_directory = "python3";
        break;
    case TSC:
        language_directory = "typescript";
        break;
    default:
        return;
    }

    const string resource_root = FindLpgResourceRoot(argv[0]);
    if (resource_root.empty())
        return;

    auto append_path = [this](Tuple<const char *> &paths,
                              const filesystem::path &additional)
    {
        error_code error;
        if (! filesystem::is_directory(additional, error))
            return;

        string combined;
        for (int i = 0; i < paths.Length(); ++i)
        {
            if (! combined.empty())
                combined += ';';
            combined += paths[i];
        }
        if (! combined.empty())
            combined += ';';
        combined += additional.string();
        ProcessPath(paths, combined.c_str());
    };

    const filesystem::path language(language_directory);
    append_path(template_search_directory,
                filesystem::path(resource_root) / "templates" / language);
    append_path(include_search_directory,
                filesystem::path(resource_root) / "include" / language);
}


const char *Option::GetPrefix(const char *filename)
{
    const char *forward_slash = strrchr(filename, '/'),
               *backward_slash = strrchr(filename, '\\'),
               *slash = (forward_slash > backward_slash ? forward_slash : backward_slash),
               *colon = strrchr(filename, ':'), // Windows files may use format: d:filename
               *separator = (colon > slash ? colon : slash);
    int length = (separator == NULL ? 0 : separator - filename + 1);
    return NewString(filename, length);
}


const char *Option::GetFile(const char *directory, const char *file_suffix, const char *file_type)
{
    assert(directory);

    int dir_length = strlen(directory),
        filename_length = dir_length + strlen(file_prefix) + strlen(file_suffix) + strlen(file_type);
    char *temp = NewString(filename_length + 2);
    strcpy(temp, directory);
    if (dir_length > 0 && temp[dir_length - 1] != '/' && temp[dir_length - 1] != '\\')
        strcat(temp, "/");
    strcat(temp, file_prefix);
    strcat(temp, file_suffix);
    strcat(temp, file_type);

    return temp;
}

void Option::RelocateListingFile(const char *destination)
{
    const filesystem::path old_path =
        filesystem::absolute(lis_file).lexically_normal();
    const filesystem::path new_path =
        filesystem::absolute(destination).lexically_normal();

    if (old_path == new_path)
    {
        lis_file = destination;
        return;
    }

    if (fflush(syslis) != 0 || fclose(syslis) != 0)
    {
        syslis = NULL;
        throw LpgError(12, string("Unable to finalize listing file \"") +
                               old_path.string() + "\"");
    }
    syslis = NULL;

    error_code error;
    filesystem::copy_file(old_path,
                          new_path,
                          filesystem::copy_options::overwrite_existing,
                          error);
    if (! error)
        filesystem::remove(old_path, error);

    if (error)
    {
        syslis = fopen(old_path.string().c_str(), "a");
        throw LpgError(12, string("Unable to move listing file to \"") +
                               new_path.string() + "\": " + error.message());
    }

    syslis = fopen(new_path.string().c_str(), "a");
    if (syslis == NULL)
        throw LpgError(12, string("Unable to reopen listing file \"") +
                               new_path.string() + "\"");

    lis_file = destination;
}


//
// Strip out the directory prefix if any and return just
// the filename.
//
const char *Option::GetFilename(const char *filespec)
{
    const char *forward_slash = strrchr(filespec, '/'),
               *backward_slash = strrchr(filespec, '\\'),
               *slash = (forward_slash > backward_slash ? forward_slash : backward_slash),
               *colon = strrchr(filespec, ':'), // Windows files may use format: d:filename
               *separator = (colon > slash ? colon : slash);
    return (separator ? separator + 1 : filespec);
}


const char *Option::GetType(const char *filespec)
{
    const char *start = GetFilename(filespec),
               *dot = strrchr(filespec, '.');
    int length = (dot == NULL ? strlen(start) : dot - start);
    return NewString(start, length);
}


const char *Option::ExpandFilename(const char *filename)
{
    return (*filename == '*'
                       ? GetFile(out_directory, filename + 1, "")
                       : filename);
}


void Option::CheckDirectory(Token *directory_location, const char *directory)
{
    Tuple<const char *> msg;
    directory_location = directory_location == NULL ? lex_stream -> GetTokenReference(0) : directory_location;

    //
    //
    //
    int directory_length = strlen(directory);
    char *temp = new char[directory_length + 2];
    strcpy(temp, directory);
    if (directory_length > 0 &&
	temp[directory_length - 1] != '/' &&
        temp[directory_length - 1] != '\\')
    {
        strcat(temp, "/");
        directory_length++;
    }
    
    int start_index = (Code::IsAlpha(temp[0]) && temp[1] == ':'
                            ? 2 // possible for WIN32 only
                            : 0);
    if (temp[start_index] == '/' || temp[start_index] == '\\')
    {
        temp[start_index] = '/';  // make sure it's a forward slash
        start_index++;
    }

    int i = start_index;
    while (i < directory_length)
    {
        while (temp[i] == '.')
            i++;
        if (temp[i] == '/' || temp[i] == '\\')
        {
            temp[i] = '/';  // make sure it's a forward slash
            i++;
            continue;
        }
        break;
    }

    //
    //
    //
    while (i < directory_length)
    {
        while (temp[i] != '/' && temp[i] != '\\')
            i++;
        temp[i] = '\0'; // test this prefix

        int rc = SystemMkdir(temp);
        if (rc == ENOENT)
        {
            msg.Reset();
            msg.Next() = "Unable to create directory: \"";
            msg.Next() = temp;
            msg.Next() = "\"";
            EmitError(directory_location, msg);
    
            break;
        }
        else if (rc == ENOTDIR)
        {
            msg.Reset();
            msg.Next() = "The file \"";
            msg.Next() = temp;
            msg.Next() = "\" is not a directory";
            EmitError(directory_location, msg);

            break;
        }
        temp[i] = '/';  // restore slash
        i++;
    }

    delete [] temp;

    return;
}

bool Option::IsTopLevel() const
{
	return automatic_ast == TOPLEVEL;
}

bool Option::IsPackage() const
{
    return package != NULL && package[0] != '\0';
}

void Option::CompleteOptionProcessing()
{
    //
    //
    //
    if (escape == ' ')
    {
        if(       programming_language == DART ||
                  programming_language == TSC )
        {
            escape = '%';
        }
        else
        {
            escape = '$';
           
        }
                            
    }

    //
    // Always keep package as a valid C-string. Emitters use IsPackage() /
    // package[0]; never leave NULL for strlen() under Release (-DNDEBUG).
    //
    if (package == NULL)
        package = NewString("");

    //
    //
    //
    if (prefix == NULL)
        prefix = NewString("");

    //
    //
    //
    if (suffix == NULL)
        suffix = NewString("");

    //
    // Default to the deterministic parser template when neither the
    // command line nor the grammar set -template / %Options template=.
    // Search paths from AddDefaultResourcePaths resolve the basename.
    //
    if (template_name == NULL || template_name[0] == '\0')
        template_name = NewString("dtParserTemplateF.gi");

    assert(file_prefix);

    //
    // The out_directory must be processed prior to all the output files
    // which will use it: prs_file, sym_file, dcl_file, def_file,
    // exp_file, imp_file and the action files.
    //
    if (out_directory == NULL)
         out_directory = NewString(home_directory, strlen(home_directory));
    else if (write) CheckDirectory(out_directory_location, out_directory);

    if (write)
        RelocateListingFile(GetFile(out_directory, ".", "l"));

    // Gate product opens before ProcessBlock / AST emitters create
    // ActionFileSymbol temps beside the grammar (or -out directory).
    OutputTransaction::Instance().SetPublishEnabled(write);

    //
    //
    //
    {
        for (int i = 0; i < action_options.Length(); i++)
            ProcessBlock(action_options[i]);
    }
    {
        for (int i = 0; i < header_options.Length(); i++)
            ProcessHeader(header_options[i]);
    }
    {
        for (int i = 0; i < trailer_options.Length(); i++)
            ProcessTrailer(trailer_options[i]);
    }

	
    //
    // If no action block was specified for the default
    // action filename and the default block-begin marker,
    // we specify one here.
    //
    default_block = action_blocks.FindBlockname(default_block_begin, strlen(default_block_begin));
    if (default_block == NULL)
    {
        assert(action_blocks.FindFilename("", 0) == 0);
        BlockInfo default_info;
        default_info.Set(lex_stream -> GetTokenReference(0), "", default_block_begin, default_block_end);
        ProcessBlock(default_info);
        default_block = action_blocks.FindBlockname(default_block_begin, strlen(default_block_begin));
    }
	
    default_action_file = default_block -> ActionfileSymbol();
    default_action_prefix = GetPrefix(default_action_file -> Name());
    action_type = GetType(default_action_file -> Name());


    ast_block = action_blocks.FindBlockname(default_ast_block_begin, strlen(default_ast_block_begin));
    if (ast_block == NULL)
    {
        assert(action_blocks.FindFilename("", 0) == 0);
        BlockInfo default_info;
        default_info.Set(lex_stream->GetTokenReference(0), "", default_ast_block_begin, default_ast_block_end);
        ProcessBlock(default_info);
        ast_block = action_blocks.FindBlockname(default_ast_block_begin, strlen(default_ast_block_begin));
    }

	
    //
    //
    //
    CheckGlobalOptionsConsistency();

    //
    //
    //
    if (ast_directory == NULL)
        ast_directory = NewString("");
    // else: The ast directory will be checked if automatic_ast generation is requested. See CheckAutomaticAst.

    //
    //
    //
    if (automatic_ast != NONE)
    {
        if (variables == NONE)
            variables = BOTH;

        if (automatic_ast == TOPLEVEL)
            CheckAutomaticAst();
    }

    //
    //
    //
    if (ast_package == NULL)
        ast_package = NewString("");

    //
    //
    //
    if (ast_type == NULL)
        ast_type = NewString("Ast");

    //
    //
    //
    if (visitor_type == NULL)
        visitor_type = NewString("Visitor");

    //
    //
    //

    auto help_get_file = [&](const char* file_suffix)
    {

        const char* file_type = "";
            switch (programming_language)
            {
            case  CPP:
            case  CPP2:
                file_type = "h"; break;
            case  JAVA:
                file_type = "java"; break;
            case  TSC:
                file_type = "ts"; break;
            case  DART:
                file_type = "dart"; break;
            case  GO:
                file_type = "go"; break;
            case  RUST:
                file_type = "rs"; break;
            case  PYTHON3:
                file_type = "py"; break;

            case  CSHARP:
                file_type = "cs"; break;
            default:
                file_type = "java";
            }
        return GetFile(out_directory,file_suffix, file_type);
    };

	
    if (prs_file == NULL)
    {
        prs_file = help_get_file("prs."); 
    }
    prs_type = GetType(prs_file);

    //
    //
    //
    if (sym_file == NULL)
    {
        sym_file = help_get_file("sym."); 
    }
    sym_type = GetType(sym_file);
	
	if(NULL ==top_level_ast_file)
	{
        top_level_ast_file = help_get_file("_top_level_ast."); 
	}
    top_level_ast_file_prefix = GetType(top_level_ast_file);

    //
    // The dat_directory must be processed prior to the 
    // dat_file which uses it.
    //
    if (dat_directory == NULL)
         dat_directory = NewString(home_directory, strlen(home_directory));
    else CheckDirectory(dat_directory_location, dat_directory);

    if (dat_file == NULL)
        dat_file = GetFile(dat_directory, "dcl.", "data");

    //
    //
    //
    auto help_get_def_or_del_file = [&](const char* file_suffix)
    {

        const char* file_type = "";
        switch (programming_language)
        {
        case  CPP:
        case  CPP2:
            file_type = "cpp"; break;
        case  JAVA:
            file_type = "java"; break;
        case  CSHARP:
            file_type = "cs"; break;
        case  TSC:
            file_type = "ts"; break;
        case  DART:
            file_type = "dart"; break;
        case  GO:
            file_type = "go"; break;
        case  RUST:
            file_type = "rs"; break;
        case  PYTHON3:
            file_type = "py"; break;
        default:
            file_type = "java";
        }
        return GetFile(out_directory, file_suffix, file_type);
    };

    if (dcl_file == NULL)
    {
        dcl_file = help_get_def_or_del_file((programming_language == CPP || programming_language == CPP2) ? "prs." : "dcl.");
    }
    dcl_type = GetType(dcl_file);

    //
    //
    //
    if (def_file == NULL)
    {
        def_file = help_get_def_or_del_file("def.");
    }
    def_type = GetType(def_file);

    //
    //
    //
    if (directory_prefix == NULL)
    {
        char *p = NewString(1);
        *p = '\0';
        directory_prefix = p;
    }
    else
    {
        NormalizeSlashes((char *) directory_prefix); // turn all backward slashes into forward slashes.
        //
        // Remove all extraneous trailing slashes, if any.
        //
        size_t dir_len = strlen(directory_prefix);
        if (dir_len > 0)
        {
            for (char *tail = (char *) &(directory_prefix[dir_len - 1]); tail > directory_prefix; tail--)
            {
                if (*tail == '/')
                   *tail = '\0';
              else break;
            }
        }
    }

    //
    // Check export_terminals option.
    //
    if (exp_file == NULL)
    {
        exp_file = help_get_file("exp."); 
    }
    else exp_file = ExpandFilename(exp_file);

    exp_type = GetType(exp_file);

    if (exp_prefix == NULL)
        exp_prefix = NewString("");

    if (exp_suffix == NULL)
        exp_suffix = NewString("");

    if (factory == NULL)
    {
        if(programming_language == GO)
        {
            factory = NewString("New");
        }
        else if(PYTHON3 == programming_language || DART == programming_language)
        {
            factory = NewString("");
        }
        else if(RUST == programming_language)
        {
            // Rust constructs AST nodes via associated functions (ClassName::new(...)),
            // so no prefix keyword is emitted before the class name.
            factory = NewString("");
        }else
        {
            factory = NewString("new ");
        }
       
    }

    //
    //
    //
    if (imp_file == NULL)
    {
        imp_file = help_get_file("imp.");
    }
    imp_type = GetType(imp_file);

    //
    //
    //
    if (soft_keywords)
    {
        lalr_level = 1;
        single_productions = false;
        backtrack = true;
    }

    //
    //
    //
    if (glr)
    {
        // -glr generates the same multi-action conflict tables as -backtrack
        // (plus GLR AST nextAst scaffolding). soft_keywords also forces
        // backtrack; combining the two is fine.
        // Copy before any further option string churn; health JSON reads the flag later.
        std::string tmpl = (template_name != NULL) ? template_name : "";
        const bool uses_glr_template =
            tmpl.find("glrParserTemplate") != std::string::npos;
        glr_template_hint_needed = !uses_glr_template;
        if (uses_glr_template)
        {
            EmitWarning(0,
                        "-glr: GLR conflict tables generated; link a runtime with "
                        "GLRParser (Java/C++/TypeScript/C#/Python3/Go/Rust/Dart)");
        }
        else
        {
            EmitWarning(0,
                        "-glr: GLR conflict tables generated, but the active "
                        "template is not glrParserTemplateF.gi. Pass "
                        "-template=<templates>/<lang>/glrParserTemplateF.gi "
                        "(any shipped backend) to wire the GLR driver.");
        }
        lalr_level = 1;
        single_productions = false;
        backtrack = true;
    }

    //
    //
    //
    if (verbose)
    {
        first = true;
        follow = true;
        list = true;
        states = true;
        xref = true;
        warnings = true;
    }

    return;
}

const char* Option::get_programing_language_str()
{
    switch (programming_language)
    {
    case  CPP:
        return "cpp";
    case  CPP2:
        return "rt_cpp";
    case  JAVA:
        return "java";
    case  CSHARP:
        return "csharp";
    case  TSC:
        return "typescript";
    case  DART:
        return "dart";
    case  GO:
        return "go";
    case  RUST:
        return "rust";
    case  PYTHON3:
        return "python3";
    case  NONE:
        return "none";
    default:
        return "java";
    }
};
//
// Here we print all options set by the user.
//
void Option::PrintOptionsInEffect()
{
    if (this -> quiet)
        return;

    assert(grm_file);
    report.Put("\nOptions in effect for ");
    report.Put(grm_file);
    report.Put(":\n\n");
    for (int i = 0; i < action_blocks.NumActionBlocks(); i++)
    {
        BlockSymbol *block = action_blocks[i];

        report.Put("    ");
        report.Put(block -> BlockKind() == BlockSymbol::MAIN_BLOCK
                                    ? "ACTION-BLOCK"
                                    : block ->BlockKind() == BlockSymbol::HEADER_BLOCK
                                             ? "HEADERS"
                                             : "TRAILERS");
        report.Put("=(\"");
        report.Put(block -> ActionfileSymbol() -> Name());
        report.Put("\",\"");
        report.Put(block -> BlockBegin());
        report.Put("\",\"");
        report.Put(block -> BlockEnd());
        report.Put("\")");
        if (action_blocks.IsIgnoredBlock(block -> BlockBegin(), block -> BlockBeginLength()))
            report.Put(" : IGNORED");
        report.PutChar('\n');
    }
    report.PutChar('\n');

    Tuple<const char *> opt_string;

    opt_string.Next() = AllocateString("AST-DIRECTORY", ast_directory);
    opt_string.Next() = AllocateString("AST-TYPE", ast_type);
    opt_string.Next() = AllocateString(attributes ? "ATTRIBUTES" : "NOATTRIBUTES");
    opt_string.Next() = AllocateString(automatic_ast == NONE
                                           ? "NOAUTOMATIC-AST"
                                           : automatic_ast == NESTED
                                                 ? "AUTOMATIC-AST=NESTED"
                                                 : "AUTOMATIC-AST=TOPLEVEL");
    opt_string.Next() = AllocateString(backtrack ? "BACKTRACK" : "NOBACKTRACK");
    if (byte)
        opt_string.Next() = AllocateString("BYTE");
    opt_string.Next() = AllocateString(conflicts ? "CONFLICTS" : "NOCONFLICTS");
    opt_string.Next() = AllocateString(fail_on_conflicts ? "FAIL_ON_CONFLICTS" : "NOFAIL_ON_CONFLICTS");
    opt_string.Next() = AllocateString("DAT-DIRECTORY", dat_directory);
    opt_string.Next() = AllocateString("DAT-FILE", dat_file);
    opt_string.Next() = AllocateString("DCL-FILE", dcl_file);
    opt_string.Next() = AllocateString(debug ? "DEBUG" : "NODEBUG");
    opt_string.Next() = AllocateString("DEF-FILE", def_file);
    opt_string.Next() = AllocateString("DIRECTORY-PREFIX", directory_prefix);
    opt_string.Next() = AllocateString(edit ? "EDIT" : "NOEDIT");
    opt_string.Next() = AllocateString(error_maps ? "ERROR-MAPS" : "NOERROR-MAPS");
    opt_string.Next() = AllocateString("ESCAPE", escape);

    //
    // Reconstruct the export terminals information
    //
    {
        char *export_info = new char[strlen(exp_file) +
                                     strlen(exp_prefix) +
                                     strlen(exp_suffix) + 11]; // 11 = 6 quotes, 2 commas, 2 parentheses, \0
        strcpy(export_info, "(\"");
        strcat(export_info, exp_file);
        strcat(export_info, "\",\"");
        strcat(export_info, exp_prefix);
        strcat(export_info, "\",\"");
        strcat(export_info, exp_suffix);
        strcat(export_info, "\")");
        opt_string.Next() = AllocateString("EXPORT-TERMINALS", export_info);
        delete [] export_info;
    }
    if (extends_parsetable == NULL)
         opt_string.Next() = AllocateString("EXTENDS-PARSETABLE");
    else if (strlen(extends_parsetable) == 0)
         opt_string.Next() = AllocateString("EXTENDS-PARSETABLE");
    else opt_string.Next() = AllocateString("EXTENDS-PARSETABLE", extends_parsetable);
    opt_string.Next() = AllocateString("FACTORY", factory);
    opt_string.Next() = AllocateString("FILE-PREFIX", file_prefix);
    if (filter)
        opt_string.Next() = AllocateString("FILTER", filter);
    opt_string.Next() = AllocateString(first ? "FIRST" : "NOFIRST");
    opt_string.Next() = AllocateString(follow ? "FOLLOW" : "NOFOLLOW");
    opt_string.Next() = AllocateString(glr ? "GLR" : "NOGLR");
    opt_string.Next() = AllocateString(goto_default ? "GOTO-DEFAULT" : "NOGOTO-DEFAULT");
    opt_string.Next() = AllocateString("GRM-FILE", grm_file);
    if (imp_file)
        opt_string.Next() = AllocateString("IMP-FILE", imp_file);
    if(import_terminals)
        opt_string.Next() = AllocateString("IMPORT-TERMINALS", import_terminals);
    opt_string.Next() = AllocateString("INCLUDE-DIRECTORY", include_directory);
    if (! slr)
        opt_string.Next() = AllocateString("LALR", lalr_level);
    opt_string.Next() = AllocateString(legacy ? "LEGACY" : "NOLEGACY");
    opt_string.Next() = AllocateString(list ? "LIST" : "NOLIST");
    opt_string.Next() = AllocateString("MARGIN", margin);
    opt_string.Next() = AllocateString("MAX-CASES", max_cases);
    opt_string.Next() = AllocateString(names == MAXIMUM
                                              ? "NAMES=MAXIMUM"
                                              : (names == MINIMUM
                                                        ? "NAMES=MINIMUM"
                                                        : "NAMES=OPTIMIZED"));
    opt_string.Next() = AllocateString(nt_check ? "NT-CHECK" : "NONT-CHECK");
    opt_string.Next() = AllocateString("OR_MARKER", or_marker);
    opt_string.Next() = AllocateString("OUT-DIRECTORY", out_directory);
    opt_string.Next() = AllocateString("PACKAGE", package);
    opt_string.Next() = AllocateString(parent_saved ? "PARENT-SAVED" : "NOPARENT-SAVE");
    if (parsetable_interfaces == NULL)
         opt_string.Next() = AllocateString("NOPARSETABLE-INTERFACES");
    else if (strlen(parsetable_interfaces) == 0)
         opt_string.Next() = AllocateString("PARSETABLE-INTERFACES");
    else opt_string.Next() = AllocateString("PARSETABLE-INTERFACES", parsetable_interfaces);
    opt_string.Next() = AllocateString("PREFIX", prefix);
    opt_string.Next() = AllocateString(priority ? "PRIORITY" : "NOPRIORITY");
    std::string temp = "PROGRAMMING_LANGUAGE=";
    temp +=get_programing_language_str();
	for(size_t i = 0; i < temp.length(); ++i)
	{
        temp[i] = static_cast<char>(toupper(temp[i]));
	}
	
    opt_string.Next() = AllocateString(temp.c_str());
    opt_string.Next() = AllocateString("PRS-FILE", prs_file);
    opt_string.Next() = AllocateString(quiet ? "QUIET" : "NOQUIET");
    opt_string.Next() = AllocateString(read_reduce ? "READ-REDUCE" : "NOREAD-REDUCE");
    opt_string.Next() = AllocateString(remap_terminals ? "REMAP-TERMINALS" : "NOREMAP-TERMINALS");
    opt_string.Next() = AllocateString(rule_classnames == SEQUENTIAL ? "RULE_CLASSNAMES=SEQUENTIAL" : "RULE_CLASSNAMES=STABLE");
    opt_string.Next() = AllocateString(scopes ? "SCOPES" : "NOSCOPES");
    opt_string.Next() = AllocateString(serialize ? "SERIALIZE" : "NOSERIALIZE");
    opt_string.Next() = AllocateString(shift_default ? "SHIFT-DEFAULT" : "NOSHIFT-DEFAULT");
    if (! byte)
        opt_string.Next() = AllocateString("SHORT");
    opt_string.Next() = AllocateString(single_productions ? "SINGLE-PRODUCTIONS" : "NOSINGLE-PRODUCTIONS");
    if (slr)
        opt_string.Next() = AllocateString("SLR");
    opt_string.Next() = AllocateString(soft_keywords ? "SOFT-KEYWORDS" : "NOSOFT-KEYWORDS");
    opt_string.Next() = AllocateString(states ? "STATES" : "NOSTATES");
    opt_string.Next() = AllocateString("SUFFIX", suffix);
    opt_string.Next() = AllocateString("SYM-FILE", sym_file);
    opt_string.Next() = AllocateString("TAB-FILE", tab_file);
    opt_string.Next() = AllocateString(table ? "TABLE" : "NOTABLE");
    opt_string.Next() = AllocateString("TEMPLATE", template_name);
    opt_string.Next() = AllocateString(trace == NONE
                                              ? "NOTRACE"
                                              : (trace == CONFLICTS
                                                        ? "TRACE=CONFLICTS"
                                                        : "TRACE=FULL"));
    opt_string.Next() = AllocateString(variables == NONE
                                           ? "NOVARIABLES"
                                           : (variables == BOTH
                                                  ? "VARIABLES"
                                                  : (variables == TERMINALS
                                                         ? "VARIABLES=TERMINALS"
                                                          : "VARIABLES=NONTERMINALS")));
    opt_string.Next() = AllocateString(verbose ? "VERBOSE" : "NOVERBOSE");
    opt_string.Next() = AllocateString(visitor == NONE
                                           ? "NOVISITOR"
                                           : visitor == DEFAULT
                                                 ? "VISITOR=DEFAULT"
                                                 : "VISITOR=PREORDER");
    opt_string.Next() = AllocateString("VISITOR-TYPE", visitor_type);
    opt_string.Next() = AllocateString(warnings ? "WARNINGS" : "NOWARNINGS");
    opt_string.Next() = AllocateString(xref ? "XREF" : "NOXREF");

    char output_line[81];
    strcpy(output_line, "    ");
    for (int k = 0; k < opt_string.Length(); k++)
    {
        if (strlen(opt_string[k]) > 80 - 4)
        {
            report.Put(output_line);
            report.PutChar('\n');
            strcpy(output_line, "    ");
            report.Put(output_line);
            report.Put(opt_string[k]);
            report.PutChar('\n');
        }
        else
        {
            if (strlen(output_line) + strlen(opt_string[k]) > 80)
            {
                report.Put(output_line);
                report.PutChar('\n');
                strcpy(output_line, "    ");
            }
            strcat(output_line, opt_string[k]);
            if (strlen(output_line) + 2 < 80)
                strcat(output_line, "  ");
        }

        delete [] ((char *) opt_string[k]);
    }

    report.Put(output_line);
    report.Put("\n\n");

    return;
}


//
//
//
void Option::PrintOptionsList(void)
{
    cout << "\n"
         << Control::HEADER_INFO
         << "\n(C) Copyright LPG Group. 1984, 2021..\n"
            "Usage: lpg [options] [filename[.extension]]\n\n"
         << OptionDescriptor::describeAllOptions()
         << "\n"
            "Options must be separated by a space.  "
            "Any non-ambiguous initial prefix of a\n"
            "valid option may be used as an abbreviation "
            "for that option.  When an option is\n"
            "composed of two separate words, an "
            "abbreviation may be formed by concatenating\n"
            "the first character of each word.  "
            "Options that are switches may be negated by\n"
            "prefixing them with the string \"no\".  "
            "Default input file extension is \".g\"\n"
         << "\nVersion " << Control::VERSION << " \n";
}

bool Option::IsNested() const {
    return automatic_ast == Option::NESTED;
}
