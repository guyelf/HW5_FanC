#ifndef COMPILATION_HW3_TABLE_H
#define COMPILATION_HW3_TABLE_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct SymbolsTableRecord {
    string name;
    string type;
    int offset;
    SymbolsTableRecord(string _name, string _type, int _offset){
        name = _name;
        type = _type;
        offset = _offset;
    }
};

class Table {
public:
    vector<SymbolsTableRecord> scope_table;
    void InsertNewRecord(string name, string type, int offset);
    bool Find(string name);
    string GetType(string name);
    int GetOffset(string name);
};


#endif //COMPILATION_HW3_TABLE_H
