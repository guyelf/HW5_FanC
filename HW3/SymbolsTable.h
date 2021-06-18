#ifndef COMPILATION_HW3_SYMBOLSTABLE_H
#define COMPILATION_HW3_SYMBOLSTABLE_H

#include <vector>
#include <string>
#include "hw3_output.hpp"
#include "Table.h"

using namespace std;
using namespace output;

vector<string> split(string name,char delimiter);

class SymbolsTable {
public:
    vector<Table> tablesStack;
    vector<int> offsetsStack;
    SymbolsTable();
    void PushNewTable();
    void PushNewRecord(string name, string type, int type_size=1);
    void PushNewFunction(string name, string type, vector<pair<string,string>> arguments);
    void PopTable();
    string GetType(string name);
    pair<string,vector<string>> GetFunctionType(string name);
    int Contains(string name);
    void printST();
};


#endif //COMPILATION_HW3_SYMBOLSTABLE_H
