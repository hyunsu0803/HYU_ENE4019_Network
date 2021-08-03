//2019023436_������
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <conio.h>
#include <process.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

#define PORT	7777

void Chatting(void); // ä���� �Է��Ͽ� sendToServer�� �ѱ��. is_closed�� 1�̸� ä���� �����Ѵ�. 

// Socket ���� �Լ�
void ErrorHandling(const char*); //�����޼��� ���
void InitConn();		// ���� ����, ����
void CloseConn();		// ���� Ŭ����
void SendToServer(char*); // �޼����� ������ ������.
unsigned _stdcall RecvFromServer(void*); // �����κ��� ���� �޼����� ����Ѵ�. �޼����� close��� CloseConn()�� ȣ���� ������ �ݴ´�.

// Socket ���� ���� ����
WSADATA wsaData;
SOCKET cSocket;
SOCKADDR_IN servAddr;
char message[100];

int is_closed; // ������ Ŭ���� �Ǿ����� ���� ����

int main(int argc, char* argv[])
{
	system("mode con cols=50 lines=25");		// �ܼ� ũ�� ����

	InitConn();

	_beginthreadex(NULL, 0, RecvFromServer, 0, 0, NULL); // �����κ��� �޼����� �޴� ������ ����

	Chatting();

	printf("end of Chatting\n");

	return 0;
}

unsigned _stdcall RecvFromServer(void* args)
{
	char message[100] = { 0, };

	while (1) {

		int res = 0;
		memset(message, 0x00, sizeof(message));

		res = recv(cSocket, message, sizeof(message) - 1, 0);

		if (-1 == res) {
			break;
		}

		if (0 == strncmp(message, "close", sizeof("close"))) {
			CloseConn();
			break;
		}

		printf("[Chatting] %s\n", message);

	}

	return 0;
}

void SendToServer(char* msg)
{
	strcpy(message, msg);

	send(cSocket, message, sizeof(message), 0);
}

void InitConn()
{
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		ErrorHandling("WSAStartup() error!");
	}

	cSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (INVALID_SOCKET == cSocket) {
		ErrorHandling("socket() error!");
	}

	// ���� ���� ���� �ʱ�ȭ
	memset(&servAddr, 0x00, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = PORT;

	// ����
	if (SOCKET_ERROR == connect(cSocket, (SOCKADDR*)&servAddr, sizeof(servAddr))) {
		ErrorHandling("connect() error!");
	}
}

void CloseConn()
{
	closesocket(cSocket);

	is_closed = 1;
	
	printf("\nClose client.\n");

	WSACleanup();
}

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stdout);
	exit(-1);

}

void Chatting(void)
{
	char cmd[100] = { 0 };

	while (1) {
		if (is_closed) {
			break;
		}

		std::cin.getline(cmd, 100);
	
		SendToServer(cmd);
	}
}
