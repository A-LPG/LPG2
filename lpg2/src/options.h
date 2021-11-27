#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED

/*
 *  options.h
 *  lpg
 *
 *  Created by Robert M. Fuhrer on 3/13/11.
 *  Copyright 2011 IBM. All rights reserved.
 */

#include "code.h"
#include "util.h"

#include <string>
#include <list>

#ifndef WIN32
#define _strdup strdup
#endif

enum OptionType {
    BOOLEAN_TYPE,
    CHAR_TYPE,
    ENUM,
    INTEGER,
    STRING,
    STRING_LIST,
    PATH,
    PATH_LIST
};

class Option;
class OptionValue;

class OptionProcessor {
public:
    typedef bool Option::*BooleanValueField;
    typedef int Option::*IntegerValueField;
    typedef const char *Option::*StringValueField;
    typedef char Option::*CharValueField;

    typedef void (OptionProcessor::*ValueHandler)(OptionValue *);

    OptionProcessor(Option *);

    void processActionBlock(OptionValue *v);
    void processExportTerminals(OptionValue *v);
    void processFilter(OptionValue *v);
    void processIgnoreBlock(OptionValue *v);
    void processImportTerminals(OptionValue *v);
    void processIncludeDir(OptionValue *v);
    void processTable(OptionValue *v);
    void processTrailers(OptionValue *v);

    Option *getOptions() const { return options; }

private:
    Option *options;
};

class OptionDescriptor {
public:
    OptionDescriptor(OptionType type, const char *word1, const char *descrip, OptionProcessor::ValueHandler handler, bool valueOptional = false);
    OptionDescriptor(OptionType type, const char *word1, const char *word2, const char *descrip, OptionProcessor::ValueHandler handler, bool valueOptional = false);

    const char *getWord1() const { return word1; }
    const char *getWord2() const { return (word2 != NULL) ? word2 : ""; }
    const std::string& getName() const { return name; }
    const char *getDescription() const { return description; }
    OptionType getType() const { return type; }
    virtual std::string getTypeDescriptor() const;
    bool isValueOptional() const { return valueOptional; }

    OptionValue *createValue(bool noFlag);

    virtual void processSetting(OptionProcessor *processor, OptionValue *value);

    static const std::list<OptionDescriptor*>& getAllDescriptors();
    static void initializeAll(OptionProcessor *processor);

    static std::string describeAllOptions();

protected:
    OptionDescriptor(OptionType type, const char *word1, const char *word2, const char *descrip, bool valueOptional);

    void setupName();
    
    virtual void initializeValue(OptionProcessor *processor);

    const OptionType type;
    const char *word1;
    const char *word2; // may be null
    std::string name;
    const char *description;
    const bool valueOptional;
    OptionProcessor::ValueHandler valueHandler;

    static std::list<OptionDescriptor*> allOptionDescriptors;
};

class BooleanOptionDescriptor : public OptionDescriptor {
public:
    BooleanOptionDescriptor(const char *word1, const char *descrip, bool initValue, OptionProcessor::BooleanValueField, bool valueOptional=true);
    BooleanOptionDescriptor(const char *word1, const char *word2, const char *descrip, bool initValue, OptionProcessor::BooleanValueField, bool valueOptional=true);

    void processSetting(OptionProcessor *, OptionValue *);

private:
    void initializeValue(OptionProcessor *processor);

    bool initValue;
    OptionProcessor::BooleanValueField boolField;
};

class IntegerOptionDescriptor : public OptionDescriptor {
public:
    IntegerOptionDescriptor(const char *word1, int min, int max, int initValue, const char *descrip,
                            OptionProcessor::IntegerValueField, bool valueOpt=false);
    IntegerOptionDescriptor(const char *word1, const char *word2, int min, int max, int initValue, const char *descrip,
                            OptionProcessor::IntegerValueField, bool valueOpt=false);
    
    int getMinValue() const { return minValue; }
    int getMaxValue() const { return maxValue; }
    
    std::string getTypeDescriptor() const;
    
    void processSetting(OptionProcessor *, OptionValue *);
    
private:
    void initializeValue(OptionProcessor *processor);

    int initValue;
    int minValue, maxValue;
    OptionProcessor::IntegerValueField intField;
};

class StringOptionDescriptor : public OptionDescriptor {
public:
    StringOptionDescriptor(const char *word1, const char *descrip, const char *initValue,
                           OptionProcessor::StringValueField, bool emptyOk=false);
    StringOptionDescriptor(const char *word1, const char *word2, const char *descrip, const char *initValue,
                           OptionProcessor::StringValueField, bool emptyOk=false);

    void processSetting(OptionProcessor *, OptionValue *);

protected:
    StringOptionDescriptor(OptionType type, const char *word1, const char *word2, const char *descrip,
                           const char *defValue,
                           OptionProcessor::StringValueField, bool emptyOk=false);
    void initializeValue(OptionProcessor *processor);

    const char *initValue;
    bool emptyOk;
    OptionProcessor::StringValueField stringField;
};

class CharOptionDescriptor : public StringOptionDescriptor {
public:
    CharOptionDescriptor(const char *word1, const char *descrip, const char *initValue,
                         OptionProcessor::CharValueField);
    CharOptionDescriptor(const char *word1, const char *word2, const char *descrip, const char *initValue,
                         OptionProcessor::CharValueField);

    void processSetting(OptionProcessor *, OptionValue *);

private:
    void initializeValue(OptionProcessor *processor);

    OptionProcessor::CharValueField charField;
};

template<class F, class S>
class Pair {
public:
    Pair(F fst, S snd) : _first(fst), _second(snd) { }

    F first() const { return _first; }
    S second() const { return _second; }

private:
    F _first;
    S _second;
};

typedef Pair<const char *, int> EnumValue;

class EnumOptionDescriptor : public OptionDescriptor {
public:
    typedef std::list<EnumValue*> EnumValueList;

    EnumOptionDescriptor(const char *word1, const char *descrip, OptionProcessor::ValueHandler handler,
                         const char *initValue, EnumValue *value, ...);
    EnumOptionDescriptor(const char *word1, const char *word2, const char *descrip, OptionProcessor::ValueHandler handler,
                         const char *initValue, EnumValue *value, ...);

    EnumOptionDescriptor(const char *word1, const char *descrip, OptionProcessor::IntegerValueField field,
                         const char *initValue, const char *defValue, const char *noValue, EnumValue *value, ...);
    EnumOptionDescriptor(const char *word1, const char *word2, const char *descrip, OptionProcessor::IntegerValueField field,
                         const char *initValue, const char *defValue, const char *noValue, EnumValue *value, ...);

    const EnumValueList& getLegalValues() const { return legalValues; }
    const std::string& getDefaultValue() const { return defaultValue; }

    std::string getTypeDescriptor() const;

    void processSetting(OptionProcessor *, OptionValue *);

private:
    void initializeValue(OptionProcessor *processor);
    
    EnumValue *findEnumByName(const std::string& name);

    EnumValueList legalValues;
    OptionProcessor::IntegerValueField intField;
    const std::string initValue;
    const std::string defaultValue;
    const std::string noValue;
};

class PathOptionDescriptor : public StringOptionDescriptor {
public:
    PathOptionDescriptor(const char *word1, const char *word2, const char *descrip,
                         const char *initValue,
                         OptionProcessor::StringValueField, bool emptyOk=false);

    void processSetting(OptionProcessor *, OptionValue *);
};

class ValueFormatException {
public:
    ValueFormatException(const char *msg, OptionDescriptor *od)
    : msg(msg),  optDesc(od)
    { }
    
    ValueFormatException(const char *msg, const std::string& s, OptionDescriptor *od)
    : msg(msg), valueStr(s), optDesc(od)
    { }
    
    ValueFormatException(const std::string& msg, const std::string& s, OptionDescriptor *od)
    : msg(msg), valueStr(s), optDesc(od)
    { }
    
    const std::string& message() { return msg; }
    const std::string& value() { return valueStr; }
    const OptionDescriptor *optionDescriptor() { return optDesc; }
    
private:
    std::string msg;
    std::string valueStr;
    OptionDescriptor *optDesc;
};

class OptionValue : public Code, public Util {
public:
    OptionDescriptor *getOptionDescriptor() const { return optionDesc; }

    virtual void parseValue(std::string *v)  = 0;
    virtual const std::string *toString() = 0;

    void processSetting(OptionProcessor *processor);

protected:
    OptionValue(OptionDescriptor *od, bool noFlagg) : optionDesc(od), noFlag(noFlagg) { }

    OptionDescriptor *optionDesc;
    bool noFlag;
};

class BooleanOptionValue : public OptionValue {
public:
    BooleanOptionValue(OptionDescriptor *od, bool noFlag) : OptionValue(od, noFlag) { }
    
    void setValue(bool v) { value = v; }
    bool getValue() { return value; }
    
    void parseValue(std::string *v) ;
    const std::string *toString();
    
private:
    bool value;
};

class IntegerOptionValue : public OptionValue {
public:
    IntegerOptionValue(OptionDescriptor *od, bool noFlag) : OptionValue(od, noFlag) { }
    
    void setValue(int v) { value = v; }
    int getValue() { return value; }
    
    void parseValue(std::string *v) ;
    const std::string *toString();

private:
    int value;
};
class CharOptionValue : public OptionValue {
public:
    CharOptionValue(OptionDescriptor* od, bool noFlag) : OptionValue(od, noFlag) { }

    void setValue(std::string v) { value = v; }
    std::string getValue() { return value; }

    void parseValue(std::string* v);
    const std::string* toString();

private:
    std::string value;
};
class StringOptionValue : public OptionValue {
public:
    StringOptionValue(OptionDescriptor *od, bool noFlag) : OptionValue(od, noFlag) { }

    void setValue(const char *v) { value = v; }
    const std::string& getValue() { return value; }

    void parseValue(std::string *v);
    const std::string *toString();

protected:
    std::string value;
};

class EnumOptionValue : public StringOptionValue {
public:
    EnumOptionValue(OptionDescriptor *od, bool noFlag) : StringOptionValue(od, noFlag) { }

    void parseValue(std::string *v) ;

protected:
    std::string describeLegalValues();
};

class PathOptionValue : public StringOptionValue {
public:
    PathOptionValue(OptionDescriptor *od, bool noFlag) : StringOptionValue(od, noFlag) { }
};

class StringListOptionValue : public OptionValue {
public:
    StringListOptionValue(OptionDescriptor *od, bool noFlag) : OptionValue(od, noFlag) { }

    const std::list<std::string>& getValue() { return values; }
    void addValue(const char *v);
    void addValues(const StringListOptionValue &other);

    void parseValue(std::string *v) ;
    const std::string *toString();

protected:
    std::list<std::string> values;
};

class PathListOptionValue : public StringListOptionValue {
public:
    PathListOptionValue(OptionDescriptor *od, bool noFlag) : StringListOptionValue(od, noFlag) { }

    void parseValue(std::string *v) ;
    const std::string *toString();
};

class OptionParser : public Code, public Util {
public:
    OptionParser(const std::list<OptionDescriptor*> descriptors);

    OptionValue *parse(const char *&start) ;

private:
    OptionDescriptor *findOption(const char *&start, bool& flag);
    bool IsDelimiter(char c);
    std::string *getOptionValue(const char *&p);
    
    std::list<OptionDescriptor*> allOptions;
};

#endif
