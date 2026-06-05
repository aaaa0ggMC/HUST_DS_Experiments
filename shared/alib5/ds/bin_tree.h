/**
 * @file bin_tree.h
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 简单的二叉树实现，我发现不需要我给出额外的ADL实现，因此怎么爽怎么来
 * @version 5.0
 * @date 2026-06-04
 * 
 * @copyright Copyright (c) 2026
 */
#ifndef ALIB5_DS_BIN_TREE
#define ALIB5_DS_BIN_TREE
#include <iostream>

namespace alib5::ds{
    template<class T>
    struct BinTree{
        /// 实际上保存的数据
        T data;

        /// 左边左边，个人不习惯lchild,rchild，不如left right
        BinTree<T> * left = nullptr;
        /// 右边
        BinTree<T> * right = nullptr;
    
        /// 给自己数据赋值
        void set(T & value){
            data = value;
        }

        /// 给左边赋值，自动创建新的
        void set_left(T & value){
            if(!left)left = new BinTree<T>;
            left->data = value;
        }

        /// 给右边赋值，自动创建新的
        void set_right(T & value){
            if(!right)right = new BinTree<T>;
            right->data = value;
        }

        /// 左边是否是空的，为啥没有empty()呢，因为根节点本来就不是空的了
        bool left_empty(){
            return left == nullptr;
        }

        bool right_empty(){
            return right == nullptr;
        }

        /// 计算深度，使用递归的DFS
        int depth(int base = 1){
            int left_max = base;
            int right_max = base;

            if(left)left_max = left->depth(base + 1);
            if(right)right_max = right->depth(base + 1);
            
            return std::max(left_max,right_max);
        }

        void clear_left(){
            if(left){
                left->clear_left();
                left->clear_right();

                delete left;
                left = nullptr;
            }else throw std::runtime_error("Left node is empty");
            // 我觉得还是干脆抛出异常吧，省心一点
        }
        
        void clear_right(){
            if(right){
                right->clear_left();
                right->clear_right();

                delete right;
                right = nullptr;
            }else throw std::runtime_error("Right node is empty");
        }
    };
}

#endif