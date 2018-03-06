/*
	简单的HTTP的服务器。
	工作方式：阻塞+多线程
	功能：能访问通，且可以解析HTTP请求，并处理返回相应网页和数据

*/

#pragma once 
#include<winsock2.h>
#include<thread>

//信息格式
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
	//常量定义
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
	bool isActive[MAX];		//当前活动连接
	int serverSocket;
	int clientSocket;
	friend void handleMessage(message msg);
	friend void listenForClose(closeMessage msg);
};

