/*
 *  optionDesc.cpp
 *  lpg
 *
 *  Created by Robert M. Fuhrer on 3/21/11.
 *  Copyright 2011 IBM.
 */

#include "options.h"

#include <iostream>
#include <limits>
#include <cstdarg>

using namespace std;

OptionDescriptor::OptionDescriptor(OptionType t, const char *wd1, const char *wd2, const char *descrip, bool valueOpt)
: type(t), word1(wd1), word2(wd2), description(descrip), valueOptional(valueOpt)
{
    setupName();
    allOptionDescriptors.push_back(this);
}

OptionDescriptor::OptionDescriptor(OptionType t, const char *wd1, const char *descrip,
                                   OptionProcessor::ValueHandler handler, bool valueOpt)
: type(t), word1(wd1), word2(NULL), description(descrip), valueOptional(valueOpt), valueHandler(handler)
{
    setupName();
    allOptionDescriptors.push_back(this);
}

OptionDescriptor::OptionDescriptor(OptionType t, const char *wd1, const char *wd2, const char *descrip,
                                   OptionProcessor::ValueHandler handler, bool valueOpt)
: type(t), word1(wd1), word2(wd2), description(descrip), valueOptional(valueOpt), valueHandler(handler)
{
    setupName();
    allOptionDescriptors.push_back(this);
}

void
OptionDescriptor::setupName()
{
    name = word1;
    if (word2 != NULL && strlen(word2) > 0) {
        name.append("-");
        name.append(word2);
    }
}

const std::list<OptionDescriptor*>&
OptionDescriptor::getAllDescriptors()
{
    return allOptionDescriptors;
}

std::string
OptionDescriptor::getTypeDescriptor() const
{
    OptionType type = getType();
    std::string result;
    
    switch (type) {
        case BOOLEAN: {
            result += "boolean";
            break;
        }
        case CHAR: {
            result += "char";
            break;
        }
        case STRING: {
            result += "string";
            break;
        }
        case STRING_LIST: {
            result += "string_list";
            break;
        }
        case ENUM: {
            result += "enum";
            break;
        }
        case PATH: {
            result += "path";
            break;
        }
        case PATH_LIST: {
            result += "path_list";
            break;
        }
        default: {
            result += "invalid type";
            break;
        }
    }
    return result;
}

std::string
OptionDescriptor::describeAllOptions()
{
    std::string result;

    result += "Options:\n";
    result += "========\n\n";
    for (std::list<OptionDescriptor*>::iterator i= allOptionDescriptors.begin(); i != allOptionDescriptors.end(); i++) {
        OptionDescriptor *od = *i;
        result += "  -";
        result += od->getName();
        if (od->isValueOptional()) {
            result += "[";
        }
        result += "=";
        result += od->getTypeDescriptor();
        if (od->isValueOptional()) {
            result += "]";
        }
        if (od->getDescription() != NULL && strcmp(od->getDescription(), "???")) {
            result += "\n    ";
            result += od->getDescription();
        }
        result += '\n';
    }
    return result;
}

void
OptionDescriptor::initializeAll(OptionProcessor *processor)
{
    for (std::list<OptionDescriptor*>::iterator i= allOptionDescriptors.begin(); i != allOptionDescriptors.end(); i++) {
        OptionDescriptor *od = *i;
        od->initializeValue(processor);
    }
}

void
OptionDescriptor::processSetting(OptionProcessor *processor, OptionValue *v)
{
//  cerr << "Setting option '" << getName() << "' to value " << *v->toString() << endl;
    
    (processor->*valueHandler)(v);
}

void
OptionDescriptor::initializeValue(OptionProcessor *processor)
{
//  cerr << "*** " << getName() << " uses default implementation of setDefault()!" << endl;
}

OptionValue *
OptionDescriptor::createValue(bool noFlag)
{
    switch (getType()) {
        case BOOLEAN: {
            BooleanOptionValue *bv = new BooleanOptionValue(this, noFlag);
            return bv;
        }
        case ENUM: {
            EnumOptionValue *ev = new EnumOptionValue(this, noFlag);
            return ev;
        }
        case INTEGER: {
            IntegerOptionValue *iv = new IntegerOptionValue(this, noFlag);
            return iv;
        }
        case PATH: {
            PathOptionValue *pv = new PathOptionValue(this, noFlag);
            return pv;
        }
        case STRING: {
            StringOptionValue *sv = new StringOptionValue(this, noFlag);
            return sv;
        }
        case STRING_LIST: {
            StringListOptionValue *slv = new StringListOptionValue(this, noFlag);
            return slv;
        }
        case PATH_LIST: {
            PathListOptionValue *plv = new PathListOptionValue(this, noFlag);
            return plv;
        }
        default:
            return NULL; // shouldn't happen
    }
}

//
// =================================================================================
//

BooleanOptionDescriptor::BooleanOptionDescriptor(const char *wd1, const char *wd2, const char *descrip, bool defValue,
                                                 OptionProcessor::BooleanValueField field, bool valueOpt)

: OptionDescriptor(BOOLEAN, wd1, wd2, descrip, valueOpt), initValue(defValue), boolField(field)
{
}

BooleanOptionDescriptor::BooleanOptionDescriptor(const char *wd1, const char *descrip, bool defValue,
                                                 OptionProcessor::BooleanValueField field, bool valueOpt)

: OptionDescriptor(BOOLEAN, wd1, NULL, descrip, valueOpt), initValue(defValue), boolField(field)
{
}

void
BooleanOptionDescriptor::processSetting(OptionProcessor *processor, OptionValue *v)
{
    BooleanOptionValue *bv = static_cast<BooleanOptionValue*> (v);
    Option *options = processor->getOptions();

    options->*boolField = bv->getValue();
}

void
BooleanOptionDescriptor::initializeValue(OptionProcessor *processor)
{
   processor->getOptions()->*boolField = initValue;
}

//
// =================================================================================
//

IntegerOptionDescriptor::IntegerOptionDescriptor(const char *wd1, int initVal, int min, int max,
                                                 const char *descrip,
                                                 OptionProcessor::IntegerValueField field, bool valueOpt)
: OptionDescriptor(INTEGER, wd1, NULL, descrip, valueOpt), initValue(initVal), minValue(min), maxValue(max), intField(field)
{
}

IntegerOptionDescriptor::IntegerOptionDescriptor(const char *wd1, const char *wd2, int initVal, int min, int max,
                                                 const char *descrip,
                                                 OptionProcessor::IntegerValueField field, bool valueOpt)
: OptionDescriptor(INTEGER, wd1, wd2, descrip, valueOpt), initValue(initVal), minValue(min), maxValue(max), intField(field)
{
}

void
IntegerOptionDescriptor::processSetting(OptionProcessor *processor, OptionValue *v)
{
    IntegerOptionValue *iv = static_cast<IntegerOptionValue*> (v);
    Option *options = processor->getOptions();

    options->*intField = iv->getValue();
}

void
IntegerOptionDescriptor::initializeValue(OptionProcessor *processor)
{
    processor->getOptions()->*intField = initValue;
}

std::string
IntegerOptionDescriptor::getTypeDescriptor() const
{
    std::string result;
    
    result += "int";

    if (this->minValue != std::numeric_limits<int>::min() && this->maxValue == std::numeric_limits<int>::max()) {
        IntToString minStr(this->minValue);
        result += "[ >= ";
        result += minStr.String();
        result += "]";
    } else if (this->minValue == std::numeric_limits<int>::min() && this->maxValue != std::numeric_limits<int>::max()) {
        IntToString maxStr(this->maxValue);
        result += "[ <= ";
        result += maxStr.String();
        result += "]";
    } else {
        IntToString minStr(this->minValue);
        IntToString maxStr(this->maxValue);
        result += "[";
        result += minStr.String();
        result += "..";
        result += maxStr.String();
        result += "]";
    }
    
    return result;
}

//
// =================================================================================
//

StringOptionDescriptor::StringOptionDescriptor(const char *wd1, const char *descrip, const char *initVal,
                                               OptionProcessor::StringValueField field, bool emptyOk)
: OptionDescriptor(STRING, wd1, NULL, descrip, false), emptyOk(emptyOk), stringField(field), initValue(initVal)
{
}

StringOptionDescriptor::StringOptionDescriptor(const char *wd1, const char *wd2, const char *descrip, const char *initVal,
                                               OptionProcessor::StringValueField field, bool emptyOk)
: OptionDescriptor(STRING, wd1, wd2, descrip, false), emptyOk(emptyOk), stringField(field), initValue(initVal)
{
}

StringOptionDescriptor::StringOptionDescriptor(OptionType t, const char *wd1, const char *wd2, const char *descrip,
                                               const char *initVal,
                                               OptionProcessor::StringValueField field, bool emptyOk)
: OptionDescriptor(t, wd1, wd2, descrip, false), emptyOk(emptyOk), stringField(field), initValue(initVal)
{
}

void
StringOptionDescriptor::processSetting(OptionProcessor *processor, OptionValue *v)
{
    StringOptionValue *sv = static_cast<StringOptionValue*> (v);
    Option *options = processor->getOptions();

    if (v == NULL || sv->getValue().length() == 0) {
        if (emptyOk) {
            options->*stringField = _strdup("");
        } else {
            throw ValueFormatException("Empty string not allowed for option", *v->toString(), v->getOptionDescriptor());
        }

    } else {
        options->*stringField = _strdup(sv->getValue().c_str());
    }
}

void
StringOptionDescriptor::initializeValue(OptionProcessor *processor)
{
    processor->getOptions()->*stringField = (initValue != NULL) ? _strdup(initValue) : NULL;
}

//
// =================================================================================
//

CharOptionDescriptor::CharOptionDescriptor(const char *wd1, const char *wd2, const char *descrip,
                                           const char *initValue,
                                           OptionProcessor::CharValueField field)
: StringOptionDescriptor(CHAR, wd1, wd2, descrip, initValue, NULL,false), charField(field)
{
}

CharOptionDescriptor::CharOptionDescriptor(const char *wd1, const char *descrip,
                                           const char *initValue,
                                           OptionProcessor::CharValueField field)
: StringOptionDescriptor(CHAR, wd1, NULL, descrip, initValue, NULL,false), charField(field)
{
}

void
CharOptionDescriptor::processSetting(OptionProcessor *processor, OptionValue *v)
{
    StringOptionValue *sv = static_cast<StringOptionValue*> (v);
    Option *options = processor->getOptions();

    if (v == NULL || sv->getValue().length() == 0) {
        throw ValueFormatException("Empty string not allowed for option", *v->toString(), v->getOptionDescriptor());
    } else {
        options->*charField = sv->getValue().at(0);
    }
}

void
CharOptionDescriptor::initializeValue(OptionProcessor *processor)
{
    processor->getOptions()->*charField = initValue[0];
}

//
// =================================================================================
//

PathOptionDescriptor::PathOptionDescriptor(const char *wd1, const char *wd2, const char *descrip,
                                           const char *initValue,
                                           OptionProcessor::StringValueField field, bool emptyOk)
: StringOptionDescriptor(PATH, wd1, wd2, descrip, initValue, field, emptyOk)
{
}

void
PathOptionDescriptor::processSetting(OptionProcessor *processor, OptionValue *v)
{
    StringOptionDescriptor::processSetting(processor, v);

    // TODO Verify that path exists?
}

//
// =================================================================================
//

EnumOptionDescriptor::EnumOptionDescriptor(const char *word1, const char *descrip,
                                           OptionProcessor::IntegerValueField field,
                                           const char *initVal, const char *defValue,
                                           const char *noVal, EnumValue * value, ...)
: OptionDescriptor(ENUM, word1, NULL, descrip, false), intField(field),
  initValue(initVal), defaultValue(defValue), noValue(noVal)
{
    setupName();
    // set up values list from varargs param
    va_list ap;
    va_start(ap, value);
    for (EnumValue *v=value; v != NULL; v = va_arg(ap, EnumValue*)) {
        legalValues.push_back(v);
    }
    va_end(ap);
}

EnumOptionDescriptor::EnumOptionDescriptor(const char *word1, const char *word2, const char *descrip,
                                           OptionProcessor::IntegerValueField field,
                                           const char *initVal, const char *defValue,
                                           const char *noVal, EnumValue * value, ...)
: OptionDescriptor(ENUM, word1, word2, descrip, false), intField(field),
  initValue(initVal), defaultValue(defValue), noValue(noVal)
{
    setupName();
    // set up values list from varargs param
    va_list ap;
    va_start(ap, value);
    for (EnumValue *v=value; v != NULL; v = va_arg(ap, EnumValue*)) {
        legalValues.push_back(v);
    }
    va_end(ap);
}

EnumOptionDescriptor::EnumOptionDescriptor(const char *word1, const char *word2, const char *descrip,
                                           OptionProcessor::ValueHandler handler,
                                           const char *initVal, EnumValue *value, ...)
: OptionDescriptor(ENUM, word1, word2, descrip, handler, true), intField(NULL),
  initValue(initVal), defaultValue(""), noValue("")
{
    setupName();
    // set up values list from varargs param
    va_list ap;
    va_start(ap, value);
    for (EnumValue *v=value; v != NULL; v = va_arg(ap, EnumValue*)) {
        legalValues.push_back(v);
    }
    va_end(ap);
}

EnumOptionDescriptor::EnumOptionDescriptor(const char *word1, const char *descrip,
                                           OptionProcessor::ValueHandler handler,
                                           const char *initVal, EnumValue *value, ...)
: OptionDescriptor(ENUM, word1, NULL, descrip, handler, true), intField(NULL),
  initValue(initVal), defaultValue(""), noValue("")
{
    setupName();
    // set up values list from varargs param
    va_list ap;
    va_start(ap, value);
    for (EnumValue *v=value; v != NULL; v = va_arg(ap, EnumValue*)) {
        legalValues.push_back(v);
    }
    va_end(ap);
}

std::string
EnumOptionDescriptor::getTypeDescriptor() const
{
    std::string result;
    const std::list<EnumValue*>& legalValues = getLegalValues();
    
    for (EnumValueList::const_iterator i= legalValues.begin(); i != legalValues.end(); i++) {
        if (i != legalValues.begin()) {
            result += " | ";
        }
        result += (*i)->first();
    }
    return result;
}

EnumValue *
EnumOptionDescriptor::findEnumByName(const std::string& nm)
{
    if (nm.length() == 0) {
        if (defaultValue.length() > 0) {
            return findEnumByName(defaultValue);
        }
    }
    for (EnumValueList::const_iterator iter = legalValues.begin(); iter != legalValues.end(); iter++) {
        EnumValue *ev = *iter;
        const char *eName = ev->first();

        if (!nm.compare(eName)) {
            return ev;
        }
    }
    return NULL;
}

void
EnumOptionDescriptor::processSetting(OptionProcessor *processor, OptionValue *value)
{
    EnumOptionValue *ev = static_cast<EnumOptionValue*>(value);

    EnumValue *val = findEnumByName(ev->getValue());

    if (val != NULL) {
        int intValue = val->second();

        if (intField != 0) {
//          cerr << "int field ptr = " << (&(processor->getOptions()->*intField)) << endl;
            processor->getOptions()->*intField = intValue;
        } else {
            OptionDescriptor::processSetting(processor, value);
        }
    } else {
        OptionDescriptor::processSetting(processor, value);
    }
}

void
EnumOptionDescriptor::initializeValue(OptionProcessor *processor)
{
    EnumValue *ev = findEnumByName(initValue);

    if (ev != NULL) {
        processor->getOptions()->*intField = ev->second();
    }
}
