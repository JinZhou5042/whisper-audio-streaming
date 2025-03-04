#include <iostream>
#include <vector>

#include <simpleble/Adapter.h>
#include <simpleble/Characteristic.h>
#include <simpleble/Descriptor.h>
#include <simpleble/Peripheral.h>
#include <simpleble/Service.h>
#include <simpleble/SimpleBLE.h>

int test_simpleble() {
  // Check if bluetooth is enabled
  if (!SimpleBLE::Adapter::bluetooth_enabled()) {
    std::cout << "Bluetooth is not enabled" << std::endl;
    return 1;
  } else {
    std::cout << "Bluetooth is enabled" << std::endl;
  }

  // Check if there's adapters
  auto adapters = SimpleBLE::Adapter::get_adapters();
  if (adapters.empty()) {
    std::cout << "No Bluetooth adapters found" << std::endl;
    return 1;
  } else {
    std::cout << "Number of adapters: " << std::size(adapters) << std::endl;
  }

  // Use the first adapter, get its info
  auto adapter = adapters[0];
  std::cout << "Adapter identifier: " << adapter.identifier() << std::endl;
  std::cout << "Adapter address: " << adapter.address() << std::endl;

  // Set the callback to be called when the scan starts
  adapter.set_callback_on_scan_start([]() {
     std::cout << "Scan started" << std::endl;
  });

  // Set the callback to be called when the scan stops
  adapter.set_callback_on_scan_stop([]() {
     std::cout << "Scan stopped" << std::endl;
  });

  // Set the callback to be called when the scan finds a new peripheral
  std::vector<SimpleBLE::Peripheral> peripherals;
  adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral) {
      std::cout << "Found device: " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
      if (peripheral.is_connectable()) {
          peripherals.push_back(peripheral);
      }
  });

  // Set the callback to be called when a peripheral property has changed
  adapter.set_callback_on_scan_updated([](SimpleBLE::Peripheral peripheral) {
     std::cout << "Peripheral updated: " << peripheral.identifier() << std::endl;
  });

  // Scan for peripheral for 5000ms
  adapter.scan_for(5000);

  // Print out all peripherals
  for (size_t i = 0; i < peripherals.size(); i++) {
    std::cout << "[" << i << "] " << peripherals[i].identifier() << " [" << peripherals[i].address() << "]" << std::endl;
  }

  return 0;
}

