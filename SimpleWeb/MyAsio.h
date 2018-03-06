/*
	boost::asio实现的网络接入部分服务器
	echo server
*/
#pragma once
#include<iostream>
#include<memory>
#include<array>
#include<boost\asio.hpp>

using namespace std;

class session :public std::enable_shared_from_this<session>
{
	////enable_shared_from_this，是一个以其派生类为模板类型实参的基础模板，继承它，this指针就能变成shared_ptr。
public:


private:

};

