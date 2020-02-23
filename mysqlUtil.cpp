//#define WIN32_LEAN_AND_MEAN 
#include <Winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <iostream>

#include "mysqlUtil.h"

using namespace std;


mysqlUtil::mysqlUtil()
{
	initConnection();
}

//  该模块出现了一点点的内存泄漏
//  计划采用数据库连接池进行操作

bool mysqlUtil::initConnection()
{
	try {
		//初始化连接处理程序  
		mysql_init(&m_sqlCon);
		char mysqlFlag = false;
		if (mysql_real_connect(&m_sqlCon, host, user, pswd, table, port, NULL, 0))
		{
			mysqlFlag = true;
		}
		else
		{
			cout << "Failed to connect to database:" << mysql_error(&m_sqlCon) << endl;
		}

		return mysqlFlag;
	}
	catch (...) {
		std::cout << "error!!!" << std::endl;
		return false;
	}

}

// 断开连接
mysqlUtil::~mysqlUtil()
{
	mysql_free_result(result);
	mysql_close(&m_sqlCon);
	mysql_library_end();
}


//---- 查询跌倒时间操作
MYSQL_RES * mysqlUtil::QueryFallTime()
{
	static MYSQL_RES *result;
	if (mysql_query(&m_sqlCon, "select max(fallTime) from falldetect"))
	{
		std::cout << "查询失败" << std::endl;
		return NULL;
	}
	result = mysql_store_result(&m_sqlCon);

	return result;
}

//---- 插入操作
void mysqlUtil::skeletonInsert(float *usingSkeleton)
{
	// 内存不足就会引发堆栈溢出
	static char query[500];

	sprintf(query, "insert into skeleton(bodyHeight,leftHipAngle,rightHipAngle,leftKneeAngle,rightKneeAngle,chestAngle,chestKneeAngle,footRightPos,footLeftPos,indexList,timeIndex) values(%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, null)",
		usingSkeleton[0], usingSkeleton[1], usingSkeleton[2], usingSkeleton[3], usingSkeleton[4], usingSkeleton[5], usingSkeleton[6], usingSkeleton[7], usingSkeleton[8], usingSkeleton[9]);

	int ret = mysql_query(&m_sqlCon, query);

	if (ret != 0)
	{
		std::cout << "插入数据失败" << std::endl;
	}
	else
		std::cout << "插入数据成功,共插入：" << mysql_affected_rows(&m_sqlCon) << "行" << std::endl;
}



