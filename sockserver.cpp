//2019023436_������
#define _WINSOCK_DEPRECATED_NO_WARNINGS		// ���� ������ ������Ʈ�Կ� ���� ������ ���Ǵ� �Լ��� �����ϴ� ���� ����

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>  // ����, �����쿡�� ����� �����ϴ� ���
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT		7777
#define MAX_CLIENT	3

void InitServer(); // ������ �����ϰ� Ŭ���̾�Ʈ ������ �����
void RunServer();  // Ŭ���̾�Ʈ�� ��� �����ߴٸ� �����带 ����� Ŭ���̾�Ʈ 2�κ��� �޼����� �޾Ƽ� �ٸ� Ŭ���̾�Ʈ�鿡�� ����
void CloseServer(); // Ŭ���̾�Ʈ�� ������ ��� �����ϰ� ���� ���ϵ� �����Ѵ�.
void ErrorHandling(const char*);	// ����ó�� �Լ�

unsigned _stdcall Client1RecvThread(void*);			// Ŭ���̾�Ʈ 1�κ��� �޼����� �޾Ƽ� �ٸ� Ŭ���̾�Ʈ�鿡�� ����
unsigned _stdcall Client3RecvThread(void*);			// Ŭ���̾�Ʈ 3�κ��� �޼����� �޾Ƽ� �ٸ� Ŭ���̾�Ʈ�鿡�� ����

WSADATA wsaData;
SOCKET servSock, clientSock[MAX_CLIENT + 1];			// ��ũ����
SOCKADDR_IN servAddr, clientAddr[MAX_CLIENT + 1];	// �ּҿ� ���õ� ����ü

int clientAddrLen = 0;	// Ŭ���̾�Ʈ �ּ� ������ ������ �ִ� �������� ����
char client1Cmd[100];	// client 1�κ��� �޴� ���
char client2Cmd[100];	// client 2�κ��� �޴� ���
char client3Cmd[100];	// client 3�κ��� �޴� ���

HANDLE hand1;
HANDLE hand3;

int main(int argc, char* argv[])
{
	system("mode con cols=50 lines=30");		// �ܼ� ũ�� ����

	InitServer();

	RunServer();

	// �����尡 ��� ����� ������ ��ٸ���
	WaitForSingleObject(hand1, INFINITE);
	WaitForSingleObject(hand3, INFINITE);

	CloseHandle(hand1);
	CloseHandle(hand3);

	printf("BYE!\n");

	return 0;
}

void InitServer()
{
	// ���� ���̺귯�� �ʱ�ȭ (����, IP ����(V4, V6))
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		ErrorHandling("WSAStartup() error!");
	}

	// ��ȭ�⸦ ��ġ�Ѵ�.							
	servSock = socket(AF_INET, SOCK_STREAM, 0);

	if (INVALID_SOCKET == servSock) {
		ErrorHandling("socket() error!");
	}

	// ���� ���� �ʱ�ȭ								
	memset(&servAddr, 0x00, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);		// ������ IP �ʱ�ȭ
	servAddr.sin_port = PORT;							// ��Ʈ �ʱ�ȭ

	// ���Ͽ� IP �ּҿ� PORT ��ȣ ����
	if (SOCKET_ERROR == bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr))) {	
		ErrorHandling("bind() error");
	}

	// Ŭ���̾�Ʈ ���� ���
	if (SOCKET_ERROR == listen(servSock, MAX_CLIENT)) {	
		ErrorHandling("listen() error");
	}

	fputs("���� ���� �����...\n", stdout);

	clientAddrLen = sizeof(clientAddr[0]);	// Ŭ���̾�Ʈ�� �ּ� ������ ���̴� �������� �����ϱ� ������ �߿��� ���

	for (int idx = 1; idx <= MAX_CLIENT; idx++) {

		// Ŭ���̾�Ʈ ���� ����
		clientSock[idx] = accept(servSock, (SOCKADDR*)&(clientAddr[idx]), &clientAddrLen);

		if (INVALID_SOCKET == clientSock[idx]) {
			ErrorHandling("accept() error");
		}
		else {
			printf("Client %d Connection Complete! \n", idx);
		}

	}
}

void RunServer()
{
	printf("START Chatting!\n");

	// 1,3 player ���� ������
	hand1 = (HANDLE)_beginthreadex(NULL, 0, Client1RecvThread, 0, 0, NULL);
	hand3 = (HANDLE)_beginthreadex(NULL, 0, Client3RecvThread, 0, 0, NULL);

	// 2 player�� ��û�� ó���ϴ� ����
	while (1) {

		int res = 0;
		memset(client2Cmd, 0x00, sizeof(client2Cmd));

		res = recv(clientSock[2], client2Cmd, sizeof(client2Cmd) - 1, 0);		// 2�����κ��� ���� �����͸� �Ѿ��� ���. �� ������.

		if (-1 == res) {
			break;
		}

		if (0 == strcmp(client2Cmd, "exit")) {
			send(clientSock[1], "close", sizeof("close") - 1, 0);
			send(clientSock[2], "close", sizeof("close") - 1, 0);
			send(clientSock[3], "close", sizeof("close") - 1, 0);
			CloseServer();
			break;
		}

		if (0 == client2Cmd[0] && 0 == client2Cmd[1]) {
			continue;
		}

		send(clientSock[1], client2Cmd, sizeof(client2Cmd) - 1, 0);
		send(clientSock[3], client2Cmd, sizeof(client2Cmd) - 1, 0);

		printf("[SYSTEM] Client 2�� �޼����� �����߽��ϴ� \n2p : %s \n", client2Cmd);

	}
}

unsigned _stdcall Client1RecvThread(void* args)
{
	while (1) {

		int res = 0;
		memset(client1Cmd, 0x00, sizeof(client1Cmd));

		res = recv(clientSock[1], client1Cmd, sizeof(client1Cmd) - 1, 0);		// 1�����κ��� ���� �����͸� �Ѿ��� ���. �� ������.

		if (-1 == res) {
			break;
		}
		
		if (0 == strcmp(client1Cmd, "exit")) {
			send(clientSock[1], "close", sizeof("close") - 1, 0);
			send(clientSock[2], "close", sizeof("close") - 1, 0);
			send(clientSock[3], "close", sizeof("close") - 1, 0);
			CloseServer();
			break;
		}

		if (0 == client1Cmd[0] && 0 == client1Cmd[1]) {
			continue;
		}

		send(clientSock[2], client1Cmd, sizeof(client1Cmd) - 1, 0);
		send(clientSock[3], client1Cmd, sizeof(client1Cmd) - 1, 0);

		printf("[SYSTEM] Client 1�� �޼����� �����߽��ϴ� \n1p : %s \n", client1Cmd);

	}

	return 0;
}

unsigned _stdcall Client3RecvThread(void* args)
{
	while (1) {

		int res = 0;
		memset(client3Cmd, 0x00, sizeof(client3Cmd));

		res = recv(clientSock[3], client3Cmd, sizeof(client3Cmd) - 1, 0);		// 3�����κ��� ���� �����͸� �Ѿ��� ���. �� ������.

		if (-1 == res) {
			break;
		}

		if (0 == strcmp(client3Cmd, "exit")) {
			send(clientSock[1], "close", sizeof("close") - 1, 0);
			send(clientSock[2], "close", sizeof("close") - 1, 0);
			send(clientSock[3], "close", sizeof("close") - 1, 0);
			CloseServer();
			break;
		}

		if (0 == client3Cmd[0] && 0 == client3Cmd[1]) {
			continue;
		}

		send(clientSock[1], client3Cmd, sizeof(client3Cmd) - 1, 0);
		send(clientSock[2], client3Cmd, sizeof(client3Cmd) - 1, 0);

		printf("[SYSTEM] Client 3�� �޼����� �����߽��ϴ� \n3p : %s \n", client3Cmd);

	}

	return 0;
}

void CloseServer()
{
	printf("\n");
	for (int idx = 0; idx < MAX_CLIENT; idx++) {
		closesocket(clientSock[idx]);
		printf("Client %d closed.\n", idx + 1);
	}

	closesocket(servSock);
	printf("\nServer closed.\n");

	WSACleanup();
}

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stdout);
	exit(-1);

}