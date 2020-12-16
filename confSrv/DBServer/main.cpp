#include "StdAfx.h"
#include <dbghelp.h>
#include <mbctype.h>
#include <conio.h>

#include "PathThreadLocale.h"

//定义转储文件名称
extern const TCHAR szExceptionDumpFile[] = _T("./DBEngine.dmp");
BOOL SetupDBEngineConfig(CDBServer *lpDBEngine);
VOID ServerMain();
VOID ServiceMain(int argc, char** argv);

//数据引擎版本号
#define DBEGN_KN_VERION			MAKEFOURCC(18, 7, 9, 1)
//数据引擎的数据结构版本

//#define	DBSDATATYPE_VERSION		0x010A1B0A


int initLocalePath()
{
	//_MB_CP_SBCS   视所有字符均作为单字节字符，以便同时支持UTF8以及MBCS
	//Use single-byte code page. 
	//When the code page is set to _MB_CP_SBCS, a routine such as _ismbblead always returns false.
	_setmbcp(_MB_CP_SBCS);
	//★关键★ 设置"C"locale，视所有字符均作为单字节字符，以便同时支持UTF8以及MBCS
	return InstallThreadLocalePath("C");
}

void TestMemory()
{
#ifdef _DEBUG

#else
	/*
	DWORD dwStart = GetTickCount();
	for(int i = 0; i < 1000000; i++) { char *p = (char*)malloc(i%1000000); free(p); } 

	DWORD dwEnd = GetTickCount();
	printf("***********************",dwEnd - dwStart);
	printf("100wMemoryTest:%d ms\n",dwEnd - dwStart);
	*/

#endif
}


class CDbService : public CWinService
{
public:
	CDbService(LPCSTR pServiceName, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS):CWinService(pServiceName, dwServiceType){};
	int AppMain()
	{	
		SetCurrentDirectory("../");		//CWinService里会把当前目录改成exe允许的目录，所以这里再转一下		
		CFileLogger flog(_T("log/DBServer_%s.log.html"), getCurrentTimeDesc());
		CDBServer *pDBEngine = new CDBServer();

		if ( !SetupDBEngineConfig(pDBEngine) )
		{
			OutputMsg( rmError, _T("读入配置文件失败，服务停止！"));
		}	
		else if ( pDBEngine->Startup() )
		{
			while (!m_boServiceExit)
			{
				Sleep(1000);
			}
			pDBEngine->Shutdown();
		}

		delete pDBEngine;

		return 0;
	}
};

int main(int argc, TCHAR* argv[])
{
	//char tmp[128];
	//memset(tmp,0,sizeof(tmp));
	//EncryptPassword(tmp, sizeof(tmp), "8FzW^C*e4kqV", "123456abc123456a");
	//printf(tmp);
	unsigned short version;
	WSADATA wsadata;
	version = MAKEWORD(2, 2);
	WSAStartup(version, &wsadata);
	SetUnhandledExceptionFilter( DefaultUnHandleExceptionFilter );
	InitDefMsgOut();
	CoInitialize(NULL);
	CTimeProfDummy::SetOpenFlag(false);
	if (initLocalePath())
	{
		OutputMsg( rmError, _T("can not set locale path") );
		getc(stdin);
		return -2;
	}
	CTimeProfDummy::SetOpenFlag(false);
#ifndef _SERVICE
	//if (argc == 2 && _tcsncicmp("/cmd",argv[1],4)==0)//平时调试用
	{
		SetCurrentDirectory("../");
		ServerMain();
	}
#else
	//else
	{
		ServiceMain(argc, argv);
	}
#endif

	CoUninitialize();

	printf("AbortCloseThread ...\n");
	CloseServerHelper::GetInst().AbortCloseThread();

	printf("UninitDefMsgOut ...\n");
	UninitDefMsgOut();
	printf("UninitDefMsgOut finish ...\n");
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
	TestMemory();
	CDbService service("DbService");
	service.Run(sCmd);
}

BOOL WINAPI CustomConsoleCtrlHandler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT)
	{
		Beep( 600, 200 ); 
		printf( "Ctrl-Close event\n\n" );
		return TRUE;
	}
	else
		return FALSE;
}

VOID ServerMain()
{
	int nError;
	TCHAR sCmdBuf[512];
	CFileLogger flog(_T("log/DBServer_%s.log.txt"), getCurrentTimeDesc());
	CDBServer *pDBEngine = new CDBServer();
	CDBServer::s_pDBEngine = pDBEngine;
	//SetConsoleCtrlHandler(CustomConsoleCtrlHandler, TRUE);
	//DeleteMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);
	if (initLocalePath())
	{
		OutputMsg( rmError, _T("can not set locale path") );
		getc(stdin);
		return ;
	}

	
	if ( !SetupDBEngineConfig(pDBEngine) )
	{
		OutputMsg( rmError, _T("读入配置文件失败，服务停止！"));
	}	
	else if ( pDBEngine->Startup() )
	{
		//设置窗口标题
		CTimeProfMgr::getSingleton().InitMgr();
		
		char *pBuff  = (char*)malloc(256);
		in_addr ia;
		ia.S_un.S_addr = DBEGN_KN_VERION;
		strcpy(pBuff, pDBEngine->getServerName());
		strcat(pBuff,"-V");
		strcat(pBuff,inet_ntoa(ia));
		SetConsoleTitle(pBuff);
		free(pBuff);
		pBuff =NULL;

		TestMemory();
		
		OutputMsg( rmTip, _T("-------------------------------------------") );
		OutputMsg( rmTip, _T("数据服务器启动成功，核心版本号是%s"), inet_ntoa(ia) );
		
		//OutputMsg( rmTip, _T("数据结构版本号是：%X"), DBSDATATYPE_VERSION );
//		OutputMsg( rmTip, _T("角色基础数据结构大小是：%d"), sizeof(DBCHARBASICDATA) );
		OutputMsg( rmTip, _T("quit命令可以停止服务并退出程序") );
		OutputMsg( rmTip, _T("lgr 命令可以重新加载游戏网关路由表") );
		OutputMsg( rmTip, _T("-------------------------------------------") );
		unsigned long long startTick = _getTickCount();
		while ( pDBEngine->GetDbStartFlag() )
		{
			sCmdBuf[0] = 0;

			if(_kbhit())
			{
				_getts(sCmdBuf);
			}
			else
			{
				Sleep(100);
				continue;
			}

			//重新加载路由命令
			if ( _tcsncicmp(sCmdBuf, _T("lgr"), 3) == 0 )
			{
				nError = pDBEngine->LoadGameServerRoute();
				if ( nError >= 0 )
					OutputMsg( rmTip, _T("已加载%d个游戏网关路由数据"), nError);
				else OutputMsg( rmError, _T("加载游戏网关路由数据失败"));
				continue;
			}
			//退出命令
			if ( _tcsncicmp(sCmdBuf, _T("\\q"), 2) == 0
				|| _tcsncicmp(sCmdBuf, _T("exit"), 4) == 0
				|| _tcsncicmp(sCmdBuf, _T("quit"), 4) == 0 )
			{
				OutputMsg( rmTip, _T("正在停止网关管理器...") );
				break;
			}
			else if ( _tcsncicmp(sCmdBuf, _T("spf"), 3) ==0) 
			{
				CTimeProfMgr::getSingleton().dump();
				pDBEngine->TraceGameServerRoute();
			}
			// asi interval (interval:save time interval, s)
			else if (_tcsncicmp(sCmdBuf, _T("asi"), 3) == 0)
			{
				char *pParam = sCmdBuf + 3;
				while (*pParam == ' ')
					pParam++;
				if (pParam)
				{
					UINT_PTR nInterval = atoi(pParam);
					if (nInterval > 0)
						CDBDataCache::s_nActorCacheSaveInterval = nInterval*1000;
				}
			}
			else if(_tcsncicmp(sCmdBuf,"dmp",3) ==0 ) 
			{
				DebugBreak();
			}
			else if(_tcsncicmp(sCmdBuf,"opentrace",9) ==0 ) 
			{
				CTimeProfDummy::SetOpenFlag(true);
			}
			else if(_tcsncicmp(sCmdBuf,"closetrace",10) ==0 ) 
			{
				CTimeProfDummy::SetOpenFlag(false);
			}
			else if(_tcsncicmp(sCmdBuf,"memory",6) ==0 ) 
			{
				pDBEngine->Trace();
			}
			else if (_tcsncicmp(sCmdBuf, "tickcount", 9) == 0)
			{
				char *pParam = sCmdBuf + 9;
				while (*pParam == ' ')
					pParam++;
				if (pParam)
				{
					int nInterval = atoi(pParam);
					if (nInterval > 0)
						TickCount::g_maxTick = nInterval;
					OutputMsg(rmNormal, _T("TickCount::g_maxTick=%d"), TickCount::g_maxTick);
				}				 
			}
			else if (_tcsncicmp(sCmdBuf, _T("loginlog"), 8) == 0) // loginlog 0或1
			{
				char *pParam = sCmdBuf + 8;
				while (*pParam == ' ')
					pParam++;

				if (*pParam != '\0')
				{
					UINT_PTR is_print = atoi(pParam);
					get_global_data().set_print_login_log(is_print > 0 ? true : false);

					OutputMsg(rmTip, is_print > 0 ? _T("-------enable login log") : _T("-------disable login log"));
				}
			}
			
			Sleep(10);
		}

		//pDBEngine->Shutdown();
	}

	printf("delete pDBEngine ...\n");
	delete pDBEngine;
	printf("delete pDBEngine finish ...\n");
}



BOOL SetupDBEngineConfig(CDBServer *lpDBEngine)
{
	CDBConfig	config;
	if ( !config.ReadConfig() )
		return FALSE;

	lpDBEngine->RunEsqlFile(config.m_EsqlToolPath,config.DbConf.szDbName);
	lpDBEngine->SetServerName(config.ServerName);
	lpDBEngine->SetServerIndex(config.ServerIndex);
	lpDBEngine->SetGateServiceAddress(config.GateAddr.szAddr, config.GateAddr.nPort);
	//lpDBEngine->SetSessionServerAddress(config.SessionAddr.szAddr, config.SessionAddr.nPort);	
	lpDBEngine->SetNameSyncServerAddress(config.NameAddr.szAddr, config.NameAddr.nPort);
	lpDBEngine->SetDataServiceAddress(config.DataAddr.szAddr, config.DataAddr.nPort);
	lpDBEngine->SetSQLConfig(config.DbConf.szHost, config.DbConf.nPort, config.DbConf.szDbName, config.DbConf.szUser, config.DbConf.szPassWord);
	
	//启动日志客户端
	lpDBEngine->SetLogServerAddress(config.LogAddr.szAddr, config.LogAddr.nPort);
	//lpDBEngine->SetDBCenterAddress(config.DBCenterAddr.szAddr, config.DBCenterAddr.nPort);


	//lpDBEngine->LoadActorNameList(config.m_sBoyFileName,config.m_sGirlFileName);

	return TRUE;
}
