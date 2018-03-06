/*
	�����̻߳��ࣨδʵ�֣�
	���ܣ��ṩ�̵߳Ĵ����������������رյ�
*/

#pragma once
#include<Windows.h>
#include<vector>
#include<thread>
typedef unsigned int UNIT;
//�ṩ�߳���ؿ��ƣ��Զ�����Ϣ����Ϣ��
class CTaskSvc
{
public:

	UNIT activate(int num = 1);//Activate���ڼ���һ�������Ĺ������̣߳�Ĭ�ϼ�������Ϊ1�����ص�ǰ�̶߳��д�С
	UNIT getThreadsNum();//��ȡ�̶߳��еĴ�С
protected:
	CTaskSvc();	//ֻ��������ܽ��й���
		~CTaskSvc();

	virtual void svc()=0;	//����Ӧ�����¶��幤���̵߳�ϸ��

	void close();	//�ȴ��̲߳��رգ��˳��߳����������

private:

	static UNIT workThread(LPVOID param); //�������̷߳��ʽӿ�

	std::vector<std::thread*> m_vecThreads;
};

