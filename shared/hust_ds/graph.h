/**
 * @file graph.h
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 图实现，这次没写基础模型，因为我发现数据结构实验更讲究算法，写基础模型套起来如套
 * @version 5.0
 * @date 2026-06-05
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef HUST_DS_GRAPH_ADL_H
#define HUST_DS_GRAPH_ADL_H
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <queue>
#include <utility>

namespace hust_ds::graph {
    /// 顶点的基础数据类型
    struct VertexType {
        /// 关键字(unique)
        int         key;      
        /// 其他数据
        std::string others;   

        VertexType():key(0) {}
        VertexType(int k):key(k) {}
        VertexType(int k,const std::string& o):key(k),others(o) {}
    };

    /// 边表节点（弧）
    struct ArcNode {
        /// 该弧所指向的顶点在顶点数组中的位置
        int      adjvex;
        /// 指向下一条弧的指针
        ArcNode* nextarc;

        ArcNode(int adj, ArcNode* next = nullptr):adjvex(adj),nextarc(next) {}
    };

    /// 顶点表节点
    struct VNode{
        /// 顶点信息
        VertexType data;     
        /// 第一条依附该顶点的弧
        ArcNode*   firstarc; 

        VNode(const VertexType& d):data(d),firstarc(nullptr) {}
    };

    /// 邻接表表示的图物理结构
    struct ALGraph{
        /// 顶点表
        std::vector<VNode> vertices;
        /// 图的边数
        int arcnum;

        ALGraph():arcnum(0) {}
    };

    using Graph = ALGraph*;
    /// 图构建的结束标志
    constexpr int DEF_END = -1;

    /// 传入图初始化的关系结构
    struct ArcDef{
        int v;
        int w;
    };


    namespace detail {
        /// 释放所有的边表内存
        inline void clear_edges(Graph G){
            if(!G) return;

            for(auto& vertex : G->vertices){
                ArcNode* p = vertex.firstarc;
                
                while(p){
                    ArcNode* temp = p;
                    p = p->nextarc;
                    delete temp;
                }
                vertex.firstarc = nullptr;
            }
            G->arcnum = 0;
        }
    }

    /// 初始化一个空图
    inline void InitGraph(Graph& G){
        if(G) throw std::runtime_error("InitGraph: Graph is already inited.");
        G = new ALGraph();
    }

    /// 销毁Graph
    inline void DestroyGraph(Graph& G){
        if(!G) throw std::runtime_error("DestroyGraph: Graph is already null.");
        detail::clear_edges(G);
        delete G;
        G = nullptr;
    }

    /// 查找对应的Vex，返回的是Graph中的索引
    inline int LocateVex(Graph G, int key){
        if(!G) throw std::runtime_error("DestroyGraph: Graph is invalid,please init first!");
        
        for(size_t i = 0; i < G->vertices.size(); ++i){
            if(G->vertices[i].data.key == key){
                return static_cast<int>(i);
            }
        }
        
        return -1;
    }

    /// 创建无向图
    inline void CreateGraph(Graph& G, const VertexType V[], const ArcDef VR[]){
        if(!G) InitGraph(G);

        detail::clear_edges(G);
        G->vertices.clear();

        // 插入顶点
        for(int i = 0; V[i].key != DEF_END; ++i){

            if(LocateVex(G, V[i].key) != -1){
                throw std::invalid_argument("CreateGraph: Duplicate vertex key.");
            }
            
            G->vertices.emplace_back(V[i]);
        }
        if(G->vertices.empty()){
            throw std::invalid_argument("CreateGraph: Graph must have at least one vertex.");
        }

        // 插入边
        for(int k = 0; VR[k].v != DEF_END; ++k){
            int loc1 = LocateVex(G, VR[k].v);
            int loc2 = LocateVex(G, VR[k].w);

            if(loc1 == -1 || loc2 == -1){
                throw std::invalid_argument("CreateGraph: Invalid edge reference (vertex not found).");
            }
            // 忽略无向图里的自环
            if(loc1 == loc2) continue;

            // 检查是否重复边
            bool exists = false;
            ArcNode* p = G->vertices[loc1].firstarc;
            while(p){
                if(p->adjvex == loc2){ 
                    exists = true; 
                    break; 
                }
                p = p->nextarc;
            }

            // 虽然可以搞一个表，但是这里的操作其实算是最方便的哈哈
            if(!exists){
                // 头插法对称插入无向边
                G->vertices[loc1].firstarc = new ArcNode(loc2, G->vertices[loc1].firstarc);
                G->vertices[loc2].firstarc = new ArcNode(loc1, G->vertices[loc2].firstarc);
                G->arcnum++;
            }
        }
    }

    /// 对key为u的定点赋值为新的value
    inline void PutVex(Graph G, int u, const VertexType& value){
        int loc = LocateVex(G, u);
        if(loc == -1) throw std::runtime_error("PutVex: Vertex not found.");

        // 检查key是不是已经被占用了
        if(value.key != u){
            if(LocateVex(G, value.key) != -1){
                throw std::runtime_error("PutVex: New key already exists.");
            }
        }
        G->vertices[loc].data = value;
    }


    /// 获得位序u对应顶点的第一个邻接顶点的位序，失败返回 -1
    inline int FirstAdjVex(Graph G, int u_loc){
        if(!G || u_loc < 0 || u_loc >= (int)G->vertices.size()) return -1;
        ArcNode* p = G->vertices[u_loc].firstarc;
        return p ? p->adjvex : -1;
    }

    /// 获得位序v的顶点相对于位序w的下一个邻接顶点的位序，失败或最后返回 -1
    inline int NextAdjVex(Graph G, int v_loc, int w_loc){
        if(!G || v_loc < 0 || v_loc >= (int)G->vertices.size()) return -1;

        ArcNode* p = G->vertices[v_loc].firstarc;
        while(p){
            if(p->adjvex == w_loc){
                return p->nextarc ? p->nextarc->adjvex : -1;
            }
            p = p->nextarc;
        }
        return -1;
    }

    /// 增加新的顶点
    inline void InsertVex(Graph G, const VertexType& v){
        if(!G) throw std::runtime_error("InsertVex: Graph is null.");
        
        if(LocateVex(G, v.key) != -1){
            throw std::runtime_error("InsertVex: Duplicate vertex key.");
        }

        G->vertices.emplace_back(v);
    }


    /// 删除关键字为v的顶点，还有弧
    inline void DeleteVex(Graph G, int v){
        if(!G) throw std::runtime_error("DeleteVex: Graph is null.");
        
        int loc = LocateVex(G, v);
        if(loc == -1) throw std::runtime_error("DeleteVex: Vertex not found.");
        if(G->vertices.size() == 1) throw std::runtime_error("DeleteVex: Cannot delete the last vertex.");

        // 删除所有关联到此顶点的弧,并在对方的邻接表中摘除自己
        ArcNode* p = G->vertices[loc].firstarc;
        while(p){
            int neighbor_loc = p->adjvex;
            ArcNode* q = G->vertices[neighbor_loc].firstarc;
            ArcNode* pre = nullptr;

            while(q){
                if(q->adjvex == loc){
                    if(!pre){
                        G->vertices[neighbor_loc].firstarc = q->nextarc;
                    } else pre->nextarc = q->nextarc;
                    
                    delete q;
                    break;
                }
                pre = q;
                q = q->nextarc;
            }
            
            ArcNode* temp = p;
            p = p->nextarc;
            delete temp;
            G->arcnum--;
        }

        // 从容器中移除顶点
        G->vertices.erase(G->vertices.begin() + loc);

        // 同步更新所有邻接表中的位序索引，大于loc的索引需要减1以填补空缺
        for (auto& vertex : G->vertices) {
            ArcNode* curr = vertex.firstarc;
            while (curr) {
                if (curr->adjvex > loc) {
                    curr->adjvex--;
                }
                curr = curr->nextarc;
            }
        }
    }

    /// 在无向图中增加弧
    inline void InsertArc(Graph G, int v, int w){
        if(!G) throw std::runtime_error("InsertArc: Graph is null.");

        int loc_v = LocateVex(G, v);
        int loc_w = LocateVex(G, w);

        if (loc_v == -1 || loc_w == -1) throw std::runtime_error("InsertArc: Vertex not found.");
        if (loc_v == loc_w) throw std::invalid_argument("InsertArc: Self-loops are not allowed.");

        // 防重复检查
        ArcNode* p = G->vertices[loc_v].firstarc;
        while (p) {
            if (p->adjvex == loc_w) throw std::runtime_error("InsertArc: Arc already exists.");
            p = p->nextarc;
        }

        // 无向图双向插入
        G->vertices[loc_v].firstarc = new ArcNode(loc_w, G->vertices[loc_v].firstarc);
        G->vertices[loc_w].firstarc = new ArcNode(loc_v, G->vertices[loc_w].firstarc);
        G->arcnum++;
    }

    /// 在无向图中删除弧
    inline void DeleteArc(Graph G, int v, int w) {
        if(!G) throw std::runtime_error("DeleteArc: Graph is null.");
        
        int loc_v = LocateVex(G, v);
        int loc_w = LocateVex(G, w);
        if (loc_v == -1 || loc_w == -1) 
            throw std::runtime_error("DeleteArc: Vertex not found.");

        bool found = false;
        // 清理 v 的边表
        ArcNode* p = G->vertices[loc_v].firstarc;
        ArcNode* pre = nullptr;
        while (p) {
            if (p->adjvex == loc_w) {
                found = true;
                
                if(!pre){
                    G->vertices[loc_v].firstarc = p->nextarc;
                }else{
                    pre->nextarc = p->nextarc;
                }
                
                delete p;
                break;
            }
            pre = p;
            p = p->nextarc;
        }
        if(!found) throw std::runtime_error("DeleteArc: Arc not found.");

        // 清理w中的弧
        p = G->vertices[loc_w].firstarc;
        pre = nullptr;
        while(p){
            if(p->adjvex == loc_v){
                
                if(!pre){
                    G->vertices[loc_w].firstarc = p->nextarc;
                }else pre->nextarc = p->nextarc;
                
                delete p;
                break;
            }
            pre = p;
            p = p->nextarc;
        }

        G->arcnum--;
    }


    /// DFS遍历
    template <class VisitFn>
    inline void DFSTraverse(Graph G, VisitFn&& visit){
        if(!G) return;
        if(G->vertices.empty()) return;

        /// 因为我无法知道层序关系，所以需要visited来排除"children"
        std::vector<bool> visited(G->vertices.size(), false);
        struct DFSHelper{
            static void dfs(Graph g, int v, std::vector<bool>& vis, VisitFn& func){
                vis[v] = true;
                func(g->vertices[v].data);

                ArcNode* p = g->vertices[v].firstarc;
                while(p){
                    if(!vis[p->adjvex]){
                        dfs(g, p->adjvex, vis, func);
                    }
                    p = p->nextarc;
                }
            }
        };

        for(size_t i = 0; i < G->vertices.size(); ++i){
            if (!visited[i]) {
                DFSHelper::dfs(G, i, visited, visit);
            }
        }
    }


    /// BFS遍历
    template <class VisitFn>
    inline void BFSTraverse(Graph G, VisitFn&& visit){
        if(!G) return;
        if(G->vertices.empty()) return;

        std::vector<bool> visited(G->vertices.size(), false);
        std::queue<int> q;
        for(size_t i = 0; i < G->vertices.size(); ++i){
            if (!visited[i]){
                
                visited[i] = true;
                visit(G->vertices[i].data);
                q.push(i);

                while(!q.empty()){
                    int v = q.front();
                    q.pop();

                    ArcNode* p = G->vertices[v].firstarc;
                    // 压入下一层中
                    while(p){
                        if(!visited[p->adjvex]){
                            visited[p->adjvex] = true;
                            visit(G->vertices[p->adjvex].data);
                            q.push(p->adjvex);
                        }
                        p = p->nextarc;
                    }
                }
            }
        }
    }


    /// 与v距离小于K的顶点的集合
    inline std::vector<VertexType> VerticesSetLessThanK(Graph G, int v, int k) {
        if(!G) throw std::runtime_error("VerticesSetLessThanK: Graph is null.");
        
        std::vector<VertexType> result;
        if(k <= 0) return result;

        int start_loc = LocateVex(G, v);
        if(start_loc == -1) throw std::runtime_error("VerticesSetLessThanK: Vertex not found.");

        std::vector<bool> visited(G->vertices.size(), false);
        // (loc,distance)
        std::queue<std::pair<int, int>> q;

        visited[start_loc] = true;
        q.push({start_loc, 0});
        
        while(!q.empty()){
            auto [curr_loc, dist] = q.front();
            q.pop();

            // 自己也算上
            if(dist >= 0 && dist < k){
                result.push_back(G->vertices[curr_loc].data);
            }

            // 由于没有权重，每次都是1,因此如果这次是k-1,那么下次没必要探索了
            if(dist + 1 >= k) continue;

            ArcNode* p = G->vertices[curr_loc].firstarc;
            while(p){
                if(!visited[p->adjvex]){
                    visited[p->adjvex] = true;
                    q.push({p->adjvex, dist + 1});
                }
                p = p->nextarc;
            }

        }

        return result;
    }

    /// 计算vw之间最短路径长度
    inline int ShortestPathLength(Graph G, int v, int w) {
        if(!G) throw std::runtime_error("ShortestPathLength: Graph is null.");
        
        int loc_v = LocateVex(G, v);
        int loc_w = LocateVex(G, w);

        if (loc_v == -1 || loc_w == -1) throw std::runtime_error("ShortestPathLength: Vertex not found.");
        if (loc_v == loc_w) return 0;

        std::vector<bool> visited(G->vertices.size(), false);
        // 这里也是(loc,distance)
        std::queue<std::pair<int, int>> q; 
        
        visited[loc_v] = true;
        q.push({loc_v, 0});

        while(!q.empty()){
            auto [curr_loc, dist] = q.front();
            q.pop();

            ArcNode* p = G->vertices[curr_loc].firstarc;
            
            // 这里我用BFS遍历，似乎和洪水算法很像还是就是洪水算法?
            // 反正这里探索的是每一层，因此找到了就return完全正确
            while(p){
                // 找到了
                if(p->adjvex == loc_w)return dist + 1;
                
                if(!visited[p->adjvex]){
                    visited[p->adjvex] = true;
                    q.push({p->adjvex, dist + 1});
                }

                p = p->nextarc;
            }
        }

        throw std::runtime_error("ShortestPathLength: Path not exists.");
    }

    /// 返回图的连通分量的个数
    inline int ConnectedComponentsNums(Graph G){
        if(!G) throw std::runtime_error("ConnectedComponentsNums: Graph is null.");
        if(G->vertices.empty()) return 0;

        int components = 0;
        std::vector<bool> visited(G->vertices.size(), false);

        for(size_t i = 0; i < G->vertices.size(); ++i){
            if(!visited[i]){
                components++;

                // BFS遍历找出连通 
                std::queue<int> q;
                visited[i] = true;
                q.push(i);
                
                while(!q.empty()){
                    int curr = q.front();
                    q.pop();
                    
                    ArcNode* p = G->vertices[curr].firstarc;
                    while(p){
                        if(!visited[p->adjvex]){
                            visited[p->adjvex] = true;
                            q.push(p->adjvex);
                        }
                        p = p->nextarc;
                    }
                }

            }
        }
        return components;
    }

    /// 保存进入文件
    inline void SaveGraph(Graph G, const char* filename){
        if(!G) throw std::runtime_error("Graph: Graph is null.");

        FILE* fp = fopen(filename, "w");
        if(!fp) throw std::runtime_error("SaveGraph: Cannot open file for writing.");

        fputs("AGRH\n", fp);
        fprintf(fp, "%zu %d\n", G->vertices.size(), G->arcnum);

        // 保存顶点信息
        for(const auto& vertex : G->vertices){
            fprintf(fp, "%d %s\n", vertex.data.key, vertex.data.others.empty() ? "null" : vertex.data.others.c_str());
        }

        // 弧信息
        for(const auto& vertex : G->vertices){
            ArcNode* p = vertex.firstarc;
            while(p){
                fprintf(fp, "%d ", p->adjvex);
                p = p->nextarc;
            }
            fprintf(fp, "-1\n");
        }

        fclose(fp);
    }

    /// 从文件中读取
    inline void LoadGraph(Graph& G, const char* filename){
        FILE* fp = fopen(filename, "r");
        if(!fp) throw std::runtime_error("LoadGraph: Cannot open file for reading.");

        char magic[5] = {0};
        fscanf(fp, "%4s", magic);
        if(std::strcmp(magic, "AGRH") != 0){
            fclose(fp);
            throw std::runtime_error("LoadGraph: Invalid file format (bad magic).");
        }

        if(!G) InitGraph(G);
        detail::clear_edges(G);
        G->vertices.clear();

        size_t v_num = 0;
        int a_num = 0;
        fscanf(fp, "%zu %d", &v_num, &a_num);
        G->arcnum = a_num;

        // 读取顶点数据
        for(size_t i = 0; i < v_num; ++i){
            int key;
            char others[256] = {0};
            fscanf(fp, "%d %255s", &key, others);
            
            std::string o_str = (std::strcmp(others, "null") == 0) ? "" : others;
            G->vertices.emplace_back(VertexType(key, o_str));
        }

        // 尾插法重建邻接表
        for(size_t i = 0; i < v_num; ++i){
            ArcNode* tail = nullptr;
            int adjvex = -1;
            while(fscanf(fp, "%d", &adjvex) == 1 && adjvex != -1){
                ArcNode* node = new ArcNode(adjvex);
                
                if(!G->vertices[i].firstarc){
                    G->vertices[i].firstarc = node;
                }else{
                    tail->nextarc = node;
                }

                tail = node;
            }
        }

        fclose(fp);
    }
}

#endif