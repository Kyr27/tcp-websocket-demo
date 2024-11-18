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
#include <chrono>
#include <set>
#include "asio.hpp"
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> connections;
std::mutex connectionsMutex;

void shutdown();
void OnMessage(server* wsServer, websocketpp::connection_hdl hdl, server::message_ptr msg);
void signal_handler(int signum);

#ifdef _WIN32
	BOOL WINAPI ConsoleHandler(DWORD dwCtrlType);
#endif

static server wsServer; // needs to be global so that we can control it from the console handler
std::atomic<bool> wsServerRunning(true);


void OnServerOpen(websocketpp::connection_hdl hdl)
{
	std::lock_guard<std::mutex> lock(connectionsMutex);
	connections.insert(hdl);
}

void OnServerClose(websocketpp::connection_hdl hdl)
{
	std::lock_guard<std::mutex> lock(connectionsMutex);
	connections.erase(hdl);
}

int main()
{
	// cross platform way of handling normal application exits, the extent of "normal" depends on the platform
	atexit(shutdown);

	// Platform specific cleanup
	#ifdef _WIN32
		// Register Console Control Handler if running on Windows, very versatile in what it can do.
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

	// Log all info and errors
	wsServer.set_access_channels(websocketpp::log::alevel::all);
	wsServer.set_error_channels(websocketpp::log::elevel::all);

	// Initialize websocketpp using standalone asio
	wsServer.init_asio();

	// Set websocketpp open and close handlers
	wsServer.set_open_handler(&OnServerOpen);
	wsServer.set_close_handler(&OnServerClose);

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
			// run in a non-blocking manner, so we can check for wsServerRunning on every iteration
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
	std::this_thread::sleep_for(std::chrono::seconds(1));
}

void shutdown()
{
	static std::atomic<bool> shutdownHandled(false);

	if (shutdownHandled.exchange(true)) return;

	std::cout << "Shutting down server...\n";
	wsServerRunning.store(false);

	try 
	{
		wsServer.stop_listening();

		// Go through each connection, and tell them we are closing
		std::lock_guard<std::mutex> lock(connectionsMutex);
		for (auto hdl : connections) {
			websocketpp::lib::error_code ec;
			wsServer.close(hdl, websocketpp::close::status::going_away, "Server is shutting down", ec);

			if (ec) {
				std::cerr << "Error while trying to close connections: " << ec.message() << '\n';
			}
		}

		// Process any remaining close handshake messages
		while (!connections.empty()) {
			wsServer.poll_one(); // Process pending events
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		wsServer.stop();
	}
	catch (const std::exception& e) {
		std::cerr << "Error during shutdown: " << e.what() << '\n';
	}

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

			// Stop Windows from immediately exiting, before shutdown has had enough time to close all connections, errors arise without this part
			std::this_thread::sleep_for(std::chrono::seconds(1));

			return TRUE;
		default:
			return FALSE;
		}
	}
#endif