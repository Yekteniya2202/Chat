// Client.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#pragma comment(lib,"Ws2_32.lib")
#include <Winsock2.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include "MyClientSocket.h"
#pragma warning(disable : 4996) 

// TCP-порт сервера
#define SERVICE_PORT 1234

int main(int argc, char** argv)
try
{

    if (argc >= 2) {
        std::string name = argv[1];
        name.insert(name.begin(), '/');
        MyClientSocket S(AF_INET, SOCK_STREAM, 0, name);
        S.connectServerSock(AF_INET, "127.0.0.1", SERVICE_PORT);
        S.RunSession();
    }
    else {
        std::cout << "No username!" << std::endl;
    }

    return 0;
}
catch (std::exception& ex) {
    std::cout << ex.what() << std::endl;
    system("pause");
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
