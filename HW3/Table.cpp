#include "Table.h"

void Table::InsertNewRecord(string name, string type, int offset) {
    SymbolsTableRecord new_record = SymbolsTableRecord(name,type,offset);
    scope_table.push_back(new_record);
}

bool Table::Find(string name) {
    return !GetType(name).empty();
}

string Table::GetType(string name) {
    for(SymbolsTableRecord record:scope_table){
        if(name == record.name){
            return record.type;
        }
    }
    return string();
}
