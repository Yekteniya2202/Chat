#include "MySocket.h"
#include <thread>



void socket_work(std::string addr, int16_t port) {
	MySocket sock(AF_INET, SOCK_STREAM, 0);
	sock.bindsock(AF_INET, addr.c_str(), port);
	sock.listensock(5);
	while (sock.HandleConnetctions()) {
		sock.RunSession();
	}
}
int main(void)
try
{
	setlocale(LC_ALL, "rus");
	socket_work("127.0.0.1", 1234);
	return 0;
}
catch (std::exception e) {
	std::cout << e.what() << std::endl;
}