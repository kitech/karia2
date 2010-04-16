// resourcemonitor.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-05 11:02:35 +0800
// Version: $Id$
// 

#include <cassert>

#ifdef WIN32
#else
#include <unistd.h>
#endif

#include <QtCore>
#include <QProcess>

#include "resourcemonitor.h"


//static 
ResourceMonitor * ResourceMonitor::mHandle = 0 ;

//static 
ResourceMonitor * ResourceMonitor::instance( )
{
	if (ResourceMonitor::mHandle == 0) {
		ResourceMonitor::mHandle = new ResourceMonitor(0);
	}
	return ResourceMonitor::mHandle ;

}
ResourceMonitor::ResourceMonitor(QObject *parent)
	: QObject(parent)
{
#ifdef WIN32
	memset(&mLastUserTime  ,0,sizeof( FILETIME ));
	memset(&mLastKernelTime  ,0,sizeof( FILETIME ));
#endif

	mLastSysTime = 0 ;
	mLastCurrentProcessTime = 0 ;
}

ResourceMonitor::~ResourceMonitor()
{

}

QString ResourceMonitor::getMemoryUsage()
{
	QString usage ;

#ifdef WIN32
	usage = this->getWin32MemoryUsage();
#else
	usage = this->getUnixMemoryUsageByTop();
#endif
	
	return usage ;
}

QString ResourceMonitor::getCPUUsage()
{
	QString usage ;

#ifdef WIN32
	usage = this->getWin32CPUUsage();
#else
	usage = this->getUnixCPUUsage();
#endif
	
	return usage ;
}


QString ResourceMonitor::seprateByColon(int num , int len )
{
	QString sep , tmp ;

	tmp = QString("%1").arg(num);
	sep = tmp ;

	return sep ;
}

#ifdef WIN32
QString ResourceMonitor::getWin32MemoryUsage()
{
	QString usage = "PM: %1 , VM: %2 TM: %3 " ;
	HANDLE pid ;
	PROCESS_MEMORY_COUNTERS  pmc ;	//PROCESS_MEMORY_COUNTERS_EX 在XP上没有定义？？
	MEMORYSTATUS    ms ;

	pid = ::GetCurrentProcess();	//当前进程的句柄

	::GlobalMemoryStatus(&ms);		//系统物理内存及虚拟内存用量	
	bool ret = ::GetProcessMemoryInfo(pid,&pmc,sizeof(pmc));	//该进程物理内存及虚拟内存用量		
	if( ret == true )
	{
		usage = QString("PFC:%1,PWSS:%2,WSS:%3,QPPPU:%4, QPPU:%5,QPNPPU:%6,QNPPU:%7,PU:%8,PPU:%9")
			.arg(pmc.PageFaultCount).arg(pmc.PagefileUsage).arg(pmc.PeakPagefileUsage)
			.arg(pmc.PeakWorkingSetSize).arg(pmc.QuotaNonPagedPoolUsage).arg(pmc.QuotaPagedPoolUsage)
			.arg(pmc.QuotaPeakNonPagedPoolUsage).arg(pmc.QuotaPeakPagedPoolUsage).arg(pmc.WorkingSetSize) ;

		//qDebug()<< usage ;
		
		int pm = pmc.WorkingSetSize / 1024 ;
		int vm = pmc.PagefileUsage / 1024 ;
		int tm = ms.dwTotalPhys / 1024 / 1024 ;

		usage = QString(tr("PM: %1 K, VM: %2 K, TM: %3 M")).arg(seprateByColon(pm))
			.arg(seprateByColon(vm)).arg(seprateByColon(tm) );

	}
	else
	{
		qDebug()<<__FUNCTION__<<__LINE__<<::GetLastError();
	}

	return usage ;
}
QString ResourceMonitor::getWin32CPUUsage()
{
	QString usage ;
	DWORD * allpids = 0 ;
	DWORD   pidarrlen = 0 ;
	DWORD   gotpids = 0 ;
	DWORD   realpidcnt = 0 ;

	HANDLE pid ;
	FILETIME ct ;
	FILETIME et ;
	FILETIME kt ;
	FILETIME ut ;
	FILETIME cptt ; //当前进程所用的总时间，用户时间＋系统时间
	quint64  cpit ;// 转换为nano second 的值

	pid = ::GetCurrentProcess();	//当前进程的句柄
	bool ret = ::GetProcessTimes(pid,&ct,&et,&kt,&ut);
	cptt.dwHighDateTime = kt.dwHighDateTime + ut.dwHighDateTime;
	cptt.dwLowDateTime = kt.dwLowDateTime + ut.dwLowDateTime ;
	memcpy(&cpit,&cptt,sizeof(quint64));

	while( true )
	{
		pidarrlen += sizeof(DWORD) ;
		if( allpids == 0 )
			allpids = (DWORD *)malloc(sizeof(DWORD) * pidarrlen);
		else
			allpids = (DWORD *)realloc(allpids,sizeof(DWORD) * pidarrlen );

		bool ret = ::EnumProcesses( allpids,pidarrlen,&gotpids);
		if( ret == false && pidarrlen > 500 )
			break;	//出错次数也太多了点吧。
		else if ( ret == false && pidarrlen <= 500 )
			continue;
		else
		{
			//qDebug()<<" ret :"<< ret << " LE:"<<::GetLastError() ;
			;;;;;;;;;;;;;;;
		}

		if( gotpids >= pidarrlen )
			continue ;	//可能是数组大小不够大
		if( gotpids < pidarrlen )
		{
			realpidcnt = gotpids/sizeof(DWORD) ;
			//qDebug()<<" process count:"<<realpidcnt << " can len:"<<pidarrlen <<" byte:"<< gotpids ;

			//long long  tut = 0 ;
			//long long  tkt = 0 ;
			FILETIME   tut = {0,0};
			FILETIME   tkt = {0,0};
			for( int i = 0 ; i < realpidcnt ; i ++ )
			{
				//if( allpids [i] == 0 ) continue ;//忽略pid 为 0的进程，它为SystemIdle process 
				
				HANDLE hProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,allpids[i]);
				if( hProcess == 0 )
				{
					//qDebug()<<" OpenProcess"<<allpids[i]<< " error:"<<::GetLastError() ;
					//发错误的一般是系统进程，alg.exe , svchost.exe,csrss.exe
					//遇到的错误: 5 ERROR_ACCESS_DENIED
					//87		ERROR_INVALID_PARAMETER The parameter is incorrect. 
				}
				else
				{
					FILETIME act ;
					FILETIME aet ;
					FILETIME akt ;
					FILETIME aut ;

					bool aret = ::GetProcessTimes(hProcess,&act,&aet,&akt,&aut);
					
					tut.dwHighDateTime += aut.dwHighDateTime ;
					tut.dwLowDateTime += aut.dwLowDateTime ;
					tkt.dwHighDateTime += akt.dwHighDateTime;
					tkt.dwLowDateTime += akt.dwLowDateTime ;
					////////////////////////////////
					char fileName[1024] = {0} ;
					//::GetProcessImageFileNameA(hProcess,fileName,sizeof(fileName));
					//qDebug()<<" Process File Name :"<< fileName ;
				}
			}	// end for( int i = 0 ; i < realpidcnt ; i ++ )

			if( this->mLastSysTime == 0 )
			{
				usage = " 16% " ;
				
				FILETIME cft ;
				SYSTEMTIME cst ;
				memset(&cst,0,sizeof(SYSTEMTIME));
				::GetSystemTime(&cst);
				::SystemTimeToFileTime(&cst,&cft);
				
				memcpy(&this->mLastSysTime,&cft,sizeof(quint64));
				this->mLastCurrentProcessTime = cpit;
			}
			else
			{
				FILETIME dut ;
				FILETIME dkt ;		//d => delta 		
				quint64  dst  ;
				FILETIME cft ;	//当前系统文件时间
				this->mLastUserTime = tut ;
				this->mLastKernelTime = tkt ;
				SYSTEMTIME cst ;
				memset(&cst,0,sizeof(SYSTEMTIME));
				::GetSystemTime(&cst);
				::SystemTimeToFileTime(&cst,&cft);
				memcpy(&dst,&cft,sizeof(quint64) );

				dst = dst - this->mLastSysTime ;	//从上次获取到现在过去的时间。

				//qDebug()<<" time inteval:"<< dst ;	// 是nano second 微秒 ，即 1/1000000 秒

				usage = QString("%1%").arg( ( cpit - this->mLastCurrentProcessTime ) *10 / dst );

				//qDebug()<<" current process usage :"<< ( cpit - this->mLastCurrentProcessTime ) 
				//	<< ( cpit - this->mLastCurrentProcessTime ) *100 / dst ;
				
				memcpy(&this->mLastSysTime,&cft,sizeof(quint64));
				this->mLastCurrentProcessTime = cpit ;

			}
			//qDebug()<<" U:"<<tut.dwHighDateTime << tut.dwLowDateTime
			//	<<" K:"<< tkt.dwHighDateTime << tkt.dwLowDateTime ;

			break ;
		}
		else
		{
			assert(1 == 2 );
		}
	}

	free(allpids);

	return usage ;
}
#else
QString ResourceMonitor::getUnixMemoryUsageByProc()
{
    
}

QString ResourceMonitor::getUnixMemoryUsageByTop()
{
    QString usage ;

    QString logname = QString( getlogin() );
    QString cmd = QString( " top -b -n 1 -u %1").arg(logname) ;
    QProcess p;
    p.start(cmd);
    if( ! p.waitForStarted() )
    {
        qDebug()<<"wait start error"<<__FUNCTION__<<__LINE__;
    }
    if( ! p.waitForReadyRead() )
    {
        qDebug()<<" wait read error"<<__FUNCTION__<<__LINE__;
    }
    if( ! p.waitForFinished() )//use it for truncated result lines
    {
        qDebug()<<" wait finish error"<<__FUNCTION__<<__LINE__;
    }
        
    QByteArray outString = p.readAllStandardOutput();
	//qDebug()<< outString ;

    int rmemFieldIndex = -1;
    int cpuFieldIndex = -1 ;
    int vmemFieldIndex = -1 ;
    int cmdFieldIndex = -1 ;

    QString prevLine ;
    QString totalMemory ;
    QString outLines = QString(outString);
    QStringList outList = outLines.split("\n");
    for( int i = 0 ; i < outList.count() ; i ++ )
    {
        QStringList oneLine = outList.at(i).split(" ",QString::SkipEmptyParts);
		                
        if( i > 0 && prevLine.trimmed().length() == 0)
        {                          
			//�ҵ�������Ҫ���ֶε�λ��
            for( int f = 0 ; f < oneLine.count() ; f ++ )
            {
                if( oneLine.at(f).trimmed() == "%CPU")
                {
                    cpuFieldIndex = f ; 
                }
                else if ( oneLine.at(f).trimmed() == "RES")
                {
                    rmemFieldIndex = f ;
                }
                else if ( oneLine.at(f).trimmed() == "SHR") //VIRT = RES + SWAP
                {
                    vmemFieldIndex = f ;
                }
                else if ( oneLine.at(f).trimmed() == "COMMAND")
                {
                    cmdFieldIndex = f ;
                }
            }
                    
        }
        //
        if(oneLine.count() > 0 && oneLine.at(0).trimmed() == "Mem:")
        {
            totalMemory = oneLine.at(1);
        }
        ////
#define MAX(x,y) ((x>y)?(x):(y))

		//qDebug()<<"rmem:"<<rmemFieldIndex<<" vmem:"<<vmemFieldIndex <<" cpu:"<< cpuFieldIndex ;
        if( cmdFieldIndex != -1 && oneLine.count() > MAX( MAX( rmemFieldIndex,vmemFieldIndex), cpuFieldIndex)		
            && oneLine.at(cmdFieldIndex).trimmed() == "NullGet")
        {
            usage = QString(tr("PM: %1 , VM: %2 , TM: %3 && %4%")).arg(oneLine.at(rmemFieldIndex))
                    .arg(oneLine.at(vmemFieldIndex)).arg( totalMemory ).arg(oneLine.at(cpuFieldIndex));

            break ;
        }
        prevLine = QString(outList.at(i));
    }

    QByteArray errString = p.readAllStandardError();
	//qDebug()<< errString ;
        
    return usage ;
}
QString ResourceMonitor::getUnixCPUUsage()
{
	QString usage ;

	return usage ;
}

#endif

/*

在CSDN搜到这两个API：GetProcessMemoryInfo 和 GetPerformanceInfo

用GetProcessMemoryInfo可以获得当前进程的占用内存数，接下来我如何获得整机的内存数，或者直接可以获得内存占用率？

还有CPU占用率怎么获得？GetPerformanceInfo在MSDN2001July里没查到。下了个Platform SDK 2001，在psapi.h里找到了它，不过参数没有传handle，所以估计也没法得到当前进程的CPU占用率。

先谢过。

		
取系统快照: 

CreateToolhelp32Snapshot

然后
Process32First, Process32Next 来得到各进程的信息.

?		
内存数

GlobalMemoryStatus


?		
http://www.vckbase.com/document/viewdoc/?id=1271

	
应该使用Performance counters

?		
http://www.codeproject.com/threads/Get_CPU_Usage.asp


?		
看了各位提供的内容，GetProcessTimes应该是最符合我需求的API。

现在还有个问题，就是不太清楚kerneltime和usertime与CPU利用率的关系。

看了goodboyws(深夜不眠者(VCMVP)) 给的，网上也搜了下，模糊的知道该如何得到CPU利用率。但还是不太清楚他们具体的含义和对应关系。希望哪位再给讲讲。

谢谢！

//////////////////
获得进程的CPU占用率(linux 版本)

#include <stdio.h>; 
#include <stdlib.h>; 
#include <unistd.h>; 
struct occupy 
{ 
　   char name[20]; 
　   unsigned int user; 
　   unsigned int nice; 
　   unsigned int system; 
　   unsigned int idle; 
}; 
float g_cpu_used; 
int cpu_num; 
void cal_occupy(struct occupy *, struct occupy *); 
void get_occupy(struct occupy *); 
int main() 
{ 
struct occupy ocpu[10]; 
struct occupy ncpu[10]; 
int i; 

cpu_num = sysconf(_SC_NPROCESSORS_ONLN); 
for(;;) 
{ 
　 sleep(1); 
　 get_occupy(ocpu); 
　 sleep(1); 
　 get_occupy(ncpu); 
　 for (i=0; i<cpu_num; i++) 
　 { 
　  　  cal_occupy(&ocpu[I], &ncpu[I]); 
　  　  printf("%f \n", g_cpu_used); 
　 } 
} 
} 

void 
cal_occupy (struct occupy *o, struct occupy *n) 
{ 
　　double od, nd; 
　　double id, sd; 
　　double scale; 
　　od = (double) (o->;user + o->;nice + o->;system + 
　  　  　 o->;idle); 
　  　  　  　  　  　  　  　  　  　  　  　  　  　  　  　　  
　　nd = (double) (n->;user + n->;nice + n->;system + 
　  　  　 n->;idle); 
　　scale = 100.0 / (float)(nd-od); 
　　id = (double) (n->;user - o->;user); 
　　sd = (double) (n->;system - o->;system); 
　　g_cpu_used = ((sd+id)*100.0)/(nd-od); 
} 

void 
get_occupy (struct occupy *o) 
{ 
　　FILE *fd; 
　　int n; 
　　char buff[1024]; 
　  　  　  　  　  　  　  　  　  　  　  　  　  　  　  　　  
　　fd = fopen ("/proc/stat", "r"); 
　　fgets (buff, sizeof(buff), fd); 
　　for(n=0;n<cpu_num;n++) 
　　{ 
　 fgets (buff, sizeof(buff),fd); 
　 sscanf (buff, "%s %u %u %u %u", &o[n].name, &o[n].user, &o[n].nice, 
　  　  　　&o[n].system, &o[n].idle); 
　   fprintf (stderr, "%s %u %u %u %u\n", o[n].name, o[n].user, o[n].nice, 
　  　  　 o[n].system, o[n].idle); 
　　} 
　 fclose(fd); 
} 

//////////////////////////////////////
　　因此，为了你系统的健康，关机还是按照正常顺序来，数据的安全性往往比节省的那几十秒钟重要的多。
代码：

C++

    HINSTANCE h=LoadLibrary("ntdll.dll");
    if(!h)
    {
        MessageBox("加载'ntdll.dll'失败！");
        return;
    }

    typedef int (__stdcall *AdjustPrivilege)(int,int,int,int *);
    AdjustPrivilege SetPrivilege=NULL;
    SetPrivilege=(AdjustPrivilege)GetProcAddress(h,"RtlAdjustPrivilege");
    if(!SetPrivilege)
    {
        MessageBox("加载'RtlAdjustPrivilege'失败！");
        return;
    }

    typedef int (__stdcall *ShutdownSystem)(int);
    ShutdownSystem Shutdown=NULL;
    Shutdown=(ShutdownSystem)GetProcAddress(h,"NtShutdownSystem");
    if(!Shutdown)
    {
        MessageBox("加载'NtShutdownSystem'失败！");
        return;
    }

    const int SE_SHUTDOWN_PRIVILEGE = 19;
    const int shutdown = 0;
    const int RESTART = 1;
    const int POWEROFF = 2;
    int *a;
    (*SetPrivilege)(SE_SHUTDOWN_PRIVILEGE,true,false,a);
    (*Shutdown)(shutdown);
/////////////////////////////////////////


*/



