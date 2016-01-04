// EchoTCPServerDemo.cpp : �������̨Ӧ�ó������ڵ㡣
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

// ���ӵ�winsock2��Ӧ��lib�ļ��� Ws2_32.lib 
#pragma comment (lib, "Ws2_32.lib") 

#define DEFAULT_BUFLEN 1024    //Ĭ�ϻ���������
#define DEFAULT_PORT "27015"  //Ĭ�Ϸ������˿ں�Ϊ27015

DWORD WINAPI AnewThread(LPVOID lParam);

int main(void)
{
	WSADATA wsaData;
	int iResult;
	SOCKET ListenSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int recvbuflen = DEFAULT_BUFLEN;


	// ��ʼ�� Winsock 
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSA����ʧ�ܣ�������: %d\n", iResult);
		return 1;
	}
	ZeroMemory(&hints, sizeof(hints));

	//����IPv4��ַ�壬��ʽ�׽��֣�TCPЭ��
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	// ���÷�������ַ�Ͷ˿ں�



	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("��ȡ��ַʧ�ܣ�������: %d\n", iResult);
		WSACleanup();
		return 1;
	}



	// Ϊ�������ӵķ����������׽���
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("Socket����ʧ�ܣ�������: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	

	// Ϊ�׽��ְ󶨵�ַ�Ͷ˿ں�
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);



	if (iResult == SOCKET_ERROR)
	{
		printf("��ʧ�ܣ�������: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);

	// ������������
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("����ʧ�ܣ�������: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	
	printf("TCP��������\n");

	struct sockaddr_in sa;
	int len = sizeof(sa);
		
		
	while (1) {

		SOCKET ClientSocket = INVALID_SOCKET;

		// ���ܿͻ����������󣬷��������׽���ClientSocket 
			ClientSocket = accept(ListenSocket, (SOCKADDR*)&sa, &len); //accept(ListenSocket, NULL, NULL);

		if (!getpeername(ClientSocket, (struct sockaddr *)&sa, &len))
		{
			printf("\n---------------------------------\n�ͻ���IP��ַ��%s \n", inet_ntoa(sa.sin_addr));
			printf("�ͻ��˶˿ڵ�ַ��%d \n\n", ntohs(sa.sin_port));
		}

		if (ClientSocket == INVALID_SOCKET)
		{
			int errorid = WSAGetLastError();
			if (errorid == WSAEWOULDBLOCK) //�޷�������ɷ�����socket�ϵĲ���
			{
				printf("�޷�������ɷ�����socket�ϵĲ���\n");
				Sleep(100);
			}
			closesocket(ClientSocket);
			continue;
		}
		cout << "---------------------------------\n�ͻ������ӳɹ�\n";
		

		if (ClientSocket == INVALID_SOCKET)
		{
			printf("����ʧ�ܣ�������: %d\n", WSAGetLastError());
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
	cout << "�ļ���Ϊ  " << filename << endl;
	

	iResult = recv(ClientSocket, recvbuf, sizeof(char *), 0);
	if (iResult >0 )
	{
		invfilelen = *((int *)recvbuf);
		cout << "���ݳ��ȵ������ֽ���Ϊ  " << invfilelen << endl;
		filelen = ntohl(invfilelen);
		cout << "���ݳ���Ϊ  " << filelen << endl;
	}
	file = fopen(filename, "wb+");
	if (file == NULL) { cout << "���ļ�ʧ�ܣ�" << endl; return -1; }

	int torecvlen = filelen;
	// �����������ݣ�ֱ���Է��ر����� 
	do
	{
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			cout << "����" << torecvlen << "���ֽ���Ҫ����" << endl;
			int recvlen=iResult;
			if (iResult > torecvlen)recvlen = torecvlen;
			fwrite(recvbuf, 1, recvlen, file);
			torecvlen -= recvlen;
			if (torecvlen <= 0)break;

			//���1���ɹ����յ�����
			//printf("���յ�����:  %s(%d)\n", recvbuf, iResult);

		}
		else if (iResult == 0)
		{
			//���2�����ӹر�
			printf("���ӹر�...\n");
		}
		else
		{
			//���3�����շ�������
			printf("���շ������󣡴�����: %d\n", WSAGetLastError());
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