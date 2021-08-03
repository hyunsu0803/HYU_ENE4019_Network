//2019023436_김현수
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

void Chatting(void); // 채팅을 입력하여 sendToServer에 넘긴다. is_closed가 1이면 채팅을 종료한다. 

// Socket 관련 함수
void ErrorHandling(const char*); //에러메세지 출력
void InitConn();		// 소켓 생성, 연결
void CloseConn();		// 소켓 클로즈
void SendToServer(char*); // 메세지를 서버로 보낸다.
unsigned _stdcall RecvFromServer(void*); // 서버로부터 받은 메세지를 출력한다. 메세지가 close라면 CloseConn()을 호출해 소켓을 닫는다.

// Socket 관련 전역 변수
WSADATA wsaData;
SOCKET cSocket;
SOCKADDR_IN servAddr;
char message[100];

int is_closed; // 소켓이 클로즈 되었는지 상태 저장

int main(int argc, char* argv[])
{
	system("mode con cols=50 lines=25");		// 콘솔 크기 조절

	InitConn();

	_beginthreadex(NULL, 0, RecvFromServer, 0, 0, NULL); // 서버로부터 메세지를 받는 스레드 생성

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

	// 서버 소켓 정보 초기화
	memset(&servAddr, 0x00, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = PORT;

	// 연결
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
