/*
	IOCP��ɶ˿��ࣨҲ���Գ�Ϊ�����ࣩ�������װ��IOCP����ز���
	���ܣ�IOCP�Ĵ�������������Ϣ������ȡ���ַ������������ر�
*/
#pragma once
#include<windows.h>
#include<assert.h>
/*
	IOCP��ز�����װ
*/

//һ��IOCPҲ���Գ�֮Ϊ����
class CIOCP
{
public:
	CIOCP()
	{
		m_hIOCP = NULL;

	}
	~CIOCP()
	{
		if (m_hIOCP!=NULL)
		{
			CloseHandle(m_hIOCP);
		}
	}

	//�ر�IOCP
	bool close()
	{
		bool bRet = CloseHandle(m_hIOCP);
		m_hIOCP = NULL;
		return	bRet;
	}

	//����һ��IOCP�����ƶ������̲߳�����,һ��Ϊcpu*2
	bool create(int nMaxConcurrency=0)
	{
		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, nMaxConcurrency);
		assert(m_hIOCP != NULL);
		return (m_hIOCP != NULL);
	}

	//Ϊ�豸(�ļ���socket���ʼ����ܵ���)����һ��IOCP
	bool associateDevice(HANDLE hDevice, ULONG_PTR ComKey)
	{
		return((CreateIoCompletionPort(hDevice,m_hIOCP,ComKey,0)==m_hIOCP));
	}

	//Ϊsocket����һ��IOCP
	bool associateSocket(SOCKET hSocket, ULONG_PTR CompKey)
	{
		return(associateDevice((HANDLE)hSocket, CompKey));
	}

	//IOCP�����¼�֪ͨ
	bool postStatus(ULONG_PTR CompKey,DWORD dwNumBytes=0,OVERLAPPED* po =NULL)
	{
		bool ret = PostQueuedCompletionStatus(m_hIOCP, dwNumBytes, CompKey, po);
		return ret;
	}

	//��IOCP��ȡ��Ϣ
	bool getStatus(ULONG_PTR* pCompKey, PDWORD pdwNumBytes, OVERLAPPED** ppo, DWORD dwMilliseconds = INFINITE)
	{
		return(GetQueuedCompletionStatus(m_hIOCP, pdwNumBytes, pCompKey, ppo, dwMilliseconds));
	}

	//��ȡIOCP����
	const HANDLE  GetIOCP()
	{
		return m_hIOCP;
	}
private:
	HANDLE m_hIOCP;
};

