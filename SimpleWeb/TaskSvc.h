/*
	工作线程基类（未实现）
	功能：提供线程的创建，管理，工作，关闭等
*/

#pragma once
#include<Windows.h>
#include<vector>
#include<thread>
typedef unsigned int UNIT;
//提供线程相关控制，自定义消息和消息块
class CTaskSvc
{
public:

	UNIT activate(int num = 1);//Activate用于激活一定数量的工作者线程，默认激活数量为1。返回当前线程队列大小
	UNIT getThreadsNum();//获取线程队列的大小
protected:
	CTaskSvc();	//只有子类才能进行构造
		~CTaskSvc();

	virtual void svc()=0;	//子类应该重新定义工作线程的细节

	void close();	//等待线程并关闭，退出线程由子类控制

private:

	static UNIT workThread(LPVOID param); //工作者线程访问接口

	std::vector<std::thread*> m_vecThreads;
};

