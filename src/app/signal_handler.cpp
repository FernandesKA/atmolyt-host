/**
 * @file signal_handler.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Signal handler implementation for application
 * @version 0.1
 * @date 2025-12-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "app/signal_handler.h"

#include <csignal>
#include <unistd.h>
#include <iostream>
#include <cstring>

namespace app {

void signal_handler::install()
{
	std::signal(SIGINT, [](int s){ signal_handler::handle_signal(s); });
	std::signal(SIGTERM, [](int s){ signal_handler::handle_signal(s); });
	std::signal(SIGHUP, [](int s){ signal_handler::handle_signal(s); });
	std::signal(SIGQUIT, [](int s){ signal_handler::handle_signal(s); });
	std::signal(SIGABRT, [](int s){ signal_handler::handle_signal(s); });
}

bool signal_handler::shutdown_requested()
{
	return requested_.load(std::memory_order_acquire);
}

void signal_handler::set_user_callback(std::function<void(int)> cb)
{
	user_cb_ = std::move(cb);
}

void signal_handler::handle_signal(int signo) noexcept
{
	requested_.store(true, std::memory_order_release);
	last_signal_.store(signo, std::memory_order_release);

	const char *msg = "termination signal received\n";
	::write(STDERR_FILENO, msg, std::strlen(msg));
}

void signal_handler::poll_and_handle()
{
	int sig = last_signal_.exchange(0, std::memory_order_acq_rel);
	if (sig != 0)
	{
		try {
			if (user_cb_)
				user_cb_(sig);
		}
		catch (...) {
			// swallow
		}
	}
}

} // namespace app
