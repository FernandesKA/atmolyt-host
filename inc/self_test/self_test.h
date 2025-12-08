/**
 * @file self_test.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief
 * @version 0.1
 * @date 2025-12-09
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <string>
#include <memory>

namespace st
{
    class self_test
    {
    public:
        explicit self_test(const std::string &config_path = "./config/atmolyt.json", bool json_output = false);
        ~self_test();

        // Run self-test. Returns 0 on success, non-zero on failure
        int run();

    private:
        std::string config_path_;
        bool json_output_ = false;
    };
}
