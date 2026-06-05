/**
 * @file fwd_list.h
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 单链表的ADT实现
 * @version 5.0
 * @date 2026-05-14
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef HUST_DS_FWD_LIST_H
#define HUST_DS_FWD_LIST_H
#include <alib5/ds/forward_list.h>
#include <cstdio>
#include <cstring>
#include <functional>

namespace hust_ds::fwd_list{
    using namespace alib5::ds;
    constexpr ForwardListTraits traits = [](){
        ForwardListTraits traits;
        // 1-based index support
        traits.offset = -1;
        return traits;
    }();

    using Type = int;
    using Allocator = std::allocator<int>;
    using Container = ForwardList<int,traits,Allocator>;
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
        int pos = 1;
        auto p = list->m_head;
        while(p){
            if(compare(p->data, e))return pos;
            p = p->next;
            pos++;
        }
        return 0;
    }

    inline Status PriorElem(List & list,Type cur_e,Type & pre_e){
        using enum Status;
        if(!list)return Infeasible;
        if(list->prior_elem(cur_e, pre_e))return OK;
        return Error;
    }

    inline Status NextElem(List & list,Type cur_e,Type & next_e){
        using enum Status;
        if(!list)return Infeasible;
        if(list->next_elem(cur_e, next_e))return OK;
        return Error;
    }

    inline Status ListInsert(List & list,int i,Type e){
        using enum Status;
        if(!list)return Infeasible;
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
        list->traverse(std::forward<VisitFn>(visit));
        return OK;
    }

    //// 附加功能包装
    
    inline Status ReverseList(List & list){
        using enum Status;
        if(!list)return Infeasible;
        list->reverse();
        return OK;
    }

    inline Status RemoveNthFromEnd(List & list, int n){
        using enum Status;
        if(!list)return Infeasible;
        list->remove_nth_from_end(n);
        return OK;
    }

    inline Status SortList(List& list){
        if(!list)return Status::Infeasible;
        list->sort(std::less<Type>());
        return Status::OK;
    }

    /// 文件特征 FWDL[Length][SizeofType][Data...]
    inline Status SaveList(List& list, const char* filename){
        if(!list)return Status::Infeasible;

        FILE* fp = fopen(filename, "wb");
        if(!fp)return Status::Error;
        
        fwrite("FWDL", 1, 4, fp);
        
        size_t len = list->length();
        size_t sz = sizeof(Type);

        fwrite(&len, sizeof(size_t), 1, fp);
        fwrite(&sz, sizeof(size_t), 1, fp);
        
        // 链表内存不连续，采用遍历的方式依次落盘
        list->traverse([&](const Type& data){
            fwrite(&data, sz, 1, fp);
        });
        
        fclose(fp);
        return Status::OK;
    }

    inline Status LoadList(List& list, const char* filename){
        if(!list)return Status::Infeasible;

        FILE* fp = fopen(filename, "rb");
        if(!fp)return Status::Error;

        char magic[4]; 
        fread(magic, 1, 4, fp);
        if(memcmp(magic, "FWDL", 4) != 0){ 
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
        
        // 为了使得插入维持在 O(N) 的时间复杂度，我们每次采用头插法 O(1)
        // 全部读取完毕后再利用纯指针逆置 reverse() 达到顺序加载，整体耗时仍为 O(N)
        for(size_t i = 0; i < len; ++i){
            Type element;
            fread(&element, sz, 1, fp);
            list->push_front(std::move(element));
        }
        list->reverse();

        fclose(fp);
        return Status::OK;
    }
}

#endif