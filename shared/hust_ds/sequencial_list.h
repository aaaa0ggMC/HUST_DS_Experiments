/**
 * @file sequencial_list.h
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 顺序表
 * @version 5.0
 * @date 2026-04-18
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef HUST_DS_SEQUENCIAL_LIST
#define HUST_DS_SEQUENCIAL_LIST
#include "base.h"
#include <cstdlib>
#include <concepts>

namespace AAAA0GGMC_DS_NAMESPACE {
    constexpr size_t INIT_LIST_DEFAULT_SIZE = 64;

    /// 是否支持比较
    template<class Fn,class T>
    concept IsCompareFn = requires(Fn & fn,const T & a,const T & b){
        { fn(a,b) } -> std::convertible_to<bool>;
    };

    /// @brief 一些特质信息,也许可以搭配出很多种组合,哈哈
    struct SequencialListTraits{
        /// @brief 自动扩容
        ///  如果为false,那么Status会返回Overflow
        bool auto_expand { false };
        /// @brief 自动初始化list并且自动删除list
        bool enable_raii { false };
        /// @brief 索引偏移,一般选择0 / 1就够了
        int offset { 1 };
        /// @brief 获取前驱的策略
        enum class PriorStrategy{
            StopAtFirst, ///< 遇到第一个就停止,也就是如果data[0]匹配,那么直接NotFound而不是接着查找
            StopAtValid ///< 遇到第一个有效数据,也就是data[0]匹配不会返回notfound而是接着查询
        };
        PriorStrategy prior_strategy { PriorStrategy::StopAtFirst };
    };

    /// 默认配置
    constexpr SequencialListTraits default_sequencial_list_traits = {
        .auto_expand = false,
        .enable_raii = false,
        .offset = 1,
        .prior_strategy = SequencialListTraits::PriorStrategy::StopAtFirst
    };


    /// @brief 本身结构很简单,因此全inline了
    template<class T,SequencialListTraits traits = default_sequencial_list_traits >
    struct SequencialList{
    private:
        using enum Status;
        friend class _ForceVisit;

        /// @brief 这个是内部存储的数据
        T * data { nullptr };
        /// @brief 数据大小
        size_t length { 0 };
        /// @brief 数据容量
        size_t capacity { 0 };
    
    public:
        /// @brief 数据类型
        using value_type = T;
        /// @brief 位序类型,使用int是为了适应更多的offset
        using index_type = int;
    
    
        SequencialList(size_t capacity = INIT_LIST_DEFAULT_SIZE){
            if constexpr(traits.enable_raii){
                Init(capacity);
            }
        }
        ~SequencialList(){
            if constexpr(traits.enable_raii){
                if(data)Destroy();
            }
        }

        /// 初始化list
        Expected<void> Init(size_t capacity = INIT_LIST_DEFAULT_SIZE){
            if(!data) [[likely]] {
                // 这里只是分配了内存,实际上,后面我们使用的时候应该要手动
                // 进行 T() 和 ~T(),这个是通用性并且还要使用malloc的代价
                data = (value_type *)std::malloc(sizeof(value_type) * capacity);
                if(!data)return BadAlloc; 
                length = 0;
                this->capacity = capacity; 
                return OK;  
            }else return Infeasible; 
        }

        /// 销毁list
        Expected<void> Destroy(){
            if(data) [[likely]] {
                /// 清理资源
                Clear();
                /// 删除资源
                free(data);
                data = nullptr;
                capacity = 0;
                return OK;
            }else return Infeasible;
        }

        /// 清除list
        Expected<void> Clear(){
            if(data) [[likely]] {
                /// 这里需要对所有成员执行~T()
                if constexpr(requires(value_type & t){ t.~value_type(); }){
                    for(size_t i = 0;i < length;++i){
                        data[i].~value_type();
                    }
                }
                length = 0;
            }else return Infeasible;
        }

        /// 是否为空
        Expected<bool> Empty(){
            if(data) [[likely]] {
                return (length != 0);
            }else return Infeasible;
        }

        /// 数组长度
        Expected<size_t> Length(){
            if(data) [[likely]] {
                return length;
            }else return Infeasible;
        }

        /// 获取对应位序的元素
        Expected<void> GetElem(int outer_index,T & output){
            if(data) [[likely]] {
                // 转换成内部的索引顺序
                int index = _get_0_based(outer_index);
                if(index < 0 || index >= length){
                    return OutOfBounds;
                }
                output = data[index];
                return OK;
            }else return Infeasible;
        }

        /// 定位元素
        template<IsCompareFn<value_type> CompareFn >
        Expected<int> LocateElem(
            const value_type & input,
            CompareFn && compare
        ){
            if(data) [[likely]] {
                for(int i = 0; i < length; ++i){
                    if(compare(input,data[i])){
                        return _get_x_based(i);
                    }
                }
                return NotFound;
            }else return Infeasible;
        }

        /// 获得前驱
        Expected<void> Prior(const value_type & input,value_type & prior){
            if(data) [[likely]] { 
                Status status = NotFound;
                if(length >= 1 && data[0] == input){
                    if constexpr(
                        traits.prior_strategy == SequencialListTraits::PriorStrategy::StopAtFirst
                    ){ // Stop At First策略
                        return NoPrior;
                    }else{ // Stop At Valid策略
                        status = NoPrior;
                    }
                }

                for(size_t i = 1;i < length;++i){
                    if(data[i] == input){
                        prior = data[i - 1];
                        return OK;
                    }
                }
                return status;
            }else return Infeasible;
        }

        /// 获取后继
        Expected<void> Next(const value_type & input,value_type & next){
            if(data) [[likely]] { 
                for(size_t i = 0;i < length - 1;++i){
                    if(data[i] == input){
                        next = data[i + 1];
                        return OK;
                    }
                }
                if(length > 0 && data[length - 1] == input){
                    return NoNext;
                }
                return NotFound;
            }else return Infeasible;
        }



        /// @brief 返回 0-based 的索引
        int _get_0_based(int index){
            return index - traits.offset;
        }
        /// @brief 返回特定based的索引
        int _get_x_based(int index){
            return index + traits.offset;
        }

        /// @brief 测试使用
        struct _ForceVisit{
            T** data;
            size_t * length;
            size_t * capacity;

            _ForceVisit(SequencialList & l)
            :data(&l.data)
            ,length(&l.length)
            ,capacity(&l.capacity){}
        };
        _ForceVisit _debug_force_visit(){
            return ForceVisit(*this);
        }
    };

};



#endif