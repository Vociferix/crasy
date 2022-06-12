#include <crasy/crasy.hpp>

#include <iostream>

crasy::future<void> crasy_main() {
    std::cout << "yay!\n";
    co_return;
}

int main() {
    crasy::executor().block_on(crasy_main);
    return 0;
}
