/**
 * @file application.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief
 * @version 0.1
 * @date 2025-12-06
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

namespace app
{
    class atmolyt
    {
    public:
        atmolyt(int argc, char *argv[]);
        ~atmolyt();

    private:
        bool is_periphery_init = false;
    };

};