/**
 * @file main.cpp
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 二叉树演示系统，我觉得还是cli更对我胃口，不仅方便还可以使用文件重定向实现快速测试
 * @version 6.0
 * @date 2026-06-05
 * 
 * @copyright Copyright (c) 2026
 */
#include <hust_ds/bin_tree.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace hust_ds::bin_tree;

struct TreeInstance {
    std::string name;
    Tree tree = nullptr;
};

class CLIApp {
private:
    std::vector<TreeInstance> trees;
    int active_idx = -1;
    
    Tree& curr() {
        if (active_idx == -1 || active_idx >= (int)trees.size())
            throw std::runtime_error("No active tree! Use 'create_tree' or 'switch_tree'.");
        return trees[active_idx].tree;
    }

    // 辅助打印访问者
    static void print_node(Node* n) {
        if (n) std::cout << "[" << n->data.key << ":" << n->data.others << "] ";
    }

    // 命令分发处理器
    void process_command(const std::string& cmd, std::istringstream& args) {
        if (cmd == "help") {
            show_help();
        }
        else if(cmd == "cls") {
            system("clear || cls");
        }
        else if (cmd == "create_tree") {
            std::string name;
            if (args >> name) {
                trees.push_back({name, nullptr});
                if (active_idx == -1) active_idx = 0;
                std::cout << "Tree '" << name << "' created. (Index: " << trees.size() << ")\n";
            } else {
                std::cout << "Usage: create_tree <name>\n";
            }
        }
        else if (cmd == "list_trees") {
            if (trees.empty()) {
                std::cout << "No trees available.\n";
                return;
            }
            std::cout << "ID\tActive\tName\t\tDepth\tEmpty\n";
            std::cout << "--------------------------------------------------\n";
            for (int i = 0; i < (int)trees.size(); ++i) {
                // UI 展示使用 1-based (i + 1)
                std::cout << (i + 1) << "\t" 
                          << (i == active_idx ? "  *   " : "      ") << "\t"
                          << std::left << std::setw(12) << trees[i].name << "\t"
                          << BiTreeDepth(trees[i].tree) << "\t"
                          << (BiTreeEmpty(trees[i].tree) ? "TRUE" : "FALSE") << "\n";
            }
        }
        else if (cmd == "delete_tree") {
            std::string target;
            if (args >> target) {
                int del_idx = -1;
                // 1. 定位要删除的索引 (复用查找逻辑)
                if (isdigit(target[0])) {
                    del_idx = std::stoi(target) - 1; 
                } else {
                    for (int i = 0; i < (int)trees.size(); ++i) {
                        if (trees[i].name == target) {
                            del_idx = i;
                            break;
                        }
                    }
                }

                // 2. 执行删除
                if (del_idx >= 0 && del_idx < (int)trees.size()) {
                    // 重要：先销毁二叉树内存，防止内存泄漏
                    DestroyBiTree(trees[del_idx].tree);
                    std::string deleted_name = trees[del_idx].name;
                    trees.erase(trees.begin() + del_idx);
                    
                    std::cout << "Tree '" << deleted_name << "' has been removed.\n";

                    // 3. 维护当前活跃索引 active_idx
                    if (trees.empty()) {
                        active_idx = -1;
                    } else if (active_idx == del_idx) {
                        // 如果删的是当前正在用的，就把指针指回第一个
                        active_idx = 0;
                        std::cout << "Active tree was deleted. Switched to '" << trees[active_idx].name << "'.\n";
                    } else if (active_idx > del_idx) {
                        // 如果删的是当前索引前面的，当前索引需要减1
                        active_idx--;
                    }
                } else {
                    std::cout << "Error: Tree '" << target << "' not found.\n";
                }
            } else {
                std::cout << "Usage: delete_tree <index|name>\n";
            }
        }
        else if (cmd == "find_tree") {
            std::string name;
            if (args >> name) {
                bool found = false;
                for (int i = 0; i < (int)trees.size(); ++i) {
                    if (trees[i].name == name) {
                        std::cout << "Found: '" << name << "' at Index " << (i + 1) << "\n";
                        found = true;
                    }
                }
                if (!found) std::cout << "Tree '" << name << "' not found.\n";
            } else std::cout << "Usage: find_tree <name>\n";
        }
        else if (cmd == "switch_tree") {
            std::string target;
            if (args >> target) {
                int new_idx = -1;
                // 判断是否是纯数字索引
                if (isdigit(target[0])) {
                    int id = std::stoi(target);
                    new_idx = id - 1; // UI 是 1-based，转为 0-based
                } else {
                    // 按名称查找
                    for (int i = 0; i < (int)trees.size(); ++i) {
                        if (trees[i].name == target) {
                            new_idx = i;
                            break;
                        }
                    }
                }

                if (new_idx >= 0 && new_idx < (int)trees.size()) {
                    active_idx = new_idx;
                    std::cout << "Switched to Tree [" << (new_idx + 1) << "] '" << trees[active_idx].name << "'\n";
                } else {
                    std::cout << "Invalid index or tree name not found.\n";
                }
            } else std::cout << "Usage: switch_tree <index|name>\n";
        }
        else if (cmd == "init") {
            InitBiTree(curr());
            std::cout << "Tree initialized (root set to nullptr).\n";
        }
        else if (cmd == "destroy") {
            DestroyBiTree(curr());
            std::cout << "Tree destroyed and memory freed.\n";
        }
        else if (cmd == "clear") {
            ClearBiTree(curr());
            std::cout << "Tree cleared.\n";
        }
        else if (cmd == "empty") {
            std::cout << "Is Empty: " << (BiTreeEmpty(curr()) ? "TRUE" : "FALSE") << "\n";
        }
        else if (cmd == "depth") {
            std::cout << "Depth: " << BiTreeDepth(curr()) << "\n";
        }
        else if (cmd == "build") {
            std::vector<ElemType> def;
            int key; std::string others;
            while (args >> key >> others) {
                def.push_back(ElemType(key, others));
                if (key == DEF_END) break;
            }
            if (def.empty() || def.back().key != DEF_END) {
                def.push_back(ElemType(DEF_END, "end"));
            }
            ClearBiTree(curr()); // 构建前先清空
            CreateBiTree(curr(), def.data());
            std::cout << "Tree built from definition sequence.\n";
        }
        else if (cmd == "locate") {
            int key;
            if (args >> key) {
                Node* p = LocateNode(curr(), key);
                if (p) std::cout << "Found Node: [" << p->data.key << ":" << p->data.others << "]\n";
                else std::cout << "Node not found.\n";
            } else std::cout << "Usage: locate <key>\n";
        }
        else if (cmd == "assign") {
            int old_key, new_key; std::string new_others;
            if (args >> old_key >> new_key >> new_others) {
                Assign(curr(), old_key, ElemType(new_key, new_others));
                std::cout << "Assigned new value.\n";
            } else std::cout << "Usage: assign <old_key> <new_key> <new_others>\n";
        }
        else if (cmd == "sibling") {
            int key;
            if (args >> key) {
                Node* p = GetSibling(curr(), key);
                if (p) std::cout << "Sibling Node: [" << p->data.key << ":" << p->data.others << "]\n";
                else std::cout << "No sibling found.\n";
            } else std::cout << "Usage: sibling <key>\n";
        }
        else if (cmd == "insert") {
            int p_key, lr, n_key; std::string n_others;
            if (args >> p_key >> lr >> n_key >> n_others) {
                InsertNode(curr(), p_key, lr, ElemType(n_key, n_others));
                std::cout << "Node inserted.\n";
            } else std::cout << "Usage: insert <parent_key> <LR:-1|0|1> <new_key> <new_others>\n";
        }
        else if (cmd == "delete") {
            int key;
            if (args >> key) {
                DeleteNode(curr(), key);
                std::cout << "Node deleted.\n";
            } else std::cout << "Usage: delete <key>\n";
        }
        else if (cmd == "preorder") {
            std::cout << "PreOrder: ";
            PreOrderTraverse(curr(), print_node);
            std::cout << "\n";
        }
        else if (cmd == "inorder") {
            std::cout << "InOrder: ";
            InOrderTraverse(curr(), print_node);
            std::cout << "\n";
        }
        else if (cmd == "postorder") {
            std::cout << "PostOrder: ";
            PostOrderTraverse(curr(), print_node);
            std::cout << "\n";
        }
        else if (cmd == "levelorder") {
            std::cout << "LevelOrder: ";
            LevelOrderTraverse(curr(), print_node);
            std::cout << "\n";
        }
        else if (cmd == "maxpath") {
            std::cout << "Max Path Sum: " << MaxPathSum(curr()) << "\n";
        }
        else if (cmd == "lca") {
            int k1, k2;
            if (args >> k1 >> k2) {
                Node* lca = LowestCommonAncestor(curr(), k1, k2);
                if (lca) std::cout << "LCA Node: [" << lca->data.key << ":" << lca->data.others << "]\n";
                else std::cout << "LCA not found (one or both nodes might not exist).\n";
            } else std::cout << "Usage: lca <key1> <key2>\n";
        }
        else if (cmd == "invert") {
            InvertTree(curr());
            std::cout << "Tree inverted (Left and Right swapped).\n";
        }
        else if (cmd == "save") {
            std::string path;
            if (args >> path) {
                SaveBiTree(curr(), path.c_str());
                std::cout << "Tree saved to " << path << "\n";
            } else std::cout << "Usage: save <filepath>\n";
        }
        else if (cmd == "load") {
            std::string path;
            if (args >> path) {
                LoadBiTree(curr(), path.c_str());
                std::cout << "Tree loaded from " << path << "\n";
            } else std::cout << "Usage: load <filepath>\n";
        }
        else {
            std::cout << "Unknown command: '" << cmd << "'. Type 'help' for available commands.\n";
        }
    }

    void show_help() const {
        std::cout << "=== Multi-Tree System Commands ===\n"
                  << "[Multi-Tree Management] (Tree index is 1-based)\n"
                  << "  create_tree <name>                   : Create a new tree entry\n"
                  << "  list_trees                           : List all managed trees\n"
                  << "  delete_tree <index|name>             : Delete a tree entry and free memory\n"
                  << "  find_tree <name>                     : Find a tree's index by name\n"
                  << "  switch_tree <index|name>             : Switch active tree by index or name\n"
                  << "\n[Basic Operations (on active tree)]\n"
                  << "  init                                 : Initialize tree (set root null)\n"
                  << "  destroy                              : Destroy tree (free memory)\n"
                  << "  clear                                : Clear tree to empty\n"
                  << "  empty                                : Check if tree is empty\n"
                  << "  depth                                : Get tree depth\n"
                  << "  build <key str ... -1 end>           : Build from preorder sequence (0 = empty, -1 = end)\n"
                  << "  locate <key>                         : Locate node by key\n"
                  << "  assign <old_key> <new_key> <str>     : Change node value\n"
                  << "  sibling <key>                        : Get sibling of node\n"
                  << "  insert <parent_key> <LR> <key> <str> : Insert node (LR: -1=root, 0=left, 1=right)\n"
                  << "  delete <key>                         : Delete node by key\n"
                  << "\n[Traversals]\n"
                  << "  preorder | inorder | postorder | levelorder\n"
                  << "\n[Advanced Operations]\n"
                  << "  maxpath                              : Root-to-leaf max path sum\n"
                  << "  lca <key1> <key2>                    : Lowest common ancestor\n"
                  << "  invert                               : Invert the tree (swap L/R)\n"
                  << "  save <filepath>                      : Save to file\n"
                  << "  load <filepath>                      : Load from file\n"
                  << "\n[System]\n"
                  << "  help                                 : Show this help message\n"
                  << "  exit / quit                          : Exit program\n";
    }

public:
    void run() {
        std::cout << "Welcome to BinTree Shell V6.0! Type 'help' for commands.\n";
        std::string line;
        
        while (true) {
             if (active_idx != -1) {
                std::cout << "(" << trees[active_idx].name << ") >>> ";
            } else {
                std::cout << "(No Active Graph) >>> ";
            }
            if (!std::getline(std::cin, line)) break; // EOF (Ctrl+D / Ctrl+Z)
            
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;

            if (cmd == "exit" || cmd == "quit") {
                break;
            }

            try {
                process_command(cmd, iss);
            } catch (const std::exception& e) {
                // 捕获由 ADL 抛出的异常 (如重复Key, 父节点没找到等)
                std::cout << "[Error] " << e.what() << "\n";
            }
        }
        std::cout << "Bye!\n";
    }
};

int main() {
    CLIApp app;
    app.run();
    return 0;
}