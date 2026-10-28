#include <iostream>

#include "fastkv/fastkv.hpp"

int main() {
    fastkv::FastKV kv;

    kv.put("name", "isok");
    std::cout << kv.get("name").value_or("<absent>") << "\n";
    std::cout << kv.get("missing").value_or("<absent>") << "\n";

    return 0;
}
