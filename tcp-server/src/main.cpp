#define ASIO_STANDALONE // using asio without boost
#define _WEBSOCKETPP_CPP11_STL_ // tell websocketpp that we don't need boost stuff, as it's standard in C++ 11 and after

#ifdef _WIN32
	#define _WIN32_WINNT 0x0A00 // Windows 10
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <csignal>
#include <atomic>
#include "asio.hpp"
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

void shutdown();
void OnMessage(server* wsServer, websocketpp::connection_hdl hdl, server::message_ptr msg);
void signal_handler(int signum);
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType);

static server wsServer; // needs to be global so that we can control it from the console handler
std::atomic<bool> wsServerRunning(true);
int main()
{
	// cross platform way of handling normal application exits, the extent "normal" depends on the platform
	atexit(shutdown);

	// Platform specific cleanup
	#ifdef _WIN32
		// Register Console Control Handler if running on Windows
		if (!SetConsoleCtrlHandler(ConsoleHandler, true))
		{
			std::cerr << "Failed to set console control handler\n";
			return 1;
		}
	#else
		// Use signals for other platforms (less versatile)
		std::signal(SIGINT, signal_handler);  // handle Ctrl + C gracefully
		std::signal(SIGTERM, signal_handler); // handle termination gracefully
	#endif

	// Initialize websocketpp using standalone asio
	wsServer.init_asio();

	// Call OnMessage function when the server receives a message
	wsServer.set_message_handler(std::bind(&OnMessage, &wsServer, std::placeholders::_1, std::placeholders::_2));

	// Listen on localhost with port 9002 for messages
	wsServer.listen(9002);

	// Start
	wsServer.start_accept();

	std::cout << "C++ - Server Online\n";

	while (wsServerRunning.load())
	{
		try
		{
			wsServer.run_one();
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error during server execution: " << e.what() << '\n';
			break;
		}
	}

	std::cout << "C++ - Server shutdown\n";
	return 0;
}

void OnMessage(server* wsServer, websocketpp::connection_hdl hdl, server::message_ptr msg)
{
	std::cout << "C++ - Message received: " << msg->get_payload() << '\n';
	std::cout << "C++ - Sending data to C#...\n";
	wsServer->send(hdl, "Hello client, this is coming from the server!", websocketpp::frame::opcode::text);
	std::cout << "C++ - Sent message to C#\n";
}

void signal_handler(int signum) {
	shutdown();
}

void shutdown()
{
	static std::atomic<bool> shutdownHandled(false);

	// exchange the value of the variable with a new value) and return the old value.
	if (shutdownHandled.exchange(true))
	{
		// if the previous value was true, then we already handled it
		return;
	}

	std::cout << "Shutting down server...\n";
	wsServerRunning.store(false);
	wsServer.stop_listening();
	wsServer.stop();

	std::cout << "Server has shutdown\n";
}

#ifdef _WIN32
	BOOL WINAPI ConsoleHandler(DWORD dwCtrlType)
	{
		switch (dwCtrlType) {
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			shutdown();

			return TRUE;
		default:
			return FALSE;
		}
	}
#endif