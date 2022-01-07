/*
 *  options.cpp
 *  lpg
 *
 *  Created by Robert M. Fuhrer on 3/13/11.
 *  Copyright 2011 IBM. All rights reserved.
 */

#include "options.h"
#include "option.h"

#include <limits.h>

OptionProcessor::OptionProcessor(Option *option)
: options(option)
{ }

std::string
trimQuotes(std::string& s)
{
    std::string result;
    int begin= 0, len = s.length();

    while (s[begin] == ' ') {
        begin++; len--;
    }
    if (s[begin] == '"') {
        begin++; len--;
    }
    if (s[begin+len-1] == '"') {
        len--;
    }
    result += s.substr(begin, len);
    return result;
}

std::list<OptionDescriptor*> OptionDescriptor::allOptionDescriptors;

OptionDescriptor *actionBlock = new OptionDescriptor(STRING_LIST, "action", "block", "???", &OptionProcessor::processActionBlock);

void
OptionProcessor::processActionBlock(OptionValue *v)
{
    StringListOptionValue *slv = static_cast<StringListOptionValue*> (v);
    std::list<std::string> values = slv->getValue();
    std::list<std::string>::iterator i = values.begin();
    std::string fileName = trimQuotes(*i++);
    std::string blockBegin = trimQuotes(*i++);
    std::string blockEnd = trimQuotes(*i++);

    Option::BlockInfo& actionBlock = options->action_options.Next();
    Token *optionLoc = options->GetTokenLocation(options->parm_ptr, 0);
    actionBlock.Set(optionLoc, _strdup(fileName.c_str()), _strdup(blockBegin.c_str()), _strdup(blockEnd.c_str()));
}

OptionDescriptor *astDirectory = new PathOptionDescriptor("ast", "directory",
                                                          "the directory in which generated AST classes will be placed, if automatic-ast is 'toplevel'",
                                                          NULL,
                                                          &Option::ast_directory, true);

OptionDescriptor *astType = new StringOptionDescriptor("ast", "type", "the name of the AST root class", NULL, &Option::ast_type, false);

OptionDescriptor *attributes = new BooleanOptionDescriptor("attributes", "???", false, &Option::attributes);

OptionDescriptor *automaticAST = new EnumOptionDescriptor("automatic", "ast", 
                                                          "determines where generated AST classes will be placed",
                                                          &Option::automatic_ast, "none", "nested", "none",
                                                          new EnumValue("none", Option::NONE),
                                                          new EnumValue("nested", Option::NESTED),
                                                          new EnumValue("toplevel", Option::TOPLEVEL), NULL);

OptionDescriptor *backtrack = new BooleanOptionDescriptor("backtrack", "???", false,
                                                          &Option::backtrack);

OptionDescriptor *byte = new BooleanOptionDescriptor("byte", "???", true,
                                                     &Option::byte);

OptionDescriptor *conflicts = new BooleanOptionDescriptor("conflicts", "???", true,
                                                          &Option::conflicts);

OptionDescriptor *dataDirectory = new PathOptionDescriptor("dat", "directory", "???", NULL,
                                                       &Option::dat_directory);

OptionDescriptor *dataFile = new PathOptionDescriptor("dat", "file", "???", NULL,
                                                      &Option::dat_file);

OptionDescriptor *dclFile = new PathOptionDescriptor("dcl", "file", "???", NULL,
                                                     &Option::dcl_file);

OptionDescriptor *defFile = new PathOptionDescriptor("def", "file", "???", NULL,
                                                     &Option::def_file);

OptionDescriptor *dirPrefix = new PathOptionDescriptor("directory", "prefix", "???", NULL,
                                                       &Option::directory_prefix, true);

OptionDescriptor *debug = new BooleanOptionDescriptor("debug", "???", false, &Option::debug);

OptionDescriptor *edit = new BooleanOptionDescriptor("edit", "???", false, &Option::edit);

OptionDescriptor *errorMaps = new BooleanOptionDescriptor("error", "maps", "???", false, &Option::error_maps);

OptionDescriptor *escapeChar = new CharOptionDescriptor("escape", "???", " ",
                                                        &Option::escape);

OptionDescriptor *exportsTerminals = new OptionDescriptor(STRING_LIST, "export", "terminals", "???",
                                                          &OptionProcessor::processExportTerminals);

void
OptionProcessor::processExportTerminals(OptionValue *v)
{
    StringListOptionValue *slv = static_cast<StringListOptionValue*> (v);
    const std::list<std::string>& values = slv->getValue();
    if (values.size() < 1 || values.size() > 3) {
        throw ValueFormatException("Export-terminals value must be a string list of 1 to 3 elements", *v->toString(), v->getOptionDescriptor());
    }
    std::list<std::string>::const_iterator iter = values.begin();
    options->exp_file = _strdup((*iter++).c_str());
    if (iter != values.end()) {
        options->exp_prefix = _strdup((*iter++).c_str());
    }
    if (iter != values.end()) {
        options->exp_suffix = _strdup((*iter++).c_str());
    }
}

OptionDescriptor *extendsParseTable = new StringOptionDescriptor("extends", "parsetable", "???", NULL,
                                                                 &Option::extends_parsetable, false);

OptionDescriptor *factory = new StringOptionDescriptor("factory", "???", NULL,
                                                       &Option::factory, false);

OptionDescriptor *filePrefix = new StringOptionDescriptor("file", "prefix", "???", NULL,
                                                          &Option::file_prefix, true);

//
// Can't just do the following b/c the 'filter' option affects two Option fields (filter and filter_file)
//OptionDescriptor *filter = new StringOptionDescriptor("filter", NULL, "???",
//                                                      &Option::filter, false);
//
OptionDescriptor *filter = new OptionDescriptor(STRING, "filter", "???", &OptionProcessor::processFilter, false);

void
OptionProcessor::processFilter(OptionValue *v)
{
    StringOptionValue *sv = static_cast<StringOptionValue*> (v);
    options->filter_file.Reset();
    const char *valStr = _strdup(sv->getValue().c_str());
    options->filter_file.Next() = valStr;
    options->filter = valStr;
}

OptionDescriptor *first = new BooleanOptionDescriptor("first", "???", false, &Option::first);

OptionDescriptor *follow = new BooleanOptionDescriptor("follow", "???", false, &Option::follow);

OptionDescriptor *glr = new BooleanOptionDescriptor("glr", "???", false, &Option::glr);

OptionDescriptor *gotoDefault = new BooleanOptionDescriptor("goto", "default", "???", false,
                                                            &Option::goto_default);

OptionDescriptor *ignoreBlock = new OptionDescriptor(STRING, "ignore", "block", "???",
                                                     &OptionProcessor::processIgnoreBlock);
void
OptionProcessor::processIgnoreBlock(OptionValue *v)
{
    StringOptionValue *sv = static_cast<StringOptionValue*> (v);
    if (sv->getValue().length() < 1) {
        throw ValueFormatException("Ignore-block value must be a non-empty string", *v->toString(), v->getOptionDescriptor());
    }
    const char *ignore_block = _strdup(sv->getValue().c_str());
    options->action_blocks.FindOrInsertIgnoredBlock(ignore_block, strlen(ignore_block));
}

OptionDescriptor *impFile = new StringOptionDescriptor("imp", "file", "???",
                                                       &Option::imp_file);

// Can't just do the following b/c the 'import-terminals' option affects two Option fields (import_file and import_terminals)
//OptionDescriptor *importTerminals = new StringOptionDescriptor("import", "terminals", "???",
//                                                               &Option::import_terminals, false);
OptionDescriptor *importTerminals = new OptionDescriptor(STRING, "import", "terminals", "???",
                                                         &OptionProcessor::processImportTerminals);
void
OptionProcessor::processImportTerminals(OptionValue *v)
{
    StringOptionValue *sv = static_cast<StringOptionValue*> (v);
    options->import_file.Reset();
    const char *valStr = _strdup(sv->getValue().c_str());
    options->import_file.Next() = valStr;
    options->import_terminals = valStr;
}

OptionDescriptor *includeDirs = new OptionDescriptor(PATH_LIST, "include", "directory",
                                                     "a semi-colon separated list of directories to search when processing include directives",
                                                     &OptionProcessor::processIncludeDir, false);

void
OptionProcessor::processIncludeDir(OptionValue *v)
{
    PathListOptionValue *plv = static_cast<PathListOptionValue*> (v);
    std::list<std::string> values = plv->getValue();
    std::string includeDirOption;

    options->include_search_directory.Reset();
    options->include_search_directory.Next() = _strdup(options->home_directory);
    for(std::list<std::string>::iterator i= values.begin(); i != values.end(); i++) {
        std::string path = *i;
//      cerr << "include-dir path: " << path << endl;
        options->include_search_directory.Next() = _strdup(path.c_str());
        if (includeDirOption.length() > 0) {
            includeDirOption += ";";
        }
        includeDirOption += path;
    }
    options->include_directory = _strdup(includeDirOption.c_str());
}

OptionDescriptor *lalr = new IntegerOptionDescriptor("lalr", "", 1, 1, INT_MAX,
                                                     "determines how many tokens of look-ahead can be used to disambiguate",
                                                     &Option::lalr_level);

OptionDescriptor *legacy = new BooleanOptionDescriptor("legacy", "???", true, &Option::legacy);

OptionDescriptor *list = new BooleanOptionDescriptor("list", "???", false, &Option::list);

OptionDescriptor *margin = new IntegerOptionDescriptor("margin", 0, 1, INT_MAX,
                                                       "???",
                                                       &Option::margin);

OptionDescriptor *maxCases = new IntegerOptionDescriptor("max", "cases", 1024, 1, INT_MAX,
                                                         "???",
                                                         &Option::max_cases);

OptionDescriptor *names = new EnumOptionDescriptor("names", "???", &Option::names, "optimized", "", "",
                                                   new EnumValue("optimized", Option::OPTIMIZED),
                                                   new EnumValue("minimum", Option::MINIMUM),
                                                   new EnumValue("maximum", Option::MAXIMUM), NULL);

OptionDescriptor *ntCheck = new BooleanOptionDescriptor("nt", "check", "???", false, &Option::nt_check);

OptionDescriptor *orMarker = new CharOptionDescriptor("or", "marker", "???", 
                                                      "|",
                                                      &Option::or_marker);

OptionDescriptor *outDirectory = new StringOptionDescriptor("out", "directory", "???", NULL,
                                                            &Option::out_directory, true);

OptionDescriptor *package = new StringOptionDescriptor("package", "???", NULL,
                                                       &Option::package, false);

OptionDescriptor* parentSaved = new BooleanOptionDescriptor("parent", "saved", "???", false, &Option::parent_saved);

OptionDescriptor *parseTableInterfaces = new StringOptionDescriptor("parsetable", "interfaces", "???", NULL,
                                                                    &Option::parsetable_interfaces, false);

OptionDescriptor *precedence = new BooleanOptionDescriptor("precedence",
                                                           "if true, allow conflicting actions to be ordered",
                                                           false, &Option::precedence, false);

OptionDescriptor *prefix = new StringOptionDescriptor("prefix", "???", NULL,
                                                      &Option::prefix, true);

OptionDescriptor *priority = new BooleanOptionDescriptor("priority", "???", true,
                                                         &Option::priority);

OptionDescriptor *programmingLang = new EnumOptionDescriptor("programming", "language",
                                                             "identifies the desired parser implementation language",
                                                             &Option::programming_language, "xml", "", "",
														     new EnumValue("c", Option::C),
															 new EnumValue("rt_cpp", Option::CPP2),
                                                             new EnumValue("cpp", Option::CPP),
                                                             new EnumValue("c++", Option::CPP),
															 new EnumValue("csharp", Option::CSHARP),
															 new EnumValue("c#", Option::CSHARP),
                                                             new EnumValue("java", Option::JAVA),
                                                             new EnumValue("ml", Option::ML),
                                                             new EnumValue("plx", Option::PLX),
                                                             new EnumValue("plxasm", Option::PLXASM),
                                                             new EnumValue("xml", Option::XML),
    new EnumValue("python3", Option::PYTHON3),
    new EnumValue("python2", Option::PYTHON2),
    new EnumValue("dart", Option::DART),
    new EnumValue("go", Option::GO),
    new EnumValue("rust", Option::RUST),
    new EnumValue("typescript", Option::TSC), NULL);

OptionDescriptor *prsFile = new StringOptionDescriptor("prs", "file", "???", NULL,
                                                       &Option::prs_file, false);

OptionDescriptor *quiet = new BooleanOptionDescriptor("quiet",
                                                      "???", false,
                                                      &Option::quiet);

OptionDescriptor *readReduce = new BooleanOptionDescriptor("read", "reduce",
                                                           "???", true,
                                                           &Option::read_reduce);

OptionDescriptor *remapTerminals = new BooleanOptionDescriptor("remap", "terminals",
                                                               "???", true,
                                                               &Option::remap_terminals);

OptionDescriptor *ruleClassNames = new EnumOptionDescriptor("rule", "classnames", "???",
                                                            &Option::rule_classnames,
                                                            "sequential", "", "",
                                                            new EnumValue("sequential", Option::SEQUENTIAL),
                                                            new EnumValue("stable", Option::STABLE), NULL);

OptionDescriptor *scopes = new BooleanOptionDescriptor("scopes",
                                                       "???", false,
                                                       &Option::scopes);

OptionDescriptor *serialize = new BooleanOptionDescriptor("serialize",
                                                          "???", false,
                                                          &Option::serialize);

OptionDescriptor *shiftDefault = new BooleanOptionDescriptor("shift", "default",
                                                             "???", false,
                                                             &Option::shift_default);

OptionDescriptor *singleProductions = new BooleanOptionDescriptor("single", "productions",
                                                           "???", false,
                                                           &Option::single_productions);

OptionDescriptor *slr = new BooleanOptionDescriptor("slr",
                                                    "???", false,
                                                    &Option::slr);

OptionDescriptor *softKeywords = new BooleanOptionDescriptor("soft", "keywords",
                                                             "if true, try treating keywords as identifiers if parsing fails otherwise",
                                                             false,
                                                             &Option::soft_keywords);

OptionDescriptor *states = new BooleanOptionDescriptor("states",
                                                       "???", false,
                                                       &Option::states);

OptionDescriptor *suffix = new StringOptionDescriptor("suffix", "???", NULL,
                                                      &Option::suffix, true);

OptionDescriptor *symFile = new StringOptionDescriptor("sym", "file", "???", NULL,
                                                       &Option::sym_file, false);

OptionDescriptor *tabFile = new StringOptionDescriptor("tab", "file", "???", NULL,
                                                       &Option::tab_file, false);

OptionDescriptor *table = new EnumOptionDescriptor("table", "???",
                                                   &OptionProcessor::processTable,
                                                   "",
                                                   new EnumValue("c", Option::C),
												   new EnumValue("rt_cpp", Option::CPP2),
                                                   new EnumValue("cpp", Option::CPP),
                                                   new EnumValue("c++", Option::CPP),
                                                   new EnumValue("java", Option::JAVA),
												   new EnumValue("csharp", Option::CSHARP),
												   new EnumValue("c#", Option::CSHARP),
                                                   new EnumValue("ml", Option::ML),
                                                   new EnumValue("none", Option::XML),
                                                   new EnumValue("plx", Option::PLX),
                                                   new EnumValue("plxasm", Option::PLXASM),
                                                   new EnumValue("xml", Option::XML),
    new EnumValue("python3", Option::PYTHON3),
    new EnumValue("python2", Option::PYTHON2),
    new EnumValue("dart", Option::DART),
    new EnumValue("go", Option::GO),
    new EnumValue("rust", Option::RUST),
    new EnumValue("typescript", Option::TSC),
    NULL);
void
OptionProcessor::processTable(OptionValue *v)
{
    EnumOptionValue *ev = static_cast<EnumOptionValue*> (v);
    std::string  value = ev->getValue();

    if (!value.compare("none")) {
        options->table = false;
        options->programming_language = Option::XML;
    } else {
        if (!value.compare("c")) {
            options->programming_language = Option::C;
        } else if (!value.compare("cpp") || !value.compare("c++")) {
            options->programming_language = Option::CPP;
        }
        else if (!value.compare("rt_cpp")) {
            options->programming_language = Option::CPP2;
        }
        else if (!value.compare("c#")  || !value.compare("csharp")) {
            options->programming_language = Option::CSHARP;
        }
        else if (!value.compare("typescript")) {
            options->programming_language = Option::TSC;
        }
        else if (!value.compare("python3")) {
            options->programming_language = Option::PYTHON3;
        }
        else if (!value.compare("python2")) {
            options->programming_language = Option::PYTHON2;
        }
        else if (!value.compare("dart")) {
            options->programming_language = Option::DART;
        }
        else if (!value.compare("go")) {
            options->programming_language = Option::GO;
        }
        else if (!value.compare("rust")) {
            options->programming_language = Option::RUST;
        }
    	else if (!value.compare("java")) {
            options->programming_language = Option::JAVA;
        } else if (!value.compare("ml")) {
            options->programming_language = Option::ML;
        } else if (!value.compare("plx")) {
            options->programming_language = Option::PLX;
        } else if (!value.compare("plxasm")) {
            options->programming_language = Option::PLXASM;
        } else if (!value.compare("xml")) {
            options->programming_language = Option::XML;
        }
        options->table = true;
    }
}

OptionDescriptor *template_ = new StringOptionDescriptor("template", "???", NULL,
                                                         &Option::template_name, false);

OptionDescriptor *trace = new EnumOptionDescriptor("trace", "???",
                                                   &Option::trace,
                                                   "conflicts", "conflicts", "none",
                                                   new EnumValue("none", Option::NONE),
                                                   new EnumValue("conflicts", Option::CONFLICTS),
                                                   new EnumValue("full", Option::FULL), NULL);

OptionDescriptor *trailers = new OptionDescriptor(STRING_LIST, "trailers", "???",
                                                  &OptionProcessor::processTrailers);
void
OptionProcessor::processTrailers(OptionValue *v)
{
    StringListOptionValue *slv = static_cast<StringListOptionValue*> (v);
    const std::list<std::string>& values = slv->getValue();
    if (values.size() != 3) {
        throw ValueFormatException("Trailers value must be a string list of 3 elements", *v->toString(), v->getOptionDescriptor());
    }
    std::list<std::string>::const_iterator iter = values.begin();
    const char *filename = _strdup((*iter++).c_str());
    const char *block_begin = _strdup((*iter++).c_str());
    const char *block_end = _strdup((*iter++).c_str());
    options->trailer_options.Next().Set(NULL, filename, block_begin, block_end);
}

OptionDescriptor *variables = new EnumOptionDescriptor("variables",
                                                       "determines the set of right-hand side symbols for which local variables will be defined within action blocks",
                                                       &Option::variables,
                                                       "none", "both", "none",
                                                       new EnumValue("none", Option::NONE),
                                                       new EnumValue("both", Option::BOTH),
                                                       new EnumValue("terminals", Option::TERMINALS),
                                                       new EnumValue("nt", Option::NON_TERMINALS),
                                                       new EnumValue("nonterminals", Option::NON_TERMINALS),
                                                       new EnumValue("non-terminals", Option::NON_TERMINALS), NULL);

OptionDescriptor *verbose = new BooleanOptionDescriptor("verbose",
                                                        "???", false,
                                                        &Option::verbose);

OptionDescriptor *visitor = new EnumOptionDescriptor("visitor", "???",
                                                     &Option::visitor,
                                                     "none", "default", "none",
                                                     new EnumValue("none", Option::NONE),
                                                     new EnumValue("default", Option::DEFAULT),
                                                     new EnumValue("preorder", Option::PREORDER),
                                                     new EnumValue("all", Option::PREORDER | Option::PREORDER ),
                                                     NULL);

OptionDescriptor *visitorType = new StringOptionDescriptor("visitor", "type", "???", NULL,
                                                           &Option::visitor_type, false);

OptionDescriptor *warnings = new BooleanOptionDescriptor("warnings",
                                                         "???", true,
                                                         &Option::warnings);

OptionDescriptor *xref = new BooleanOptionDescriptor("xref",
                                                     "???", false,
                                                     &Option::xref);
