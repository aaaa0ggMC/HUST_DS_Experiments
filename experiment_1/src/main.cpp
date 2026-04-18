/**
 * @file main.cpp
 * @author aaaa0ggmc (lovelinux@yslwd.eu.org)
 * @brief 尝试把这几次试验用到的"前端"写出来?
 * @version 5.0
 * @date 2026-04-16
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <hust_ds/ui.h>
#include <functional>
#include <vector>

#define TERMINATED -1
#define SAFE_EXIT -2

int main(){
    NCursesContext context;

    // 一些坐标
    int c0 = LINES * 0.6;
    int c1 = LINES * 0.4;
    int c2 = COLS * 0.6;
    int c3 = COLS * 0.4;
    
    WINDOW * _status_win_bordered = subwin(stdscr,c0,COLS,0,0);
    WINDOW * _operations_win_bordered = subwin(stdscr,c1,c2,c0,0);
    WINDOW * _output_win_bordered = subwin(stdscr, c1, c3, c0, c2);
    VirtualCanvas bordered_status_win(600,800,_status_win_bordered);
    VirtualCanvas bordered_operations_win(600,800,_operations_win_bordered);
    VirtualCanvas bordered_output_win(600,800,_output_win_bordered);

    WINDOW * _status_win = derwin(
        _status_win_bordered,
        bordered_status_win.get_size().first - 2,
        bordered_status_win.get_size().second - 2,
        1,1
    );
    WINDOW * _operations_win = derwin(
        _operations_win_bordered,
        bordered_operations_win.get_size().first - 2,
        bordered_operations_win.get_size().second - 2,
        1,1
    );
    WINDOW * _output_win = derwin(
        _output_win_bordered,
        bordered_output_win.get_size().first - 2,
        bordered_output_win.get_size().second - 2,
        1,1
    );
    VirtualCanvas status_win(600,800,_status_win);
    VirtualCanvas operations_win(600,800,_operations_win);
    VirtualCanvas output_win(600,800,_output_win);

    struct Calls{
        std::string name;
        std::function<int(void)> caller;
    };
    std::vector<Calls> menu;
    /////////  这一块就是我在这里手动添加menu,虽然可以通过类包装,但是程序不叫小,因此我倾向于闭包 ///////
    std::vector<int> data;

    menu.emplace_back("Init List",
        []{
            return TERMINATED;
        }
    );

    menu.emplace_back("Exit",
        []{
            return SAFE_EXIT;
        }
    );

    ////////// ENDDDD //////

    int focusing = 0;
    int entered = -1;
    while(true){
        bordered_status_win.box();
        bordered_status_win.mvprint(0,0,"Status");
        bordered_operations_win.box();
        bordered_operations_win.mvprint(0,0,"Operations For Linear Table On Sequence Structure");
        bordered_output_win.box();
        bordered_output_win.mvprint(0,0,"Outputs");

        if(entered >= 0){
            if(auto exit_code = menu[entered].caller()){
                if(exit_code == TERMINATED)entered = -1;
                else if(exit_code == SAFE_EXIT){
                    return 0;
                }else return exit_code;
            }else focusing = entered;
        }else{
            /// Draw Menu Part
            operations_win.move(0, 0);
            for(size_t i = 0;i < menu.size();++i){
                operations_win.print("{}{}\n",
                        (focusing == i) ? "> " : "  "
                    ,menu[i].name);
            }
            operations_win.display();
        }

        bordered_status_win.display();
        bordered_operations_win.display();
        bordered_output_win.display();
    
        switch(getch()){
        case KEY_UP:
            --focusing;
            if(focusing < 0)focusing = menu.size() - 1;
            break;
        case KEY_DOWN:
            ++focusing;
            if(focusing >= menu.size())focusing = 0;
            break;
        case '\r':
        case '\n':
        case KEY_ENTER:
            entered = focusing;
            break;
        }
    }
    return 0;
}