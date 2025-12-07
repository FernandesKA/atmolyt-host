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

#include <boost/program_options.hpp>

namespace app
{
    class atmolyt
    {
    public:
        atmolyt(int argc, char *argv[]);
        ~atmolyt();

    private:

        int parse_inarg(int, char**);

    private:
        bool is_periphery_init = false;


    };

};