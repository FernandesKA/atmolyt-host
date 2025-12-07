/**
 * @file signal_handler.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Signal handler interface for application
 * @version 0.1
 * @date 2025-12-07
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <atomic>
#include <functional>

namespace app {

class signal_handler
{
public:
    static void install();
    static bool shutdown_requested();
    static void set_user_callback(std::function<void(int)> cb);
    static void poll_and_handle();

private:
    static void handle_signal(int signo) noexcept;

    static inline std::atomic<bool> requested_{false};
    static inline std::atomic<int> last_signal_{0};
    static inline std::function<void(int)> user_cb_{};
};

} // namespace app
