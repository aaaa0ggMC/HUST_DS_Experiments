/**
 * @file sq_list.h
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 顺序表的ADT实现
 * @version 5.0
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef HUST_DS_SQ_LIST_H
#define HUST_DS_SQ_LIST_H
#include <alib5/ds/vector.h>
#include <alib5/aalgorithm.h>

namespace hust_ds::sq_list{
    using namespace alib5::ds;
    constexpr VectorTraits traits = [](){
        VectorTraits traits;
        // 1-based index support
        traits.offset = -1;
        return traits;
    }();

    using Type = int;
    using Allocator = std::allocator<int>;
    using Container = Vector<int,traits,Allocator>;
    using List = Container*;

    enum class Status : int {
        Infeasible = -1,
        OK = 0,
        Error = 1,
    };

    inline Status InitList(List& list){
        using enum Status;
        if(list)return Infeasible;
        list = new Container();
        return OK;
    }

    inline Status DestroyList(List& list){
        using enum Status;
        if(!list)return Infeasible;
        delete list;
        list = nullptr;
        return OK;
    }

    inline Status ClearList(List& list){
        using enum Status;
        if(!list)return Infeasible;
        list->clear();
        return OK;
    }

    inline std::pair<bool,Status> ListEmpty(List & list){
        using enum Status;
        if(!list)return {false,Infeasible};
        return {list->empty(),OK};
    }

    inline int ListLength(List & list){
        if(!list)return -1;
        return static_cast<int>(list->length());
    }

    inline Status GetElem(List & list,int i,Type & e){
        using enum Status;
        if(!list)return Infeasible;
        Type * ptr = list->at(i);
        if(!ptr)return Error;
        e = *ptr;
        return OK;
    }

    template<class CompareFn>
    inline int LocateElem(List & list,Type e,CompareFn && compare){
        if(!list)return 0;
        Type * ptr = list->find(e,std::forward<CompareFn>(compare));
        if(ptr == list->end())return 0;
        // 反算 1-based index: ptr - begin() - offset
        return static_cast<int>(ptr - list->begin() - traits.offset);
    }

    inline Status PriorElem(List & list,Type cur_e,Type & pre_e){
        using enum Status;
        if(!list)return Infeasible;
        // 查找第一个匹配项
        Type * ptr = list->find(cur_e,[](const Type& a,const Type& b){return a == b;});
        if(ptr == list->end() || ptr == list->begin())return Error;
        pre_e = *(ptr - 1);
        return OK;
    }

    inline Status NextElem(List & list,Type cur_e,Type & next_e){
        using enum Status;
        if(!list)return Infeasible;
        Type * ptr = list->find(cur_e,[](const Type& a,const Type& b){return a == b;});
        if(ptr == list->end() || ptr + 1 == list->end())return Error;
        next_e = *(ptr + 1);
        return OK;
    }

    inline Status ListInsert(List & list,int i,Type e){
        using enum Status;
        if(!list)return Infeasible;
        // Vector内部会处理 offset 并在越界时 panic 或 throw
        // 1-based index: i=1 对应 offset=-1 后的 0
        list->insert(i,e);
        return OK;
    }

    inline Status ListDelete(List & list,int i,Type & e){
        using enum Status;
        if(!list)return Infeasible;
        if(i < 1 || i > (int)list->length())return Error;
        e = list->remove(i);
        return OK;
    }

    template<class VisitFn>
    inline Status ListTraverse(List & list,VisitFn && visit){
        using enum Status;
        if(!list)return Infeasible;
        for(auto & element : *list){
            visit(element);
        }
        return OK;
    }

    inline int MaxSubArray(List& list){
        if(!list || list->empty())return 0;

        // 不断计算累加和
        int max_val = (*list)[1];
        int curr_max = (*list)[1];
        for(size_t i = 2;i <= list->length();++i){
            // 如果list[i] < 0那么此时我重新开始算的话收益反而更低
            // 因此应该保留，类似 -1 1 2 3 4 -1 5 
            // 到第二个-1时如果抛弃就只有-1了
            curr_max = std::max((*list)[i], curr_max + (*list)[i]);
            max_val = std::max(max_val, curr_max);
        }
        return max_val;
    }

    inline int SubArrayNum(List& list, int k){
        if(!list || list->empty()) return 0;
        size_t len = list->length();
        
        // 复用 Container 存储前缀和 P[0...len]
        // p[1] 对应 P[0] = 0, p[2] 对应 P[1]...
        Container p; p.resize(len + 1, 0);
        for(size_t i = 1; i <= len; ++i) 
            p[i+1] = p[i] + (*list)[i];
        
        int count = 0;
        // 双重循环找差值为 k 的两项
        for(size_t j = 1; j <= len; ++j)for(size_t i = 0; i < j; ++i)
            if(p[j+1] - p[i+1] == k){
                count++;
            }
        return count;
    }

    inline Status SortList(List& list){
        if(!list)return Status::Infeasible;
        // 使用希尔排序
        // 原因： 因为快排我这里面用到了std::vector模拟堆栈
        // 这在数据结构里面应该是鸡和蛋的问题，所以使用几乎无状态
        // 同时效率也不错的shell排序是更好的选择
        // 经过测试Knuth的时间复杂度在2^20左右的数据量下约为 O(N^1.1132945334842599)，因此还挺好
        // 对比之下快排(三数取中 + Hoare分区)是 O(N^1.0120299377159614)
        alib5::algo::sort::shell<alib5::algo::sort::ShellGapType::Knuth>(
            list->begin(),
            list->end(),
            std::less<Type>()
        );
        return Status::OK;
    }

    /// 文件特征 SEQL[Length][SizeofType][Data...]
    /// 这里的SizeofType是拿来校验的
    inline Status SaveList(List& list, const char* filename){
        if(!list)return Status::Infeasible;

        FILE* fp = fopen(filename, "wb");
        if(!fp)return Status::Error;
        // 特殊序列，只要是为了保证没有读错文件然后boom
        fwrite("SEQL", 1, 4, fp);
        
        size_t len = list->length();
        size_t sz = sizeof(Type);

        fwrite(&len, sizeof(size_t), 1, fp);
        fwrite(&sz, sizeof(size_t), 1, fp);
        fwrite(list->begin(), sz, len, fp);
        fclose(fp);
        return Status::OK;
    }

    inline Status LoadList(List& list, const char* filename){
        if(!list)return Status::Infeasible;

        FILE* fp = fopen(filename, "rb");
        if(!fp)return Status::Error;

        char magic[4]; 
        fread(magic, 1, 4, fp);
        if(memcmp(magic, "SEQL", 4) != 0){ 
            fclose(fp); 
            return Status::Error; 
        }

        size_t len, sz;
        
        fread(&len, sizeof(size_t), 1, fp);
        fread(&sz, sizeof(size_t), 1, fp);
        
        if(sz != sizeof(Type)){
            fclose(fp); 
            return Status::Error; 
        }
        
        list->clear(); 
        list->resize(len);
        
        fread(list->begin(), sz, len, fp);
        fclose(fp);
        return Status::OK;
    }
}

#endif