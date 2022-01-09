//
// Created by kuafu on 2022/1/8.
//
#pragma once

#ifndef LPG2_VISITORSTAFFFACTORY_H
#define LPG2_VISITORSTAFFFACTORY_H


#include <cstring>
#include "symbol.h"

class Action;


class VisitorStaffFactory{
public:
    static const char  * argument ,
            * result ,
            * abstract ,
            * preorder ;

    VisitorStaffFactory(const char* visitorType);

    void GenerateVisitor(Action* action,
                         ActionFileLookupTable &ast_filename_table,
                         ActionFileSymbol* default_file_symbol,
                         Tuple<ActionBlockElement>& notice_actions,
                         SymbolLookupTable& type_set);

    void GenerateVisitorInterface(Action* action,
                              ActionFileLookupTable &ast_filename_table,
                              ActionFileSymbol* default_file_symbol,
                              Tuple<ActionBlockElement>& notice_actions,
                              SymbolLookupTable& type_set);

    void GenerateVisitorAbstractClass(Action* action,
                                  ActionFileLookupTable &ast_filename_table,
                                  ActionFileSymbol* default_file_symbol,
                                  Tuple<ActionBlockElement>& notice_actions,
                                  SymbolLookupTable& type_set);
    ~VisitorStaffFactory();

    char * visitor_type,
    *argument_visitor_type = nullptr,
            *result_argument_visitor_type = nullptr,

            *abstract_visitor_type = nullptr,

            *preorder_visitor_type = nullptr,
            *abstract_preorder_visitor_type = nullptr,

            *result_visitor_type = nullptr,
            *abstract_result_visitor_type = nullptr;
};

#endif //LPG2_VISITORSTAFFFACTORY_H
