/**
 * @file traits.h
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 基础特性
 * @version 5.0
 * @date 2026-04-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef ALIB5_DS_TRAITS
#define ALIB5_DS_TRAITS
#include <type_traits>

namespace alib5::ds{
    /// 所有数据结构共有的traits
    struct BasicTraits{
        /// 发生错误的时候是否抛出异常
        bool throw_exception_instead_of_panic;

        constexpr BasicTraits(){
            throw_exception_instead_of_panic = false;
        }
    };

    
    /// 所有线性表共有的访问逻辑
    struct LinearListTraits : public BasicTraits{
        /// 偏移量，一般是0,也就是0-based访问，也可以是1-based
        /// 默认值：0 ，如果选择1-based,值为-1
        int offset;

        constexpr LinearListTraits(){
            offset = 0;
        }
    };

    /// 动态顺序表的traits
    struct VectorTraits : public LinearListTraits{
        /// 初始化/发生扩容的时候最小大小
        /// 用户可以硬性设置capacity为0，不过扩容的时候会选取max(min_capacity,capacity)
        /// 默认值: 4
        std::size_t min_capacity;

        constexpr VectorTraits(){
            min_capacity = 4;
        }
    };

    /// 链表traits
    struct ForwardListTraits : public LinearListTraits {
    };
};

#endif