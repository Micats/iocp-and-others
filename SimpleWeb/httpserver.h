/*
	�򵥵�HTTP�ķ�������
	������ʽ������+���߳�
	���ܣ��ܷ���ͨ���ҿ��Խ���HTTP���󣬲���������Ӧ��ҳ������

*/

#pragma once 
#include<winsock2.h>
#include<thread>

//��Ϣ��ʽ
struct message{
	char *data;
	bool *isActive;
	int clientSocket;
	int id;
	message(char* d, bool *is, int c, int i);
};

struct closeMessage
{
	bool *isActive;
	int serverSocket;
	closeMessage(bool *is,int s);
};

class HttpServer
{
public:
	HttpServer();
	~HttpServer(){};
	bool start();

private:
	//��������
	enum{
		serverPort = 8800,
		bufferSize=13000,
		queueSize=10,
		MAX=100
	};
	char buffer[bufferSize];
	sockaddr_in serverAddr;
	std::thread* t[MAX];
	std::thread* th;
	char rootDir[50];
	char name[50];
	bool isActive[MAX];		//��ǰ�����
	int serverSocket;
	int clientSocket;
	friend void handleMessage(message msg);
	friend void listenForClose(closeMessage msg);
};

