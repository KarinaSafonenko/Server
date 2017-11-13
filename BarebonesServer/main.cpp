#include<iostream>
#include<string>
#include<WS2tcpip.h>


#pragma comment (lib, "ws2_32.lib")

using namespace std;

void main() {
	// Инициализируем винсокет
	WSADATA wsData;
	WORD ver = MAKEWORD(2,2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0) {
		cerr << "Can't initialized winsock! Quitting" << endl;
		return;
	}

	//Создаем сокет
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		cerr << "Can't create a socket! Quitting" << endl;
		return;
	}

	//Привязываем ip и порт сокету
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(5223);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	//Сказать винсокету, что сокет слушает
	listen(listening, SOMAXCONN);

	//Ждем соединения
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	if (clientSocket == INVALID_SOCKET) {
		cerr << "Can't create a socket! Quitting" << endl;
		return;
	}

	char host[NI_MAXHOST];			//Имя клиента
	char service[NI_MAXHOST];		//Порт сервера

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		cout << host << " connected on port " << service << endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << "connected on port " << ntohs(client.sin_port) << endl;
	}

	//Закрытие сокета

	closesocket(listening);


	// Принять и послать сообщение клиенту
	char buff[4096];

	while (true) {
		ZeroMemory(buff, 4096);

		//Ждем сообщения от клиента
		int bytesReceived = recv(clientSocket, buff, 4096, 0);

		if (bytesReceived == SOCKET_ERROR) {
			cerr << "Client disconnected" << endl;
			break;
		}

		if (bytesReceived == 0) {
			cout << "Client "<<host<<" disconnected" << endl;
			break;
		}

		cout << string(buff, 0, bytesReceived) << endl;

		//Отправляем сообщение слиенту
		send(clientSocket, buff, bytesReceived + 1, 0);
	}
	//Закрываем все
	closesocket(clientSocket);
	WSACleanup();
	system("pause");
}