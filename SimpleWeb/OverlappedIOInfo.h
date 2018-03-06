/*
	�ص�io��Ϣ�࣬�����װ��ÿ��io�¼�����Ϣ��Ϣ
	��Ϣ�������¼���Ϣ���¼������ı��ص�ַ���Զ˵�ַ
*/

#pragma once
#define MAXBUF 8*1024
//#include <WinSock2.h>

//I/O�Ĳ�������
enum IOOperType
{
	TYPE_ACP,		//accept�¼�
	TYPE_RECV,		//���ݽ����¼�
	TYPE_SEND,		//���ݷ����¼�
	TYPE_CLOSE,		//�ر������¼�
	TYPE_NO_OPER	//δ�����¼�
};


//������һ��WSASock�ķ��ͽ��ջ��������Զ˵�ַ
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


	SOCKET m_sSock;					//�׽���
	WSABUF m_recvBuf;				//���ջ�����������AcceptEx��WSARecv����,ר����WSASock�Ļ�����
	char m_cRecvBuf[MAXBUF];
	WSABUF m_sendBuf;				//���ͻ�����������WSASend����
	char  m_cSendBuf[MAXBUF];
	sockaddr_in    m_addr;			//�Զ˵�ַ
private:

};

