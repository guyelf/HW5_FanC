#ifndef COMPILATION_HW3_SEMANTICALSTRUCTS_H
#define COMPILATION_HW3_SEMANTICALSTRUCTS_H

#include <vector>
#include <string>
#include "SymbolsTable.h"
#include "hw3_output.hpp"
#include "CodeGeneration.h"
#include "ScopesStack.h"

using namespace std;
using namespace output;


struct SemanticAttributes{
    string case_num;
    string label;
    string default_label;
    int exp_reg;
    string string_val;
    string type;
    pair<string,string> argument;
    vector<pair<string,string>> arguments;
    vector<pair<string,int>> exp_arguments;
    vector<string> arguments_type; // exp_list
    vector<pair<int,BranchLabelIndex>> sc_list;
    vector<pair<int,BranchLabelIndex>> next_list;
    vector<pair<string,string>> case_list;
};

#define YYSTYPE SemanticAttributes

void openGlobalScope(SymbolsTable& symbols_table);
void closeGlobalScope(SymbolsTable& symbols_table);
void openFunctionScope(SymbolsTable& symbols_table, string ret_type, string func_name, vector<pair<string,string>> arguments, int lineno);
void closeFunctionScope();
void openScope(SymbolsTable& symbols_table, int is_while = 0, int is_switch = 0, int is_if = 0, int is_else = 0, string while_label = "");
void closeScope(SymbolsTable& symbols_table, int is_while = 0, int is_switch = 0);
int callFunction(SymbolsTable& symbols_table, string func_name,vector<pair<string,int>> arguments,int lineno);
void closeFunction();
void returnFunction(string ret_type, int reg=-1);
void gen_local_var_to_default(SymbolsTable symbols_table, string id);
void gen_local_var_to_reg(SymbolsTable symbols_table, string id, int r_reg);
void set_local_var_to_reg(SymbolsTable symbols_table, string id, int r_reg);
int set_value_to_new_reg(string string_val);
int set_string_val_to_new_reg(string string_val);
int get_reg_from_id(SymbolsTable symbols_table, string id);
int genBinop(int reg1, string op, int reg2, string reg1_type,string reg2_type);
int genRelop(int reg1, string op, int reg2);
int genNotExp(int reg1);
void checkLogicalExp(int reg1, string op,vector<pair<int,BranchLabelIndex>> &sc_list);
int finishLogicalExp(int reg1, string op,vector<pair<int,BranchLabelIndex>> &sc_list);
void openIf(int cond_reg);
void closeIf();
void openElse();
void closeBlock();
void closeElseBlock();
string genWhileLabel();
void openWhile(int cond_reg);
void closeWhile();
void continueWhile();
void breakBlock();
string gen_label();
void openSwitch();
void closeCase();
void switchBlock(int exp_reg, string label, vector<pair<string,string>>& case_list);





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
