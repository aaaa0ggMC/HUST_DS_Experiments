/**
 * @file bin_tree.h
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 二叉树的ADT实现，这次打算全部用异常，方便ui程序展示错误
 * @version 5.0
 * @date 2026-06-03
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef HUST_DS_BIN_TREE_ADT_H
#define HUST_DS_BIN_TREE_ADT_H
#include <alib5/ds/bin_tree.h>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <stack>
#include <queue>
#include <algorithm>
#include <utility>

namespace hust_ds::bin_tree {
    /// 数据类型，包含关键表示和附加字符串
    /// 使用十分安全的std::string进行管理
    struct ElemType{
        int         key;
        std::string others;

        ElemType():key(0) {}
        ElemType(int k):key(k) {}
        ElemType(int k,const std::string& o):key(k),others(o) {}
    };

    /// 物理结构
    using Node = alib5::ds::BinTree<ElemType>;
    /// 整个二叉树，使用指针，因为根节点在使用中不一定存在
    using Tree = Node*;                           

    //// 带线索的前序遍历build时的线索 ////
    /// 结束定义
    constexpr int DEF_END  = -1;
    /// 空子树   
    constexpr int DEF_NULL =  0;

    namespace detail {

        /// 递归清楚二叉树
        inline void clear_tree(Tree& t){
            if(!t) return;
            clear_tree(t->left);
            clear_tree(t->right);
            delete t;
            t = nullptr;
        }

        
        /// 防止delete node throw太多次
        inline bool delete_node_impl(Tree& t, int key) {
            if(!t) return false;

            if(t->data.key == key){
                Node* target = t;
                if(!t->left){ // 入度 0,1,2
                    t = t->right;
                }else if (!t->right){ // 入度 0,1
                    t = t->left;
                }else{ // 入度0,找到最边上的对象
                    Node* lc = t->left;
                    Node* rc = t->right;

                    Node* rightmost = lc;
                    while (rightmost->right)
                        rightmost = rightmost->right;
                    rightmost->right = rc;
                    t = lc;
                }

                delete target;
                return true;
            }
            return delete_node_impl(t->left, key) ||
                delete_node_impl(t->right, key);
        }

    }


    /// 初始化二叉树，有数据时初始化报错
    /// 没数据时重复初始化没问题
    inline void InitBiTree(Tree& T){
        if(T != nullptr) throw std::runtime_error("Tree has been inited already.");
        T = nullptr;
    }

    /// 销毁二叉树，重复销毁报错
    inline void DestroyBiTree(Tree& T){
        if(T == nullptr) throw std::runtime_error("Tree has been destroyed already.");
        detail::clear_tree(T);
        T = nullptr;
    }


    /// 按照线索前序遍历创建二叉树
    inline void CreateBiTree(Tree& T, const ElemType definition[]){
        // 检查key是不是唯一的，重复的话报错
        for(int i = 0;definition[i].key != DEF_END;++i){
            if(definition[i].key == DEF_NULL) continue;

            // 其实对于小数据可以按位来看，算是一种哈希处理
            // 但是保险起见还是O(N) search
            for(int j = 0; j < i; ++j){
                if(definition[j].key == definition[i].key){
                    throw std::runtime_error("CreateBiTree: duplicate key in definition");
                }
            }
        }

        // CreateBiTree的参数传入基本限制了，因此用builder这个辅助对象实现递归
        struct Builder{
            static void build(Tree& node,const ElemType def[],int& idx){
                if(def[idx].key == DEF_END){
                    node = nullptr;
                    return;
                }
                if(def[idx].key == DEF_NULL){
                    node = nullptr;
                    ++idx;
                    return;
                }

                node = new Node();
                node->data = def[idx];
                ++idx;
                
                build(node->left,  def, idx);
                build(node->right, def, idx);
            }
        };

        int idx = 0;
        Builder::build(T, definition, idx);
    }


    /// 清除二叉树数据
    inline void ClearBiTree(Tree& T){
        detail::clear_tree(T);
        T = nullptr;
    }

    /// 判断二叉树是不是空的
    inline bool BiTreeEmpty(const Tree& T){
        return T == nullptr;
    }


    /// 返回二叉树的深度
    inline int BiTreeDepth(const Tree& T){
        if(!T) return 0;
        // 简单递归一下
        int ld = BiTreeDepth(T->left);
        int rd = BiTreeDepth(T->right);
        // 其实就是 0 + 1 + 1 + ... 一直加过去
        return std::max(ld, rd) + 1;
    }


    /// 找到key对应的node，这个没找到不抛出异常，而是返回nullptr
    inline Node* LocateNode(Tree T, int key){
        if(!T) return nullptr;
        if(T->data.key == key)return T;
        
        // 找左边找右边
        Node* p = LocateNode(T->left, key);
        if(p) return p;
        return LocateNode(T->right, key);
    }

    /// 给指定key的节点赋值
    inline void Assign(Tree T, int key, const ElemType& value){
        Node* target = LocateNode(T, key);
        if(!target){
            throw std::runtime_error("Assign: node with key not found");
        }

        if (value.key != key){
            Node* conflict = LocateNode(T, value.key);
            if (conflict){
                throw std::runtime_error("Assign: new key already exists in tree");
            }
        }

        target->data = value;
    }


    /// 获取key的一个兄弟节点
    inline Node* GetSibling(Tree T, int key) {
        if(!T) return nullptr;

        if(T->left && T->left->data.key == key)
            return T->right;
        if(T->right && T->right->data.key == key)
            return T->left;

        Node* sib = GetSibling(T->left, key);
        if(sib) return sib;
        return GetSibling(T->right, key);
    }

    /// 按照LR这个规则插入新的节点
    /// LR -1 : 作为root,原本的root变成右节点
    ///     0 : 左边
    ///     1 : 右边
    /// key为查找对象，如果LR==-1,那么这个忽略
    inline void InsertNode(Tree& T, int key, int LR, const ElemType& c){
        if(LocateNode(T, c.key)) throw std::runtime_error("InsertNode: duplicate key");

        Node* newNode = new Node();
        newNode->data  = c;
        newNode->left  = nullptr;
        newNode->right = nullptr;

        // root
        if(LR == -1){
            newNode->right = T;
            T = newNode;
            return;
        }

        // 查找parent
        Node* target = LocateNode(T, key);
        if(!target){
            delete newNode;
            throw std::runtime_error("InsertNode: parent node not found");
        }

        if(LR == 0){// left
            newNode->right = target->left;
            target->left   = newNode;
        }else if(LR == 1){// right
            newNode->right = target->right;
            target->right  = newNode;
        }else{
            delete newNode;
            throw std::runtime_error("InsertNode: invalid LR (must be -1, 0, or 1)");
        }
    }


    /// 删除key对应的节点
    inline void DeleteNode(Tree& T, int key) {
        if (!detail::delete_node_impl(T, key))
            throw std::runtime_error("DeleteNode: key not found");
    }


    /// 前序遍历 root-left-right
    template <class VisitFn>
    inline void PreOrderTraverse(Tree T, VisitFn&& visit){
        if(!T) return;
        visit(T);
        PreOrderTraverse(T->left,  std::forward<VisitFn>(visit));
        PreOrderTraverse(T->right, std::forward<VisitFn>(visit));
    }


    /// 中序遍历 left-root-right
    template <class VisitFn>
    inline void InOrderTraverse(Tree T, VisitFn&& visit){
        if(!T) return;
        InOrderTraverse(T->left,  std::forward<VisitFn>(visit));
        visit(T);
        InOrderTraverse(T->right, std::forward<VisitFn>(visit));
    }


    /// 后序遍历(非递归) left-right-root 
    template <class VisitFn>
    inline void PostOrderTraverse(Tree T, VisitFn&& visit){
        if(!T) return;
        // 模拟栈，这里使用std::stack进行模拟
        std::stack<Node*> stk;
        // 当前位置
        Node* p         = T;
        // 上一次访问
        Node* lastVisit = nullptr;

        while(p || !stk.empty()){
            // 最后访问自己，不断往左侧深入
            if(p){
                stk.push(p);
                p = p->left;
            }else{
                // 最左侧到了nullptr,开始“回溯”
                Node* peek = stk.top();

                // 如果有右边那就往右边先走
                // 通过lastVisit来标记peek是否已经走访过
                if (peek->right && peek->right != lastVisit) {
                    p = peek->right;
                }else{
                    // 没有右侧了，说明是最终的节点
                    visit(peek);
                    
                    lastVisit = peek;
                    stk.pop();
                }
            }
        }
    }


    /// 层序遍历 depth(0 to ...)
    template <class VisitFn>
    inline void LevelOrderTraverse(Tree T, VisitFn&& visit){
        if(!T) return;
        std::queue<Node*> q;
        q.push(T);

        while(!q.empty()){
            Node* node = q.front();
            q.pop();
            
            visit(node);
            // push到后面作为新的层级
            if(node->left)  q.push(node->left);
            if(node->right) q.push(node->right);
        }
    }

    /// 最大路径和
    inline int MaxPathSum(Tree T){
        if(!T) throw std::runtime_error("Invalid tree,please init the tree first!");
        // 叶子节点，只有自己的key
        if(!T->left && !T->right) return T->data.key;
        
        // 只有右节点，肯定只要访问右边
        if(!T->left)return T->data.key + MaxPathSum(T->right);
        // 同理
        if(!T->right)return T->data.key + MaxPathSum(T->left);
        
        // 都有，取最大值
        return T->data.key + std::max(MaxPathSum(T->left), MaxPathSum(T->right));
    }

    /// 最近公共祖先 
    inline Node* LowestCommonAncestor(Tree T, int key1, int key2) {
        // 如果某个节点不存在肯定没有公共祖先
        if(!LocateNode(T, key1) || !LocateNode(T, key2))return nullptr;
        
        struct LcaHelper{

            static Node* find(Tree t, int k1, int k2){
                if(!t) return nullptr;
                // 当前node为k1或者k2,到达最深处，返回
                if(t->data.key == k1 || t->data.key == k2) return t;
                // 查找左侧最接近k1/k2的
                Node* leftLCA  = find(t->left,  k1, k2);
                // 查找右侧最接近k1/k2
                Node* rightLCA = find(t->right, k1, k2);

                // 两个都找到了，那就是其本身了
                //  - b
                // a      这个时候a找left找right就刚好是的，显然就是LCA
                //  - c
                // 如果是 a - d - {b,c} 因为左侧继续find,所以 leftLCA为d也是满足情况的 
                if(leftLCA && rightLCA) return t;
                return leftLCA ? leftLCA : rightLCA;
            }

        };

        return LcaHelper::find(T, key1, key2);
    }


    /// 反转树
    inline void InvertTree(Tree T){
        if(!T) return;
        
        std::swap(T->left, T->right);
        InvertTree(T->left);
        InvertTree(T->right);
    }

    /// 保存树，MAGIC为"BTRE(Binary Tree)"
    inline void SaveBiTree(Tree T, const char* filename){
        FILE* fp = fopen(filename, "w");
        if(!fp) throw std::runtime_error("SaveBiTree: cannot open file for writing");

        fputs("BTRE", fp);

        /// 依旧递归保存，带线索的前序遍历，刚好可以喂给Create
        struct Saver{
            static void save(Tree node, FILE* file){
                if(!node){
                    fputs(" 0 null", file);
                }else{
                    fprintf(file, " %d %s",
                            node->data.key, node->data.others.c_str());
                    save(node->left,  file);
                    save(node->right, file);
                }
            }
        };

        Saver::save(T, fp);
        fputc('\n', fp);
        fclose(fp);
    }

    /// 加载树
    inline void LoadBiTree(Tree& T, const char* filename) {
        FILE* fp = fopen(filename, "r");
        if(!fp) throw std::runtime_error("LoadBiTree: cannot open file for reading");

        // verify magic
        char magic[5] = {0};
        fscanf(fp, "%4s", magic);
        if(std::strcmp(magic, "BTRE") != 0){
            fclose(fp);
            throw std::runtime_error("LoadBiTree: invalid file format (bad magic)");
        }

        // 清除目前的树，也可以抛出异常，但是毕竟语义是load，可以override
        ClearBiTree(T);

        struct Loader{
            static void load(Tree& node, FILE* file){
                int key;
                char others[256] = {0};
                if(fscanf(file, "%d %255s", &key, others) == 2){
                    if(key == 0){
                        node = nullptr;
                    }else{
                        node = new Node();
                        node->data = ElemType(key, others);
                        load(node->left,  file);
                        load(node->right, file);
                    }
                }else{
                    node = nullptr;
                }
            }
        };

        Loader::load(T, fp);
        fclose(fp);
    }

}

#endif