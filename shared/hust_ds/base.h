#ifndef HUST_DS_BASE_H
#define HUST_DS_BASE_H

#ifndef USE_FOR_MY_OWN_LIBRARY
    #define AAAA0GGMC_DS_NAMESPACE ds
#else 
    #define AAAA0GGMC_DS_NAMESPACE alib5::ds
#endif

namespace AAAA0GGMC_DS_NAMESPACE {
    /// @brief 一些状态码
    enum class Status : int {
        OK,
        Error,
        Infeasible,
        Overflow,
        BadAlloc,
        OutOfBounds,
        NotFound,
        NoPrior,
        NoNext
    };

    /// @brief 类似Rust的处理,用于表示数据与状态(头歌实验里的版本数据和状态是耦合的)
    /// 因为我使用C++20,此时还没有std::expected,因此这里手写
    template<class T>
    struct Expected{
        /// 这里理论上是可以为引用的,只要我指定了,因此还是不错的
        T data;
        /// 错误码
        Status status;

        bool has_error(){
            return status != Status::OK;
        }

        Expected(Status s):data{},status(s){} 
        // 有数据那肯定OK啊
        Expected(T d):data(d),status(Status::OK){}

        /** 
            T是bool那么这里会不会产生歧义?暂时空着
            operator bool(){
                return status != Status::OK;
            }
        **/
    };

    /// 统一一下样式
    template<>
    struct Expected<void>{
        /// 错误码
        Status status;

        /// 支持隐式转换
        Expected(Status s):status(s){}

        bool has_error(){
            return status != Status::OK;
        }
    };

}

#endif