#include "stdafx.h"
#include "MyIOCP.h"
#include<assert.h>
#include<iostream>
#include"log.h"
using namespace std;

int MyIOCP::m_iConnNum = 0;

std::vector<SOCKET> MyIOCP::m_vecAccept;				
std::vector<COverlappedIOInfo*>MyIOCP::m_vecContInfo;

MyIOCP::MyIOCP()
{
	
	init();		//初始化服务器
	OutputDebugStringA("服务器初始化成功\n");
	if (m_iocp.create() == NULL)
	{
		assert(false);
	}
	if (!m_iocp.associateSocket(m_serverSocket, TYPE_ACP))
	{
		assert(false);
	}
	OutputDebugStringA("关联完成端口成功\n");
	
	//监听Socket和完成端口绑定
	if (!getLPFNAcceptEXAndGetAcceptSockAddrs())
	{
		assert(false);
	}
	OutputDebugStringA("监听端口成功\n");
	startThreadPull();
	OutputDebugStringA("工作线程启动成功\n");
	for (int i = 0; i < ACCEPT_SOCKET_NUM; i++)
	{
		COverlappedIOInfo* info = new COverlappedIOInfo();
		if (!postAccept(info))
		{
			delete info;
			info = nullptr;
			return;
		}
	}
	OutputDebugStringA("消息投递成功\n");

}


MyIOCP::~MyIOCP()
{
}

void MyIOCP::init()
{

	WSADATA wsData;
	if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
	{
		assert(false);
	}
	m_serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	assert(m_serverSocket!=SOCKET_ERROR);
	memset(&m_serverAddr, 0, sizeof(m_serverAddr));
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(PORT);
	m_serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	int ret = ::bind(m_serverSocket, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
	if (ret<0)
	{
		int err = WSAGetLastError();
		assert(false);
	}
	if (listen(m_serverSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		cout << WSAGetLastError() << endl;
		assert(false);
	}
	printf("init success\n");
}

void MyIOCP::listenMode()
{

}

void MyIOCP::work()
{

}

UINT MyIOCP::startThreadPull()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_nProcessors = si.dwNumberOfProcessors;	//系统cpu数
	printf("cpu num:%d\n", si.dwNumberOfProcessors);
	//m_workThread = new std::thread[m_nProcessors * 2]; //创建的线程
	for (int i = 0; i < m_nProcessors*2; i++)
	{
		m_workThread[i] = new std::thread(&MyIOCP::svc,this,i);
		m_workThread[i]->detach();
	}
	return 0;
}

bool MyIOCP::getLPFNAcceptEXAndGetAcceptSockAddrs()
{
	DWORD BytesReturned = 0;
	//获取acceptEx函数指针
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	if (SOCKET_ERROR == WSAIoctl(
		m_serverSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(m_lpfnAcceptEx),
		&BytesReturned,
		NULL,
		NULL
		))
	{
		assert(false);
		return false;
	}

	//获取GetAcceptexSockAddrs函数指针
	GUID GuidGetAcceptexSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	if (SOCKET_ERROR == WSAIoctl(
		m_serverSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptexSockAddrs,
		sizeof(GuidGetAcceptexSockAddrs),
		&m_lpfnGetAcceptSockAddrs,
		sizeof(m_lpfnGetAcceptSockAddrs),
		&BytesReturned,
		NULL, NULL))
	{
		assert(false);
		return false;
	}
	return true;
}

bool MyIOCP::postAccept(COverlappedIOInfo* ol)
{
	OutputDebugStringA("postAccept\n");
	if (m_lpfnAcceptEx == NULL)
	{
		return false;
	}
	SOCKET s = ol->m_sSock;
	ol->resetRecvBuffer();
	ol->resetSendBuffer();
	ol->resetOverlapped();
	ol->m_sSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);//预先建立链接，而不是等有请求后在建立，AcceptEx高性能的关键
	if (ol->m_sSock == INVALID_SOCKET)
		assert(false);
	//这里建立的socket用来和对端建立连接，终会加入m_vecContInfo列表
	//调用acceptex将accept socket绑定至完成端口，并开始进行事件监听
	//这里需要传递Overlapped，new一个COverlappedIOInfo
	//AcceptEx是m_listen的监听事件，m_listen已经绑定了完成端口；虽然ol->m_sSock已经创建，
	//但未使用，现在不必为ol->m_sSock绑定完成端口。在AcceptEx事件发生后，再为ol->m_sSock绑定IOCP
	DWORD byteReceived = 0;
	if (FALSE == m_lpfnAcceptEx(
		m_serverSocket,
		ol->m_sSock,
		ol->m_recvBuf.buf,//缓冲区包含了三个信息：一是客户端发来的第一组数据，二是server的地址，三是client地址
		ol->m_recvBuf.len - (sizeof(SOCKADDR_IN)+16) * 2,//如果此参数=0，则Accept时将不会待数据到来，而直接返回，如果此参数不为0，那么一定得等接收到数据了才会返回
		sizeof(SOCKADDR_IN)+16,  //存放本地址地址信息的空间大小
		sizeof(SOCKADDR_IN)+16,		//存放本远端地址信息的空间大小
		&byteReceived,
		ol
		))
	{
		//int err = WSAGetLastError();//997
		//assert(false);
		DWORD res = WSAGetLastError();
		if (ERROR_IO_PENDING != res)   //排除997，此时是非阻塞的模式
		{
			cout << "AcceptEx error , error code " << res << endl;
			return false;
		}

	  }
	//查找等待队列是否有此socket 
	m_glock.lock();
	auto iter = m_vecAccept.begin();
	for (; iter != m_vecAccept.end(); iter++)
	{
		if (*iter == s)
		{
			*iter = ol->m_sSock;
		}
	}
	if (iter == m_vecAccept.end())
	{
		m_vecAccept.push_back(ol->m_sSock);
	}
	m_glock.unlock();
	return true;
}

bool MyIOCP::doAccept(COverlappedIOInfo* ol, DWORD NumberOfBytes)
{
	//分支用于获取远端地址。
	//如果接收TYPE_ACP同时收到第一帧数据，则第一帧数据内包含远端地址
	//如果没有收到第一帧数据，则通过getpeername获取远端地址
	OutputDebugStringA("Accepted\n");
	SOCKADDR_IN* ClientAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN);
	if (NumberOfBytes > 0)
	{
		OutputDebugStringA("NumberOfBytes > 0\n");
		//接受的数据分成3部分，第1部分是客户端发来的数据，第2部分是本地地址，第3部分是远端地址
		if (m_lpfnGetAcceptSockAddrs)
		{
			//OutputDebugPrintf("m_lpfnGetAcceptSockAddrs指针存在");
			SOCKADDR_IN* LocalAddr = NULL;
			int localLen = sizeof(SOCKADDR_IN);
			m_lpfnGetAcceptSockAddrs(
				ol->m_recvBuf.buf,
				ol->m_recvBuf.len - (sizeof(SOCKADDR_IN)+16) * 2,
				sizeof(SOCKADDR_IN)+16,
				sizeof(SOCKADDR_IN)+16,
				(LPSOCKADDR*)&LocalAddr,
				&localLen,
				(LPSOCKADDR*)&ClientAddr,
				&remoteLen
				);
			//OutputDebugPrintf("m_lpfnGetAcceptSockAddrs函数完成");
			
		}
		//这里接受第一段数据，数据就是ol->m_recvBuf.buf，之后post就可以了
		printf("%s\n",ol->m_recvBuf.buf);
	}
	else if (NumberOfBytes == 0)
	{
		//未收到第一帧数据
		if (SOCKET_ERROR == getpeername(ol->m_sSock, (sockaddr*)ClientAddr, &remoteLen))
		{
			//cout << "getpeername error,error code " << WSAGetLastError() << endl;
		}
		else
		{
			//cout << "收到新的连接请求,ip=" << inet_ntoa(ClientAddr->sin_addr) << ",port=" << ClientAddr->sin_port << endl;
		}
	}
	COverlappedIOInfo* pol = new COverlappedIOInfo;
	pol->m_sSock = ol->m_sSock;
	pol->m_addr = *ClientAddr;
	//服务端只收取recv，同时监听recv和send可用设计位偏移，用或运算实现
	if (m_iocp.associateSocket(pol->m_sSock, TYPE_RECV))
	{
		//doRecv(pol);
		postRecv(pol);
		m_vecContInfo.push_back(pol);
	}
	else
	{
		delete pol;
		pol = nullptr;
		return false;
	}
	return true;
}

bool MyIOCP::postRecv(COverlappedIOInfo* ol)
{
	OutputDebugStringA("postRecv\n");
	DWORD BytesRecvd = 0;
	DWORD dwFlags = 0;
	ol->resetOverlapped();
	ol->resetRecvBuffer();
	int recvnum = WSARecv(ol->m_sSock, &ol->m_recvBuf, 1, &BytesRecvd, &dwFlags, (OVERLAPPED*)ol, NULL);
	//OutputDebugStringA("postRecvEnd\n");
	//cout << "ssssssss" << endl;
	//printf_s("%s", ol->m_recvBuf.buf, BytesRecvd+1);
	if (recvnum != 0)
	{
		int res = WSAGetLastError();
		if (WSA_IO_PENDING != res)
		{
			cout << "WSARecv error,error code " << res << endl;
		}
	}

	return true;
}

bool MyIOCP::doRecv(COverlappedIOInfo* ol)
{
	OutputDebugStringA("doRecv\n");
	cout << "收到客户端数据：ip=" << inet_ntoa(ol->m_addr.sin_addr) << ",port=" << ol->m_addr.sin_port <<
		"；内容=" << ol->m_recvBuf.buf << endl;

	return true;
}

bool MyIOCP::deleteLink(SOCKET s)
{
	m_glock.lock();
	auto iter = m_vecContInfo.begin();
	for (; iter != m_vecContInfo.end(); iter++)
	{
		if (s == (*iter)->m_sSock)
		{
			COverlappedIOInfo* ol = *iter;
			closesocket(s);
			m_vecContInfo.erase(iter);
			delete ol;
			ol = nullptr;
			break;
		}
	}
	m_glock.unlock();
	return true;
}

void MyIOCP::closeServer()
{
	//1：清空IOCP线程队列，退出线程,有多少个线程发送多少个PostQueuedCompletionStatus信息
	int threadnum = m_nProcessors*2;
	for (int i = 0; i < threadnum; i++)
	{
		if (FALSE == m_iocp.postStatus(TYPE_CLOSE))
		{
			cout << "PostQueuedCompletionStatus error,error code " << WSAGetLastError() << endl;
		}
	}
	//2:清空等待accept的套接字m_vecAcps
	std::vector<SOCKET>::iterator iter = m_vecAccept.begin();
	for (; iter != m_vecAccept.end(); iter++)
	{
		SOCKET s = *iter;
		closesocket(s);
	}
	m_vecAccept.clear();
	//3:清空已连接的套接字m_vecContInfo并清空缓存
	std::vector<COverlappedIOInfo*>::iterator iter2 = m_vecContInfo.begin();
	for (; iter2 != m_vecContInfo.end(); )
	{
		COverlappedIOInfo* ol = *iter2;
		closesocket(ol->m_sSock);
		iter2=m_vecContInfo.erase(iter2);
		delete ol;
		ol = nullptr;
	}
	m_vecContInfo.clear();
}

void MyIOCP::svc(int i)
{
	//工作线程，用来排队监视完成端口是否有完成的网络操作
	
	while (true)
	{
		DWORD  NumberOfBytes = 0;
		unsigned long CompletionKey = 0;
		OVERLAPPED*    ol = NULL;
		printf("线程%d等待中。。。\n",i);
		if (FALSE != GetQueuedCompletionStatus(
			m_iocp.GetIOCP(),//建立的那个唯一的完成端口   
			&NumberOfBytes,//操作完成后返回的字节数
			&CompletionKey,//我们建立完成端口的时候绑定的那个自定义结构体参数
			&ol,		//连入Socket的时候一起建立的那个重叠结构
			WSA_INFINITE   //等待完成端口的超时时间，如果线程不需要做其他的事情，那就INFINITE就行了  
			))//false的时候会线程睡眠，直至有请求
		{
			
			OutputDebugStringA("deal net msgs\n");
			if (CompletionKey == TYPE_CLOSE)
			{
				OutputDebugStringA("CompletionKey == TYPE_CLOSE");
				break;
			}
			if (NumberOfBytes == 0 && (CompletionKey == TYPE_RECV || CompletionKey == TYPE_SEND))
			{
				//客户端断开连接
				OutputDebugStringA("shutdownLink\n");
				COverlappedIOInfo* olinfo = (COverlappedIOInfo*)ol;
				cout << "客户端断开连接,ip=" << inet_ntoa(olinfo->m_addr.sin_addr) << ",port=" << olinfo->m_addr.sin_port << endl;
				deleteLink(olinfo->m_sSock);
				continue;
			}
			COverlappedIOInfo* olinfo = (COverlappedIOInfo*)ol;
			switch (CompletionKey)
			{
			case TYPE_ACP:
			{
							 m_iConnNum ++;
							 OutputDebugPrintf("%d\n", m_iConnNum);
							 OutputDebugStringA("CompletionKey == TYPE_ACP\n");
				doAccept(olinfo, NumberOfBytes);
				postAccept(olinfo);
			}
				break;
			case TYPE_RECV:
			{
							  OutputDebugStringA("CompletionKey == TYPE_RECV\n");
							  
				doRecv(olinfo);
				postRecv(olinfo);
			}
				break;
			case TYPE_SEND:
			{

			}
				break;
			default:
				break;
			}
		}
		else
		{
			int res = WSAGetLastError();
			switch (res)
			{
			case ERROR_NETNAME_DELETED:
			{
				COverlappedIOInfo* olinfo = (COverlappedIOInfo*)ol;
				if (olinfo)
				{
					OutputDebugStringA("client exit unexpectedly\n");
					cout << "客户端异常退出,ip=" << inet_ntoa(olinfo->m_addr.sin_addr) << ",port=" << olinfo->m_addr.sin_port << endl;
					deleteLink(olinfo->m_sSock);
				}
			}
				break;
			default:
				cout << "workthread GetQueuedCompletionStatus error,error code " << WSAGetLastError() << endl;
				break;
			}
			continue;
		}
	}
	cout << "workthread stop" << endl;
}