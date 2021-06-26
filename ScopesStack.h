#ifndef HW5_FANC_SCOPESSTACK_H
#define HW5_FANC_SCOPESSTACK_H

#include <unordered_map>
#include <vector>
#include "bp.hpp"
using namespace std;

struct Scope{
    bool is_if;
    bool is_while;
    vector<pair<int,BranchLabelIndex>> while_list;
    vector<pair<int,BranchLabelIndex>> while_next_list;
    string func_ret_type;
    int stack_base_ptr;
    unordered_map<string,int> arg_names_map;
    unordered_map<string,int> id_reg_map;
    Scope(string func_ret_type, int stack_base_ptr, unordered_map<string,int>& arg_names_map, unordered_map<string,int>& id_reg_map, bool is_while, bool is_if):
    func_ret_type(func_ret_type), stack_base_ptr(stack_base_ptr),arg_names_map(arg_names_map), id_reg_map(id_reg_map), is_while(is_while), is_if(is_if) {};
};

class ScopesStack {
public:
    vector<Scope> scopes;
    void OpenScope(string ret_type, int stack_bp, unordered_map<string,int> arg_names_map,
                   unordered_map<string,int> id_reg_map, bool is_while = 0, bool is_if = 0){
        Scope scope = Scope(ret_type, stack_bp, arg_names_map, id_reg_map, is_while, is_if);
        scopes.push_back(scope);
    }

    void setList(vector<pair<int,BranchLabelIndex>> &list){
        scopes.back().while_list = list;
    }
    void setNextList(vector<pair<int,BranchLabelIndex>> &next_list){
        scopes.back().while_list = next_list;
    }

    void CloseScope() {
        scopes.pop_back();
    }
};


#endif //HW5_FANC_SCOPESSTACK_H
