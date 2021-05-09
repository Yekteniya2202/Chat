#include "MySocket.h"
#include <thread>
#include <vector>
#include <mutex>
#include <map>

std::mutex broadcast_mutex;
//для пользователя задаётся вектор пар - другой сокет и булевское значение - может ли общаться 
std::map<SOCKET, std::map<SOCKET, bool>> relation_map;
std::map<SOCKET, std::string> socket_user;

std::mutex users_mutex;
void MySocket::HandleMessages(SOCKET sock, std::string& receivedmsg, std::string user_to_handle)
{
	std::string servermsg1;
	//команда времени
	if (strcmp(receivedmsg.c_str(), "TIME") == 0) {
		time_t t;
		struct tm* t_m;
		t = time(NULL);
		t_m = localtime(&t);
		std::stringstream ss;
		std::string h(std::to_string(t_m->tm_hour)), m(std::to_string(t_m->tm_min)), s(std::to_string(t_m->tm_sec));
		if (std::stoi(h) < 10) h.insert(h.begin(), '0');
		if (std::stoi(m) < 10) m.insert(m.begin(), '0');
		if (std::stoi(s) < 10) s.insert(s.begin(), '0');
		ss << t_m->tm_mday << '/' << t_m->tm_mon + 1 << '/' << t_m->tm_year + 1900 << ' '
			<< h << ":" << m << ":" << s;
		servermsg1 = "Server time - ";
		servermsg1 += ss.str();
		servermsg1 += "\r\n";
		send_string(sock, servermsg1.c_str(), servermsg1.size(), 0);
	}
	//принимаем сообщения с командами
	//просмотр текущих пользователей
	else if (receivedmsg.substr(0, 6) == "ONLINE")
	{
		servermsg1 += "Currently online: ";
		for (auto& [usock, user_name] : socket_user)
		{
			servermsg1 += user_name;
			servermsg1 += ' ';
		}
		
		servermsg1 += "\r\n";
		send_string(sock, servermsg1.c_str(), servermsg1.size(), 0);
	}
	//личное сообщение пользователю
	else if (receivedmsg.substr(0, 2) == "LS")
	{
		std::string rwithout_command = receivedmsg.substr(3); // получаем сообщение без команды LS
		std::stringstream ss;
		std::string msg;
		ss << rwithout_command; //пихаем в поток
		//получаем пользователя
		std::string dest_user;
		ss >> dest_user; // до пробела вытягиваем пользователя
		dest_user.insert(dest_user.begin(), '/');
		ss.ignore(1); // пропускаем пробел
		std::getline(ss, msg); // получаем остальное сообщение



		servermsg1 = "";
		servermsg1 += socket_user[sock];
		servermsg1 += " (LS): ";
		servermsg1 += msg;
		servermsg1 += "\r\n";
		//синхронизируем отправку сообщений между потоками
		broadcast_mutex.lock();
		for (auto& [usock, user_name] : socket_user) {
			//если это наш пользователь И он нас не заблокировал
			if (user_name == dest_user && relation_map[usock][sock]) {
				send_string(usock, servermsg1.c_str(), servermsg1.size(), 0);
			}
		}
		broadcast_mutex.unlock();
	}
	//заблокировать пользователя
	//необходимо сделать карту отношений - кто кого заблокировал
	else if (receivedmsg.substr(0, 5) == "BLOCK")
	{
		std::string rwithout_command = receivedmsg.substr(6); // получаем сообщение без команды LS
		std::string user_to_block = rwithout_command;
		user_to_block.insert(user_to_block.begin(), '/');

		broadcast_mutex.lock();
		for (auto& [Sock, username] : socket_user) {
			if (username == user_to_block) {
				//ключ NS - получаем свой вектор отношений

				relation_map[sock][Sock] = false;
			}
		}
		broadcast_mutex.unlock();

	}
	else // если не команда - отправить всем сообщение, кроме себя
	{
		servermsg1 = "";
		servermsg1 += socket_user[sock];
		servermsg1 += ': ';
		servermsg1 += receivedmsg;
		servermsg1 += "\r\n";
		//синхронизируем отправку сообщений между потоками
		//PrintRelations();
		broadcast_mutex.lock();
		for (auto& [usock, user_name] : socket_user) {
			
			if (usock == sock) continue; //не отсылая сообщение самому себе
			if (relation_map[usock][sock]) {
				send_string(usock, servermsg1.c_str(), servermsg1.size(), 0); //отсылаем его другим
			}
		}
		broadcast_mutex.unlock();
	}
}

void RunPS(MySocket& s, sockaddr_in clnt_addr, SOCKET NS)
{
	std::cout << "NS = " << NS << std::endl;
	s.RunPersonalSession(clnt_addr, NS);

}

void MySocket::RunSession()
{
	sockaddr_in clnt_addr;
	int addrlen = sizeof(clnt_addr);
	memset(&clnt_addr, 0, sizeof(clnt_addr));


	NS = accept(S, (sockaddr*)&clnt_addr, &addrlen); // получаем сокет
	if (NS == INVALID_SOCKET)
	{
		throw std::exception("Can not accept connection");
	}

	//с сокета сразу же необходимо считать имя пользователя
	std::thread t1(RunPS, std::ref(*this), clnt_addr, NS);
	t1.detach();

	// закрываем присоединенный сокет
}

void MySocket::RunPersonalSession(sockaddr_in clnt_addr, SOCKET NS)
{

	int addrlen = sizeof(serv_addr);
	getsockname(NS, (sockaddr*)&serv_addr, &addrlen);
	printf("Accepted connection on %s:%d ",
		inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
	printf("from client %s:%d\n",
		inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

	std::string servermsg1 = "";

	///////НАЧАЛО СЕССИИ

	std::string user_to_handle = "";
	bool welcome = true;
	while (true)
	{
		char sReceiveBuffer1[1024] = { 0 };
		servermsg1 = "";
		int nReaded = recv(NS, sReceiveBuffer1, sizeof(sReceiveBuffer1) - 1, 0);
		// В случае ошибки (например, отсоединения клиента) выходим
		if (nReaded <= 0) break;
		// Мы получаем поток байт, поэтому нужно самостоятельно 
		// добавить завержающий 0 для ASCII строки 
		sReceiveBuffer1[nReaded] = 0;

		// Отбрасываем символы превода строк
		for (char* pPtr = sReceiveBuffer1; *pPtr != 0; pPtr++)
		{
			if (*pPtr == '\n' || *pPtr == '\r')
			{
				*pPtr = 0;
				break;
			}
		}
		// Пропускаем пустые строки
		if (sReceiveBuffer1[0] == 0) continue;

		//Здесь получаем имя со слешем '/'
		std::string receivedmsg = sReceiveBuffer1;
		printf("Received data: %s\n", sReceiveBuffer1);

		if (receivedmsg[0] == '/') { //если принимаем слеш - регистрируем пользователя (а старого удаляем, иначе словим утечку памяти)

			DeleteUser(NS);
			//записали себя
			socket_user[NS] = receivedmsg;

			std::map<SOCKET, bool> tmp_relations;
			//устанавливаем отношения нового пользователя у всех
			for (auto& [sock, username] : socket_user) { //пробегаемся по остальным сокетам
				if (sock == NS) continue; //встретили себя - пропускаем
				//пусть для нового пользователя все - друзья
				tmp_relations[sock] = true;
			}

			broadcast_mutex.lock();
			//записываем для данного пользователя
			relation_map[NS] = tmp_relations;
			
			//для остальных добавить этого пользователя и выставить отношения в true

			for (auto& [sock, map] : relation_map) {
				if (sock == NS) continue;
				map[NS] = true;
			}
			
			broadcast_mutex.unlock();

			PrintRelations();

			//добавляем пару
			//ключ - сокет
			//значение - пользователь под этим сокетом
		}

		//единожды приветствуем пользователя
		if (welcome)
		{
			std::string servermsg1 = "* * * Welcome to simple UPCASE TCP-server * * *\r\n";
			send_string(NS, servermsg.c_str(), servermsg.size(), 0);
			welcome = false;
		}
		//далее - обработка сообщений
		HandleMessages(NS, receivedmsg, user_to_handle);
	}

	closesocket(NS);
	DeleteUser(NS);
}

void MySocket::PrintRelations()
{
	broadcast_mutex.lock(); 
	for (auto [sock, map] : relation_map) {
		std::cout << "User: " << socket_user[sock] << ". Relations:" << std::endl;
		for (auto [othersock, bool_relation] : map) {
			std::cout << socket_user[othersock] << ':' << bool_relation << std::endl;
		}
	}
	broadcast_mutex.unlock();
}

void MySocket::DeleteUser(SOCKET NS)
{
	users_mutex.lock();
	socket_user.erase(NS);
	for (auto& [sock, map] : relation_map) { //пробегаем по ассоциативному массиву каждого пользователя
		if (sock == NS) continue; // пропуская удаляемого пользователя
		map.erase(NS); // удаляем у остальных его
	}
	relation_map.erase(NS);
	users_mutex.unlock();
}




int MySocket::send_string(SOCKET sock, const char* sString, int len, int flags)
{
	int total = 0;
	int n = -1;
	while (total < len)
	{
		n = send(sock, sString + total, len - total, flags);
		if (n == -1) break;
		total += n;
	}
	return (n == -1 ? -1 : total);
}

MySocket::MySocket(int family, int type, int protocol)
{
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	if ((S = socket(family, type, protocol)) == INVALID_SOCKET)
	{
		throw std::exception("Can not create socket!");
	}
}

MySocket::~MySocket()
{
	WSACleanup();
}

std::string MySocket::getname()
{
	gethostname(sName, sizeof(sName));
	return std::string(sName);
}

void MySocket::bindsock(int family, std::string addr, int port)
{
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = family;
	// Разрешаем работу на всех доступных сетевых интерфейсах,
	// в частности на localhost
	serv_addr.sin_addr.s_addr = inet_addr(addr.c_str());
	// обратите внимание на преобразование порядка байт
	serv_addr.sin_port = htons((u_short)port);
	if (bind(S, (sockaddr*)&serv_addr, sizeof(serv_addr)) == INVALID_SOCKET)
	{
		throw std::exception("Can not bind socket!");
	}
}

void MySocket::listensock(int qsize)
{
	if (listen(S, qsize) == INVALID_SOCKET)
	{
		throw std::exception("Can not listen!");
	}
}

bool MySocket::HandleConnetctions()
{
	return !bTerminate;
}
