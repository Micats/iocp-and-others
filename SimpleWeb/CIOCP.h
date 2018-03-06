/*
	IOCP完成端口类（也可以成为队列类），抽象封装了IOCP的相关操作
	功能：IOCP的创建、关联、消息处理（读取，分发）、监听、关闭
*/
#pragma once
#include<windows.h>
#include<assert.h>
/*
	IOCP相关操作封装
*/

//一个IOCP也可以称之为队列
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

	//关闭IOCP
	bool close()
	{
		bool bRet = CloseHandle(m_hIOCP);
		m_hIOCP = NULL;
		return	bRet;
	}

	//创建一个IOCP，并制定最大的线程并发数,一般为cpu*2
	bool create(int nMaxConcurrency=0)
	{
		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, nMaxConcurrency);
		assert(m_hIOCP != NULL);
		return (m_hIOCP != NULL);
	}

	//为设备(文件，socket，邮件，管道等)关联一个IOCP
	bool associateDevice(HANDLE hDevice, ULONG_PTR ComKey)
	{
		return((CreateIoCompletionPort(hDevice,m_hIOCP,ComKey,0)==m_hIOCP));
	}

	//为socket关联一个IOCP
	bool associateSocket(SOCKET hSocket, ULONG_PTR CompKey)
	{
		return(associateDevice((HANDLE)hSocket, CompKey));
	}

	//IOCP发送事件通知
	bool postStatus(ULONG_PTR CompKey,DWORD dwNumBytes=0,OVERLAPPED* po =NULL)
	{
		bool ret = PostQueuedCompletionStatus(m_hIOCP, dwNumBytes, CompKey, po);
		return ret;
	}

	//从IOCP获取信息
	bool getStatus(ULONG_PTR* pCompKey, PDWORD pdwNumBytes, OVERLAPPED** ppo, DWORD dwMilliseconds = INFINITE)
	{
		return(GetQueuedCompletionStatus(m_hIOCP, pdwNumBytes, pCompKey, ppo, dwMilliseconds));
	}

	//获取IOCP对象
	const HANDLE  GetIOCP()
	{
		return m_hIOCP;
	}
private:
	HANDLE m_hIOCP;
};

