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

    VisitorStaffFactory();

    void GenerateCreatVisitor(Action* action,
                              ActionFileLookupTable &ast_filename_table,
                              ActionFileSymbol* default_file_symbol,
                              Tuple<ActionBlockElement>& notice_actions,
                              SymbolLookupTable& type_set);

    ~VisitorStaffFactory();
};

#endif //LPG2_VISITORSTAFFFACTORY_H
