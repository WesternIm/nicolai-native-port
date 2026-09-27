#include "nicolai/legacy_runtime_state.hpp"
#include <iostream>
#include <sstream>
#include <string>

int main() {
    const char* paths[]{"invalid", "dropped", "cross", "initial", "ordinary", "deferred"};
    std::string line;
    int records = 0;
    while (std::getline(std::cin, line)) {
        std::istringstream input(line);
        int count, index, nodes, cross, started;
        std::string extra;
        if (!(input >> count >> index >> nodes >> cross >> started) || input >> extra ||
            (cross != 0 && cross != 1) || (started != 0 && started != 1)) return 2;
        const auto route = nicolai::legacy_runtime_route_m36(count, index, nodes, cross != 0, started != 0);
        std::cout << paths[static_cast<int>(route.path)] << '\n';
        ++records;
    }
    return records ? 0 : 2;
}
