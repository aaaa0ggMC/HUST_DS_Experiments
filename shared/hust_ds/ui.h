/**
 * @file ui.cpp
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 封装一下显示
 * @version 5.0
 * @date 2026-04-16
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef DS_SHARED_UI
#define DS_SHARED_UI
#include <clocale>
#include <cursesw.h>
#include <string_view>
#include <format>

struct NCursesContext{
    NCursesContext(){
        std::setlocale(LC_ALL, "");
        initscr();
        // 行缓冲,回显,特殊按键等等
        cbreak();
        noecho();
        keypad(stdscr,TRUE);
        curs_set(0);
    }
    ~NCursesContext(){
        endwin();
    }
};


/// @brief 这里我其实是借鉴了slidev/SFML的想法,就是虚拟画布
struct VirtualCanvas{
    float fixed_rows;
    float fixed_cols;
    WINDOW * win { stdscr };

    VirtualCanvas(float f_rows = 600,float f_cols = 800,WINDOW * win = stdscr)
    :fixed_cols(f_cols)
    ,fixed_rows(f_rows)
    ,win(win){
        assert(f_rows >= 0 && f_cols >= 0);
        assert(win);
    }

    void box(unsigned int a = 0,unsigned int b = 0){
        ::box(win,a,b);
    }

    std::pair<int,int> get_size(){
        int rows,cols;
        ::getmaxyx(win ,rows , cols);
        return {rows,cols};
    }
    int get_rows(){ return get_size().first; }
    int get_cols(){ return get_size().second; }

    std::pair<int,int> get_cursor_real(){
        int rows,cols;
        ::getyx(win ,rows , cols);
        return {rows,cols};
    }

    std::pair<float,float> get_cursor(){
        int rows,cols;
        ::getyx(win ,rows , cols);

        auto size = get_size();
        return {rows / (float)size.first * fixed_rows,cols / (float)size.second * fixed_cols};
    }


    void move(float row,float col){
        ::wmove(win,row / fixed_rows * get_rows(), col / fixed_cols * get_cols());
    }

    void move_real(int row,int col){
        ::wmove(win,row , col);
    }

    void display(){
        wrefresh(win);
    }

    template<class... Args>
    void print(std::string_view fmt,Args&&... args){
        if constexpr(sizeof...(Args) == 0){
            waddnstr(win,fmt.data(),fmt.size());
        }else{
            std::string str;
            // 处理一下可能出现的异常
            try{
                str = std::vformat(
                    fmt,
                    // 这里不完美转发的原因是make_format_args要求左值
                    std::make_format_args(args...) 
                );
            }catch(...){
                str += "[FMT ERROR]";
                str += fmt;
            }

            waddnstr(win,str.data(),str.size());
        }
    }

    template<class... Args>
    void mvprint(float rows,float cols,std::string_view str,Args&&... args){
        this->move(rows,cols);
        // 我们自己的print是左右通吃的,因此直接pass过去
        print(str,std::forward<Args>(args)...); 
    }

    template<class... Args>
    void mvprint_real(int rows,int cols,std::string_view str,Args&&... args){
        this->move_real(rows,cols);
        // 我们自己的print是左右通吃的,因此直接pass过去
        print(str,std::forward<Args>(args)...); 
    }
};

#endif
