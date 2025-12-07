/**
 * @file uart_connection.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  UART connection implementation
 * @version 0.1
 * @date 2025-12-07
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "connections/uart_connection.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>

namespace connections
{

    uart_connection::uart_connection(std::string_view device_path,
                                     const uart_config &config)
        : connection_iface(device_path), fd_(-1), config_(config)
    {
    }

    uart_connection::~uart_connection()
    {
        deinitialize();
    }

    Status uart_connection::initialize()
    {
        if (initialized_)
        {
            return Status::Success;
        }

        fd_ = open(config_path_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (fd_ < 0)
        {
            return Status::ErrorHardware;
        }

        fcntl(fd_, F_SETFL, 0);

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

    void uart_connection::deinitialize()
    {
        if (fd_ >= 0)
        {
            close(fd_);
            fd_ = -1;
        }
        initialized_ = false;
    }

    bool uart_connection::is_ready() const
    {
        return initialized_ && fd_ >= 0;
    }

    speed_t uart_connection::baudrate_to_speed(uint32_t baudrate) const
    {
        switch (baudrate)
        {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        case 460800:
            return B460800;
        case 500000:
            return B500000;
        case 576000:
            return B576000;
        case 921600:
            return B921600;
        case 1000000:
            return B1000000;
        case 1152000:
            return B1152000;
        case 1500000:
            return B1500000;
        case 2000000:
            return B2000000;
        case 2500000:
            return B2500000;
        case 3000000:
            return B3000000;
        case 3500000:
            return B3500000;
        case 4000000:
            return B4000000;
        default:
            return B115200;
        }
    }

    Status uart_connection::apply_config()
    {
        struct termios options;

        if (tcgetattr(fd_, &options) < 0)
        {
            return Status::ErrorHardware;
        }

        speed_t speed = baudrate_to_speed(config_.baudrate);
        cfsetispeed(&options, speed);
        cfsetospeed(&options, speed);

        options.c_cflag |= (CLOCAL | CREAD);

        options.c_cflag &= ~CSIZE;
        switch (config_.data_bits)
        {
        case 5:
            options.c_cflag |= CS5;
            break;
        case 6:
            options.c_cflag |= CS6;
            break;
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            options.c_cflag |= CS8;
            break;
        }

        switch (config_.parity)
        {
        case uart_parity::none:
            options.c_cflag &= ~PARENB;
            break;
        case uart_parity::even:
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            break;
        case uart_parity::odd:
            options.c_cflag |= PARENB;
            options.c_cflag |= PARODD;
            break;
        }

        if (config_.stop_bits == uart_stop_bits::two)
        {
            options.c_cflag |= CSTOPB;
        }
        else
        {
            options.c_cflag &= ~CSTOPB;
        }

        if (config_.hardware_flow_control)
        {
            options.c_cflag |= CRTSCTS;
        }
        else
        {
            options.c_cflag &= ~CRTSCTS;
        }

        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
        options.c_oflag &= ~OPOST;

        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = 10;

        if (tcsetattr(fd_, TCSANOW, &options) < 0)
        {
            return Status::ErrorHardware;
        }

        return Status::Success;
    }

    Status uart_connection::read(std::span<uint8_t> buffer)
    {
        if (!is_ready())
        {
            return Status::ErrorNotInitialized;
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

    Status uart_connection::write(std::span<const uint8_t> data)
    {
        if (!is_ready())
        {
            return Status::ErrorNotInitialized;
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

    Status uart_connection::read_timeout(std::span<uint8_t> buffer, uint32_t timeout_ms)
    {
        if (!is_ready())
        {
            return Status::ErrorNotInitialized;
        }

        struct termios options;
        tcgetattr(fd_, &options);

        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = timeout_ms / 100;
        tcsetattr(fd_, TCSANOW, &options);

        Status result = read(buffer);

        options.c_cc[VTIME] = 10;
        tcsetattr(fd_, TCSANOW, &options);

        return result;
    }

    Status uart_connection::write_timeout(std::span<const uint8_t> data, uint32_t timeout_ms)
    {
        return write(data);
    }

    Status uart_connection::reset()
    {
        deinitialize();
        return initialize();
    }

    void uart_connection::flush()
    {
        if (is_ready())
        {
            tcflush(fd_, TCIOFLUSH);
        }
    }

    void uart_connection::set_config(const uart_config &config)
    {
        config_ = config;
        if (is_ready())
        {
            apply_config();
        }
    }

    size_t uart_connection::available() const
    {
        if (!is_ready())
        {
            return 0;
        }

        int bytes_available = 0;
        ioctl(fd_, FIONREAD, &bytes_available);
        return bytes_available;
    }

} // namespace connections
