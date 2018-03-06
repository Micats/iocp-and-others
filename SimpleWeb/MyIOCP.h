/*
	服务器类，实现了服务器相关操作的抽象与封装
	功能：服务器的创建，监听，工作，关闭等
*/


#pragma once
#include<Windows.h>
#include<winsock2.h>
#include<Mswsock.h>

#include<thread>
#include <mutex>
#include<vector>
#include"OverlappedIOInfo.h"
#include"CIOCP.h"
#pragma comment(lib,"ws2_32.lib")
//#pragma comment(lib,"Mswsock.lib")

class MyIOCP
{
public:
	MyIOCP();
	~MyIOCP();
protected:
	void init();
	void listenMode();//阻塞的accept
	void work(); //线程的工作函数
	void svc(int i);
private:
	enum{
		PORT =8888,
		ACCEPT_SOCKET_NUM=100
	};
	const char* IPADDR = "127.0.0.1";
	int m_nProcessors;
	static int m_iConnNum;
	HANDLE m_hIOCompletionPort;	//完成端口队列
	std::thread* m_workThread[30];	//线程句柄
	SOCKET m_serverSocket;		//服务器监听socket
	sockaddr_in m_serverAddr;  //服务器地址

	//这容器不加锁的话，多个线程进行更新和写操作时，会让其他线程拥有的iterator失效
	static std::vector<SOCKET> m_vecAccept;				//等待链接的队列，未使用过的socket,
	static std::vector<COverlappedIOInfo*>m_vecContInfo;	//已建立连接的信息，每个结构含有一个套接字、发送缓冲和接收缓冲，以及对端地址
	std::mutex m_glock;		//操作???的互斥锁
	CIOCP m_iocp;
	LPFN_ACCEPTEX m_lpfnAcceptEx;   //AcceptEx函数指针
	LPFN_GETACCEPTEXSOCKADDRS    m_lpfnGetAcceptSockAddrs;  //GetAcceptSockAddrs函数指针

	//启动CPU*2个线程，返回已启动线程个数
	UINT startThreadPull();
	//获取AcceptEx和GetAcceptExSockaddrs函数指针
	bool    getLPFNAcceptEXAndGetAcceptSockAddrs();
	//利用AcceptEx监听accept请求
	bool    postAccept(COverlappedIOInfo* ol);
	//处理accept请求,NumberOfBytes=0表示没有收到第一帧数据，>0表示收到第一帧数据
	bool    doAccept(COverlappedIOInfo* ol, DWORD NumberOfBytes = 0);
	//投递recv请求
	bool    postRecv(COverlappedIOInfo* ol);
	//处理recv请求
	bool    doRecv(COverlappedIOInfo* ol);
	//从已连接socket列表中移除socket及释放空间
	bool    deleteLink(SOCKET s);
	//释放3个部分步骤：
	//1：清空IOCP线程队列，退出线程
	//2: 清空等待accept的套接字m_vecAcps
	//3: 清空已连接的套接字m_vecContInfo并清空缓存
	void    closeServer();
};

