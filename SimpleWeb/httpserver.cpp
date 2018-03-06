#define _CRT_SECURE_NO_WARNINGS
#include"stdafx.h"
#include"httpserver.h"
#include<assert.h>
#include<string>
#include<iostream>
#include<fstream>
using namespace std;
  


message::message(char* d, bool *is, int c, int i) :data(d), isActive(is), clientSocket(c), id(i){};
closeMessage::closeMessage(bool* is, int s) : isActive(is), serverSocket(s) {};

void sendMessage(std::string path,message msg)
{
	std::ifstream in(path, std::ios::binary);
	int sp;
	if (!in)
	{
		strcpy(msg.data,"HTTP/1.1 404 Not Found\n");
		printf("file on open");
	}
	else
	{
		in.seekg(0,std::ios_base::end);
		sp = in.tellg();
		char length[20];
		sprintf(length, "%d", sp);
		strcpy(msg.data, "HTTP/1.1 200 OK\r\n");
		strcat(msg.data, "Content-Type: text/html;charset=ISO-8859-1\nContent-Length: ");
		strcat(msg.data, length);
		strcat(msg.data, "\r\n");
		int totalSize = 0;

	}
}

void handleMessage(message msg)
{
	int i = 0, cnt = 0;
	bool flag = false;
	bool post_flag = false;
	std::string str = "";
	std::string type = "";
	std::string data = "";
	printf("你好呀！我是线程%d\ndata = %s\n", msg.id, msg.data);
	if (msg.data == "" || msg.data == "\n") {
		*msg.isActive = false;
		return;
	}
	//解析http头部  
	while (1) {
		//if (msg.data == "\0")break;
		if (msg.data[i] == '\n' && msg.data[i + 2] == '\n')break;
		if (msg.data[i] == ' ') {
			if (flag) {
				data = str;
				flag = false;
				break;
			}
			else if (str == "GET") {
				type = str;
				flag = true;
			}
			else if (str == "POST") {
				type = str;

			}
			str = "";
		}
		else if (msg.data[i] == '\n');
		else {
			str = str + msg.data[i];
		}
		i++;
	}

	if (type == "POST") {

		bool login_flag = false;
		bool pass_flag = false;
		std::string name = "";
		std::string passwd = "";
		str = "";
		for (int j = i + 3; j <= strlen(msg.data); j++) {
			if (msg.data[j] == '&' || msg.data[j] == '=' || j == strlen(msg.data)) {
				std::cout << str << std::endl;
				if (login_flag) {
					if (str == "123") {
						name = str;
						passwd = "123";
					}
					else {
						passwd = "";
					}
					login_flag = false;
				}
				else if (pass_flag) {

					if (str == passwd && str != "") {
						std::cout << "str=" << str << " " << "paw=" << passwd << std::endl;
						char response[200];
						strcpy(response, "<html><body>欢迎您,");
						strcat(response, name.c_str());
						strcat(response, "!</body></html>\n");
						int len = strlen(response);
						char length[20];
						sprintf(length, "%d", len);
						strcpy(msg.data, "HTTP/1.1 200 OK\n");
						strcat(msg.data, "Content-Type: text/html;charset=gb2312\nContent-Length: ");
						strcat(msg.data, length);
						strcat(msg.data, "\n\n");
						strcat(msg.data, response);
						printf("%s\n", msg.data);
						int r = send(msg.clientSocket, msg.data, 10000, 0);

						if (r == SOCKET_ERROR) {
							printf("send failed\n");
							*msg.isActive = false;
							return;
						}
						printf("send success\n");
						*msg.isActive = false;
						return;
					}
					else {
						std::cout << "str=" << str << " " << "paw=" << passwd << std::endl;
						char response[200];
						strcpy(response, "<html><body>登录失败</body></html>\n");
						int len = strlen(response);
						char length[20];
						sprintf(length, "%d", len);
						strcpy(msg.data, "HTTP/1.1 200 OK\n");
						strcat(msg.data, "Content-Type: text/html;charset=gb2312\nContent-Length: ");
						strcat(msg.data, length);
						strcat(msg.data, "\n\n");
						strcat(msg.data, response);
						printf("%s\n", msg.data);
						int r = send(msg.clientSocket, msg.data, 10000, 0);

						if (r == SOCKET_ERROR) {
							printf("send failed\n");
							*msg.isActive = false;
							return;
						}
						printf("send success\n");
						*msg.isActive = false;
						return;
					}
					pass_flag = false;
				}
				else if (str == "login") {
					login_flag = true;
				}
				else if (str == "pass") {
					pass_flag = true;
				}
				if (j == data.size())break;
				str = "";
			}
			else {
				str = str + msg.data[j];
			}
		}
		*msg.isActive = false;
		return;
	}
	else if (type == "GET" && data != "") {

		memset(msg.data, 0, sizeof(msg.data));
		if (data.substr(0, 5) == "/net/") {
			std::string str = "";
			std::string str1 = "";
			std::string passwd;
			std::string name;
			std::string path;

			bool txt_flag = false;
			for (int i = 5; i < data.size(); i++) {
				if (data[i] == '.') {
					flag = true;
				}
				else if (flag) {
					str = str + data[i];
				}
			}

			if (str == "") {
				*msg.isActive = false;
				return;
			}
			std::cout << "str=" << str << "," << std::endl;
			if (str == "txt") {
				path = "catalog/txt/" + data.substr(5);
			}
			else if (str == "html") {
				std::cout << "yes" << std::endl;
				path = "catalog/html/" + data.substr(5);
				std::cout << "path=" << path << std::endl;
			}
			std::cout << "str=" << str << std::endl;
			sendMessage(path, msg);
		}
		else if (data.substr(0, 5) == "/img/") {
			int total_size;
			int s;
			std::string path = "catalog/img/" + data.substr(5);
			sendMessage(path, msg);
		}
		else if (data.substr(0, 1) == "/") {
			char response[200];
			strcpy(response, "<html><body>hello</body></html>\n");
			int len = strlen(response);
			char length[20];
			sprintf(length, "%d", len);
			strcpy(msg.data, "HTTP/1.1 200 OK\n");
			strcat(msg.data, "Content-Type: text/html;charset=gb2312\nContent-Length: ");
			strcat(msg.data, length);
			strcat(msg.data, "\n\n");
			strcat(msg.data, response);
			printf("%s\n", msg.data);
			int r = send(msg.clientSocket, msg.data, 10000, 0);

			if (r == SOCKET_ERROR) {
				printf("send failed\n");
				*msg.isActive = false;
				return;
			}
			printf("send success\n");
			*msg.isActive = false;
			return;
		}
	}
	closesocket(msg.clientSocket);
	*msg.isActive = false;
}
	
void listenForClose(closeMessage msg)
{
	std::string str;
	while (1) {
		std::cin >> str;

		if (str == "quit") {
			while (1) {
				bool flag = true;
				for (int i = 0; i < HttpServer::MAX; i++) {
					if (msg.isActive[i]) {
						flag = false;
						break;
					}
				}
				if (flag) {
					closesocket(msg.serverSocket);
					exit(0);
				}
			}
		}
		else {
			printf("syntex error!\n");
		}
	}
}

HttpServer::HttpServer()
{
#pragma comment(lib, "Ws2_32.lib")
	//winsocket初始化
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2),&wsaData)!=0)
	{
		assert(false);
	}
}

bool HttpServer::start()
{
	int on = 1;
	memset(isActive, false, sizeof(isActive));
	closeMessage msg(isActive, serverSocket);
	th = new thread(listenForClose, msg);
	//初始化服务器
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(serverPort);
	//创建套接字
	serverSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	assert(serverSocket>=0);
	setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR,(char*)&on,sizeof(on));
	//绑定
	int ret = ::bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (ret<0)
	{
		printf("bind failed");
	}
	//监听
	if (listen(serverSocket,1)==-1)
	{
		
		printf("listen failed:%d", WSAGetLastError());
	}
	//等待连接
	int len = sizeof(serverAddr);
	while (true)
	{
		//阻塞，10个线程处理连接
		printf("wait connect\n");
		clientSocket = accept(serverSocket, (sockaddr*)&serverAddr, &len);
		//printf("has connected");
		if (clientSocket<0)
		{
			printf("connect failed");
			//return;
		}
		else
		{
			memset(buffer, 0, sizeof(buffer));
			int ret = recv(clientSocket, buffer, bufferSize, 0); //等待？
			if (ret == SOCKET_ERROR)
				printf("ret == SOCKET_ERROR");
			if (ret == 0)
				printf("ret==0,socket close");
			for (int i = 0; i < MAX; i++) {
				if (!isActive[i]) {
					isActive[i] = true;
					message msg(buffer, &isActive[i], clientSocket, i);
					t[i] = new std::thread(handleMessage, msg);
					break;
				}
			}
		}
		
	}

}