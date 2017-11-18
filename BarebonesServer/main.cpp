#include<iostream>
#include<string>
#include<WS2tcpip.h>
#include<signal.h>



#pragma comment (lib, "ws2_32.lib")

using namespace std;
sockaddr_in* adresses[100];

char buff[4096]; //для тестирования сигнала
void handler(int sig);

void main() {

	signal(SIGINT, &handler);
	u_long cmdptr = 1;
	ZeroMemory(buff, 4096);

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
	sockaddr_in hint, clientaddr;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(5223);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	//Сказать винсокету, что сокет слушает
	listen(listening, SOMAXCONN);

	//Перевод сокета в неблокирующий режим
	/*u_long cmdptr = 1;
	int nonBlock = ioctlsocket(listening, FIONBIO, (u_long*)&cmdptr);
	if (nonBlock == SOCKET_ERROR)
	{
		cerr << "Turning on non blocking mode failed." << endl;
	}*/

	fd_set master;
	FD_ZERO(&master);
	FD_SET(listening, &master);

	int iSize;
	int startWrite = 0;

	while (true) {
		//Sleep(1);
		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		char idle[] = "";
		char clientAccepted[] = "";
		char host[NI_MAXHOST];

		for (int i = 0; i < socketCount; i++) {
			SOCKET sock = copy.fd_array[i];
			if (sock == listening) {
				//char buff[4096];
				//ZeroMemory(buff, 4096);
				//char idle[] = "";
				char clientAccepted[] = "";
				iSize = sizeof(clientaddr); //
				SOCKET client = accept(listening, (struct sockaddr *)&clientaddr, &iSize);///
				int nonBlock = ioctlsocket(client, FIONBIO, (u_long*)&cmdptr);
				if (nonBlock == SOCKET_ERROR)
				{
					cerr << "Turning on non blocking mode failed." << endl;
				}
				int count = master.fd_count;
				adresses[count-1] = (struct sockaddr_in *)&clientaddr;
				FD_SET(client, &master);
				//sprintf(idle,"%s%d%s","[",(int)client, "]: idle\n");
				//cout << idle;
				ZeroMemory(host, NI_MAXHOST);
				inet_ntop(AF_INET, &adresses[count-1]->sin_addr, host, NI_MAXHOST);//&clientaddr.sin_addr
				sprintf(clientAccepted,"%s%d%s%s%s","[",(int)client,"]: accept new client ",host ,"\n");
				cout << clientAccepted;
			}
			else {
				char buff[4096];
				ZeroMemory(buff, 4096);
				char clientDisconnected[] = "";
				int bytesIn = recv(sock, buff, 4096, 0);
				if (bytesIn <= 0) {
					closesocket(sock);
					FD_CLR(sock, &master);
					ZeroMemory(host, NI_MAXHOST);
					inet_ntop(AF_INET, &adresses[i]->sin_addr, host, NI_MAXHOST);
					sprintf(clientDisconnected, "%s%d%s%s%s", "[", (int)sock, "]: client ", host, " disconnected\n");
					cout << clientDisconnected;
					adresses[i] = NULL;
					int k = i;
					k++;
					while (adresses[k] != NULL && k!=0) {
						adresses[k - 1] = adresses[k];
						k++;
					}
				}
				else {
					cout << string(buff, 0, bytesIn) << endl; // ? Проверить
					send(sock, buff, strlen(buff) + 1, 0);
					//send(client, wel.c_str(), wel.size() + 1, 0);
				}
			}
		}
	}
	
	WSACleanup();
	system("pause");
}


void handler(int sig)
{
	string filename = "backup.txt";
	FILE *tempfile = fopen("tmp\\backup.txt", "wt");
	if (tempfile == 0) {
		cout << "Can't open file" << endl;
	}
	fwrite(buff, sizeof(char), 4096, tempfile);
	fclose(tempfile);

	cout << filename << endl;

	ZeroMemory(buff, 4096);
}

/*
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
closesocket(clientSocket);*/