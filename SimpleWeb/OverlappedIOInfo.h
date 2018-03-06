/*
	重叠io信息类，抽象封装了每个io事件的消息信息
	信息包含：事件信息，事件发生的本地地址及对端地址
*/

#pragma once
#define MAXBUF 8*1024
//#include <WinSock2.h>

//I/O的操作类型
enum IOOperType
{
	TYPE_ACP,		//accept事件
	TYPE_RECV,		//数据接收事件
	TYPE_SEND,		//数据发送事件
	TYPE_CLOSE,		//关闭连接事件
	TYPE_NO_OPER	//未定义事件
};


//定义了一个WSASock的发送接收缓冲区及对端地址
class COverlappedIOInfo :public OVERLAPPED
{
public:
	COverlappedIOInfo(){
		m_sSock = INVALID_SOCKET;
		resetOverlapped();
		resetRecvBuffer();
		resetSendBuffer();
	}
	~COverlappedIOInfo(){
		if (m_sSock != INVALID_SOCKET)
		{
			closesocket(m_sSock);
			m_sSock = INVALID_SOCKET;
		}
	}
	void resetOverlapped()
	{
		Internal = InternalHigh = 0;
		Offset = OffsetHigh = 0;
		hEvent = NULL;
	}
	void resetRecvBuffer()
	{
		ZeroMemory(m_cRecvBuf, MAXBUF);
		m_recvBuf.buf = m_cRecvBuf;
		m_recvBuf.len = MAXBUF;
	}
	void resetSendBuffer()
	{
		ZeroMemory(m_cSendBuf, MAXBUF);
		m_sendBuf.buf = m_cSendBuf;
		m_sendBuf.len = MAXBUF;
	}


	SOCKET m_sSock;					//套接字
	WSABUF m_recvBuf;				//接收缓冲区，用于AcceptEx、WSARecv操作,专用于WSASock的缓冲区
	char m_cRecvBuf[MAXBUF];
	WSABUF m_sendBuf;				//发送缓冲区，用于WSASend操作
	char  m_cSendBuf[MAXBUF];
	sockaddr_in    m_addr;			//对端地址
private:

};

