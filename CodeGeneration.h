#ifndef HW5_FANC_CODEGENERATION_H
#define HW5_FANC_CODEGENERATION_H

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
    void initial_emits();
    string t(int reg);
    int get_new_reg();
    void set_reg_with_constant_val(int reg_id, int val);
    int gen_new_reg_with_constant_val(int val);
    int gen_new_reg_with_constant_string(string val);
    void truncate(int out_reg, int in_reg, int bytes);
    void zext(int out_reg, int in_reg, int bytes);
    void gen_binop(int l_reg, string op, int r_reg1, int r_reg2);
    string convertType(const string& type);
    string convertBinop(const string& op);
    string convertRelop(const string& op);
    void gen_relop(int l_reg, string op, int r_reg1, int r_reg2);
    void gen_byte_binop(int l_reg, string op, int r_reg1, int r_reg2);
    string bpatch_new_label(vector<pair<int,BranchLabelIndex>>& list);
    void compare(int l_reg, string op, int r_reg1, int r_reg2);
    int br_cond(int bool_reg);
    void call_function(const string& ret_val, const string& func_name, vector<pair<string,int>>& args, int ret_reg = -1);
    void division(int l_reg, int r_reg1, int r_reg2);
    string check_logical_block_aux(int cur_val, vector<pair<int,BranchLabelIndex>>& list, BranchLabelIndex first, BranchLabelIndex second);
    string check_logical_block(int cur_val, const string& logical_op, vector<pair<int,BranchLabelIndex>>& list);
    void call_phi(vector<pair<int, BranchLabelIndex>> phi_result_list, const string& true_label, const string& false_label, int res_reg);
    void finish_logical_block(int cur_val, const string& logical_op, vector<pair<int,BranchLabelIndex>>& sc_list, int res_reg);
    void jmp_to_constant_addr(string addr);
    string close_block(vector<pair<int,BranchLabelIndex>>& nextlist);
    void open_if(int reg_cond,vector<pair<int,BranchLabelIndex>>& false_list);
    void open_else(vector<pair<int,BranchLabelIndex>>& false_list, vector<pair<int,BranchLabelIndex>>& next_list);
    void open_while(int reg_cond,vector<pair<int,BranchLabelIndex>>& loop_end);
    string gen_new_label();
    string close_while(vector<pair<int,BranchLabelIndex>>& loop_end, vector<pair<int,BranchLabelIndex>>& next_list, string while_label, bool is_break);
    void open_case(vector<string>& quad_list, vector<int>& value_list, int case_val);
    void close_case(vector<pair<int,BranchLabelIndex>>& next_list);
    int compare_vals(int val1, int val2);
    void switchBlock(int exp_reg, string label, vector<pair<string,string>>& case_list);
    void store_reg(int stack_ptr, int reg_ptr);
    void assign(int l_reg, int r_reg);
    void store_default_val(int ptr);
    void load_reg(int ptr, int reg_val);
    void printBuffs();
    pair<unordered_map<string, int>, int> define_function(string ret_type, string func_name, vector<string>& arg_names);
    void return_function(const string& cur_func_ret_type, int reg = -1);
    void close_function();
    int get_element_ptr(int base_ptr, int offset);
    void _allocate(int reg_ptr, int _size);
};


#endif //HW5_FANC_CODEGENERATION_H
