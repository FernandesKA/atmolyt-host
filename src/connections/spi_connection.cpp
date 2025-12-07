/**
 * @file spi_connection.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  SPI connection implementation
 * @version 0.1
 * @date 2025-12-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "connections/spi_connection.h"
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <vector>

namespace connections
{

    spi_connection::spi_connection(std::string_view device_path,
                                   const spi_config &config)
        : addressable_connection_iface(device_path), fd_(-1), config_(config)
    {
    }

    spi_connection::~spi_connection()
    {
        deinitialize();
    }

    Status spi_connection::initialize()
    {
        if (initialized_)
        {
            return Status::Success;
        }

        fd_ = open(config_path_.c_str(), O_RDWR);
        if (fd_ < 0)
        {
            return Status::ErrorHardware;
        }

        Status status = apply_config();
        if (status != Status::Success)
        {
            close(fd_);
            fd_ = -1;
            return status;
        }

        initialized_ = true;
        return Status::Success;
    }

    void spi_connection::deinitialize()
    {
        if (fd_ >= 0)
        {
            close(fd_);
            fd_ = -1;
        }
        initialized_ = false;
    }

    bool spi_connection::is_ready() const
    {
        return initialized_ && fd_ >= 0;
    }

    Status spi_connection::apply_config()
    {
        if (ioctl(fd_, SPI_IOC_WR_MODE, &config_.mode) < 0)
        {
            return Status::ErrorHardware;
        }

        if (ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &config_.bits_per_word) < 0)
        {
            return Status::ErrorHardware;
        }

        if (ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &config_.speed_hz) < 0)
        {
            return Status::ErrorHardware;
        }

        return Status::Success;
    }

    Status spi_connection::read(std::span<uint8_t> buffer)
    {
        return Status::ErrorInvalidParam;
    }

    Status spi_connection::write(std::span<const uint8_t> data)
    {
        return Status::ErrorInvalidParam;
    }

    Status spi_connection::transfer(uint8_t cs_pin, std::span<const uint8_t> tx_data,
                                    std::span<uint8_t> rx_data)
    {
        if (!is_ready())
        {
            return Status::ErrorNotInitialized;
        }

        if (tx_data.size() != rx_data.size())
        {
            return Status::ErrorInvalidParam;
        }

        struct spi_ioc_transfer xfer;
        memset(&xfer, 0, sizeof(xfer));

        xfer.tx_buf = reinterpret_cast<uintptr_t>(tx_data.data());
        xfer.rx_buf = reinterpret_cast<uintptr_t>(rx_data.data());
        xfer.len = tx_data.size();
        xfer.speed_hz = config_.speed_hz;
        xfer.bits_per_word = config_.bits_per_word;

        if (ioctl(fd_, SPI_IOC_MESSAGE(1), &xfer) < 0)
        {
            return Status::ErrorHardware;
        }

        return Status::Success;
    }

    Status spi_connection::read(uint8_t cs_pin, std::span<uint8_t> buffer)
    {
        std::vector<uint8_t> tx_dummy(buffer.size(), 0xFF);
        return transfer(cs_pin, tx_dummy, buffer);
    }

    Status spi_connection::write(uint8_t cs_pin, std::span<const uint8_t> data)
    {
        std::vector<uint8_t> rx_dummy(data.size());
        return transfer(cs_pin, data, rx_dummy);
    }

    Status spi_connection::read_register(uint8_t cs_pin, uint8_t reg_addr,
                                         std::span<uint8_t> buffer)
    {
        if (!is_ready())
        {
            return Status::ErrorNotInitialized;
        }

        std::vector<uint8_t> tx_data(1 + buffer.size());
        std::vector<uint8_t> rx_data(1 + buffer.size());

        tx_data[0] = reg_addr | 0x80;
        std::fill(tx_data.begin() + 1, tx_data.end(), 0xFF);

        Status status = transfer(cs_pin, tx_data, rx_data);
        if (status != Status::Success)
        {
            return status;
        }

        std::copy(rx_data.begin() + 1, rx_data.end(), buffer.begin());
        return Status::Success;
    }

    Status spi_connection::write_register(uint8_t cs_pin, uint8_t reg_addr,
                                          std::span<const uint8_t> data)
    {
        if (!is_ready())
        {
            return Status::ErrorNotInitialized;
        }

        std::vector<uint8_t> tx_data(1 + data.size());
        std::vector<uint8_t> rx_data(1 + data.size());

        tx_data[0] = reg_addr & 0x7F;
        std::copy(data.begin(), data.end(), tx_data.begin() + 1);

        return transfer(cs_pin, tx_data, rx_data);
    }

    Status spi_connection::reset()
    {
        deinitialize();
        return initialize();
    }

    void spi_connection::flush()
    {
    }

    void spi_connection::set_config(const spi_config &config)
    {
        config_ = config;
        if (is_ready())
        {
            apply_config();
        }
    }

} // namespace connections
