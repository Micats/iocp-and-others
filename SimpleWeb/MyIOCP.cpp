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
	
	init();		//��ʼ��������
	OutputDebugStringA("��������ʼ���ɹ�\n");
	if (m_iocp.create() == NULL)
	{
		assert(false);
	}
	if (!m_iocp.associateSocket(m_serverSocket, TYPE_ACP))
	{
		assert(false);
	}
	OutputDebugStringA("������ɶ˿ڳɹ�\n");
	
	//����Socket����ɶ˿ڰ�
	if (!getLPFNAcceptEXAndGetAcceptSockAddrs())
	{
		assert(false);
	}
	OutputDebugStringA("�����˿ڳɹ�\n");
	startThreadPull();
	OutputDebugStringA("�����߳������ɹ�\n");
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
	OutputDebugStringA("��ϢͶ�ݳɹ�\n");

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
	m_nProcessors = si.dwNumberOfProcessors;	//ϵͳcpu��
	printf("cpu num:%d\n", si.dwNumberOfProcessors);
	//m_workThread = new std::thread[m_nProcessors * 2]; //�������߳�
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
	//��ȡacceptEx����ָ��
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

	//��ȡGetAcceptexSockAddrs����ָ��
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
	ol->m_sSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);//Ԥ�Ƚ������ӣ������ǵ���������ڽ�����AcceptEx�����ܵĹؼ�
	if (ol->m_sSock == INVALID_SOCKET)
		assert(false);
	//���ｨ����socket�����ͶԶ˽������ӣ��ջ����m_vecContInfo�б�
	//����acceptex��accept socket������ɶ˿ڣ�����ʼ�����¼�����
	//������Ҫ����Overlapped��newһ��COverlappedIOInfo
	//AcceptEx��m_listen�ļ����¼���m_listen�Ѿ�������ɶ˿ڣ���Ȼol->m_sSock�Ѿ�������
	//��δʹ�ã����ڲ���Ϊol->m_sSock����ɶ˿ڡ���AcceptEx�¼���������Ϊol->m_sSock��IOCP
	DWORD byteReceived = 0;
	if (FALSE == m_lpfnAcceptEx(
		m_serverSocket,
		ol->m_sSock,
		ol->m_recvBuf.buf,//������������������Ϣ��һ�ǿͻ��˷����ĵ�һ�����ݣ�����server�ĵ�ַ������client��ַ
		ol->m_recvBuf.len - (sizeof(SOCKADDR_IN)+16) * 2,//����˲���=0����Acceptʱ����������ݵ�������ֱ�ӷ��أ�����˲�����Ϊ0����ôһ���õȽ��յ������˲Ż᷵��
		sizeof(SOCKADDR_IN)+16,  //��ű���ַ��ַ��Ϣ�Ŀռ��С
		sizeof(SOCKADDR_IN)+16,		//��ű�Զ�˵�ַ��Ϣ�Ŀռ��С
		&byteReceived,
		ol
		))
	{
		//int err = WSAGetLastError();//997
		//assert(false);
		DWORD res = WSAGetLastError();
		if (ERROR_IO_PENDING != res)   //�ų�997����ʱ�Ƿ�������ģʽ
		{
			cout << "AcceptEx error , error code " << res << endl;
			return false;
		}

	  }
	//���ҵȴ������Ƿ��д�socket 
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
	//��֧���ڻ�ȡԶ�˵�ַ��
	//�������TYPE_ACPͬʱ�յ���һ֡���ݣ����һ֡�����ڰ���Զ�˵�ַ
	//���û���յ���һ֡���ݣ���ͨ��getpeername��ȡԶ�˵�ַ
	OutputDebugStringA("Accepted\n");
	SOCKADDR_IN* ClientAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN);
	if (NumberOfBytes > 0)
	{
		OutputDebugStringA("NumberOfBytes > 0\n");
		//���ܵ����ݷֳ�3���֣���1�����ǿͻ��˷��������ݣ���2�����Ǳ��ص�ַ����3������Զ�˵�ַ
		if (m_lpfnGetAcceptSockAddrs)
		{
			//OutputDebugPrintf("m_lpfnGetAcceptSockAddrsָ�����");
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
			//OutputDebugPrintf("m_lpfnGetAcceptSockAddrs�������");
			
		}
		//������ܵ�һ�����ݣ����ݾ���ol->m_recvBuf.buf��֮��post�Ϳ�����
		printf("%s\n",ol->m_recvBuf.buf);
	}
	else if (NumberOfBytes == 0)
	{
		//δ�յ���һ֡����
		if (SOCKET_ERROR == getpeername(ol->m_sSock, (sockaddr*)ClientAddr, &remoteLen))
		{
			//cout << "getpeername error,error code " << WSAGetLastError() << endl;
		}
		else
		{
			//cout << "�յ��µ���������,ip=" << inet_ntoa(ClientAddr->sin_addr) << ",port=" << ClientAddr->sin_port << endl;
		}
	}
	COverlappedIOInfo* pol = new COverlappedIOInfo;
	pol->m_sSock = ol->m_sSock;
	pol->m_addr = *ClientAddr;
	//�����ֻ��ȡrecv��ͬʱ����recv��send�������λƫ�ƣ��û�����ʵ��
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
	cout << "�յ��ͻ������ݣ�ip=" << inet_ntoa(ol->m_addr.sin_addr) << ",port=" << ol->m_addr.sin_port <<
		"������=" << ol->m_recvBuf.buf << endl;

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
	//1�����IOCP�̶߳��У��˳��߳�,�ж��ٸ��̷߳��Ͷ��ٸ�PostQueuedCompletionStatus��Ϣ
	int threadnum = m_nProcessors*2;
	for (int i = 0; i < threadnum; i++)
	{
		if (FALSE == m_iocp.postStatus(TYPE_CLOSE))
		{
			cout << "PostQueuedCompletionStatus error,error code " << WSAGetLastError() << endl;
		}
	}
	//2:��յȴ�accept���׽���m_vecAcps
	std::vector<SOCKET>::iterator iter = m_vecAccept.begin();
	for (; iter != m_vecAccept.end(); iter++)
	{
		SOCKET s = *iter;
		closesocket(s);
	}
	m_vecAccept.clear();
	//3:��������ӵ��׽���m_vecContInfo����ջ���
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
	//�����̣߳������ŶӼ�����ɶ˿��Ƿ�����ɵ��������
	
	while (true)
	{
		DWORD  NumberOfBytes = 0;
		unsigned long CompletionKey = 0;
		OVERLAPPED*    ol = NULL;
		printf("�߳�%d�ȴ��С�����\n",i);
		if (FALSE != GetQueuedCompletionStatus(
			m_iocp.GetIOCP(),//�������Ǹ�Ψһ����ɶ˿�   
			&NumberOfBytes,//������ɺ󷵻ص��ֽ���
			&CompletionKey,//���ǽ�����ɶ˿ڵ�ʱ��󶨵��Ǹ��Զ���ṹ�����
			&ol,		//����Socket��ʱ��һ�������Ǹ��ص��ṹ
			WSA_INFINITE   //�ȴ���ɶ˿ڵĳ�ʱʱ�䣬����̲߳���Ҫ�����������飬�Ǿ�INFINITE������  
			))//false��ʱ����߳�˯�ߣ�ֱ��������
		{
			
			OutputDebugStringA("deal net msgs\n");
			if (CompletionKey == TYPE_CLOSE)
			{
				OutputDebugStringA("CompletionKey == TYPE_CLOSE");
				break;
			}
			if (NumberOfBytes == 0 && (CompletionKey == TYPE_RECV || CompletionKey == TYPE_SEND))
			{
				//�ͻ��˶Ͽ�����
				OutputDebugStringA("shutdownLink\n");
				COverlappedIOInfo* olinfo = (COverlappedIOInfo*)ol;
				cout << "�ͻ��˶Ͽ�����,ip=" << inet_ntoa(olinfo->m_addr.sin_addr) << ",port=" << olinfo->m_addr.sin_port << endl;
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
					cout << "�ͻ����쳣�˳�,ip=" << inet_ntoa(olinfo->m_addr.sin_addr) << ",port=" << olinfo->m_addr.sin_port << endl;
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