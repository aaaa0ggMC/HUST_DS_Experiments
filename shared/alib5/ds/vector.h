/**
 * @file vector.h
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 顺序表
 * @version 5.0
 * @date 2026-04-30
 * 
 * @copyright Copyright (c) 2026
 * @par 顺序表介绍
 * 顺序表是物理地址连续的存储单元一次存储数据元素的线性结构,
 * 在c++里体现在 <vector> (动态) <inplace_vector>(c++26,静态)
 * <array> 静态(但是严格来讲不太能算有操作的数据结构)
 * 这里主要按照数据结构实验要求大致实现vector和inplace_vector
 */
#ifndef ALIB5_DS_VECTOR
#define ALIB5_DS_VECTOR
#include <alib5/ds/base.h>
#include <alib5/adebug.h>
#include <type_traits>
#include <cstring>
#include <memory>

namespace alib5::ds{

    template<
        class T,
        VectorTraits traits = VectorTraits(),
        class Allocator = std::allocator<T> 
    >
    struct Vector{
        /// 这里实现通用的分配逻辑，可以更加方便地处理内存
        using value_type = T;
        using AllocatorTraits = std::allocator_traits<Allocator>;
        using Alloc = typename AllocatorTraits::template rebind_alloc<T>;

        Alloc allocator;

        Vector(const Allocator & alloc = std::allocator<T>()):allocator(alloc){}
        Vector(Allocator && alloc):allocator(std::move(alloc)){}
    
        /// 预留空间，capacity可以小于当前的capacity，效果等于shrink
        void reserve(size_t capacity);

        /// 确保至少有这么多空间，不会shrink只会扩增
        void ensure(size_t capacity){
            if(capacity > this->m_capacity){
                reserve(capacity);
            }
        }

        /// 重新设置大小,会把超过的数据直接截断，不影响capacity
        /// 后面的args为对每个对象构造的参数
        /// 要求： args均为copyable
        template<class... Args>
        void resize(size_t size,Args... args);

        //// 数据量大小关系
        /// 数据大小
        size_t size() const { return m_length; }
        size_t length() const { return size(); }
        /// 数据容量
        size_t capacity() const { return m_capacity; }
        /// 是否为空
        bool empty() const { return !size(); }
        /// 是否为满
        bool full() const { return size() == m_capacity; }

        /// 遍历上
        T* begin(){ return m_data; }
        T* end(){ return m_data + m_length; }
        const T* begin() const { return m_data; }
        const T* end() const { return m_data + m_length; }
        const T* cbegin() const { return m_data; }
        const T* cend() const { return m_data + m_length; }

        //// 查找函数，因为存入的数据没啥特征，所以时间复杂度为O(N)
        /// 返回指针，如果查询不到返回end()
        /// 这是通用版本，要求的comparefn第一个参数为const T&,第二个能接受Tp
        template<class Tp,class CompareFn>
        requires requires(const T& data,Tp & val,CompareFn & fn){ 
            { fn(data,val) } -> std::convertible_to<bool>; 
        }
        const T* find(Tp && value,CompareFn && fn) const {
            for(const T& element : *this){
                if(fn(element,std::forward<Tp>(value)))return &element;
            }
            return end();
        }
        /// 非const版本
        template<class Tp,class CompareFn>
        requires requires(const T& data,Tp & val,CompareFn & fn){ 
            { fn(data,val) } -> std::convertible_to<bool>; 
        }
        T* find(Tp && value,CompareFn && fn){
            return const_cast<T*>((const Vector &)(*this).find(
                std::forward<Tp>(value),
                std::forward<CompareFn>(fn)
            ));
        }

        /// 元素加入 & 插入
        /// 返回插入的指针
        T* insert(int index,T && value){
            return _insert(index + traits.offset, std::move(value));
        }
        T* insert(int index,const T & value){
            T v = value;
            return insert(index,std::move(v));
        }
        /// 这里支持迭代器
        T* insert(T* it,T && value){
            return _insert(it - begin(),std::move(value));
        }
        T* insert(T* it,const T & value){
            T v = value;
            return insert(it,std::move(v));
        }
        /// 往后面添加元素 O(1) 操作
        T* push_back(T && value){
            return _insert(m_length,std::move(value));
        }
        T* push_back(const T& value){
            T v = value;
            return _insert(m_length,std::move(v)); 
        }
        /// 往前面添加元素 O(L) 操作
        T* push_front(T && value){
            return _insert(0,std::move(value));
        }
        T* push_front(const T& value){
            T v = value;
            return _insert(0,std::move(v)); 
        }

        /// 元素的删除
        void erase(int index){
            _erase(index + traits.offset);
        }
        void erase(T* it){
            _erase(it - begin());
        }
        T remove(int index){
            T data = std::move((*this)[index]);
            _erase(index + traits.offset);
            return std::move(data);
        }
        T remove(T * it){
            return std::move(remove(it - begin()));
        }
        T pop_back(){
            return std::move(remove(end() - 1));
        }
        /// O(L) 操作
        T pop_front(){
            return std::move(remove(begin()));
        }
        
        //// 元素访问
        /// 访问不存在的index的时候返回nullptr
        T* at(int index){
            return _at(traits.offset + index);
        }
        const T* at(int index) const {
            return _at(traits.offset + index);
        }
        /// 编写访问
        const T& operator[](int index) const {
            const T * ptr = at(index);
            if(ptr)return *ptr;
            _panic_bounds(index);
            // 这里绝对不会被访问到，但是还是显示声明一下
            return *ptr;
        }
        T& operator[](int index){
            return const_cast<T&>((const Vector &)(*this)[index]);
        }

        /// 这里都是基于0-based的方法，下划线开头
        const T* _at(int index) const {
            if(index < 0 || index >= m_length)return nullptr;
            return &m_data[index];
        }
        T* _at(int index){
            return const_cast<T*>((const Vector &)(*this)._at(
                index
            ));
        }
        // 插入新的元素
        T* _insert(int index,T && value);
        /// 删除对应位置的元素
        void _erase(int index);
        // index越界报错
        void _panic_bounds(int index){
            if constexpr(traits.throw_exception_instead_of_panic){
                throw std::out_of_range("Index out of bounds!");
            }else{
                panicf_if(index < 0,"The 0-based index({}) < 0.",index);
                panicf_if(index >= m_length,"The 0-based index({}) >= length({}).",index,m_length);
            }
        }

        /// @todo 也许我永远都不会做下面的几个功能了，等我用到了再做
        ///     1. assgin
        ///     2. insert

        /// 支持一下move
        Vector(Vector && vec){
            m_capacity = vec.m_capacity;
            m_data = vec.m_data;
            m_length = vec.m_length;

            vec.m_length = 0;
            vec.m_data = nullptr;
            vec.m_capacity = 0;
        }

        /// 支持一下copy
        Vector(const Vector & vec){
            reserve(vec.m_capacity);
            for(const T & element : vec){
                push_back(element);
            }
        }

        /// 析构函数，释放资源
        ~Vector(){
            if(m_data){
                if constexpr(!std::is_trivially_destructible_v<T>){
                    for(size_t i = 0;i < m_length;++i){
                        m_data[i].~T();
                    }
                }
                allocator.deallocate(m_data,m_capacity);
            }
        }
    private:
        /// 具体的数据
        T * m_data { nullptr };
        /// 数据预留大小
        size_t m_capacity { 0 };
        /// 当前数据的长度
        size_t m_length { 0 };
    };
}

namespace alib5::ds{
    template<
        class T,
        VectorTraits traits,
        class Allocator
    >
    template<
        class... Args
    >
    void Vector<T,traits,Allocator>::resize(size_t size,Args... args){
        ensure(size);
        // 如果非平凡，那么需要析构
        if constexpr(!std::is_trivial_v<T>){
            if(size < m_length){
                for(size_t i = size;i < m_length;++i){
                    m_data[i].~T();
                }
            }
        }
        if(size > m_length){
            for(size_t i = m_length;i < size;++i){
                // 因为args要构造这么多次，本身要求就是要copyable，不然这不炸了吗？
                new (m_data + i) T(args...);
            }
        }
        m_length = size;
    }

    template<
        class T,
        VectorTraits traits,
        class Allocator
    >
    void Vector<T,traits,Allocator>::reserve(size_t new_capacity){
        // 也就是说支持在当前大小往下面shrink
        if(new_capacity > m_length && new_capacity != m_capacity){
            T * new_data = allocator.allocate(new_capacity);
            
            /// 如果复制操作时平凡的，使用memcpy一般会比其他情况更好？
            if constexpr(std::is_trivially_copyable_v<value_type>){
                if(m_data)std::memcpy(new_data,m_data,sizeof(value_type) * m_length);
            }else{
                for(size_t i = 0;i < m_length;++i){
                    new (new_data + i) T(std::move(m_data[i]));
                }
                // 拆开不拆开一样的
                for(size_t i = 0;i < m_length;++i){
                    m_data[i].~T();
                }
            }
            allocator.deallocate(m_data,m_capacity);

            // re-bind
            m_data = new_data;
            m_capacity = new_capacity;
        }
    }

    template<
        class T,
        VectorTraits traits,
        class Allocator
    >
    void Vector<T,traits,Allocator>::_erase(int index){
        if(index < 0 || index > m_length) [[unlikely]] _panic_bounds(index);
        if(index == m_length)return;

        // 析构这个位置，然后move后面的
        // 对于pop_back这种可以先move出去然后析构也不是问题
        if constexpr(!std::is_trivially_destructible_v<T>){
            m_data[index].~T();
        }

        if(index + 1 != m_length){
            for(int i = index + 1;i < m_length;++i){
                new (m_data + i - 1) T(std::move(m_data[i]));
            }
            
            // 最后一个位置有一个未初始化的数据
            if constexpr(!std::is_trivially_destructible_v<T>){
                m_data[m_length - 1].~T();
            }
        }

        m_length -= 1;
    }

    template<
        class T,
        VectorTraits traits,
        class Allocator
    >
    T* Vector<T,traits,Allocator>::_insert(int index,T && value){
        /// 排除错误情况
        // 这里是允许m_length这个位置的出现的
        if(index < 0 || index > m_length) [[unlikely]] _panic_bounds(index);
        
        T * focus_data = m_data;
        /// 这里是记录扩容
        size_t new_capacity = 0;

        // 考虑到性能原因，扩容移动一起做而不是使用ensure
        if(index + 1 > m_capacity){
            new_capacity = std::max(
                traits.min_capacity,
                (size_t)index + 1
            ) * 2;
            
            T * new_data = allocator.allocate(new_capacity);

            // 对前面的数据进行迁移
            for(size_t i = 0;i < index;++i){
                new (new_data + i) T(std::move(m_data[i]));
            }
            
            m_data = new_data;
        }

        // 移动后面的数据
        for(int i = m_length - 1;i >= index; --i){
            new (m_data + i + 1) T(std::move(focus_data[i]));
        }

        new (m_data + index) T(std::move(value));
        m_length += 1;
        
        /// 这里是对于扩容的对象进行清理
        if(new_capacity) [[unlikely]] {
            // 析构focus_data旧的位置的数据
            if constexpr(!std::is_trivially_destructible_v<T>){
                // 前面已经加了个1可，因此这里减回来
                size_t old_length = m_length - 1;
                for(size_t i = 0;i < old_length;++i){
                    focus_data[i].~T();
                }
            }
            allocator.deallocate(focus_data,m_capacity);
            // 最后设置capacity
            m_capacity = new_capacity;
        }

        return m_data + index;
    }
}

#endif