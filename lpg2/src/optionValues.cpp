/*
 *  optionValues.cpp
 *  lpg
 *
 *  Created by Robert M. Fuhrer on 3/21/11.
 *  Copyright 2011 IBM.
 */

#include "options.h"

static const std::string *TRUE_VALUE_STR = new std::string("true");
static const std::string *FALSE_VALUE_STR = new std::string("false");

void
OptionValue::processSetting(OptionProcessor *processor)
{
    getOptionDescriptor()->processSetting(processor, this);
}

void
BooleanOptionValue::parseValue(std::string *v) throw(ValueFormatException)
{
    if (v == NULL) {
        if (optionDesc->isValueOptional()) {
            value = !noFlag;
        } else {
            throw ValueFormatException("Missing boolean value", optionDesc);
        }
    } else if (!v->compare(*TRUE_VALUE_STR)) {
        value = true;
    } else if (!v->compare(*FALSE_VALUE_STR)) {
        value = false;
    } else {
        throw ValueFormatException("Invalid boolean value", *v, optionDesc);
    }
}

const std::string *
BooleanOptionValue::toString()
{
    return value ? TRUE_VALUE_STR : FALSE_VALUE_STR;
}

void
IntegerOptionValue::parseValue(std::string *v) throw(ValueFormatException)
{
    if (noFlag) {
        throw ValueFormatException("Invalid 'no' prefix for option", optionDesc);
    }
    if (v == NULL) {
        if (optionDesc->isValueOptional()) {
            value = 0;
        } else {
            throw ValueFormatException("Missing integer value", optionDesc);
        }
    }
    
    const IntegerOptionDescriptor *iod = static_cast<const IntegerOptionDescriptor*> (optionDesc);
    const char *vs = v->c_str();
    
    if (!verify(vs)) {
        throw ValueFormatException("Invalid integer value", *v, optionDesc);
    } else {
        int iv = atoi(vs);
        if (iv < iod->getMinValue() || iv > iod->getMaxValue()) {
            throw ValueFormatException("Integer value outside allowable range", *v, optionDesc);
        }
        value = iv;
    }
}

const std::string *
IntegerOptionValue::toString()
{
    IntToString i2s(value);
    std::string *result = new std::string(i2s.String());
    return result;
}

void CharOptionValue::parseValue(std::string* v)
{
    if (v == NULL) {
        if (optionDesc->isValueOptional()) {

        }
        else {
            throw ValueFormatException("Missing char value", optionDesc);
        }
    }
    else if (!v->length() > 1) {
        throw ValueFormatException("Need  char value, no string", optionDesc);
    }
    else if (v->length() == 1) {
        value = v->at(0);
    }
    else {
        throw ValueFormatException("Invalid char value", *v, optionDesc);
    }
}

const std::string* CharOptionValue::toString()
{
    std::string* result = new std::string(value.c_str());

    return result;
}

void
StringOptionValue::parseValue(std::string *v) throw(ValueFormatException)
{
    if (v == NULL) {
        throw ValueFormatException("Missing string value", optionDesc);
    }
    if (v->length() == 0) {
        value = *v;
    } else if (v->at(0) == '\'') {
        if (v->at(v->length()-1) != '\'') {
            throw ValueFormatException("String option value must be quoted", *v, optionDesc);
        }
        // trim quotes
        value = v->substr(1, v->length() - 2);
    } else {
        value = v->substr(0, v->length());
    }
}

const std::string *
StringOptionValue::toString()
{
    std::string *result = new std::string(value.c_str());
    return result;
}

void
EnumOptionValue::parseValue(std::string *v) throw(ValueFormatException)
{
    EnumOptionDescriptor *eod = static_cast<EnumOptionDescriptor*> (optionDesc);
    
    std::list<EnumValue*> legalValues = eod->getLegalValues();

    if (v == NULL) {
        if (!eod->isValueOptional() && eod->getDefaultValue().size() == 0) {
            throw ValueFormatException("option requires a value", "", optionDesc);
        }
        value = eod->getDefaultValue();
        return;
    }
    
    int endIdx = v->find_first_of(',');
    
    std::string enumValue = v->substr(0, endIdx);

    // Check that the given value is one of the allowed values
    bool found = false;
    for (EnumOptionDescriptor::EnumValueList::iterator i = legalValues.begin(); i != legalValues.end(); i++) {
        if (!enumValue.compare((*i)->first())) {
            found = true;
            value = *v;
            return;
        }
    }
    if (!found) {
        // Say what are the legal values
        std::string msg = describeLegalValues();
        throw ValueFormatException(msg, *v, eod);
    }
}

std::string
EnumOptionValue::describeLegalValues()
{
    std::string msg;
    EnumOptionDescriptor *eod = static_cast<EnumOptionDescriptor*> (getOptionDescriptor());
    EnumOptionDescriptor::EnumValueList legalValues = eod->getLegalValues();
    
    msg.append("Legal values are: {");
    for (EnumOptionDescriptor::EnumValueList::iterator i = legalValues.begin(); i != legalValues.end(); i++) {
        if (i != legalValues.begin()) {
            msg.append(" |");
        }
        msg.append(" ");
        msg.append((*i)->first());
    }
    msg.append(" }");
    return msg;
}

void
StringListOptionValue::addValue(const char *s)
{
    std::string v = s;
    values.push_back(v);
}

void
StringListOptionValue::parseValue(std::string *v) throw(ValueFormatException)
{
    if (v == NULL) {
        throw ValueFormatException("Missing list-of-strings value", optionDesc);
    }
    
    // Trim surrounding ()'s, and split string at commas
    // Work on a copy
    const char *p = v->c_str();
    if (p[0] != '(') {
        throw ValueFormatException("String-list-valued option must be enclosed in parentheses", *v, optionDesc);
    }
    if (p[strlen(p)-1] != ')') {
        throw ValueFormatException("String-list-valued option must be enclosed in parentheses", *v, optionDesc);
    }
    char *pCopy = _strdup(p+1); // trim leading '('
    pCopy[strlen(pCopy)-1] = '\0'; // trim trailing ')'
    
    const char *pStart = pCopy;
    do {
        const char *pSep = strchr(pStart, ',');
        std::string *val;

        const char *pEnd = (pSep != NULL ? pSep : pStart + strlen(pStart));
        while (*pStart != '\0' && *pStart == ' ') { // trim leading spaces
            pStart++;
        }
        if (*pStart == '"') { // trim leading quote
            pStart++;
        }
        if (*(pEnd-1) == '"') { // trim trailing quote
            pEnd--;
        }
        val = new std::string(pStart, pEnd - pStart);
        if (pSep != NULL) {
            pStart = pSep + 1;
        } else {
            pStart = NULL;
        }
        
        values.push_back(*val);
        delete val;
    } while (pStart != NULL);
    
    free(pCopy);
}

const std::string *
StringListOptionValue::toString()
{
    std::string *result = new std::string();
    
    for (std::list<std::string>::iterator i = values.begin(); i != values.end(); i++) {
        if (i != values.begin()) {
            result->append(",");
        }
        result->append(*i);
    }
    return result;
}

void
PathListOptionValue::parseValue(std::string *v) throw(ValueFormatException)
{
    if (v == NULL) {
        throw ValueFormatException("Missing path-list value", optionDesc);
    }
    
    // Split string at semicolons
    const char *pStart = v->c_str();
    do {
        const char *pEnd = strchr(pStart, ';');
        std::string *val;
        
        if (pEnd == NULL) {
            val = new std::string(pStart);
            pStart = pEnd;
        } else {
            val = new std::string(pStart, pEnd - pStart);
            pStart = pEnd + 1;
        }
        
        values.push_back(*val);
        delete val;
    } while (pStart != NULL);
}

const std::string *
PathListOptionValue::toString()
{
    std::string *result = new std::string();
    
    for (std::list<std::string>::iterator i = values.begin(); i != values.end(); i++) {
        if (i != values.begin()) {
            result->append(";");
        }
        result->append(*i);
    }
    return result;
}
