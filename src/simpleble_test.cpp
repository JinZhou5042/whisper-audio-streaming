#include "simpleble_test.hpp"

int test_simpleble() {
    if (!SimpleBLE::Adapter::bluetooth_enabled()) {
        std::cout << "Bluetooth is not enabled" << std::endl << std::flush;
    } else {
        std::cout << "Bluetooth is enabled" << std::endl << std::flush;
    }
    return 0;
}