/**
 * @file ssd1306.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  SSD1306 OLED display implementation
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "peripheral/ssd1306.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>

static const uint8_t font5x7[95][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // \
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // }
    {0x10, 0x08, 0x08, 0x10, 0x08}, // ~
};

namespace peripherals {

ssd1306::ssd1306(connections::addressable_connection_iface<uint8_t> *conn, uint8_t address)
    : display_iface(conn, address), cursor_x_(0), cursor_y_(0), fb_fd_(-1), fb_ptr_(nullptr), fb_size_(0), fb_width_(0), fb_height_(0), fb_bpp_(0)
{
}

ssd1306::~ssd1306()
{
    deinitialize();
}

Status ssd1306::initialize()
{
    if (initialized_) {
        return Status::Success;
    }

    if (connection_ == nullptr) {
        // Framebuffer mode
        fb_fd_ = open("/dev/fb0", O_RDWR);
        if (fb_fd_ == -1) {
            std::cerr << "Warning: Failed to open /dev/fb0, framebuffer not available" << std::endl;
            // Return Success to allow display object creation, but operations will be no-ops
            initialized_ = true;
            return Status::Success;
        }

        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;
        if (ioctl(fb_fd_, FBIOGET_VSCREENINFO, &vinfo) == -1) {
            close(fb_fd_);
            return Status::ErrorCommunication;
        }

        if (ioctl(fb_fd_, FBIOGET_FSCREENINFO, &finfo) == -1) {
            close(fb_fd_);
            return Status::ErrorCommunication;
        }

        fb_width_ = vinfo.xres;
        fb_height_ = vinfo.yres;
        fb_bpp_ = vinfo.bits_per_pixel;

        fb_ptr_ = (char*)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd_, 0);
        if (fb_ptr_ == MAP_FAILED) {
            close(fb_fd_);
            return Status::ErrorCommunication;
        }

        fb_size_ = finfo.smem_len;

        initialized_ = true;
        return Status::Success;
    } else {
        // I2C mode
        // Wait for display to power up
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        Status status = init_display();
        if (status != Status::Success) {
            return status;
        }

        // Clear display after initialization to remove garbage
        status = clear();
        if (status != Status::Success) {
            return status;
        }

        initialized_ = true;
        return Status::Success;
    }
}

void ssd1306::deinitialize()
{
    if (initialized_) {
        clear();
        if (connection_ == nullptr) {
            if (fb_ptr_) {
                munmap(fb_ptr_, fb_size_);
                fb_ptr_ = nullptr;
            }
            if (fb_fd_ != -1) {
                close(fb_fd_);
                fb_fd_ = -1;
            }
        }
        initialized_ = false;
    }
}

bool ssd1306::is_connected()
{
    if (connection_ == nullptr) {
        return fb_fd_ != -1;
    } else {
        // Simple check by trying to read a register or send a command
        return send_command(0xAF) == Status::Success; // Display ON command
    }
}

Status ssd1306::reset()
{
    deinitialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return initialize();
}

Status ssd1306::clear()
{
    if (connection_ == nullptr) {
        // Framebuffer clear
        if (fb_fd_ == -1) return Status::Success; // No-op if fb not available
        memset(fb_ptr_, 0, fb_size_);
        return Status::Success;
    } else {
        // Set addressing mode to cover entire display
        Status status = send_command(0x21); // Set column address
        if (status != Status::Success) return status;
        status = send_command(0x00); // Start column 0
        if (status != Status::Success) return status;
        status = send_command(0x7F); // End column 127
        if (status != Status::Success) return status;
        
        status = send_command(0x22); // Set page address
        if (status != Status::Success) return status;
        status = send_command(0x00); // Start page 0
        if (status != Status::Success) return status;
        status = send_command(0x07); // End page 7
        if (status != Status::Success) return status;

        // Clear all 1024 bytes (128x64/8)
        uint8_t zero_data[128];
        std::memset(zero_data, 0x00, 128);
        
        for (int page = 0; page < 8; ++page) {
            status = send_data(zero_data, 128);
            if (status != Status::Success) return status;
        }
        
        return Status::Success;
    }
}

Status ssd1306::display_text(const std::string &text, uint8_t x, uint8_t y)
{
    if (connection_ == nullptr) {
        // Framebuffer mode - render text using font
        if (fb_fd_ == -1) return Status::Success; // No-op if fb not available
        uint8_t current_x = x;
        uint8_t current_y = y;
        for (char c : text) {
            if (c == '\n') {
                current_y += 8;
                current_x = x;
            } else if (c >= 32 && c <= 126) {
                const uint8_t* char_data = font5x7[c - 32];
                for (int col = 0; col < 5; ++col) {
                    uint8_t column = char_data[col];
                    for (int row = 0; row < 7; ++row) {
                        if (column & (1 << row)) {
                            draw_pixel(current_x + col, current_y + row, 1);
                        }
                    }
                }
                current_x += 6; // 5 pixels + 1 space
            }
        }
        return Status::Success;
    } else {
        // I2C mode - render text using font and send data, centered
        std::istringstream iss(text);
        std::string line;
        uint8_t current_y = y;
        while (std::getline(iss, line)) {
            // Calculate centered x
            size_t len = line.length();
            uint8_t centered_x = (128 - len * 6) / 2;
            set_cursor(centered_x, current_y);

            for (char c : line) {
                if (c >= 32 && c <= 126) {
                    const uint8_t* char_data = font5x7[c - 32];
                    // Send 5 bytes for the character
                    Status status = send_data(char_data, 5);
                    if (status != Status::Success) {
                        return status;
                    }
                    // Send a space column
                    uint8_t space = 0x00;
                    status = send_data(&space, 1);
                    if (status != Status::Success) {
                        return status;
                    }
                }
            }
            current_y += 8;
        }

        return Status::Success;
    }
}

Status ssd1306::set_cursor(uint8_t x, uint8_t y)
{
    if (connection_ == nullptr) {
        cursor_x_ = x;
        cursor_y_ = y;
        return Status::Success;
    } else {
        cursor_x_ = x;
        cursor_y_ = y;

        // Set column address
        Status status = send_command(0x21); // Set column address
        if (status != Status::Success) return status;
        status = send_command(x); // Start column
        if (status != Status::Success) return status;
        status = send_command(127); // End column
        if (status != Status::Success) return status;

        // Set page address
        status = send_command(0x22); // Set page address
        if (status != Status::Success) return status;
        status = send_command(y / 8); // Start page
        if (status != Status::Success) return status;
        status = send_command(7); // End page
        if (status != Status::Success) return status;

        return Status::Success;
    }
}

Status ssd1306::send_command(uint8_t cmd)
{
    uint8_t buffer[2] = {0x00, cmd}; // 0x00 for command
    auto status = connection_->write(device_address_, std::span(buffer, 2));
    return status == connections::Status::Success ? Status::Success : Status::ErrorCommunication;
}

Status ssd1306::send_data(const uint8_t *data, size_t len)
{
    std::vector<uint8_t> buffer;
    buffer.reserve(len + 1);
    buffer.push_back(0x40); // 0x40 for data
    buffer.insert(buffer.end(), data, data + len);

    auto status = connection_->write(device_address_, std::span(buffer));
    return status == connections::Status::Success ? Status::Success : Status::ErrorCommunication;
}

Status ssd1306::init_display()
{
    // Initialization sequence for SSD1306
    std::vector<uint8_t> init_commands = {
        0xAE, // Display OFF
        0xD5, // Set display clock divide ratio/oscillator frequency
        0x80, // Suggested ratio
        0xA8, // Set multiplex ratio
        0x3F, // 1/64 duty
        0xD3, // Set display offset
        0x00, // No offset
        0x40, // Set start line address
        0x8D, // Charge pump
        0x14, // Enable charge pump
        0x20, // Set memory addressing mode
        0x00, // Horizontal addressing mode
        0xA1, // Set segment re-map
        0xC8, // Set COM output scan direction
        0xDA, // Set COM pins hardware configuration
        0x12, // Alternative COM pin configuration
        0x81, // Set contrast control
        0xCF, // Contrast value
        0xD9, // Set pre-charge period
        0xF1, // Pre-charge period
        0xDB, // Set VCOMH deselect level
        0x40, // VCOMH deselect level
        0xA4, // Entire display ON (resume to RAM content)
        0xA6, // Set normal display
        0xAF  // Display ON
    };

    for (uint8_t cmd : init_commands) {
        Status status = send_command(cmd);
        if (status != Status::Success) {
            return status;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return Status::Success;
}

void ssd1306::draw_pixel(int x, int y, int color)
{
    if (fb_ptr_ == nullptr || x < 0 || x >= 128 || y < 0 || y >= 64) {
        return;
    }

    int scale = 8; // Scale factor for visibility on large framebuffer
    int bytes_per_pixel = fb_bpp_ / 8;

    for (int dx = 0; dx < scale; ++dx) {
        for (int dy = 0; dy < scale; ++dy) {
            int px = x * scale + dx;
            int py = y * scale + dy;
            if (px >= fb_width_ || py >= fb_height_) continue;

            long location = (px * bytes_per_pixel) + (py * fb_width_ * bytes_per_pixel);

            if (fb_bpp_ == 32) {
                *(fb_ptr_ + location) = color ? 255 : 0;        // Blue
                *(fb_ptr_ + location + 1) = color ? 255 : 0;    // Green
                *(fb_ptr_ + location + 2) = color ? 255 : 0;    // Red
                *(fb_ptr_ + location + 3) = 0;                  // Alpha
            } else {
                // Handle other bit depths if needed
            }
        }
    }
}

} // namespace peripherals