#include "SemanticalStructs.h"

string cur_func_ret_type = "";
int while_flag = 0;
int switch_flag = 0;

void openGlobalScope(SymbolsTable& symbols_table) {
    symbols_table = SymbolsTable();

    vector<string> print_types = {"STRING"};
    string print_type = makeFunctionType("VOID", print_types);
    symbols_table.PushNewRecord("print",print_type,0);

    vector<string> printi_types = {"INT"};
    string printi_type = makeFunctionType("VOID", printi_types);
    symbols_table.PushNewRecord("printi",printi_type,0);
}

void closeGlobalScope(SymbolsTable& symbols_table) {
    if(symbols_table.Contains("main")!=1){
        errorMainMissing();
        exit(0);
    }
    auto main_type = symbols_table.GetFunctionType("main");
    if(main_type.first != "VOID" || !main_type.second.empty()) {
        errorMainMissing();
        exit(0);
    }
    closeScope(symbols_table);
}

void openFunctionScope(SymbolsTable& symbols_table, string ret_type, string func_name, vector<pair<string,string>> arguments, int lineno) {
    if(symbols_table.Contains(func_name) != 0){
        errorDef(lineno, func_name);
        exit(0);
    }
    vector<string> attr_names;
    for (auto pair:arguments){ //checking if one of the params has the name of the function - shadowing is not allowed! 
        if(pair.second == func_name){
            errorDef(lineno, func_name);
            exit(0);
        }
    }
    vector<string> types;
    for(auto pair:arguments){
        types.push_back(pair.first);
    }
    symbols_table.PushNewFunction(func_name, makeFunctionType(ret_type,types),arguments);
    cur_func_ret_type = ret_type;
}

void openScope(SymbolsTable& symbols_table) {
    symbols_table.PushNewTable();
}

void closeScope(SymbolsTable& symbols_table) {
    endScope();
    symbols_table.PopTable();
}

void callFunction(SymbolsTable& symbols_table, string func_name, vector<string> arguments_type, int lineno) {
    if(symbols_table.Contains(func_name) != 1 || symbols_table.GetType(func_name).find("->") == string::npos){
        errorUndefFunc(lineno, func_name);
        exit(0);
    }
    vector<string> expected_arguments_type = symbols_table.GetFunctionType(func_name).second;
    if(!isValidArgsFunctionCall(arguments_type, expected_arguments_type)){
        errorPrototypeMismatch(lineno, func_name, expected_arguments_type);
        exit(0);
    }
}

void checkByte(string byte, int lineno) {
    if(stoi(byte) >255){
        errorByteTooLarge(lineno, byte);
        exit(0);
    }
}

bool isNumerical(string type){
    return type == "INT" || type == "BYTE";
}

bool isBool(string type){
    return type == "BOOL";
}

string getLargestRangeType(string l_type, string r_type){
    if(l_type == "INT" || r_type == "INT") return "INT";
    return "BYTE";
}

bool checkTypeValidity(string l_type, string r_type){
    if (l_type == r_type) return true;
    else if (l_type == "INT" && r_type == "BYTE") return true;
    return false;
}

bool checkRelopValidity(string l_type, string r_type){
    return isNumerical(l_type) && isNumerical(r_type);
}

bool checkLogicValidity(string l_type, string r_type){
    return isBool(l_type) && isBool(r_type);
}

bool checkBinopValidity(string l_type, string r_type){
    return isNumerical(l_type) && isNumerical(r_type);
}

bool checkFunctionDeclValidity(string ret_type, vector<pair<string,string>> argument_types){
    if (!isNumerical(ret_type) && !isBool(ret_type)){
        return false;
    }
    for(auto arg_type:argument_types){
        if (!isNumerical(arg_type.first) && !isBool(arg_type.first)){
            return false;
        }
    }
    return true;
}

bool isValidArgsFunctionCall(vector<string> argument_types, vector<string> exp_argument_types){
    if(argument_types.size() != exp_argument_types.size()) return false;
    for (size_t i = 0 ; i < argument_types.size() ; ++i) {
        if(!checkTypeValidity(exp_argument_types[i], argument_types[argument_types.size()-1-i])){
            return false;
        }
    }
    return true;
}

bool isValidRetType(string ret_type){
    return checkTypeValidity(cur_func_ret_type,ret_type);
}

void update_while_flag(int val){
    while_flag += val;
}
void update_switch_flag(int val){
    switch_flag += val;
}

void is_valid_break(int lineno){
    if(!while_flag && !switch_flag){
        errorUnexpectedBreak(lineno);
        exit(0);
    }
}
void is_valid_continue(int lineno){
    if(!while_flag){
        errorUnexpectedContinue(lineno);
        exit(0);
    }
}