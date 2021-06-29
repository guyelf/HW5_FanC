#include "SemanticalStructs.h"

int while_count;
int switch_count;
vector<pair<int,BranchLabelIndex>> last_if_next_list;
ScopesStack scope_stack;

void openGlobalScope(SymbolsTable& symbols_table) {
    CodeGeneration::initial_emits();
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
//    endScope();
    symbols_table.PopTable();
    CodeGeneration::printBuffs();
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
    vector<string> arg_types;
    vector<string> arg_names;

    for(auto pair:arguments){
        arg_types.push_back(pair.first);
        arg_names.push_back(pair.second);
    }
    symbols_table.PushNewFunction(func_name, makeFunctionType(ret_type, arg_types), arguments);
    auto allocations = CodeGeneration::define_function(ret_type,func_name,arg_names);
    auto arg_names_map = allocations.first;
    auto stack_base_ptr = allocations.second;
    auto id_reg_map = unordered_map<string,int>();
    scope_stack.OpenScope(ret_type, stack_base_ptr,arg_names_map,id_reg_map);
}

void closeFunctionScope() {
    CodeGeneration::return_function(scope_stack.scopes.back().func_ret_type);
}

void openScope(SymbolsTable& symbols_table, int is_while, int is_switch, int is_if, int is_else, string while_label) {
//    EMIT("OPENSCOPEEEE::" + to_string(is_while) + to_string(is_switch)+ to_string(is_if)+ to_string(is_else) + "\n");
    while_count += is_while;
    switch_count += is_switch;
    auto last_scope = scope_stack.scopes.back();
    scope_stack.OpenScope(last_scope.func_ret_type,last_scope.stack_base_ptr,last_scope.arg_names_map,
                          last_scope.id_reg_map, is_while,is_switch, is_if,is_else, while_label);
    symbols_table.PushNewTable();
}

void closeScope(SymbolsTable& symbols_table, int is_while, int is_switch) {
    while_count += is_while;
    switch_count += is_switch;
    endScope();
    symbols_table.PopTable();
    scope_stack.CloseScope();
}

int callFunction(SymbolsTable& symbols_table, string func_name, vector<pair<string,int>> arguments, int lineno) {
    if(symbols_table.Contains(func_name) != 1 || symbols_table.GetType(func_name).find("->") == string::npos){
        errorUndefFunc(lineno, func_name);
        exit(0);
    }
    auto function_type = symbols_table.GetFunctionType(func_name);
    string ret_type = function_type.first;
    vector<string> expected_arguments_type = function_type.second;

    vector<string> arguments_type;
    for(int i=arguments.size()-1;i>=0;i--){
        arguments_type.push_back(arguments[i].first);
    }
    if(!isValidArgsFunctionCall(arguments_type, expected_arguments_type)){
        errorPrototypeMismatch(lineno, func_name, expected_arguments_type);
        exit(0);
    }
    int ret_reg = CodeGeneration::get_new_reg();
    CodeGeneration::call_function(ret_type, func_name, arguments, ret_reg);
    return ret_reg;
}

void returnFunction(string ret_type, int reg) {
    CodeGeneration::return_function(ret_type, reg);
}
void closeFunction(){
    CodeGeneration::close_function();
}

void gen_local_var_to_default(SymbolsTable symbols_table, string id) {
    int base_ptr = scope_stack.scopes.back().stack_base_ptr;
    int reg_ptr = CodeGeneration::get_element_ptr(base_ptr, symbols_table.GetOffset(id));
    CodeGeneration::store_default_val(reg_ptr);
    scope_stack.scopes.back().id_reg_map[id] = reg_ptr;
}

void gen_local_var_to_reg(SymbolsTable symbols_table, string id, int r_reg) {
    int base_ptr = scope_stack.scopes.back().stack_base_ptr;
    int reg_ptr = CodeGeneration::get_element_ptr(base_ptr, symbols_table.GetOffset(id));
    CodeGeneration::assign(reg_ptr, r_reg);
    scope_stack.scopes.back().id_reg_map[id] = reg_ptr;
}


void set_local_var_to_reg(SymbolsTable symbols_table, string id, int r_reg) {
    int offset = symbols_table.GetOffset(id);
    int reg_ptr;
    if (offset < 0){
        reg_ptr = scope_stack.scopes.back().arg_names_map[id];
    }
    else {
        reg_ptr = scope_stack.scopes.back().id_reg_map[id];
    }
    CodeGeneration::assign(reg_ptr, r_reg);
}


int get_reg_from_id(SymbolsTable symbols_table, string id) {
    int ret_reg = CodeGeneration::get_new_reg();
    int offset = symbols_table.GetOffset(id);
    int id_ptr;
    if(offset < 0){ //function argument
        id_ptr = scope_stack.scopes.back().arg_names_map[id];
    }
    else {
        id_ptr = scope_stack.scopes.back().id_reg_map[id];
    }
    CodeGeneration::load_reg(id_ptr, ret_reg);
    return ret_reg;
}

int set_value_to_new_reg(string string_val) {
    int val = stoi(string_val);
    return CodeGeneration::gen_new_reg_with_constant_val(val);
}

int set_string_val_to_new_reg(string string_val){
    string_val = string_val.substr(1,string_val.length()-2);
    return CodeGeneration::gen_new_reg_with_constant_string(string_val);
}

int genBinop(int reg1, string op, int reg2, string reg1_type,string reg2_type) {
    int ret_reg = CodeGeneration::get_new_reg();
    string llvm_op = CodeGeneration::convertBinop(op);
    if (llvm_op == "div") {
    CodeGeneration::division(ret_reg, reg1, reg2);
    }
    else {
        if(reg1_type == "BYTE" && reg2_type == "BYTE"){
            CodeGeneration::gen_byte_binop(ret_reg,llvm_op, reg1, reg2);
        }
        else {
            CodeGeneration::gen_binop(ret_reg, llvm_op, reg1, reg2);
        }
    }
    return ret_reg;
}

int genRelop(int reg1, string op, int reg2) {
    int ret_reg = CodeGeneration::get_new_reg();
    string llvm_op = CodeGeneration::convertRelop(op);
    CodeGeneration::gen_relop(ret_reg,llvm_op, reg1, reg2);
    return ret_reg;
}

int genNotExp(int reg1) {
    int ret_reg = CodeGeneration::get_new_reg();
    auto empty_list = vector<pair<int,BranchLabelIndex>>();
    CodeGeneration::finish_logical_block(reg1,"not",empty_list, ret_reg);
    return ret_reg;
}

void checkLogicalExp(int reg1, string op, vector<pair<int, BranchLabelIndex>> &sc_list) {
    CodeGeneration::check_logical_block(reg1,op,sc_list);
}

int finishLogicalExp(int reg1, string op, vector<pair<int, BranchLabelIndex>> &sc_list) {
    int ret_reg = CodeGeneration::get_new_reg();
    CodeGeneration::finish_logical_block(reg1, op, sc_list, ret_reg);
    return ret_reg;
}

void openIf(int cond_reg) {
    for(int i=scope_stack.scopes.size()-1; i>=0; i--){
        if(scope_stack.scopes[i].is_if){
            CodeGeneration::open_if(cond_reg,scope_stack.scopes[i].while_list);
            break;
        }
    }
}

void closeIf(){
    for(int i=scope_stack.scopes.size()-1; i>=0; i--){
        if(scope_stack.scopes[i].is_if){
            CodeGeneration::close_case(scope_stack.scopes[i].while_next_list);
            last_if_next_list = scope_stack.scopes[i].while_next_list;
            CodeGeneration::close_block(scope_stack.scopes[i].while_list);
            break;
        }
    }
}

void openElse() {
    for(int i=scope_stack.scopes.size()-1; i>=0; i--){
        if(scope_stack.scopes[i].is_else){
            scope_stack.scopes[i].while_next_list = CodeBuffer::merge(scope_stack.scopes[i].while_next_list, last_if_next_list);
            break;
        }
    }
}

void closeBlock() {
    CodeGeneration::close_block(last_if_next_list);
}
void closeElseBlock(){
    for(int i=scope_stack.scopes.size()-1; i>=0; i--){
        if(scope_stack.scopes[i].is_else){
            CodeGeneration::close_block(scope_stack.scopes[i].while_next_list);
            break;
        }
    }
}

string genWhileLabel(){
    return CodeGeneration::gen_new_label();
}

void openWhile(int cond_reg) {
    CodeGeneration::open_while(cond_reg, scope_stack.scopes.back().while_list);
}

void closeWhile() {
    for(int i=scope_stack.scopes.size()-1; i>=0; i--){
        if(scope_stack.scopes[i].is_while){
            string while_label = scope_stack.scopes[i].while_label;
            bool is_break = scope_stack.scopes[i].is_break;
            CodeGeneration::close_while(scope_stack.scopes[i].while_list,scope_stack.scopes[i].while_next_list, while_label, is_break);
            break;
        }
    }
}

void continueWhile() {
    for(int i=scope_stack.scopes.size()-1; i>=0; i--){
        if(scope_stack.scopes[i].is_while){
            string while_label = scope_stack.scopes[i].while_label;
            CodeGeneration::jmp_to_constant_addr(while_label);
            break;
        }
    }
}

void breakBlock() {
    for(int i=scope_stack.scopes.size()-1; i>=0; i--){
        if(scope_stack.scopes[i].is_while || scope_stack.scopes[i].is_switch){
//            EMIT("IS SWITCH"+ to_string(scope_stack.scopes[i].is_switch));
//            EMIT("IS WHILE"+ to_string(scope_stack.scopes[i].is_while));
            scope_stack.scopes[i].is_break = true;
            CodeGeneration::close_case(scope_stack.scopes[i].while_next_list);
            break;
        }
    }
}

string gen_label() {
    return CodeGeneration::gen_new_label();
}

void openSwitch() {
    for(int i=scope_stack.scopes.size()-1; i>=0; i--){
        if(scope_stack.scopes[i].is_switch){
            CodeGeneration::close_case(scope_stack.scopes[i].while_list);
            break;
        }
    }
}

void closeCase() {
    for(int i=scope_stack.scopes.size()-1; i>=0; i--){
        if(scope_stack.scopes[i].is_switch){
            CodeGeneration::close_case(scope_stack.scopes[i].while_next_list);
            break;
        }
    }
}

void switchBlock(int exp_reg, string label, vector<pair<string,string>>& case_list) {
    if(label == ""){
        label = gen_label();
    }
    for(int i=scope_stack.scopes.size()-1; i>=0; i--){
        if(scope_stack.scopes[i].is_switch){
            CodeGeneration::close_block(scope_stack.scopes[i].while_list);
            CodeGeneration::switchBlock(exp_reg, label, case_list);
            CodeGeneration::close_block(scope_stack.scopes[i].while_next_list);
            break;
        }
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
    return checkTypeValidity(scope_stack.scopes.back().func_ret_type,ret_type);
}

void update_while_flag(int val){
    while_count += val;
}

void update_switch_flag(int val){
    switch_count += val;
}

void is_valid_break(int lineno){
    if(!while_count && !switch_count){
        errorUnexpectedBreak(lineno);
        exit(0);
    }
}
void is_valid_continue(int lineno){
    if(!while_count){
        errorUnexpectedContinue(lineno);
        exit(0);
    }
}