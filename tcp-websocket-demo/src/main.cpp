#define ASIO_STANDALONE // using asio without boost
#define _WEBSOCKETPP_CPP11_STL_ // tell websocketpp that we don't need boost stuff, as it's standard in C++ 11 and after
#include <iostream>
#include "asio.hpp"
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"


int main()
{
	std::cout << "Hello from server\n";

	static_cast<void>(getchar());
	return 0;
}