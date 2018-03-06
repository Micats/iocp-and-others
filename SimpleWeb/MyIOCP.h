/*
	�������࣬ʵ���˷�������ز����ĳ������װ
	���ܣ��������Ĵ������������������رյ�
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
	void listenMode();//������accept
	void work(); //�̵߳Ĺ�������
	void svc(int i);
private:
	enum{
		PORT =8888,
		ACCEPT_SOCKET_NUM=100
	};
	const char* IPADDR = "127.0.0.1";
	int m_nProcessors;
	static int m_iConnNum;
	HANDLE m_hIOCompletionPort;	//��ɶ˿ڶ���
	std::thread* m_workThread[30];	//�߳̾��
	SOCKET m_serverSocket;		//����������socket
	sockaddr_in m_serverAddr;  //��������ַ

	//�������������Ļ�������߳̽��и��º�д����ʱ�����������߳�ӵ�е�iteratorʧЧ
	static std::vector<SOCKET> m_vecAccept;				//�ȴ����ӵĶ��У�δʹ�ù���socket,
	static std::vector<COverlappedIOInfo*>m_vecContInfo;	//�ѽ������ӵ���Ϣ��ÿ���ṹ����һ���׽��֡����ͻ���ͽ��ջ��壬�Լ��Զ˵�ַ
	std::mutex m_glock;		//����???�Ļ�����
	CIOCP m_iocp;
	LPFN_ACCEPTEX m_lpfnAcceptEx;   //AcceptEx����ָ��
	LPFN_GETACCEPTEXSOCKADDRS    m_lpfnGetAcceptSockAddrs;  //GetAcceptSockAddrs����ָ��

	//����CPU*2���̣߳������������̸߳���
	UINT startThreadPull();
	//��ȡAcceptEx��GetAcceptExSockaddrs����ָ��
	bool    getLPFNAcceptEXAndGetAcceptSockAddrs();
	//����AcceptEx����accept����
	bool    postAccept(COverlappedIOInfo* ol);
	//����accept����,NumberOfBytes=0��ʾû���յ���һ֡���ݣ�>0��ʾ�յ���һ֡����
	bool    doAccept(COverlappedIOInfo* ol, DWORD NumberOfBytes = 0);
	//Ͷ��recv����
	bool    postRecv(COverlappedIOInfo* ol);
	//����recv����
	bool    doRecv(COverlappedIOInfo* ol);
	//��������socket�б����Ƴ�socket���ͷſռ�
	bool    deleteLink(SOCKET s);
	//�ͷ�3�����ֲ��裺
	//1�����IOCP�̶߳��У��˳��߳�
	//2: ��յȴ�accept���׽���m_vecAcps
	//3: ��������ӵ��׽���m_vecContInfo����ջ���
	void    closeServer();
};

