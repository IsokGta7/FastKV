#include <iostream>
#include "../include/fastkv/fastkv.hpp"

int main() {
    fastkv::FastKV kv;

    kv.put("name", "isok");
    std::cout << kv.get("name") << "\n";

    return 0;
}
