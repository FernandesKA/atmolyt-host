/**
 * @file i2c_connection.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief 
 * @version 0.1
 * @date 2025-12-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "i2c_connection.h"
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <vector>

namespace connections
{

    i2c_connection::i2c_connection(std::string_view device_path)
        : addressable_connection_iface(device_path), fd_(-1)
    {
    }

    i2c_connection::~i2c_connection()
    {
        deinitialize();
    }

    Status i2c_connection::initialize()
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

        initialized_ = true;
        return Status::Success;
    }

    void i2c_connection::deinitialize()
    {
        if (fd_ >= 0)
        {
            close(fd_);
            fd_ = -1;
        }
        initialized_ = false;
    }

    bool i2c_connection::is_ready() const
    {
        return initialized_ && fd_ >= 0;
    }

    Status i2c_connection::read(std::span<uint8_t> buffer)
    {
        return Status::ErrorInvalidParam;
    }

    Status i2c_connection::write(std::span<const uint8_t> data)
    {
        return Status::ErrorInvalidParam;
    }

    Status i2c_connection::read(uint8_t device_addr, std::span<uint8_t> buffer)
    {
        if (!is_ready())
        {
            return Status::ErrorNotInitialized;
        }

        if (ioctl(fd_, I2C_SLAVE, device_addr) < 0)
        {
            return Status::ErrorNack;
        }

        ssize_t result = ::read(fd_, buffer.data(), buffer.size());
        if (result < 0)
        {
            return Status::ErrorHardware;
        }
        if (result != static_cast<ssize_t>(buffer.size()))
        {
            return Status::ErrorTimeout;
        }

        return Status::Success;
    }

    Status i2c_connection::write(uint8_t device_addr, std::span<const uint8_t> data)
    {
        if (!is_ready())
        {
            return Status::ErrorNotInitialized;
        }

        if (ioctl(fd_, I2C_SLAVE, device_addr) < 0)
        {
            return Status::ErrorNack;
        }

        ssize_t result = ::write(fd_, data.data(), data.size());
        if (result < 0)
        {
            return Status::ErrorHardware;
        }
        if (result != static_cast<ssize_t>(data.size()))
        {
            return Status::ErrorTimeout;
        }

        return Status::Success;
    }

    Status i2c_connection::read_register(uint8_t device_addr, uint8_t reg_addr,
                                         std::span<uint8_t> buffer)
    {
        if (!is_ready())
        {
            return Status::ErrorNotInitialized;
        }

        if (ioctl(fd_, I2C_SLAVE, device_addr) < 0)
        {
            return Status::ErrorNack;
        }

        if (::write(fd_, &reg_addr, 1) != 1)
        {
            return Status::ErrorHardware;
        }

        ssize_t result = ::read(fd_, buffer.data(), buffer.size());
        if (result < 0)
        {
            return Status::ErrorHardware;
        }
        if (result != static_cast<ssize_t>(buffer.size()))
        {
            return Status::ErrorTimeout;
        }

        return Status::Success;
    }

    Status i2c_connection::write_register(uint8_t device_addr, uint8_t reg_addr,
                                          std::span<const uint8_t> data)
    {
        if (!is_ready())
        {
            return Status::ErrorNotInitialized;
        }

        if (ioctl(fd_, I2C_SLAVE, device_addr) < 0)
        {
            return Status::ErrorNack;
        }

        std::vector<uint8_t> tx_buffer;
        tx_buffer.reserve(1 + data.size());
        tx_buffer.push_back(reg_addr);
        tx_buffer.insert(tx_buffer.end(), data.begin(), data.end());

        ssize_t result = ::write(fd_, tx_buffer.data(), tx_buffer.size());
        if (result < 0)
        {
            return Status::ErrorHardware;
        }
        if (result != static_cast<ssize_t>(tx_buffer.size()))
        {
            return Status::ErrorTimeout;
        }

        return Status::Success;
    }

    Status i2c_connection::reset()
    {
        deinitialize();
        return initialize();
    }

    void i2c_connection::flush()
    {
    }

} // namespace connections
