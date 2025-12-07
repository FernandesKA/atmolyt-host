#include "peripheral/bme280.h"
#include <array>
#include <cstring>
#include <cmath>

namespace peripherals {

static constexpr uint8_t REG_ID = 0xD0;
static constexpr uint8_t REG_RESET = 0xE0;
static constexpr uint8_t REG_CTRL_HUM = 0xF2;
static constexpr uint8_t REG_STATUS = 0xF3;
static constexpr uint8_t REG_CTRL_MEAS = 0xF4;
static constexpr uint8_t REG_CONFIG = 0xF5;
static constexpr uint8_t REG_DATA = 0xF7; // pressure(3) + temp(3) + hum(2)

bme280::bme280(connections::addressable_connection_iface<uint8_t> *conn, uint8_t address)
    : environmental_sensor_iface(conn, address)
{
}

Status bme280::initialize()
{
    if (!connection_) return Status::ErrorNotInitialized;
    if (connection_->read_register(device_address_, REG_ID, std::span<uint8_t>(new uint8_t[1],1)) != connections::Status::Success)
        ; // fallthrough, we'll read properly below

    if (!read_calibration())
        return Status::ErrorCommunication;

    // set humidity oversampling = 1
    uint8_t val = 0x01;
    connection_->write_register(device_address_, REG_CTRL_HUM, std::span<const uint8_t>(&val, 1));
    // set ctrl_meas: temp and press oversampling = 1, mode = normal
    val = (0x01 << 5) | (0x01 << 2) | 0x03;
    connection_->write_register(device_address_, REG_CTRL_MEAS, std::span<const uint8_t>(&val, 1));

    initialized_ = true;
    return Status::Success;
}

void bme280::deinitialize()
{
    initialized_ = false;
}

bool bme280::is_connected()
{
    uint8_t id = 0;
    if (connection_->read_register(device_address_, REG_ID, std::span<uint8_t>(&id,1)) != connections::Status::Success)
        return false;
    return (id == 0x60 || id == 0x58); // 0x60 = BME280, 0x58 = BMP280
}

Status bme280::reset()
{
    uint8_t val = 0xB6;
    if (connection_->write_register(device_address_, REG_RESET, std::span<const uint8_t>(&val,1)) != connections::Status::Success)
        return Status::ErrorCommunication;
    return Status::Success;
}

bool bme280::read_calibration()
{
    // read 0x88..0xA1 and 0xE1..0xE7 blocks
    std::array<uint8_t, 26> buf1{};
    if (connection_->read_register(device_address_, 0x88, std::span<uint8_t>(buf1.data(), buf1.size())) != connections::Status::Success)
        return false;

    dig_T1 = uint16_t(buf1[0]) | (uint16_t(buf1[1]) << 8);
    dig_T2 = int16_t(uint16_t(buf1[2]) | (uint16_t(buf1[3]) << 8));
    dig_T3 = int16_t(uint16_t(buf1[4]) | (uint16_t(buf1[5]) << 8));

    dig_P1 = uint16_t(buf1[6]) | (uint16_t(buf1[7]) << 8);
    dig_P2 = int16_t(uint16_t(buf1[8]) | (uint16_t(buf1[9]) << 8));
    dig_P3 = int16_t(uint16_t(buf1[10]) | (uint16_t(buf1[11]) << 8));
    dig_P4 = int16_t(uint16_t(buf1[12]) | (uint16_t(buf1[13]) << 8));
    dig_P5 = int16_t(uint16_t(buf1[14]) | (uint16_t(buf1[15]) << 8));
    dig_P6 = int16_t(uint16_t(buf1[16]) | (uint16_t(buf1[17]) << 8));
    dig_P7 = int16_t(uint16_t(buf1[18]) | (uint16_t(buf1[19]) << 8));
    dig_P8 = int16_t(uint16_t(buf1[20]) | (uint16_t(buf1[21]) << 8));
    dig_P9 = int16_t(uint16_t(buf1[22]) | (uint16_t(buf1[23]) << 8));
    dig_H1 = buf1[25];

    std::array<uint8_t, 7> buf2{};
    if (connection_->read_register(device_address_, 0xE1, std::span<uint8_t>(buf2.data(), buf2.size())) != connections::Status::Success)
        return false;

    dig_H2 = int16_t(uint16_t(buf2[0]) | (uint16_t(buf2[1]) << 8));
    dig_H3 = buf2[2];
    dig_H4 = int16_t((int16_t(buf2[3]) << 4) | (buf2[4] & 0xF));
    dig_H5 = int16_t((int16_t(buf2[5]) << 4) | (uint8_t)(buf2[4] >> 4));
    dig_H6 = int8_t(buf2[6]);

    return true;
}

bool bme280::read_raw(int32_t &raw_t, int32_t &raw_p, int32_t &raw_h)
{
    std::array<uint8_t, 8> data{};
    if (connection_->read_register(device_address_, REG_DATA, std::span<uint8_t>(data.data(), 8)) != connections::Status::Success)
        return false;

    raw_p = (int32_t(data[0]) << 12) | (int32_t(data[1]) << 4) | (int32_t(data[2]) >> 4);
    raw_t = (int32_t(data[3]) << 12) | (int32_t(data[4]) << 4) | (int32_t(data[5]) >> 4);
    raw_h = (int32_t(data[6]) << 8) | int32_t(data[7]);
    return true;
}

Status bme280::read_temperature(temperature_data &data)
{
    int32_t raw_t, raw_p, raw_h;
    if (!read_raw(raw_t, raw_p, raw_h)) return Status::ErrorCommunication;

    // temperature compensation (Bosch algorithm)
    int32_t var1 = ((((raw_t >> 3) - (int32_t(dig_T1) << 1))) * int32_t(dig_T2)) >> 11;
    int32_t var2 = (((((raw_t >> 4) - int32_t(dig_T1)) * ((raw_t >> 4) - int32_t(dig_T1))) >> 12) * int32_t(dig_T3)) >> 14;
    t_fine = var1 + var2;
    float T = (t_fine * 5 + 128) >> 8;
    data.celsius = T / 100.0f;
    data.valid = true;
    return Status::Success;
}

Status bme280::read_pressure(pressure_data &data)
{
    int32_t raw_t, raw_p, raw_h;
    if (!read_raw(raw_t, raw_p, raw_h)) return Status::ErrorCommunication;

    // compute pressure (uses t_fine from temperature comp)
    int64_t var1 = int64_t(t_fine) - 128000;
    int64_t var2 = var1 * var1 * int64_t(dig_P6);
    var2 = var2 + ((var1 * int64_t(dig_P5)) << 17);
    var2 = var2 + (int64_t(dig_P4) << 35);
    var1 = ((var1 * var1 * int64_t(dig_P3)) >> 8) + ((var1 * int64_t(dig_P2)) << 12);
    var1 = (((int64_t(1) << 47) + var1) * int64_t(dig_P1)) >> 33;
    if (var1 == 0) return Status::ErrorCalibration; // avoid div by zero
    int64_t p = 1048576 - raw_p;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (int64_t(dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (int64_t(dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (int64_t(dig_P7) << 4);
    data.pascals = float(p) / 256.0f; // p is in Q24.8
    data.valid = true;
    return Status::Success;
}

Status bme280::read_humidity(humidity_data &data)
{
    int32_t raw_t, raw_p, raw_h;
    if (!read_raw(raw_t, raw_p, raw_h)) return Status::ErrorCommunication;

    int32_t v_x1_u32r = t_fine - 76800;
    v_x1_u32r = (((((raw_h << 14) - (int32_t(dig_H4) << 20) - (int32_t(dig_H5) * v_x1_u32r)) + 16384) >> 15) * (((((((v_x1_u32r * int32_t(dig_H6)) >> 10) * (((v_x1_u32r * int32_t(dig_H3)) >> 11) + 32768)) >> 10) + 2097152) * int32_t(dig_H2) + 8192) >> 14));
    v_x1_u32r = v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * int32_t(dig_H1)) >> 4);
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    float h = (v_x1_u32r >> 12);
    data.relative_humidity = h / 1024.0f;
    data.valid = true;
    return Status::Success;
}

Status bme280::read_data(combined_env_data &data)
{
    temperature_data t{};
    humidity_data h{};
    pressure_data p{};
    auto st = read_temperature(t);
    if (st != Status::Success) return st;
    st = read_humidity(h);
    if (st != Status::Success) return st;
    st = read_pressure(p);
    if (st != Status::Success) return st;
    data.temperature = t;
    data.humidity = h;
    data.pressure = p;
    return Status::Success;
}

// deinitialize already implemented above

} // namespace peripherals
