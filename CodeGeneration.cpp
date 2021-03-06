#include "CodeGeneration.h"

#include <string>
#include <vector>
#include <unordered_map>
#include "bp.hpp"

#define EMIT(line) CodeBuffer::instance().emit(line)
#define EMIT_GLOBAL(line) CodeBuffer::instance().emitGlobal(line)
#define GEN_LABEL() CodeBuffer::instance().genLabel()
#define BPATCH(add_list, label) CodeBuffer::instance().bpatch(add_list, label)

using namespace std;

namespace CodeGeneration {
    int last_reg = 0;
//    string new_label;
    unordered_map<int,int> reg_to_string_len;
    void initial_emits(){
        EMIT("declare i32 @printf(i8*, ...)");
        EMIT("declare void @exit(i32)");
        EMIT_GLOBAL("@.division_by_zero_error = constant [23 x i8] c\"Error division by zero\\00\"");
        EMIT_GLOBAL("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
        EMIT_GLOBAL("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");


        EMIT("define void @printi(i32) {");
        EMIT("%spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0");
        EMIT("call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)");
        EMIT("ret void");
        EMIT("}");

        EMIT("define void @print(i8*) {");
        EMIT("%spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0");
        EMIT("call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)");
        EMIT("ret void");
        EMIT("}");
    }
    string t(int reg){
        return "%t" + to_string(reg);
    }
    int get_new_reg(){
        return ++last_reg;
    }

    void set_reg_with_constant_val(int reg_id, int val){
        EMIT(t(reg_id) + " = add i32 0, " + to_string(val));
    }

    int gen_new_reg_with_constant_val(int val){
        int new_reg = get_new_reg();
        set_reg_with_constant_val(new_reg, val);
        return new_reg;
    }

    int gen_new_reg_with_constant_string(string val){
        int new_reg = get_new_reg(); // ] c\"Error division by zero\\00\""
        EMIT_GLOBAL("@.s"+ to_string(new_reg) + " = constant [" + to_string(val.length()+1) + " x i8] c\"" + val + "\\00\"");
        reg_to_string_len[new_reg] = val.length()+1;
        return new_reg;
    }

    void truncate(int out_reg, int in_reg, int bytes){
        EMIT(t(out_reg) + " = trunc i32 " + t(in_reg) + " to i"+ to_string(bytes));
    }
    void zext(int out_reg, int in_reg, int bytes){
        EMIT(t(out_reg) + " = zext i" + to_string(bytes) + " " + t(in_reg) + " to i32");
    }
    void gen_binop(int l_reg, string op, int r_reg1, int r_reg2){
        EMIT(t(l_reg) + " = " + op + " i32 " + t(r_reg1) + ", " + t(r_reg2));
    }

    void gen_byte_binop(int l_reg, string op, int r_reg1, int r_reg2){
        int temp = get_new_reg();
        int temp2 = get_new_reg();
        gen_binop(temp, op, r_reg1, r_reg2);
        truncate(temp2, temp,8);//truncates from 32 to 8 bytes
        zext(l_reg,temp2,8);
    }

    string convertType(const string& type) {
        return type == "VOID"? "void" : "i32";
    }
    string convertBinop(const string& op) {
        if(op == "*") {
            return "mul";
        }
        else if (op == "/") {
            return "div";
        }
        else if (op == "+") {
            return "add";
        }
        else if (op == "-") {
            return "sub";
        }
        else{
            return "";
        }
    }
    string convertRelop(const string& op) {
        if(op == "<") {
            return "icmp slt";
        }
        else if (op == "<=") {
            return "icmp sle";
        }
        else if (op == "==") {
            return "icmp eq";
        }
        else if (op == ">=") {
            return "icmp sge";
        }
        else if (op == ">") {
            return "icmp sgt";
        }
        else if (op == "!=") {
            return "icmp ne";
        }
        else{
            return "";
        }
    }

    void gen_relop(int l_reg, string op, int r_reg1, int r_reg2){
        int res_reg = get_new_reg();
        gen_binop(res_reg, op, r_reg1, r_reg2);
        zext(l_reg,res_reg,1);
    }



    string bpatch_new_label(vector<pair<int,BranchLabelIndex>>& list){
        string new_address = GEN_LABEL();
        BPATCH(list, new_address);
        return new_address;
    }

    void compare(int l_reg, string op, int r_reg1, int r_reg2){
        EMIT(t(l_reg) + " = icmp " + op + " i32 " + t(r_reg1) + ", " + t(r_reg2));
    }

    int br_cond(int bool_reg){
        return EMIT("br i1" + t(bool_reg) + ", label @, label @");
    }

    void call_function(const string& ret_val, const string& func_name, vector<pair<string,int>>& args, int ret_reg){
        string s_args;
        if(func_name == "print"){
            int base_reg = args[0].second;
            int length = reg_to_string_len[base_reg];
            s_args = "i8* getelementptr ([" + to_string(length) + " x i8], [" + to_string(length) + " x i8]* @.s" +
                    to_string(base_reg) + ", i32 0, i32 0)";
        }
        else{
            for(auto& arg: args){
                s_args += convertType(arg.first) + " " + t(arg.second) + ", ";
            }
            // remove last ', ' from string
            if (!args.empty()){
                s_args = s_args.substr(0,s_args.size()-2);
            }
        }
        string ret_type = ret_val == "VOID"? "void": "i32 "; //force correct calling types
        string s_ret_reg =  ret_reg == -1 || ret_type == "void" ? "" : t(ret_reg) + " = ";
        EMIT( s_ret_reg + "call " + ret_type + " @" + func_name + " (" + s_args + ")");
    }
    void division(int l_reg, int r_reg1, int r_reg2){
        int reg_res = get_new_reg();
        int reg_zero = get_new_reg();
        set_reg_with_constant_val(reg_zero, 0);
        compare(reg_res,"eq", r_reg2, reg_zero);
        int division_by_zero = br_cond(reg_res);

        string true_label = GEN_LABEL();
        EMIT("call void @print(i8* getelementptr ([23 x i8], [23 x i8]* @.division_by_zero_error, i32 0, i32 0))");
        EMIT("call void @exit(i32 1)");

        int unreachable = EMIT("br label @");
        string false_label = GEN_LABEL();
        gen_binop(l_reg, "sdiv", r_reg1, r_reg2);

        BPATCH(CodeBuffer::makelist({division_by_zero,FIRST}),true_label);
        BPATCH(CodeBuffer::makelist({division_by_zero,SECOND}),false_label);
        BPATCH(CodeBuffer::makelist({unreachable,FIRST}),false_label);
    }

    string check_logical_block_aux(int cur_val, vector<pair<int,BranchLabelIndex>>& list, BranchLabelIndex first, BranchLabelIndex second){
        int reg_bool_var = get_new_reg();
        truncate(reg_bool_var,cur_val,1);
        int branch = br_cond(reg_bool_var);

        list = CodeBuffer::merge(CodeBuffer::makelist({branch, first}), list);
        auto new_list = CodeBuffer::makelist({branch,second});
        return bpatch_new_label(new_list);
    }

    string check_logical_block(int cur_val, const string& logical_op, vector<pair<int,BranchLabelIndex>>& list){
        if(logical_op == "or"){
            return check_logical_block_aux(cur_val,list, FIRST, SECOND);
        }
        else {//if(logical_op == "and") || (logical_op == "not" && while_list.empty(){
            return check_logical_block_aux(cur_val,list, SECOND, FIRST);
        }
    }
    void call_phi(vector<pair<int, BranchLabelIndex>> phi_result_list, const string& true_label, const string& false_label, int res_reg){
        bpatch_new_label(phi_result_list);
        EMIT(t(res_reg) + " = phi i32 [1 , %" + true_label + "], [0, %" + false_label + "]");
    }
    void finish_logical_block(int cur_val, const string& logical_op, vector<pair<int,BranchLabelIndex>>& sc_list, int res_reg){
        string sc_label = check_logical_block(cur_val, logical_op, sc_list);
        int sc_br = EMIT("br label @");
        string second_label = bpatch_new_label(sc_list);
        int second_br = EMIT("br label @");
        auto phi_result_list = CodeBuffer::makelist({sc_br, FIRST});
        phi_result_list = CodeBuffer::merge(CodeBuffer::makelist({second_br, FIRST}), phi_result_list);
        if(logical_op == "and") {
            call_phi(phi_result_list, sc_label,second_label, res_reg);
        }
        else{ //if(logical_op == "and") || (logical_op == "not" && while_list.empty(){
            call_phi(phi_result_list,  second_label, sc_label, res_reg);

        }
    }

    //Control Flows: While, IF and Else Blocks:

    void jmp_to_constant_addr(string addr){ //used for while
        EMIT("br label %" + addr);
    }


    //gets a nextlist for the given block and assigns it with the new label to jump to after the block is finished
    string close_block(vector<pair<int,BranchLabelIndex>>& nextlist){
        int br_cond_addr = EMIT("br label @");
        auto exit_block_addr = CodeBuffer::makelist({br_cond_addr,FIRST});
        nextlist = CodeBuffer::merge(exit_block_addr,nextlist);
        return bpatch_new_label(nextlist);
    }

    //if statement
    void open_if(int reg_cond,vector<pair<int,BranchLabelIndex>>& false_list){
        int reg_bool_var = get_new_reg();
        truncate(reg_bool_var,reg_cond,1);
        int branch = br_cond(reg_bool_var);

        auto true_list = CodeBuffer::makelist({branch,FIRST});
        bpatch_new_label(true_list);

        auto false_jmp= CodeBuffer::makelist({branch,SECOND});
        false_list = CodeBuffer::merge(false_jmp,false_list);
    }

    //bpatchs the false_list from the IFs to here, merges the next_list of the else with the entire next_list from the block
    void open_else(vector<pair<int,BranchLabelIndex>>& false_list, vector<pair<int,BranchLabelIndex>>& next_list){
        int br_cond_addr = EMIT("br label @");
        auto exit_block_addr = CodeBuffer::makelist({br_cond_addr,FIRST});
        next_list = CodeBuffer::merge(exit_block_addr,next_list);

        bpatch_new_label(false_list);
    }

    //while statement
    void open_while(int reg_cond,vector<pair<int,BranchLabelIndex>>& loop_end){
        int reg_bool_var = get_new_reg();
        truncate(reg_bool_var,reg_cond,1);
        int branch = br_cond(reg_bool_var);

        auto loop_body = CodeBuffer::makelist({branch, FIRST});
        bpatch_new_label(loop_body);

        auto loop_end_label = CodeBuffer::makelist({branch,SECOND});
        loop_end = CodeBuffer::merge(loop_end_label,loop_end);
    }
    string gen_new_label(){
        int jmp_to_label = EMIT("br label @");
        string new_label = GEN_LABEL();
        BPATCH(CodeBuffer::makelist({jmp_to_label, FIRST}), new_label);
        return new_label;
    }

    string close_while(vector<pair<int,BranchLabelIndex>>& loop_end, vector<pair<int,BranchLabelIndex>>& next_list, string while_label){
        jmp_to_constant_addr(while_label); //br label %loop_head
        string next_label = GEN_LABEL();

        BPATCH(next_list,next_label);
        BPATCH(loop_end, next_label);
        return next_label;
    }

    //has the anchor for each case statment
//    //creates a label that can then be used to jmp to the specific case
//    void open_case(vector<string>& quad_list, vector<int>& value_list, int case_val){
//        auto case_label = EMIT("br label @");
//        auto case_quad = CodeBuffer::makelist({case_label, FIRST});
//        auto case_addr = bpatch_new_label(case_quad);
//        quad_list.push_back(case_addr);
//        value_list.push_back(case_val);
//    }

    //after the case statement executes, jmp to the next_list
    void close_case(vector<pair<int,BranchLabelIndex>>& next_list){
        auto finish_case_statm = EMIT("br label @");
        auto case_next_list = CodeBuffer::makelist({finish_case_statm, FIRST});
        next_list = CodeBuffer::merge(case_next_list, next_list);
    }

    int compare_vals(int val1, int val2){
        int reg_val1 = get_new_reg();
        set_reg_with_constant_val(reg_val1,val1);
        int reg_val2 = get_new_reg();
        set_reg_with_constant_val(reg_val2,val2);
        int reg_res = get_new_reg();
        compare(reg_res,"eq",reg_val1,reg_val2);
        return reg_res;
    }

    void switchBlock(int exp_reg, string label, vector<pair<string,string>>& case_list){
        string s_case_list;
        for(auto c:case_list) {
            s_case_list += "i32 " + c.first +", label %" + c.second + "\n";
        }
        s_case_list = s_case_list.substr(0,s_case_list.size()-1);
        EMIT("switch i32 " + t(exp_reg) + ", label %" + label + " [ " + s_case_list + " ]");
    }
//    vector<pair<int,BranchLabelIndex>> open_switch(){
//        auto init_label = EMIT("br label @");
//        return CodeBuffer::makelist({init_label, FIRST});
//    }
//
//    void close_switch(vector<pair<int,BranchLabelIndex>>& init_list, vector<string>& quad_list, vector<int>& value_list, int statement_val, vector<pair<int,BranchLabelIndex>>& next_list){
//        bpatch_new_label(init_list);
//        auto next_if = vector<pair<int,BranchLabelIndex>>();
//        while(!quad_list.empty()){
//            if(!next_if.empty()){
//                bpatch_new_label(next_if);
//            }
//            auto value = value_list.back();
//            value_list.pop_back();
//            auto quad = quad_list.back();
//            quad_list.pop_back();
//
//            int reg_res = compare_vals(statement_val, value);
//            int reg_bool_var = get_new_reg();
//            truncate(reg_bool_var,reg_res,1);
//            int comp_label = br_cond(reg_bool_var);
//
//            auto true_label = CodeBuffer::makelist({comp_label, FIRST});
//            bpatch_new_label(true_label);
//            jmp_to_constant_addr(quad);
//
//            next_if = CodeBuffer::makelist({comp_label,SECOND});
//        }
//        next_list = CodeBuffer::merge(next_if, next_list);
//        close_block(next_list);
//    }

    void store_reg(int stack_ptr, int reg_ptr){
        EMIT("store i32 %" + to_string(stack_ptr) + ", i32* " + t(reg_ptr));
    }

    void assign(int l_reg, int r_reg){
        EMIT("store i32 " + t(r_reg) + ", i32* " + t(l_reg));
    }

    void store_default_val(int ptr){
        EMIT("store i32 0, i32* " + t(ptr));
    }

    void load_reg(int ptr, int reg_val) {
        EMIT(t(reg_val) + "= load i32, i32* " + t(ptr));
    }

    void printBuffs() {
        CodeBuffer::instance().printGlobalBuffer();
        CodeBuffer::instance().printCodeBuffer();
    }

    pair<unordered_map<string, int>, int> define_function(string ret_type, string func_name, vector<string>& arg_names) {
        string s_args;
        for(auto arg_name:arg_names){
            s_args += "i32, ";
        }
        s_args = s_args.substr(0,s_args.size()-2);
        EMIT("define " + convertType(ret_type) + " @" + func_name + "(" + s_args + ") {");
        unordered_map<string, int> arg_name_to_ptr;
        for(int i = 0; i < arg_names.size(); i++){
            int reg_ptr = get_new_reg();
            _allocate(reg_ptr,0);
            store_reg(i, reg_ptr);
            arg_name_to_ptr[arg_names[i]] = reg_ptr;
        }
        int stack_base_ptr = get_new_reg();
        _allocate(stack_base_ptr,50);
        return {arg_name_to_ptr, stack_base_ptr};
    }

    void return_function(const string& cur_func_ret_type, int reg) {
        string s_reg = "";
        string ret_type = cur_func_ret_type == "VOID"? "void": "i32 0";
        if(reg != -1){
            s_reg = t(reg);
            ret_type = ret_type.substr(0,ret_type.length() - 2);
        }
        EMIT("ret " + ret_type + " " + s_reg);
    }
    void close_function(){
        EMIT("}");
    }
    int get_element_ptr(int base_ptr, int offset){
        int ret_reg = get_new_reg();
        EMIT(t(ret_reg) + " = getelementptr i32, i32* " + t(base_ptr) + ", i32 " + to_string(offset));
        return ret_reg;
    }

    void _allocate(int reg_ptr, int _size) {
        string s_size = _size ? ", i32 " + to_string(_size) : "";
        EMIT(t(reg_ptr) + " = alloca i32" + s_size);
    }
};