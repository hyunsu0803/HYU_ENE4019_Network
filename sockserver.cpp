//2019023436_김현수
#define _WINSOCK_DEPRECATED_NO_WARNINGS		// 소켓 버전을 업데이트함에 따라 이전에 사용되던 함수를 금지하는 것을 무시

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>  // 소켓, 윈도우에서 통신을 관장하는 헤더
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT		7777
#define MAX_CLIENT	3

void InitServer(); // 소켓을 생성하고 클라이언트 접속을 대기함
void RunServer();  // 클라이언트가 모두 접속했다면 스레드를 만들고 클라이언트 2로부터 메세지를 받아서 다른 클라이언트들에게 보냄
void CloseServer(); // 클라이언트의 소켓을 모두 종료하고 서버 소켓도 종료한다.
void ErrorHandling(const char*);	// 예외처리 함수

unsigned _stdcall Client1RecvThread(void*);			// 클라이언트 1로부터 메세지를 받아서 다른 클라이언트들에게 보냄
unsigned _stdcall Client3RecvThread(void*);			// 클라이언트 3로부터 메세지를 받아서 다른 클라이언트들에게 보냄

WSADATA wsaData;
SOCKET servSock, clientSock[MAX_CLIENT + 1];			// 디스크립터
SOCKADDR_IN servAddr, clientAddr[MAX_CLIENT + 1];	// 주소에 관련된 구조체

int clientAddrLen = 0;	// 클라이언트 주소 정보를 가지고 있는 데이터의 길이
char client1Cmd[100];	// client 1로부터 받는 명령
char client2Cmd[100];	// client 2로부터 받는 명령
char client3Cmd[100];	// client 3로부터 받는 명령

HANDLE hand1;
HANDLE hand3;

int main(int argc, char* argv[])
{
	system("mode con cols=50 lines=30");		// 콘솔 크기 조절

	InitServer();

	RunServer();

	// 스레드가 모두 종료될 때까지 기다린다
	WaitForSingleObject(hand1, INFINITE);
	WaitForSingleObject(hand3, INFINITE);

	CloseHandle(hand1);
	CloseHandle(hand3);

	printf("BYE!\n");

	return 0;
}

void InitServer()
{
	// 소켓 라이브러리 초기화 (버전, IP 형식(V4, V6))
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		ErrorHandling("WSAStartup() error!");
	}

	// 전화기를 설치한다.							
	servSock = socket(AF_INET, SOCK_STREAM, 0);

	if (INVALID_SOCKET == servSock) {
		ErrorHandling("socket() error!");
	}

	// 소켓 정보 초기화								
	memset(&servAddr, 0x00, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);		// 서버의 IP 초기화
	servAddr.sin_port = PORT;							// 포트 초기화

	// 소켓에 IP 주소와 PORT 번호 결합
	if (SOCKET_ERROR == bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr))) {	
		ErrorHandling("bind() error");
	}

	// 클라이언트 연결 대기
	if (SOCKET_ERROR == listen(servSock, MAX_CLIENT)) {	
		ErrorHandling("listen() error");
	}

	fputs("유저 접속 대기중...\n", stdout);

	clientAddrLen = sizeof(clientAddr[0]);	// 클라이언트의 주소 정보의 길이는 버전마다 상이하기 때문에 중요한 요소

	for (int idx = 1; idx <= MAX_CLIENT; idx++) {

		// 클라이언트 연결 수락
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

	// 1,3 player 수신 쓰레드
	hand1 = (HANDLE)_beginthreadex(NULL, 0, Client1RecvThread, 0, 0, NULL);
	hand3 = (HANDLE)_beginthreadex(NULL, 0, Client3RecvThread, 0, 0, NULL);

	// 2 player의 요청을 처리하는 로직
	while (1) {

		int res = 0;
		memset(client2Cmd, 0x00, sizeof(client2Cmd));

		res = recv(clientSock[2], client2Cmd, sizeof(client2Cmd) - 1, 0);		// 2번으로부터 오는 데이터를 한없이 대기. 올 때까지.

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

		printf("[SYSTEM] Client 2의 메세지가 도착했습니다 \n2p : %s \n", client2Cmd);

	}
}

unsigned _stdcall Client1RecvThread(void* args)
{
	while (1) {

		int res = 0;
		memset(client1Cmd, 0x00, sizeof(client1Cmd));

		res = recv(clientSock[1], client1Cmd, sizeof(client1Cmd) - 1, 0);		// 1번으로부터 오는 데이터를 한없이 대기. 올 때까지.

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

		printf("[SYSTEM] Client 1의 메세지가 도착했습니다 \n1p : %s \n", client1Cmd);

	}

	return 0;
}

unsigned _stdcall Client3RecvThread(void* args)
{
	while (1) {

		int res = 0;
		memset(client3Cmd, 0x00, sizeof(client3Cmd));

		res = recv(clientSock[3], client3Cmd, sizeof(client3Cmd) - 1, 0);		// 3번으로부터 오는 데이터를 한없이 대기. 올 때까지.

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

		printf("[SYSTEM] Client 3의 메세지가 도착했습니다 \n3p : %s \n", client3Cmd);

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