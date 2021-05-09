#pragma once
#pragma comment(lib,"Ws2_32.lib")
#include <Winsock2.h>
#include <iostream>
#include <stdlib.h>
#include <conio.h>
#include <ctime>
#include <sstream>
#include <fstream>
#pragma warning(disable : 4996) 
class MyClientSocket {
private:
	SOCKET  S;  

	sockaddr_in serv_addr;
	WSADATA     wsadata;
	char        sName[128];
	bool        bTerminate = false;
	std::string clientmsg, receiveBuf; char sReceiveBuffer[1024] = { 0 };

	std::string username;
	void HandleMessages();
	int SendMessages();
	int SendName();

	int receive_string(char* sString, int len, int flags);
	int send_string(const char* sString, int len, int flags);
public:
	MyClientSocket(int family, int type, int protocol, std::string username);
	~MyClientSocket();
	std::string getname();
	void bindServerAddr(int family, std::string addr, int port);

	//גûחûגאול bindServerAddr
	void connectServerSock(int family, std::string addr, int port);
	//void listensock(int qsize);
	//bool HandleConnetctions();
	void RunSession();

};