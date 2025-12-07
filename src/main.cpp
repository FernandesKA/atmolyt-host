#include "app/application.h"
#include "app/signal_handler.h"
#include <iostream>
#include <thread>
#include <chrono>

using app::signal_handler;

int main(int argc, char **argv)
{
    signal_handler::install();

    signal_handler::set_user_callback([](int signo){
        std::cerr << "Signal received in main thread: " << signo << std::endl;
    });

    app::atmolyt application(argc, argv);

    while (!signal_handler::shutdown_requested())
    {
        signal_handler::poll_and_handle();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cerr << "Shutting down due to signal" << std::endl;
    return 0;
}
