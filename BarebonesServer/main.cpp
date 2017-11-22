#include<iostream>
#include<string>
#include<WS2tcpip.h>
#include<signal.h>
#include<map>
#include<vector>
#include <fstream>

#pragma comment (lib, "ws2_32.lib")

using namespace std;
map<int, string> adresses;
vector<string> buffer;

void handler(int sig);
struct timeval timeout;
int desc;

void main() {
	signal(SIGINT, &handler);
	u_long cmdptr = 1;

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

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
	int nonBlock = ioctlsocket(listening, FIONBIO, (u_long*)&cmdptr);
	if (nonBlock == SOCKET_ERROR)
	{
		cerr << "Turning on non blocking mode failed." << endl;
	}

	fd_set master;
	FD_ZERO(&master);
	FD_SET(listening, &master);

	int iSize;
	char host[NI_MAXHOST];

	SOCKET sock;
	desc = (int)listening;

	while (true) {
		char idle[] = "";
		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, &timeout);
		sprintf(idle, "%s%d%s", "[", desc , "]: idle\n");
		cout << idle;
		buffer.push_back(idle);


		for (int i = 0; i < socketCount; i++) {
			sock = copy.fd_array[i];
			desc = (int)sock;
			if (sock == listening) {
				char clientAccepted[] = "";
				iSize = sizeof(clientaddr); 
				SOCKET client = accept(listening, (struct sockaddr *)&clientaddr, &iSize);
				int nonBlock = ioctlsocket(client, FIONBIO, (u_long*)&cmdptr);
				if (nonBlock == SOCKET_ERROR)
				{
					cerr << "Turning on non blocking mode failed." << endl;
				}
				int count = master.fd_count;
				struct sockaddr_in *s = (struct sockaddr_in *)&clientaddr;
				FD_SET(client, &master);
				ZeroMemory(host, NI_MAXHOST);
				inet_ntop(AF_INET, &s->sin_addr, host, NI_MAXHOST);
				adresses.insert(pair<int, string>((int)client,string(host)));
				sprintf(clientAccepted,"%s%d%s%s%s","[",(int)client,"]: accept new client ", host ,"\n");
				cout << clientAccepted;
				buffer.push_back(clientAccepted);
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
					sprintf(clientDisconnected, "%s%d%s", "[", (int)sock, "]: client ");
					string result = clientDisconnected + adresses.find((int)sock)->second + " disconnected\n";
					cout << result;
					buffer.push_back(result);
					adresses.erase((int)sock);
				}
				else {
					char message[] = "";
					sprintf(message, "%s%d%s", "[", (int)sock, "]: ");
					string result = message + string(buff, 0, bytesIn) + "\n";
					cout << result;
					buffer.push_back(result);
					send(sock, buff, strlen(buff) + 1, 0);
				}
			}
		}
	}
	
	WSACleanup();
	system("pause");
}


void handler(int sig)
{
	string filename = "buffer.txt";
	ofstream fout("buffer.txt"); // создаём объект класса ofstream для записи и связываем его с файлом buffer.txt
	for (int i = 0; i < buffer.size(); i++) {
		fout << buffer[i]; // запись строки в файл
	}
	fout.close();
	cout << filename << endl;
	buffer.clear();
}