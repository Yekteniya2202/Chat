#include "MyClientSocket.h"
#include <thread>
#include <mutex>
#include <atomic>
std::mutex sync_output;
std::atomic<bool> handlethread_end{ false };
void MyClientSocket::HandleMessages()
{
	while (!handlethread_end) {
		int n = receive_string(sReceiveBuffer, sizeof(sReceiveBuffer) - 1, 0);
		if (n == -1) {
			throw std::exception("Can not read message");
		}
		if (n == 0) continue;
		std::string access_message = "Access deny for ";
		access_message += username;
		access_message += "\r\n";
		sReceiveBuffer[n] = '\0';
		if (access_message == sReceiveBuffer) {
			std::cout << std::endl << '<' << sReceiveBuffer;
			handlethread_end = true;
			std::string reply = "Ok";
			if (send_string(reply.c_str(), reply.size(), 0) == -1) {
				throw std::exception("Can not send message");
			}
			break;
		}
		sync_output.lock();
		std::cout << '<' << sReceiveBuffer << '>';
		sync_output.unlock();
	}
}
int MyClientSocket::SendMessages()
{
	std::string hellomsg;
	std::cout << '>'; getline(std::cin, hellomsg, '\n');
	if (hellomsg == "exit") {
		handlethread_end = true;
		return 0;
	}
	if (send_string(hellomsg.c_str(), hellomsg.size(), 0) == -1) {
		throw std::exception("Can not send message");
	}
	return 1;
}

int MyClientSocket::SendName()
{
	if (send_string(username.c_str(), username.size(), 0) == -1) {
		throw std::exception("Can not send message");
	}
	return 0;
}

int MyClientSocket::receive_string(char* sString, int len, int flags)
{
	int total = 0;
	int n = 0;
	while (total < len)
	{
		//число считанных байт или -1 в случае ошибки
		timeval timeout; timeout.tv_sec = 0; timeout.tv_usec = 100;
		fd_set set;
		FD_ZERO(&set); FD_SET(S, &set);
		int sel = select(S + 1, &set, nullptr, nullptr, &timeout);
		if (sel <= 0)
			break;
		n = recv(S, sString + total, len - total, flags);
		if (n == -1) break;
		total += n;
	}
	return (n == -1 ? -1 : total);
}

int MyClientSocket::send_string(const char* sString, int len, int flags)
{
	int total = 0;
	int n = -1;
	while (total < len)
	{
		n = send(S, sString + total, len - total, flags);
		if (n == -1) break;
		total += n;
	}
	return (n == -1 ? -1 : total);
}

MyClientSocket::MyClientSocket(int family, int type, int protocol, std::string username) : username(username)
{
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	if ((S = socket(family, type, protocol)) == INVALID_SOCKET)
	{
		throw std::exception("Can not create socket!");
	}
}

MyClientSocket::~MyClientSocket()
{
	closesocket(S);
	WSACleanup();
}

std::string MyClientSocket::getname()
{
	gethostname(sName, sizeof(sName));
	return std::string(sName);
}

void MyClientSocket::bindServerAddr(int family, std::string addr, int port)
{
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = family;
	// Разрешаем работу на всех доступных сетевых интерфейсах,
	// в частности на localhost
	serv_addr.sin_addr.s_addr = inet_addr(addr.c_str());
	// обратите внимание на преобразование порядка байт
	serv_addr.sin_port = htons((u_short)port);
}

void MyClientSocket::connectServerSock(int family, std::string addr, int port)
{

	bindServerAddr(family, addr, port);

	// соединение нашего сокета с serv addr
	if (connect(S, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
	{
		throw std::exception("Can not connect to server");
	}
}

void MyClientSocket::RunSession()
{
	SendName();
	std::thread handlethread([&]() {this->HandleMessages(); });
	while (SendMessages()) { if (handlethread_end) break; }
	handlethread_end = true;
	handlethread.join();
	system("pause");
}
