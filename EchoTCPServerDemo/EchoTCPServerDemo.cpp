// EchoTCPServerDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#undef UNICODE 
#define WIN32_LEAN_AND_MEAN
#include <windows.h> 
#include <winsock2.h> 
#include <ws2tcpip.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <iostream>
using namespace std;

// 连接到winsock2对应的lib文件： Ws2_32.lib 
#pragma comment (lib, "Ws2_32.lib") 

#define DEFAULT_BUFLEN 1024    //默认缓冲区长度
#define DEFAULT_PORT "27015"  //默认服务器端口号为27015

DWORD WINAPI AnewThread(LPVOID lParam);

int main(void)
{
	WSADATA wsaData;
	int iResult;
	SOCKET ListenSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int recvbuflen = DEFAULT_BUFLEN;


	// 初始化 Winsock 
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSA启动失败！错误编号: %d\n", iResult);
		return 1;
	}
	ZeroMemory(&hints, sizeof(hints));

	//声明IPv4地址族，流式套接字，TCP协议
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	// 设置服务器地址和端口号



	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("获取地址失败！错误编号: %d\n", iResult);
		WSACleanup();
		return 1;
	}



	// 为面向连接的服务器创建套接字
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("Socket创建失败！错误编号: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	

	// 为套接字绑定地址和端口号
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);



	if (iResult == SOCKET_ERROR)
	{
		printf("绑定失败！错误编号: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);

	// 监听连接请求
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("监听失败！错误编号: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	
	printf("TCP服务开启！\n");

	struct sockaddr_in sa;
	int len = sizeof(sa);
		
		
	while (1) {

		SOCKET ClientSocket = INVALID_SOCKET;

		// 接受客户端连接请求，返回连接套接字ClientSocket 
			ClientSocket = accept(ListenSocket, (SOCKADDR*)&sa, &len); //accept(ListenSocket, NULL, NULL);

		if (!getpeername(ClientSocket, (struct sockaddr *)&sa, &len))
		{
			printf("\n---------------------------------\n客户端IP地址：%s \n", inet_ntoa(sa.sin_addr));
			printf("客户端端口地址：%d \n\n", ntohs(sa.sin_port));
		}

		if (ClientSocket == INVALID_SOCKET)
		{
			int errorid = WSAGetLastError();
			if (errorid == WSAEWOULDBLOCK) //无法立即完成非阻塞socket上的操作
			{
				printf("无法立即完成非阻塞socket上的操作\n");
				Sleep(100);
			}
			closesocket(ClientSocket);
			continue;
		}
		cout << "---------------------------------\n客户端连接成功\n";
		

		if (ClientSocket == INVALID_SOCKET)
		{
			printf("接收失败！错误编号: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}


		DWORD dwThreadID;
		CreateThread(NULL, NULL, AnewThread, (LPVOID)ClientSocket, 0, &dwThreadID);

		if (!AnewThread)CloseHandle(AnewThread);

		//getchar();
	}



	// cleanup 
	//closesocket(ClientSocket);
	WSACleanup();

	printf("press any key to continue");
	getchar();

	return 0;
}


DWORD WINAPI AnewThread(LPVOID lParam)
{

	int iResult=0;
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	SOCKET ClientSocket = (SOCKET)(LPVOID)lParam;

	FILE *file = NULL;
	char filename[100];
	int namelen=0;
	int filelen,invfilelen;
	char buf[1];
	while(1)
	{
		iResult = recv(ClientSocket, buf, 1, 0);
		if (iResult > 0)
		{
			if (buf[0] == '#')break;
			if (buf[0] == '\\'|| buf[0] == ':')continue;
			filename[namelen++] = buf[0];
		} 
	}
	filename[namelen] = '\0';
	cout << "文件名为  " << filename << endl;
	

	iResult = recv(ClientSocket, recvbuf, sizeof(char *), 0);
	if (iResult >0 )
	{
		invfilelen = *((int *)recvbuf);
		cout << "数据长度的网络字节序为  " << invfilelen << endl;
		filelen = ntohl(invfilelen);
		cout << "数据长度为  " << filelen << endl;
	}
	file = fopen(filename, "wb+");
	if (file == NULL) { cout << "打开文件失败！" << endl; return -1; }

	int torecvlen = filelen;
	// 持续接收数据，直到对方关闭连接 
	do
	{
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			cout << "还有" << torecvlen << "个字节需要接收" << endl;
			int recvlen=iResult;
			if (iResult > torecvlen)recvlen = torecvlen;
			fwrite(recvbuf, 1, recvlen, file);
			torecvlen -= recvlen;
			if (torecvlen <= 0)break;

			//情况1：成功接收到数据
			//printf("接收到数据:  %s(%d)\n", recvbuf, iResult);

		}
		else if (iResult == 0)
		{
			//情况2：连接关闭
			printf("连接关闭...\n");
		}
		else
		{
			//情况3：接收发生错误
			printf("接收发生错误！错误编号: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			return -1;
		}
	} while (iResult > 0);

	fclose(file);
	
	// shutdown the connection since we're done 
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ClientSocket);
	cout << "\n---------------------------------\n";
	return 0;
}