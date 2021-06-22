#ifndef HW5_FANC_CODEGENERATION_H
#define HW5_FANC_CODEGENERATION_H

#include <string>
#include <vector>
#include "bp.hpp"

#define EMIT(line) CodeBuffer::instance().emit(line)
#define EMIT_GLOBAL(line) CodeBuffer::instance().emitGlobal(line)
#define GEN_LABEL() CodeBuffer::instance().genLabel()
#define BPATCH(add_list, label) CodeBuffer::instance().bpatch(add_list, label)

using namespace std;

namespace CodeGeneration {
    int last_reg = 0;
    void initial_emits(){
        EMIT("declare i32 @printf(i8*, ...)");
        EMIT("declare void @exit(i32)");
        EMIT_GLOBAL("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
        EMIT_GLOBAL("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
        EMIT_GLOBAL("@.division_by_zero_error = constant [23 x i8] c\"Error division by zero\\0\"");

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
    void truncate(int out_reg, int in_reg, int bytes){
        EMIT(t(out_reg) + " = trunc i32 " + t(in_reg) + " to i"+ to_string(bytes));
    }
    void zext(int out_reg, int in_reg, int bytes){
        EMIT(t(out_reg) + " = zext i" + to_string(bytes) + " " + t(in_reg) + " to i32");
    }
    void gen_binop(int l_reg, const char *op, int r_reg1, int r_reg2){
        EMIT(t(l_reg) + " = " + op + " i32 " + t(r_reg1) + ", " + t(r_reg2));
    }
    void gen_byte_binop(int l_reg, const char *op, int r_reg1, int r_reg2){
        int temp = get_new_reg();
        gen_binop(temp, op, r_reg1, r_reg2);
        truncate(l_reg, temp,8); //truncates from 32 to 8 bytes
    }
    string bptach_new_label(vector<pair<int,BranchLabelIndex>>& list){
        string new_address = GEN_LABEL();
        BPATCH(list, new_address);
        return new_address;
    }
    void compare(int l_reg, const char *op, int r_reg1, int r_reg2){
        EMIT(t(l_reg) + " = icmp " + op + " i32 " + t(r_reg1) + ", " + t(r_reg2));
    }
    int br_cond(int bool_reg){
        return EMIT("br i1" + t(bool_reg) + ", label @, label @");
    }
    void call_function(const string& ret_val, const string& func_name, const vector<pair<string,int>>& args){
        string s_args;
        for(const auto& arg: args){
            s_args += arg.first + " " + t(arg.second) + ", ";
        }
        // remove last ', ' from string
        s_args = s_args.substr(0,s_args.size()-2);
        EMIT("call " + ret_val + " @" + func_name + " (" + s_args + ")");
    }
    void division(int l_reg, int r_reg1, int r_reg2){
        int reg_res = get_new_reg();
        int reg_zero = get_new_reg();
        set_reg_with_constant_val(reg_zero, 0);
        compare(reg_res,"eq", r_reg2, reg_zero);
        int division_by_zero = br_cond(reg_res);

        string true_label = GEN_LABEL();
        vector<pair<string,int>> args_for_printf {{"i8*",get_new_reg()}};
        EMIT(t(args_for_printf[0].second) + " = getelementptr [23 x i8], [23 x i8]* @.division_by_zero_error, i32 0, i32 0");
        call_function("void", "printf", args_for_printf);
        vector<pair<string,int>> args_for_exit {{"i32",get_new_reg()}};
        set_reg_with_constant_val(args_for_exit[1].second,1);
        call_function("void", "exit", args_for_printf);

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
        return bptach_new_label(new_list);
    }

    string check_logical_block(int cur_val, string logical_op, vector<pair<int,BranchLabelIndex>>& list){
        if(logical_op == "or"){
            return check_logical_block_aux(cur_val,list, FIRST, SECOND);
        }
        else if (logical_op == "and"){
            return check_logical_block_aux(cur_val,list, SECOND, FIRST);
        }
    }

    void call_phi(vector<pair<int, BranchLabelIndex>> phi_result_list, string true_label, string false_label, int res_reg){
        bptach_new_label(phi_result_list);
        EMIT(t(res_reg) + " = phi i32 [1 , %" + true_label + "], [0, %" + false_label + "]");
    }

    void finish_and_segment_aux(int cur_val, vector<pair<int,BranchLabelIndex>>& false_list, int res_reg){
        string true_label = check_logical_block(cur_val, false_list);
        int true_br = EMIT("br label @");
        string false_label = bptach_new_label(false_list);
        int false_br = EMIT("br label @");
        auto phi_result_list = CodeBuffer::makelist({true_br,FIRST});
        phi_result_list = CodeBuffer::merge(CodeBuffer::makelist({false_br,SECOND}), phi_result_list);
        call_phi(phi_result_list, true_label, false_label, res_reg);
    }

    void finish_and_segment()



};


#endif //HW5_FANC_CODEGENERATION_H
