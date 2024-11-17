#define ASIO_STANDALONE // using asio without boost
#define _WEBSOCKETPP_CPP11_STL_ // tell websocketpp that we don't need boost stuff, as it's standard in C++ 11 and after
#define _WIN32_WINNT 0x0A00 // Windows 10

#include <iostream>
#include "asio.hpp"
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

void OnMessage(server* wsServer, websocketpp::connection_hdl hdl, server::message_ptr msg)
{
	std::cout << "C++ - Message received: " << msg->get_payload() << '\n';
	std::cout << "C++ - Sending data to C#...\n";
	wsServer->send(hdl, "Hello client, this is coming from the server!", websocketpp::frame::opcode::text);
	std::cout << "C++ - Sent message to C#\n";
}

int main()
{
	server wsServer;

	// Enable logging (seems to be set to this by default)
	//wsServer.set_access_channels(websocketpp::log::alevel::all);

	// Don't show messages that we are sending to the client
	//wsServer.clear_access_channels(websocketpp::log::alevel::frame_payload);

	// Initialize websocketpp using standalone asio
	wsServer.init_asio();

	// Call OnMessage function when the server receives a message
	wsServer.set_message_handler(std::bind(&OnMessage, &wsServer, std::placeholders::_1, std::placeholders::_2));

	// Listen on port 8788 for messages
	wsServer.listen(9002);

	// Start
	wsServer.start_accept();
	wsServer.run();

	std::cout << "C++ - Server Online\n";

	static_cast<void>(getchar());
	return 0;
}