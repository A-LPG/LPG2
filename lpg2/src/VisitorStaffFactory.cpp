//
// Created by kuafu on 2022/1/8.
//
#include "Action.h"
#include "VisitorStaffFactory.h"
 const char  * VisitorStaffFactory::argument = "Argument",
        * VisitorStaffFactory::result = "Result",
        * VisitorStaffFactory::abstract = "Abstract",
        * VisitorStaffFactory::preorder = "PreOrder";
VisitorStaffFactory::VisitorStaffFactory(const char* visitorType) {

    size_t len = strlen(visitorType) + 1;
    visitor_type = new char[len];
    visitor_type[len-1] = 0;
    strcat(this->visitor_type, visitorType);


    len = strlen(argument) + strlen(visitor_type) + 1;
    argument_visitor_type = new char[len];
    argument_visitor_type[len-1] = 0;
    strcpy(argument_visitor_type, argument);
    strcat(argument_visitor_type, visitor_type);


    len = strlen(result) + strlen(argument) + strlen(visitor_type) + 1;
    result_argument_visitor_type = new char[len];
    result_argument_visitor_type[len-1] = 0;
    strcpy(result_argument_visitor_type, result);
    strcat(result_argument_visitor_type, argument);
    strcat(result_argument_visitor_type, visitor_type);


    len = strlen(abstract) + strlen(visitor_type) + 1;
    abstract_visitor_type = new char[len];
    abstract_visitor_type[len-1] = 0;
    strcpy(abstract_visitor_type, abstract);
    strcat(abstract_visitor_type, visitor_type);


    len = strlen(preorder) + strlen(visitor_type) + 1;
    preorder_visitor_type = new char[len];
    preorder_visitor_type[len-1] = 0;
    strcpy(preorder_visitor_type, preorder);
    strcat(preorder_visitor_type, visitor_type);


    len = strlen(abstract) + strlen(preorder) + strlen(visitor_type) + 1;
    abstract_preorder_visitor_type = new char[len];
    abstract_preorder_visitor_type[len-1] = 0;
    strcpy(abstract_preorder_visitor_type, abstract);
    strcat(abstract_preorder_visitor_type, preorder);
    strcat(abstract_preorder_visitor_type, visitor_type);


    len = strlen(result) + strlen(visitor_type) + 1;
    result_visitor_type = new char[len];
    result_visitor_type[len-1] = 0;
    strcpy(result_visitor_type, result);
    strcat(result_visitor_type, visitor_type);



    len = strlen(abstract) + strlen(result) + strlen(visitor_type) + 1;
    abstract_result_visitor_type = new char[len];
    abstract_result_visitor_type[len-1] = 0;
    strcpy(abstract_result_visitor_type, abstract);
    strcat(abstract_result_visitor_type, result);
    strcat(abstract_result_visitor_type, visitor_type);


}

VisitorStaffFactory::~VisitorStaffFactory() {

    delete[] visitor_type;
    delete[] abstract_preorder_visitor_type;
    delete[] argument_visitor_type;
    delete[] result_visitor_type;
    delete[] result_argument_visitor_type;
    delete[] abstract_visitor_type;
    delete[] abstract_result_visitor_type;
}
void VisitorStaffFactory::GenerateVisitor(Action* action,
                                          ActionFileLookupTable &ast_filename_table,
                                          ActionFileSymbol* default_file_symbol,
                                          Tuple<ActionBlockElement>& notice_actions,
                                          SymbolLookupTable& type_set){
    GenerateVisitorInterface(action,ast_filename_table,default_file_symbol,notice_actions,type_set);
    GenerateVisitorAbstractClass(action,ast_filename_table,default_file_symbol,notice_actions,type_set);

}
void VisitorStaffFactory::GenerateVisitorInterface(Action* action,
                              ActionFileLookupTable &ast_filename_table,
                              ActionFileSymbol* default_file_symbol,
                              Tuple<ActionBlockElement>& notice_actions,
                              SymbolLookupTable& type_set){

    auto option = action->option;

    if (option->visitor & Option::DEFAULT)
    {
        if (option->IsNested())
        {
            action->GenerateSimpleVisitorInterface(default_file_symbol,
                                                   "    ", visitor_type, type_set);
            action->GenerateArgumentVisitorInterface(default_file_symbol, "    ", argument_visitor_type, type_set);
            action->GenerateResultVisitorInterface(default_file_symbol, "    ", result_visitor_type, type_set);
            action->GenerateResultArgumentVisitorInterface(default_file_symbol, "    ", result_argument_visitor_type, type_set);

        }
        else
        {
            ActionFileSymbol* file_symbol = action->GenerateTitle(ast_filename_table, notice_actions, visitor_type, false);
            action->GenerateSimpleVisitorInterface(file_symbol, "", visitor_type, type_set);
            file_symbol->Flush();

            file_symbol = action->GenerateTitle(ast_filename_table, notice_actions, argument_visitor_type, false);
            action->GenerateArgumentVisitorInterface(file_symbol, "", argument_visitor_type, type_set);
            file_symbol->Flush();

            file_symbol = action->GenerateTitle(ast_filename_table, notice_actions, result_visitor_type, false);
            action->GenerateResultVisitorInterface(file_symbol, "", result_visitor_type, type_set);
            file_symbol->Flush();

            file_symbol = action->GenerateTitle(ast_filename_table, notice_actions, result_argument_visitor_type, false);
            action->GenerateResultArgumentVisitorInterface(file_symbol, "", result_argument_visitor_type, type_set);
            file_symbol->Flush();

        }
    }
    if (option->visitor & Option::PREORDER)
    {
        if (option->IsNested())
        {
            action->GeneratePreorderVisitorInterface(default_file_symbol, "    ", preorder_visitor_type, type_set);
        }
        else
        {
            ActionFileSymbol* file_symbol = action->GenerateTitleAndGlobals(ast_filename_table, notice_actions, preorder_visitor_type, false);
            action->GeneratePreorderVisitorInterface(file_symbol, "", preorder_visitor_type, type_set);
            file_symbol->Flush();

        }
    }
}
void VisitorStaffFactory::GenerateVisitorAbstractClass(Action* action,
                                                   ActionFileLookupTable &ast_filename_table,
                                                   ActionFileSymbol* default_file_symbol,
                                                   Tuple<ActionBlockElement>& notice_actions,
                                                   SymbolLookupTable& type_set){
    auto option = action->option;
    if (option->visitor & Option::DEFAULT)
    {
        if (option->IsNested())
        {
            action->GenerateNoResultVisitorAbstractClass(default_file_symbol, "    ", abstract_visitor_type, type_set);
            action->GenerateResultVisitorAbstractClass(default_file_symbol, "    ", abstract_result_visitor_type, type_set);
        }
        else
        {
            auto file_symbol = action->GenerateTitle(ast_filename_table, notice_actions, abstract_visitor_type, false);
            action->GenerateNoResultVisitorAbstractClass(file_symbol, "", abstract_visitor_type, type_set);
            file_symbol->Flush();

            file_symbol = action->GenerateTitle(ast_filename_table, notice_actions, abstract_result_visitor_type, false);
            action->GenerateResultVisitorAbstractClass(file_symbol, "", abstract_result_visitor_type, type_set);
            file_symbol->Flush();
        }
    }
    if (option->visitor & Option::PREORDER)
    {
        if (option->IsNested())
        {
            action->GeneratePreorderVisitorAbstractClass(default_file_symbol, "    ", abstract_preorder_visitor_type, type_set);
        }
        else
        {
            auto file_symbol = action->GenerateTitleAndGlobals(ast_filename_table, notice_actions,
                                                               abstract_preorder_visitor_type, false);

            action->GeneratePreorderVisitorAbstractClass(file_symbol, "", abstract_preorder_visitor_type, type_set);
            file_symbol->Flush();
        }
    }
}
