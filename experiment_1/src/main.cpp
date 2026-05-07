/**
 * @file main.cpp
 * @brief Multi-Table Management CLI using alib5 ds
 */
#include <iostream>
#include <string>
#include <limits>
#include <hust_ds/sq_list.h>

using namespace hust_ds::sq_list;

struct TableInstance{
    Vector<char> name;
    List list = nullptr;

    // 默认构造，Vector内部会自行初始化
    TableInstance() = default;
    // 为了方便 Vector 扩容时的搬运
    TableInstance(TableInstance&&) = default;
};

class App{
private:
    // 管理层也使用自家的 Vector
    Vector<TableInstance> tables;
    int active_idx = -1;

    void pause(){
        std::cout << "\nPress Enter...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }

    List& curr(){
        return tables._at(active_idx)->list; 
    }

    // 辅助：将 std::string 转为 Vector<char>
    void set_name(Vector<char>& vec, const std::string& str){
        vec.clear();
        for(char c : str) vec.push_back(c);
        vec.push_back('\0'); // 方便后续逻辑或调试
    }

    // 辅助：打印 Vector<char>
    void print_name(const Vector<char>& vec){
        for(const char& c : vec) if(c != '\0') std::cout << c;
    }

public:
    void run(){
        std::string cmd;
        while(true){
            system("cls || clear");
            std::cout << "=== [ Multi-Table Manager (alib5 powered) ] ===\n";
            if(tables.empty()) std::cout << " (No tables created)\n";
            else {
                for(int i = 0; i < (int)tables.size(); ++i){
                    std::cout << (i == active_idx ? " -> " : "    ") << "[" << i << "] ";
                    print_name(tables._at(i)->name);
                    std::cout << "\n";
                }
            }
            std::cout << "-----------------------------------------------\n"
                      << " M1. Create Table    M2. Switch Table   M3. Remove Table\n"
                      << "-----------------------------------------------\n"
                      << " 1. Init      2. Destroy   3. Clear     4. IsEmpty\n"
                      << " 5. Length    6. GetElem   7. Locate    8. Prior\n"
                      << " 9. Next     10. Insert   11. Delete   12. Traverse\n"
                      << "-----------------------------------------------\n"
                      << " 13. MaxSubSum  14. SubSumK  15. Sort  16. Save  17. Load\n"
                      << " 0. Exit\n"
                      << "-----------------------------------------------\n"
                      << "Choice: ";
            
            if(!(std::cin >> cmd)) break;
            if(cmd == "0") break;
            if(cmd == "M1") { create_table(); continue; }
            if(cmd == "M2") { switch_table(); continue; }
            if(cmd == "M3") { remove_table(); continue; }

            try { handle_op(std::stoi(cmd)); } 
            catch(...) { std::cout << "Invalid Command!\n"; pause(); }
        }
    }

    void create_table(){
        std::cout << "Enter table name: "; std::string n; std::cin >> n;
        TableInstance ti;
        set_name(ti.name, n);
        ti.list = nullptr; // 初始未 InitList
        tables.push_back(std::move(ti));
        if(active_idx == -1) active_idx = 0;
        std::cout << "Table entry created.\n"; pause();
    }

    void switch_table(){
        std::cout << "Switch to ID: "; int id; std::cin >> id;
        if(id >= 0 && id < (int)tables.size()) active_idx = id;
        else std::cout << "Out of range!\n"; pause();
    }

    void remove_table(){
        if(active_idx == -1) return;
        if(curr()) DestroyList(curr());
        tables._erase(active_idx);
        active_idx = tables.empty() ? -1 : 0;
        std::cout << "Table removed.\n"; pause();
    }

    void handle_op(int op){
        if(active_idx == -1 && op != 0){ std::cout << "Create a table entry first!\n"; pause(); return; }
        Status s; int i, k; Type e, res; std::string path;

        switch(op){
            case 1: std::cout << "Init: " << (int)InitList(curr()) << "\n"; break;
            case 2: std::cout << "Destroy: " << (int)DestroyList(curr()) << "\n"; break;
            case 3: std::cout << "Clear: " << (int)ClearList(curr()) << "\n"; break;
            case 4: {
                auto [empty, st] = ListEmpty(curr());
                if(st == Status::OK) std::cout << "Empty: " << (empty ? "TRUE" : "FALSE") << "\n";
                else std::cout << "List not initialized!\n";
                break;
            }
            case 5: std::cout << "Length: " << ListLength(curr()) << "\n"; break;
            case 6: 
                std::cout << "Index: "; std::cin >> i;
                if(GetElem(curr(), i, e) == Status::OK) std::cout << "Element " << i << " is " << e << "\n";
                else std::cout << "Error or Out of bounds!\n";
                break;
            case 7:
                std::cout << "Value to locate: "; std::cin >> e;
                i = LocateElem(curr(), e, [](const Type& a, const Type& b){ return a == b; });
                if(i) std::cout << "First pos: " << i << "\n";
                else std::cout << "Not found.\n";
                break;
            case 8:
                std::cout << "Current value: "; std::cin >> e;
                if(PriorElem(curr(), e, res) == Status::OK) std::cout << "Prior: " << res << "\n";
                else std::cout << "No prior or not found.\n";
                break;
            case 9:
                std::cout << "Current value: "; std::cin >> e;
                if(NextElem(curr(), e, res) == Status::OK) std::cout << "Next: " << res << "\n";
                else std::cout << "No next or not found.\n";
                break;
            case 10:
                std::cout << "Index and Value: "; std::cin >> i >> e;
                if(ListInsert(curr(), i, e) == Status::OK) std::cout << "Inserted.\n";
                break;
            case 11:
                std::cout << "Delete index: "; std::cin >> i;
                if(ListDelete(curr(), i, e) == Status::OK) std::cout << "Deleted value: " << e << "\n";
                break;
            case 12:
                std::cout << "Data: [ ";
                ListTraverse(curr(), [](const Type& v){ std::cout << v << " "; });
                std::cout << "]\n"; break;
            case 13: 
                if(ListLength(curr()) > 0) std::cout << "Max SubArray Sum: " << MaxSubArray(curr()) << "\n";
                else std::cout << "List is empty!\n";
                break;
            case 14: 
                std::cout << "Target sum K: "; std::cin >> k;
                std::cout << "Count: " << SubArrayNum(curr(), k) << "\n";
                break;
            case 15: 
                if(SortList(curr()) == Status::OK) std::cout << "Sorted using Shell(Knuth).\n";
                break;
            case 16: 
                std::cout << "Save to path: "; std::cin >> path;
                if(SaveList(curr(), path.c_str()) == Status::OK) std::cout << "Saved.\n";
                break;
            case 17: 
                std::cout << "Load from path: "; std::cin >> path;
                if(LoadList(curr(), path.c_str()) == Status::OK) std::cout << "Loaded.\n";
                break;
            default: std::cout << "Invalid option.\n"; break;
        }
        pause();
    }
};

int main(){
    App app;
    app.run();
    return 0;
}