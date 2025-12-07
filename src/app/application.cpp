/**
 * @file application.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief
 * @version 0.1
 * @date 2025-12-06
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "app/application.h"

#include <iostream>
#include <cerrno>
#include <boost/program_options.hpp>

namespace app
{
    atmolyt::atmolyt(int argc, char *argv[])
    {
        // Parse input arguments and set initial state
        int rc = parse_inarg(argc, argv);
        is_periphery_init = (rc == 0);
    }

    atmolyt::~atmolyt()
    {
    }

    int atmolyt::parse_inarg(int argc, char **argv)
    {
        namespace po = boost::program_options;

        if (argc < 1 || !argv)
            return -EINVAL;

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce help message")
            ("view,v", po::bool_switch()->default_value(false), "view params from config file")
            ("st,s", po::bool_switch()->default_value(false), "begin self testing hardware");

        po::variables_map vm;
        try
        {
            po::store(po::parse_command_line(argc, argv, desc), vm);
            po::notify(vm);
        }
        catch (const po::error &e)
        {
            std::cerr << "Argument parsing error: " << e.what() << "\n";
            std::cerr << desc << std::endl;
            return -EINVAL;
        }

        if (vm.count("help") && vm["help"].as<bool>())
        {
            std::cout << desc << "\n";
            return 1;
        }

        if (vm["view"].as<bool>())
        {
            // TODO: implement viewing config file parameters
            std::cout << "View config requested\n";
        }

        if (vm["st"].as<bool>())
        {
            // TODO: implement self-test sequence
            std::cout << "Self-test requested\n";
        }

        return 0;
    }
}