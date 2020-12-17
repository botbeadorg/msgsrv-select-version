#include "StdAfx.h"
#include <string>
#include <iostream>
using namespace std;
const TCHAR szExceptionDumpFile[] = _T(".\\NameServer.dump");

BOOL SetupNameServerConfig(CNameServer *lpNameServer);
VOID ServerMain();
VOID ServiceMain(int argc, char** argv);


//会话管理器版本号
#define NAME_MGR_KN_VERION   MAKEFOURCC(17, 2, 19, 1)

#ifdef WIN32
class CNameService : public CWinService
{
public:
	CNameService(LPCSTR pServiceName, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS):CWinService(pServiceName, dwServiceType){};
	int AppMain()
	{
		SetCurrentDirectory("../");
		CFileLogger flog(_T("NameServer_%s.log.html"), getCurrentTimeDesc());

		CNameServer *pNameServer = new CNameServer();
		if ( SetupNameServerConfig(pNameServer) )
		{
			pNameServer->InitSocketLib();
			if ( pNameServer->Startup() )
			{
				
				in_addr ia;
				ia.s_addr = NAME_MGR_KN_VERION;

				OutputMsg( rmTip, _T("-------------------------------------------") );
				OutputMsg( rmTip, _T("名称服务器启动成功，核心版本号是%s"), inet_ntoa(ia) );
				OutputMsg( rmTip, _T("quit命令可以停止服务并退出程序") );
				OutputMsg( rmTip, _T("-------------------------------------------") );
				while (!m_boServiceExit)
				{
					Sleep(1000);
				}
				pNameServer->Stop();
				pNameServer->UnintSocketLib();
			}
			else
			{
		
				OutputMsg(rmError,"pNameServer->Startup() fail");
				system("pause");
			}
		}
		else
		{
			OutputMsg(rmError,"SetupNameServerConfig fail");
			system("pause");
		}
		delete pNameServer;

		return 0;
	}
};
#endif

int main(int argc, TCHAR *argv[])
{
	unsigned short version;
	WSADATA wsadata;
	version = MAKEWORD(2, 2);
	WSAStartup(version, &wsadata);
#ifdef WIN32
	SetUnhandledExceptionFilter( DefaultUnHandleExceptionFilter );
#endif

	InitDefMsgOut();

	#ifdef WIN32
			
			//flog.SetNeedOutput(true);
#else
			if (argc >=2)
			{
				//flog.SetNeedOutput(true);
			}	
			else
			{
				//flog.SetNeedOutput(false);
			}
#endif

//#ifndef _SERVICE
	//if (argc == 2 && _tcsncicmp("/cmd",argv[1],4)==0)//平时调试用
	{
		//SetCurrentDirectory("../");

#ifdef WIN32
		SetCurrentDirectory("../");
#else 
		string filename(argv[0]);   
		size_t found = filename.find_last_of("/\\");
		filename = filename.substr(0, found);
		found = filename.find_last_of("/\\");
		filename = filename.substr(0, found);
		if( filename[0] == '.' && filename.length()==1 )
			filename = "../" ;
		SetCurrentDirectory(filename.c_str());
#endif
                
				
		ServerMain();
	}
//#else
	//else
	{
		//ServiceMain(argc, argv);
	}
//#endif

	UninitDefMsgOut();
#ifdef	_MLIB_DUMP_MEMORY_LEAKS_
	_CrtDumpMemoryLeaks();
#endif
	WSACleanup();
	return 0;
}

VOID ServiceMain(int argc, char** argv)
{
	LPTSTR sCmd = NULL;
	if (argc >= 2)
	{
		sCmd = argv[1];
	}
#ifdef WIN32
	CNameService service("NameService");
	service.Run(sCmd);
#endif

}

VOID ServerMain()
{

	if (!FDOP::IsDirectory(_T("log")))
	{
		FDOP::DeepCreateDirectory(_T("log"));
	}
	CFileLogger flog(_T("log/NameServer_%s.log.html"), getCurrentTimeDesc());
	CNameServer *pNameServer = new CNameServer();
	OutputMsg( rmTip, _T("Start Name Server") );
	bool isFail =false;
	
	if ( SetupNameServerConfig(pNameServer) )
	{
		pNameServer->InitSocketLib();
		if ( pNameServer->Startup() )
		{
			
			TCHAR sCmdBuf[512];
			in_addr ia;
			ia.s_addr = NAME_MGR_KN_VERION;

			char pBuff[256] ;
			strcpy(pBuff, pNameServer->GetServerName());
			strcat(pBuff,"-V");
			strcat(pBuff,inet_ntoa(ia));
			SetConsoleTitle( pBuff );

			
			OutputMsg( rmTip, _T("-------------------------------------------") );
			OutputMsg( rmTip, _T("名称服务器启动成功，核心版本号是%s"), inet_ntoa(ia) );
			OutputMsg( rmTip, _T("quit命令可以停止服务并退出程序") );
			OutputMsg( rmTip, _T("-------------------------------------------") );

			while (true)
			{
				//_getts(sCmdBuf);
				std::cin >> (sCmdBuf);
				//退出命令
				if ( _tcsncicmp(sCmdBuf, _T("\\q"), 2) == 0 
					|| _tcsncicmp(sCmdBuf, _T("exit"), 4) == 0
					|| _tcsncicmp(sCmdBuf, _T("quit"), 4) == 0 )
				{
					OutputMsg( rmTip, _T("正在停止名称服务...") );
					break;
				}
				Sleep(10);
			}
			pNameServer->Stop();
			pNameServer->UnintSocketLib();
		}
		else
		{
			#ifdef WIN32
						OutputMsg(rmError," pNameServer->Startup() fail");
						system("pause");
			#else
						printf("pNameServer->Startup() fail,exit 3s later");
						Sleep(3000); //10秒后退出

			#endif	
		}
	}
	else
	{
		#ifdef WIN32
				OutputMsg(rmError,"SetupNameServerConfig fail");
				system("pause");
		#else
				printf("SetupNameServerConfig fail ,exit 3s later");
				Sleep(3000); //10秒后退出
		#endif	
	}
	
	delete pNameServer;
}

BOOL SetupNameServerConfig(CNameServer *lpNameServer)
{
	CNameServerConfig config;
	return config.loadConfig(lpNameServer);
}
