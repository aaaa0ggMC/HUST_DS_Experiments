/**
 * @file forward_list.h
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 为了方便操作这里的工程很“裸”，因此实际上不适用于实际使用
 * @version 5.0
 * @date 2026-05-14
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef ALIB5_DS_FWD_LIST
#define ALIB5_DS_FWD_LIST
#include <alib5/ds/base.h>
#include <alib5/adebug.h>
#include <memory>
#include <utility>
#include <stdexcept>
#include <concepts>

namespace alib5::ds{

    template<
        class T,
        ForwardListTraits traits = ForwardListTraits(),
        class Allocator = std::allocator<T>
    >
    class ForwardList{
    public:
        using value_type = T;

        struct Node {
            T data;
            Node* next;
            template<class... Args>
            Node(Args&&... args) : data(std::forward<Args>(args)...), next(nullptr) {}
        };

        using AllocatorTraits = std::allocator_traits<Allocator>;
        using NodeAlloc = typename AllocatorTraits::template rebind_alloc<Node>;

        NodeAlloc allocator;
        Node* m_head { nullptr };

    private:
        template<class... Args>
        Node* create_node(Args&&... args){
            Node* node = allocator.allocate(1);
            std::allocator_traits<NodeAlloc>::construct(allocator, node, std::forward<Args>(args)...);
            return node;
        }

        void destroy_node(Node* node){
            std::allocator_traits<NodeAlloc>::destroy(allocator, node);
            allocator.deallocate(node, 1);
        }
    public:
        ForwardList(const Allocator& alloc = Allocator()) : allocator(alloc) {}
        ForwardList(Allocator&& alloc) : allocator(std::move(alloc)) {}

        ~ForwardList(){
            clear();
        }

        /// 不断往下探，情况后面的数据
        void clear(){
            Node* curr = m_head;
            while(curr){
                Node* next = curr->next;
                destroy_node(curr);
                curr = next;
            }
            m_head = nullptr;
        }

        bool empty() const {
            return m_head == nullptr;
        }

        size_t size() const {
            size_t len = 0;
            for (Node* p = m_head; p; p = p->next) {
                ++len;
            }
            return len;
        }
        size_t length() const { return size(); }

        T* at(int index){
            return _at(index + traits.offset);
        }
        const T* at(int index) const {
            return _at(index + traits.offset);
        }

        const T& operator[](int index) const {
            const T* ptr = at(index);
            if (ptr) return *ptr;
            _panic_bounds(index + traits.offset);
            return *ptr;
        }
        T& operator[](int index){
            return const_cast<T&>(((const ForwardList&)(*this))[index]);
        }

        /// 基于 0-based 的内部访问
        const T* _at(int index) const {
            if (index < 0) return nullptr;
            Node* p = m_head;
            for (int i = 0; i < index && p; ++i) p = p->next;
            return p ? &(p->data) : nullptr;
        }
        T* _at(int index){
            return const_cast<T*>(((const ForwardList&)(*this))._at(index));
        }

        template<class Tp, class CompareFn>
        requires requires(const T& data, Tp& val, CompareFn& fn){ 
            { fn(data, val) } -> std::convertible_to<bool>; 
        }
        const T* find(Tp&& value, CompareFn&& fn) const {
            for (Node* p = m_head; p; p = p->next) {
                if (fn(p->data, std::forward<Tp>(value))) return &(p->data);
            }
            return nullptr;
        }
        template<class Tp, class CompareFn>
        T* find(Tp&& value, CompareFn&& fn) {
            return const_cast<T*>(((const ForwardList&)(*this)).find<Tp>(
                std::forward<Tp>(value),
                std::forward<CompareFn>(fn)
            ));
        }

        /// 获得前驱
        bool prior_elem(const T& cur_e, T& pre_e) const {
            if (!m_head || !m_head->next) return false;
            Node* p = m_head;
            while (p->next) {
                if (p->next->data == cur_e) {
                    pre_e = p->data;
                    return true;
                }
                p = p->next;
            }
            return false;
        }

        /// 获得后继 NextElem
        bool next_elem(const T& cur_e, T& next_e) const {
            Node* p = m_head;
            while(p && p->next){
                if(p->data == cur_e){
                    next_e = p->next->data;
                    return true;
                }
                p = p->next;
            }
            return false;
        }

        /// 插入元素 ListInsert (应用 offset)
        Node* insert(int index, T&& value){
            return _insert(index + traits.offset, std::move(value));
        }
        Node* insert(int index, const T& value){
            T v = value;
            return insert(index, std::move(v));
        }

        /// 往头部和尾部添加元素的便捷方法
        Node* push_front(T&& value){ return _insert(0, std::move(value)); }
        Node* push_front(const T& value){ T v = value; return _insert(0, std::move(v)); }

        T remove(int index){
            return _erase(index + traits.offset);
        }
        T pop_front(){
            return _erase(0);
        }

        template<class VisitFn>
        void traverse(VisitFn&& visit) const {
            for (Node* p = m_head; p; p = p->next) {
                visit(p->data);
            }
        }

        void reverse(){
            Node* prev = nullptr;
            Node* curr = m_head;
            while (curr) {
                Node* next = curr->next;
                curr->next = prev;
                prev = curr;
                curr = next;
            }
            m_head = prev;
        }

        void remove_nth_from_end(int n){
            if (n <= 0 || !m_head) return;
            Node** slow = &m_head;
            // 记录“尾部”
            Node* fast = m_head;
            for(int i = 0; i < n; ++i){
                if (!fast) return; // 不足 n 个
                fast = fast->next;
            }
            while(fast){
                fast = fast->next;
                slow = &((*slow)->next);
            }
            Node* to_delete = *slow;
            *slow = to_delete->next;
            destroy_node(to_delete);
        }

        /// 链表排序 sortList (Bubble Sort 纯交换指针区域)
        template<class CompareFn>
        void sort(CompareFn&& cmp){
            if(!m_head || !m_head->next) return;
            bool swapped;
            Node* end_node = nullptr;
            do{
                swapped = false;
                Node** curr = &m_head;
                
                while(*curr && (*curr)->next && (*curr)->next != end_node){
                    Node* a = *curr;
                    Node* b = a->next;
                    
                    if(cmp(b->data, a->data)){ 
                        // 断开原链接，重新连接达到交换效果，不移动 data
                        a->next = b->next;
                        b->next = a;
                        *curr = b;
                        swapped = true;
                    }
                    curr = &((*curr)->next);
                }
                end_node = *curr;
            } while (swapped);
        }

        Node* _insert(int index, T&& value);
        T _erase(int index);

        void _panic_bounds(int index) const {
            if constexpr(traits.throw_exception_instead_of_panic){
                throw std::out_of_range("Index out of bounds!");
            }else{
                panicf_if(index < 0, "The 0-based index({}) < 0.", index);
                panicf_if(true, "The 0-based index({}) is out of bounds.", index);
            }
        }
    };
}

namespace alib5::ds{
    template<
        class T,
        ForwardListTraits traits,
        class Allocator
    >
    typename ForwardList<T, traits, Allocator>::Node* 
    ForwardList<T, traits, Allocator>::_insert(int index, T&& value){
        if(index < 0) [[unlikely]] _panic_bounds(index);
        
        if(index == 0){
            Node* new_node = create_node(std::move(value));
            new_node->next = m_head;
            m_head = new_node;
            return m_head;
        }

        Node* p = m_head;
        for(int i = 0; i < index - 1 && p; ++i){
            p = p->next;
        }
        
        if(!p) [[unlikely]] _panic_bounds(index);

        Node* new_node = create_node(std::move(value));
        new_node->next = p->next;
        p->next = new_node;
        return new_node;
    }

    template<
        class T,
        ForwardListTraits traits,
        class Allocator
    >
    T ForwardList<T, traits, Allocator>::_erase(int index) {
        if(!m_head || index < 0) [[unlikely]] _panic_bounds(index);

        if(index == 0){
            Node* to_delete = m_head;
            m_head = m_head->next;
            T data = std::move(to_delete->data);
            destroy_node(to_delete);
            return data;
        }

        Node* p = m_head;
        for(int i = 0; i < index - 1 && p->next; ++i){
            p = p->next;
        }

        if(!p->next) [[unlikely]] _panic_bounds(index);

        Node* to_delete = p->next;
        p->next = to_delete->next;
        T data = std::move(to_delete->data);
        destroy_node(to_delete);
        
        return data;
    }

}
#endif