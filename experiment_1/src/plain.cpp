#include <iostream>
#include <alib5/ds/vector.h>

static int active_objects = 0;

struct Monitor {
    int id;
    Monitor(int i = 0) : id(i) { active_objects++; }
    Monitor(const Monitor& other) : id(other.id) { active_objects++; }
    Monitor(Monitor&& other) noexcept : id(other.id) { active_objects++; }
    ~Monitor() { active_objects--; }
    Monitor& operator=(const Monitor&) = default;
};

int main() {
    using namespace alib5::ds;
    
    try {
        std::cout << "--- Test 1: Basic Push & Resize ---" << std::endl;
        Vector<int> vec;
        vec.push_back(10);
        vec.push_back(20);
        vec.resize(5, 100); // 扩容并填充100
        
        for(int x : vec) std::cout << x << " "; // 预期: 10 20 100 100 100
        std::cout << "\nSize: " << vec.size() << " Capacity: " << vec.capacity() << std::endl;

        std::cout << "\n--- Test 2: Object Lifecycle (The Real Test) ---" << std::endl;
        {
            Vector<Monitor> v_obj;
            v_obj.reserve(10);
            v_obj.push_back(Monitor(1));
            v_obj.push_back(Monitor(2));
            std::cout << "Active objects after push: " << active_objects << std::endl;
            
            v_obj._erase(0); // 测试删除第一个元素
            std::cout << "Active objects after erase(0): " << active_objects << std::endl;
            
            v_obj.resize(0); // 清空
            std::cout << "Active objects after resize(0): " << active_objects << std::endl;
        }
        std::cout << "Active objects after Vector destroyed: " << active_objects << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}