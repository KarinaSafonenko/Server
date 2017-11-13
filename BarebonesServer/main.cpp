#include<iostream>
#include<string>
#include<WS2tcpip.h>


#pragma comment (lib, "ws2_32.lib")

using namespace std;

void main() {
	// �������������� ��������
	WSADATA wsData;
	WORD ver = MAKEWORD(2,2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0) {
		cerr << "Can't initialized winsock! Quitting" << endl;
		return;
	}

	//������� �����
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		cerr << "Can't create a socket! Quitting" << endl;
		return;
	}

	//����������� ip � ���� ������
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(5223);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	//������� ���������, ��� ����� �������
	listen(listening, SOMAXCONN);

	//���� ����������
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	if (clientSocket == INVALID_SOCKET) {
		cerr << "Can't create a socket! Quitting" << endl;
		return;
	}

	char host[NI_MAXHOST];			//��� �������
	char service[NI_MAXHOST];		//���� �������

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		cout << host << " connected on port " << service << endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << "connected on port " << ntohs(client.sin_port) << endl;
	}

	//�������� ������

	closesocket(listening);


	// ������� � ������� ��������� �������
	char buff[4096];

	while (true) {
		ZeroMemory(buff, 4096);

		//���� ��������� �� �������
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

		//���������� ��������� �������
		send(clientSocket, buff, bytesReceived + 1, 0);
	}
	//��������� ���
	closesocket(clientSocket);
	WSACleanup();
	system("pause");
}