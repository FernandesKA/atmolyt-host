#include <iostream>
#include <cassert>

#include "test_connection_mock.h"
#include "peripheral/bme280.h"
#include "peripheral/peripheral_factory.h"

using namespace peripherals;
using namespace connections;

void test_bme280_initialization()
{
    test_connection_mock conn;
    conn.initialize();

    bme280 sensor(&conn, 0x76);
    peripherals::Status st = sensor.initialize();
    assert(st == peripherals::Status::Success || st == peripherals::Status::ErrorCommunication);
    std::cout << "✓ test_bme280_initialization passed" << std::endl;
}

void test_connection_mock_read()
{
    test_connection_mock conn;
    conn.initialize();
    assert(conn.is_ready());

    uint8_t buffer[4] = {0};
    connections::Status st = conn.read_register(0x76, 0xD0, std::span<uint8_t>(buffer, 1));
    assert(st == connections::Status::Success);
    assert(buffer[0] == 0x60); // BME280 ID
    std::cout << "✓ test_connection_mock_read passed" << std::endl;
}

void test_peripheral_factory()
{
    using peripheral_factory = peripherals::peripheral_factory;

    PeripheralType t1 = peripheral_factory::string_to_type("bme280");
    assert(t1 == PeripheralType::BME280);

    t1 = peripheral_factory::string_to_type("BMP280");
    assert(t1 == PeripheralType::BMP280);

    std::string s = peripheral_factory::type_to_string(PeripheralType::BME280);
    assert(s == "bme280");

    uint8_t addr = peripheral_factory::get_default_address(PeripheralType::BME280);
    assert(addr == 0x76);

    std::cout << "✓ test_peripheral_factory passed" << std::endl;
}

int main()
{
    try {
        test_connection_mock_read();
        test_peripheral_factory();
        test_bme280_initialization();
        std::cout << "\n✓ All tests passed!" << std::endl;
        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << "✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "✗ Test failed with unknown error" << std::endl;
        return 1;
    }
}
