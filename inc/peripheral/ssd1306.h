/**
 * @file ssd1306.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  SSD1306 OLED display interface
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include "peripheral/peripheral_iface.h"

namespace peripherals {

class ssd1306 : public display_iface
{
public:
    ssd1306(connections::addressable_connection_iface<uint8_t> *conn, uint8_t address);
    ~ssd1306() override;

    Status initialize() override;
    void deinitialize() override;
    bool is_connected() override;
    Status reset() override;

    Status read_data(display_data &data) override { return Status::Success; }

    Status clear() override;
    Status display_text(const std::string &text, uint8_t x = 0, uint8_t y = 0, uint8_t scale = 1, bool bold = false) override;
    Status set_cursor(uint8_t x, uint8_t y) override;
    void draw_pixel(int x, int y, int color) override;
    void draw_line(int x0, int y0, int x1, int y1, int color) override;

private:
    Status send_command(uint8_t cmd);
    Status send_data(const uint8_t *data, size_t len);
    Status init_display();

    uint8_t cursor_x_;
    uint8_t cursor_y_;

    // Framebuffer fields
    int fb_fd_;
    char* fb_ptr_;
    size_t fb_size_;
    int fb_width_;
    int fb_height_;
    int fb_bpp_;
};

} // namespace peripherals