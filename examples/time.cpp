
#include <time.h>
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	//已知当前时间2021-3-12 11:52:38
	//转换为秒
	struct tm time;
	// time.tm_year = 2024 - 1900;//tm中的年份比实际年份小1900，需要减掉
	// time.tm_mon = 8 - 1;//tm中的月份从0开始，需要减1
	// time.tm_mday = 11;
	// time.tm_hour = 11;
	// time.tm_min = 46;
	// time.tm_sec = 42; // 631036800 + 1092311202

	time.tm_year = 1989 - 1900;//tm中的年份比实际年份小1900，需要减掉
	time.tm_mon = 12 - 1;//tm中的月份从0开始，需要减1
	time.tm_mday = 31;
	time.tm_hour = 0;
	time.tm_min = 0;
	time.tm_sec = 0;
	time_t ltime_new = mktime(&time);

	cout << "------------将2021-3-12 11:52:38转换为秒------------" << endl;
	cout << ltime_new << endl;
	cout << "------------将2021-3-12 11:52:38转换为秒------------" << endl;
	cout << "\r\n" << endl;

	//将秒数转换为年月日时分秒
	struct tm newTime;
	newTime = *localtime( &ltime_new );  /* Convert to local time. */
	
	cout << "------------------将秒数转换为时间------------------" << endl;
	//星期，一周中的第几天， 从星期日算起，0-6
	char week[10];
	switch(newTime.tm_wday)
	{
	case 0:
		sprintf(week, "%s", "星期日");
		break;
	case 1:
		sprintf(week, "%s", "星期一");
		break;
	case 2:
		sprintf(week, "%s", "星期二");
		break;
	case 3:
		sprintf(week, "%s", "星期三");
		break;
	case 4:
		sprintf(week, "%s", "星期四");
		break;
	case 5:
		sprintf(week, "%s", "星期五");
		break;
	case 6:
		sprintf(week, "%s", "星期六");
		break;
	default:
		sprintf(week, "%s", "有星期八吗？");
		break;
	}

	int nYDay = newTime.tm_yday + 1;//tm从今年1月1日到目前的天数，范围0-365,需加1
	int nYear = newTime.tm_year + 1900;//tm中的年份比实际年份小1900，需加1900
	int nMon = newTime.tm_mon + 1;//月， 从一月算起，范围0-11,需加1
	int nMDay = newTime.tm_mday;//日
	int nHour = newTime.tm_hour;//时
	int nMin = newTime.tm_min;//分
	int nSec = newTime.tm_sec;//秒
	
	cout << "当前时间为:一年中的第" << nYDay << "天" << " " << week << " "
		<< nYear << "-" << newTime.tm_mon+1 << "-" << newTime.tm_mday 
		<< " " << newTime.tm_hour << ":" << newTime.tm_min << ":" << newTime.tm_sec 
		<< endl;
	cout << "------------------将秒数转换为时间------------------" << endl;

	return 0;
}

