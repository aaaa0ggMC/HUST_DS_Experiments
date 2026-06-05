/**
 * @file main.cpp
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 图的实现，UI还是AI生成的
 * @version 5.0
 * @date 2026-06-05
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <hust_ds/graph.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace hust_ds::graph;

struct GraphInstance {
    std::string name;
    Graph graph = nullptr;
};

class CLIApp {
private:
    std::vector<GraphInstance> graphs;
    int active_idx = -1; // 内部索引为 0-based

    // 获取当前活动图（引用），如果未选中则抛出异常
    Graph& curr() {
        if (active_idx == -1 || active_idx >= (int)graphs.size())
            throw std::runtime_error("No active graph! Use 'create_graph' or 'switch_graph'.");
        return graphs[active_idx].graph;
    }

    // 辅助打印访问者
    static void print_vertex(const VertexType& v) {
        std::cout << "[" << v.key << ":" << v.others << "] ";
    }

    // 命令分发处理器
    void process_command(const std::string& cmd, std::istringstream& args) {
        // ================= 多图管理系统 =================
        if (cmd == "help") {
            show_help();
        }
        else if (cmd == "cls") {
            system("clear || cls"); // 兼容 Linux/Windows
        }
        else if (cmd == "create_graph") {
            std::string name;
            if (args >> name) {
                // 防止重名
                for (const auto& g : graphs) {
                    if (g.name == name) throw std::runtime_error("Graph name already exists!");
                }
                graphs.push_back({name, nullptr});
                if (active_idx == -1) active_idx = 0;
                std::cout << "Graph '" << name << "' created. (Index: " << graphs.size() << ")\n";
            } else {
                std::cout << "Usage: create_graph <name>\n";
            }
        }
        else if (cmd == "list_graphs") {
            if (graphs.empty()) {
                std::cout << "No graphs available.\n";
                return;
            }
            std::cout << "ID\tActive\tName\t\tVexNum\tArcNum\tEmpty\n";
            std::cout << "--------------------------------------------------------------\n";
            for (int i = 0; i < (int)graphs.size(); ++i) {
                bool is_null = (graphs[i].graph == nullptr);
                std::cout << (i + 1) << "\t" 
                          << (i == active_idx ? "  *   " : "      ") << "\t"
                          << std::left << std::setw(12) << graphs[i].name << "\t"
                          << (is_null ? 0 : graphs[i].graph->vertices.size()) << "\t"
                          << (is_null ? 0 : graphs[i].graph->arcnum) << "\t"
                          << (is_null || graphs[i].graph->vertices.empty() ? "TRUE" : "FALSE") << "\n";
            }
        }
        else if (cmd == "find_graph") {
            std::string name;
            if (args >> name) {
                bool found = false;
                for (int i = 0; i < (int)graphs.size(); ++i) {
                    if (graphs[i].name == name) {
                        std::cout << "Found: '" << name << "' at Index " << (i + 1) << "\n";
                        found = true;
                    }
                }
                if (!found) std::cout << "Graph '" << name << "' not found.\n";
            } else std::cout << "Usage: find_graph <name>\n";
        }
        else if (cmd == "switch_graph") {
            std::string target;
            if (args >> target) {
                int new_idx = -1;
                if (isdigit(target[0])) {
                    int id = std::stoi(target);
                    new_idx = id - 1; 
                } else {
                    for (int i = 0; i < (int)graphs.size(); ++i) {
                        if (graphs[i].name == target) {
                            new_idx = i;
                            break;
                        }
                    }
                }

                if (new_idx >= 0 && new_idx < (int)graphs.size()) {
                    active_idx = new_idx;
                    std::cout << "Switched to Graph [" << (new_idx + 1) << "] '" << graphs[active_idx].name << "'\n";
                } else {
                    std::cout << "Invalid index or graph name not found.\n";
                }
            } else std::cout << "Usage: switch_graph <index|name>\n";
        }
        else if (cmd == "remove_graph") {
            std::string target;
            if (args >> target) {
                int target_idx = -1;
                if (isdigit(target[0])) target_idx = std::stoi(target) - 1;
                else {
                    for (int i = 0; i < (int)graphs.size(); ++i) {
                        if (graphs[i].name == target) { target_idx = i; break; }
                    }
                }
                
                if (target_idx >= 0 && target_idx < (int)graphs.size()) {
                    if (graphs[target_idx].graph != nullptr) {
                        DestroyGraph(graphs[target_idx].graph);
                    }
                    std::cout << "Graph '" << graphs[target_idx].name << "' removed.\n";
                    graphs.erase(graphs.begin() + target_idx);
                    
                    if (graphs.empty()) active_idx = -1;
                    else if (active_idx == target_idx) active_idx = 0; // fallback to 0
                    else if (active_idx > target_idx) active_idx--; // shift index
                } else {
                    std::cout << "Invalid index or graph name not found.\n";
                }
            } else std::cout << "Usage: remove_graph <index|name>\n";
        }

        // ================= 基本图操作 =================
        else if (cmd == "init") {
            InitGraph(curr());
            std::cout << "Graph initialized (empty graph created).\n";
        }
        else if (cmd == "destroy") {
            DestroyGraph(curr());
            std::cout << "Graph destroyed and memory freed.\n";
        }
        else if (cmd == "build") {
            std::vector<VertexType> V;
            std::vector<ArcDef> VR;
            std::string token;
            
            std::cout << "Enter vertices (key others), end with '-1 end':\n> ";
            int v_key; std::string v_str;
            while (std::cin >> v_key >> v_str) {
                V.push_back(VertexType(v_key, v_str));
                if (v_key == DEF_END) break;
            }

            std::cout << "Enter edges (v w), end with '-1 -1':\n> ";
            int e1, e2;
            while (std::cin >> e1 >> e2) {
                VR.push_back({e1, e2});
                if (e1 == DEF_END) break;
            }
            std::cin.ignore(10000, '\n'); // 清理缓冲区

            CreateGraph(curr(), V.data(), VR.data());
            std::cout << "Graph built successfully.\n";
        }
        else if (cmd == "print") { // 辅助显示邻接表
            Graph G = curr();
            if (!G) { std::cout << "Graph is null.\n"; return; }
            for (const auto& v : G->vertices) {
                std::cout << v.data.key << "(" << v.data.others << ") -> ";
                ArcNode* p = v.firstarc;
                while(p) {
                    std::cout << G->vertices[p->adjvex].data.key << " ";
                    p = p->nextarc;
                }
                std::cout << "\n";
            }
        }
        else if (cmd == "locate") {
            int key;
            if (args >> key) {
                int loc = LocateVex(curr(), key);
                if(loc != -1){
                    auto & vinfo = curr()->vertices[loc];
                    std::cout << "Found Vertex(location,key,others): ("
                              << loc << "," << vinfo.data.key << "," 
                              << vinfo.data.others << ")\n";
                }else std::cout << "Vertex not found.\n";
            } else std::cout << "Usage: locate <key>\n";
        }
        else if (cmd == "put") {
            int old_key, new_key; std::string new_str;
            if (args >> old_key >> new_key >> new_str) {
                PutVex(curr(), old_key, VertexType(new_key, new_str));
                std::cout << "Vertex updated.\n";
            } else std::cout << "Usage: put <old_key> <new_key> <new_str>\n";
        }
        else if (cmd == "firstadj") {
            int key;
            if (args >> key) {
                Graph G = curr();
                int loc = LocateVex(G, key);
                if (loc == -1) throw std::runtime_error("Vertex not found.");
                
                int adj_loc = FirstAdjVex(G, loc);
                if (adj_loc != -1) {
                    std::cout << "First Adj: ";
                    print_vertex(G->vertices[adj_loc].data);
                    std::cout << "\n";
                } else std::cout << "No adjacent vertex.\n";
            } else std::cout << "Usage: firstadj <key>\n";
        }
        else if (cmd == "nextadj") {
            int v, w;
            if (args >> v >> w) {
                Graph G = curr();
                int loc_v = LocateVex(G, v);
                int loc_w = LocateVex(G, w);
                if (loc_v == -1 || loc_w == -1) throw std::runtime_error("Vertex not found.");
                
                int adj_loc = NextAdjVex(G, loc_v, loc_w);
                if (adj_loc != -1) {
                    std::cout << "Next Adj: ";
                    print_vertex(G->vertices[adj_loc].data);
                    std::cout << "\n";
                } else std::cout << "No more adjacent vertices.\n";
            } else std::cout << "Usage: nextadj <v_key> <w_key>\n";
        }
        else if (cmd == "insert_v") {
            int key; std::string str;
            if (args >> key >> str) {
                InsertVex(curr(), VertexType(key, str));
                std::cout << "Vertex inserted.\n";
            } else std::cout << "Usage: insert_v <key> <str>\n";
        }
        else if (cmd == "delete_v") {
            int key;
            if (args >> key) {
                DeleteVex(curr(), key);
                std::cout << "Vertex and its relative arcs deleted.\n";
            } else std::cout << "Usage: delete_v <key>\n";
        }
        else if (cmd == "insert_a") {
            int v, w;
            if (args >> v >> w) {
                InsertArc(curr(), v, w);
                std::cout << "Arc inserted.\n";
            } else std::cout << "Usage: insert_a <v_key> <w_key>\n";
        }
        else if (cmd == "delete_a") {
            int v, w;
            if (args >> v >> w) {
                DeleteArc(curr(), v, w);
                std::cout << "Arc deleted.\n";
            } else std::cout << "Usage: delete_a <v_key> <w_key>\n";
        }
        else if (cmd == "dfs") {
            std::cout << "DFS Traverse: ";
            DFSTraverse(curr(), print_vertex);
            std::cout << "\n";
        }
        else if (cmd == "bfs") {
            std::cout << "BFS Traverse: ";
            BFSTraverse(curr(), print_vertex);
            std::cout << "\n";
        }

        // ================= 附加功能 =================
        else if (cmd == "less_than_k") {
            int v, k;
            if (args >> v >> k) {
                auto res = VerticesSetLessThanK(curr(), v, k);
                std::cout << "Vertices with distance < " << k << " from " << v << ":\n";
                for (const auto& vex : res) print_vertex(vex);
                if (res.empty()) std::cout << "(None)";
                std::cout << "\n";
            } else std::cout << "Usage: less_than_k <v_key> <k>\n";
        }
        else if (cmd == "shortest_path") {
            int v, w;
            if (args >> v >> w) {
                int dist = ShortestPathLength(curr(), v, w);
                std::cout << "Shortest Path Length (" << v << " -> " << w << "): " << dist << "\n";
            } else std::cout << "Usage: shortest_path <v_key> <w_key>\n";
        }
        else if (cmd == "components") {
            int c = ConnectedComponentsNums(curr());
            std::cout << "Number of Connected Components: " << c << "\n";
        }
        else if (cmd == "save") {
            std::string path;
            if (args >> path) {
                SaveGraph(curr(), path.c_str());
                std::cout << "Graph saved to " << path << "\n";
            } else std::cout << "Usage: save <filepath>\n";
        }
        else if (cmd == "load") {
            std::string path;
            if (args >> path) {
                LoadGraph(curr(), path.c_str());
                std::cout << "Graph loaded from " << path << "\n";
            } else std::cout << "Usage: load <filepath>\n";
        }
        else {
            std::cout << "Unknown command: '" << cmd << "'. Type 'help' for available commands.\n";
        }
    }

    void show_help() const {
        std::cout << "=== Multi-Graph System Commands ===\n"
                  << "[Multi-Graph Management] (Index is 1-based)\n"
                  << "  create_graph <name>                  : Create a new graph entry\n"
                  << "  remove_graph <index|name>            : Remove an existing graph\n"
                  << "  list_graphs                          : List all managed graphs\n"
                  << "  find_graph <name>                    : Find a graph's index by name\n"
                  << "  switch_graph <index|name>            : Switch active graph by index or name\n"
                  << "\n[Basic Operations (on active graph)]\n"
                  << "  init                                 : Initialize an empty graph\n"
                  << "  destroy                              : Destroy graph (free memory)\n"
                  << "  build                                : Interactively build graph from lists\n"
                  << "  print                                : Print Graph adjacency list\n"
                  << "  locate <key>                         : Locate vertex by key\n"
                  << "  put <old_key> <new_key> <new_str>    : Change vertex value\n"
                  << "  firstadj <key>                       : Get first adjacent vertex\n"
                  << "  nextadj <v_key> <w_key>              : Get next adjacent vertex of v relative to w\n"
                  << "  insert_v <key> <str>                 : Insert a new vertex\n"
                  << "  delete_v <key>                       : Delete a vertex and relative arcs\n"
                  << "  insert_a <v_key> <w_key>             : Insert a new undirected arc (edge)\n"
                  << "  delete_a <v_key> <w_key>             : Delete an arc\n"
                  << "\n[Traversals]\n"
                  << "  dfs | bfs                            : Depth-first / Breadth-first traversal\n"
                  << "\n[Advanced Operations]\n"
                  << "  less_than_k <v_key> <k>              : Get vertices set with dist < k from v\n"
                  << "  shortest_path <v_key> <w_key>        : Calc shortest path length between v and w\n"
                  << "  components                           : Get number of connected components\n"
                  << "  save <filepath>                      : Save graph to file\n"
                  << "  load <filepath>                      : Load graph from file\n"
                  << "\n[System]\n"
                  << "  help                                 : Show this help message\n"
                  << "  cls                                  : Clear screen\n"
                  << "  exit / quit                          : Exit program\n";
    }

public:
    void run() {
        std::cout << "Welcome to Graph Shell V1.0! Type 'help' for commands.\n";
        std::string line;
        
        while (true) {
            if (active_idx != -1) {
                std::cout << "(" << graphs[active_idx].name << ") >>> ";
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
                // 完美捕获由 ADL 抛出的异常并给予安全友好的提示
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