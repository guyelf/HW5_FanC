#ifndef COMPILATION_HW3_SEMANTICALSTRUCTS_H
#define COMPILATION_HW3_SEMANTICALSTRUCTS_H

#include <vector>
#include <string>
#include "SymbolsTable.h"
#include "hw3_output.hpp"

using namespace std;
using namespace output;

struct SemanticAttributes{
    string string_val;
    string type;
    pair<string,string> argument;
    vector<pair<string,string>> arguments;
    vector<string> arguments_type; // exp_list
};

#define YYSTYPE SemanticAttributes

void openGlobalScope(SymbolsTable& symbols_table);
void closeGlobalScope(SymbolsTable& symbols_table);
void openFunctionScope(SymbolsTable& symbols_table, string ret_type, string func_name, vector<pair<string,string>> arguments, int lineno);
void openScope(SymbolsTable& symbols_table);
void closeScope(SymbolsTable& symbols_table);
void callFunction(SymbolsTable& symbols_table, string func_name,vector<string> arguments_type,int lineno);
void checkByte(string byte, int lineno);
bool isNumerical(string type);
bool isBool(string type);
string getLargestRangeType(string l_type, string r_type);
bool checkTypeValidity(string l_type, string r_type);
bool checkRelopValidity(string l_type, string r_type);
bool checkLogicValidity(string l_type, string r_type);
bool checkBinopValidity(string l_type, string r_type);
bool checkFunctionDeclValidity(string ret_type, vector<pair<string,string>> argument_types);
bool isValidArgsFunctionCall(vector<string> argument_types, vector<string> exp_argument_types);
bool isValidRetType(string ret_type);
void update_while_flag(int val);
void update_switch_flag(int val);
void is_valid_break(int lineno);
void is_valid_continue(int lineno);

#endif //COMPILATION_HW3_SEMANTICALSTRUCTS_H
