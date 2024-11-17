#define ASIO_STANDALONE
#include <iostream>
#include "asio.hpp"


int main()
{
	std::cout << "Hello from server\n";

	static_cast<void>(getchar());
	return 0;
}