#include "SymbolsTable.h"

SymbolsTable::SymbolsTable() {
    Table new_table = Table();
    tablesStack.push_back(new_table);
    offsetsStack.push_back(0);
}

void SymbolsTable::PushNewTable() {
    Table new_table = Table();
    tablesStack.push_back(new_table);
    offsetsStack.push_back(offsetsStack.back());
}

void SymbolsTable::PushNewRecord(string name, string type, int type_size) {
    tablesStack.back().InsertNewRecord(name,type,offsetsStack.back());
    offsetsStack.back()+= type_size;
}

void SymbolsTable::PushNewFunction(string name, string type, vector<pair<string,string>> arguments) {
    tablesStack.back().InsertNewRecord(name,type,0);
    PushNewTable();
    int offset = offsetsStack.back()-1;
    for(pair<string,string> pair:arguments){
        tablesStack.back().InsertNewRecord(pair.second, pair.first, offset);
        offset--;
    }
}

void SymbolsTable::PopTable() {
//    for(SymbolsTableRecord record : tablesStack.back().scope_table){
//        printID(record.name, record.offset, record.type);
//    }
    tablesStack.pop_back();
    offsetsStack.pop_back();
}


int SymbolsTable::Contains(string name) {
    bool cur_found;
    int prev_found = 0;
    for (Table table:tablesStack) {
        cur_found = table.Find(name);
        if(cur_found && prev_found){
            return -1;  // errorDef
        }
        else if(cur_found && !prev_found){
            prev_found = 1;  // no error - found only once
        }
    }
    return prev_found;  // 0 - errorUndef; 
}

string SymbolsTable::GetType(string name) {
    string type;
    for (Table table:tablesStack) {
        type = table.GetType(name);
        if (!type.empty()){
            return type;
        }
    }
    return string();
}

int SymbolsTable::GetOffset(string name) {
    for (Table table:tablesStack) {
        if(table.Find(name)){
            return table.GetOffset(name);
        }
    }
    return -404; // unreachable
}


pair<string,vector<string>> SymbolsTable::GetFunctionType(string name) {
    pair<string,vector<string>> res;
    string type = GetType(name);
    if(type.empty()){
        return res;
    }
    size_t arrow_pos = type.find("->");
    res.first = type.substr(arrow_pos + 2);
    vector<string> arg_types = split(type.substr(1,arrow_pos-2),',');
    for(string type:arg_types){
        if(!type.empty()) {
            res.second.push_back(type);
        }
    }
    return res;
}

void SymbolsTable::printST() {
    int i = 1;
    for(Table table:tablesStack){
        cout<<"table number "<< i << endl;
        i++;
        for(SymbolsTableRecord record:table.scope_table){
            printID(record.name,record.offset,record.type);
        }
    }
}

vector<string> split(string name,char delimiter){
    vector<string> res;
    size_t start = 0;
    size_t end = name.find(delimiter);
    while (end != std::string::npos)
    {
        res.push_back(name.substr(start, end - start));
        start = end + 1;
        end = name.find(delimiter, start);
    }
    res.push_back(name.substr(start, end));
    return res;
}