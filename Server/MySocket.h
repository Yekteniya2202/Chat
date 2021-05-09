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
class MySocket {
private:
	friend void RunPS(MySocket& s, sockaddr_in clnt_addr, SOCKET NS);
	SOCKET  S;  //дескриптор прослушивающего сокета
	SOCKET  NS; //дескриптор присоединенного сокета

	SOCKET CL1, CL2;
	sockaddr_in serv_addr;
	WSADATA     wsadata;
	char        sName[128];
	bool        bTerminate = false;
	std::string servermsg; char sReceiveBuffer[1024] = { 0 };
	void HandleMessages(SOCKET sock, std::string& receivedmsg, std::string user_to_handle);
	int send_string(SOCKET NS, const char* sString, int len, int flags);
public:
	MySocket(int family, int type, int protocol);
	~MySocket();
	std::string getname();
	void bindsock(int family, std::string addr, int port);
	void listensock(int qsize);
	bool HandleConnetctions(); 
	void RunSession();
	void RunPersonalSession(sockaddr_in clnt_addr, SOCKET NS);
	void PrintRelations();
	void DeleteUser(SOCKET NS);
};