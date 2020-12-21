#include "StdAfx.h"
#include "LogicServerConfig.h"
#include <ws2tcpip.h>

const TCHAR CLogicServerConfig::ConfigFileName[] = _T("LogicServer.txt");
const char CLogicServerConfig::strGmLevel[] = _T("../../gmlevel.txt");

#pragma pack(1)
struct conf_logicsrv{
	int srv_index;
	int spguid;
	int gate_srv_port;
	int sesn_srv_port;
	int log_srv_port;
	int db_srv_port;
	int am_srv_port;
	int back_srv_port;
	int locallog_srv_port;
    int world_srv_port;
	char spid[20];
	char zone_open_time[20];
	char zone_merge_time[20];
	char srv_name[33];
	char gate_srv_ip[33];
	char sesn_srv_ip[33];
	char log_srv_ip[33];
	char db_srv_ip[33];
	char am_srv_ip[33];
	char back_srv_ip[33];
	char locallog_srv_ip[33];
    char world_srv_ip[33];
};

struct conf_logicxsrv {
	int id;
	int port;
	int login_port;
	char host[33];
	char login_ip[33];
};
#pragma pack()

#define LOAD_GMLEVEL	// 开服屏敝掉, 内部测试开启

void read_config_new(conf_logicsrv *cbs)
{
	int r, len;
	unsigned short version;
	addrinfo hints, *res;
	char *port_to_listen = "65000";
	const char *cmdhead = "MOC.DAEBTOB.XYHH";
	const char *cmd = "logic";
	char buf[2048];
	char *buf_t = buf;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (0 != getaddrinfo("localhost", port_to_listen, &hints, &res)) {
		exit(1);
	}

	int c = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (INVALID_SOCKET == (unsigned) c) {
		exit(2);
	}

	if (SOCKET_ERROR == connect(c, res->ai_addr, res->ai_addrlen)) {
		closesocket(c);
		exit(3);
	}

	freeaddrinfo(res);

	SecureZeroMemory(buf, 2048);
	MoveMemory(buf, cmdhead, strlen(cmdhead));
	buf_t = buf + strlen(cmdhead);
	*(int *)buf_t = strlen(cmd) + 1;
	buf_t += sizeof(int);
	MoveMemory(buf_t, cmd, strlen(cmd));
	len = strlen(cmdhead)+sizeof(int) + strlen(cmd) + 1;
	send(c, buf, len, 0);

	SecureZeroMemory(buf, 2048);
	r = recv(c, buf, 2048, 0);
	if (r > 0) {
		printf("bytes received: %d\n", r);
		MoveMemory(cbs, buf, sizeof(*cbs));
	}
	else if (r == 0)
		printf("Connection closed\n");
	else
		printf("recv failed: %d\n", WSAGetLastError());
	closesocket(c);
}

void read_xconfig_new(conf_logicxsrv* *cbs,int *l)
{
	int r, len;
	unsigned short version;
	addrinfo hints, *res;
	char *port_to_listen = "65000";
	const char *cmdhead = "MOC.DAEBTOB.XYHH";
	const char *cmd = "logic_x";
	//char buf[2048];
	char *buf = new char[2048];
	char *buf_t = buf;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (0 != getaddrinfo("localhost", port_to_listen, &hints, &res)) {
		exit(1);
	}

	int c = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (INVALID_SOCKET == (unsigned) c) {
		exit(2);
	}

	if (SOCKET_ERROR == connect(c, res->ai_addr, res->ai_addrlen)) {
		closesocket(c);
		exit(3);
	}

	freeaddrinfo(res);

	SecureZeroMemory(buf, 2048);
	MoveMemory(buf, cmdhead, strlen(cmdhead));
	buf_t = buf + strlen(cmdhead);
	*(int *)buf_t = strlen(cmd) + 1;
	buf_t += sizeof(int);
	MoveMemory(buf_t, cmd, strlen(cmd));
	len = strlen(cmdhead)+sizeof(int) + strlen(cmd) + 1;
	send(c, buf, len, 0);

	SecureZeroMemory(buf, 2048);
	r = recv(c, buf, 2048, 0);
	if (r > 0) {
		printf("bytes received: %d\n", r);
		//MoveMemory(cbs, buf, sizeof(*cbs));
		*cbs = (conf_logicxsrv *)buf; 
		*l = r/(sizeof(conf_logicxsrv));
	}
	else if (r == 0)
		printf("Connection closed\n");
	else
		printf("recv failed: %d\n", WSAGetLastError());
	closesocket(c);
}

CLogicServerConfig::CLogicServerConfig()
	:Inherited()
{

}

CLogicServerConfig::~CLogicServerConfig()
{

}

void CLogicServerConfig::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

bool CLogicServerConfig::loadServerConfig(CLogicServer *lpLogicServer, const char *pszFileName)
{
	bool boResult = false;	
	if (!pszFileName)
		pszFileName = ConfigFileName;
	try
	{		
		boResult = throwLoadConfig(lpLogicServer, pszFileName);
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, (LPCSTR)s);
	}
	catch(...)
	{
		OutputMsg(rmError, _T("unexpected error on load - read ServerConfig "));
	}
	OutputMsg(rmTip,_T("load server config complete"));
	return boResult;
}

bool CLogicServerConfig::LoadGmLevel(CLogicServer *lpLogicServer, const char* pszFileName)
{
	bool boResult = false;

#ifdef LOAD_GMLEVEL
	if (!pszFileName)
		pszFileName = strGmLevel;
	try
	{
		boResult = throwGmLevelCfg(lpLogicServer, pszFileName);
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, (LPCSTR)s);
	}
	catch(...)
	{
		OutputMsg(rmError, _T("unexpected error on load - read gmlevel "));
	}
	OutputMsg(rmTip, _T("load server gmlevel complete"));
#endif
	return boResult;
}

bool CLogicServerConfig::throwLoadConfig(CLogicServer *lpLogicServer, const char *pszFileName)
{
	/*
	wylib::stream::CMemoryStream ms;	
	if ( ms.loadFromFile(pszFileName) <= 0 )
	{
		showErrorFormat(_T("unable to load from %s"), pszFileName);
		return false;
	}

	if ( !setScript((LPSTR)ms.getMemory()) )
		return false;
	*/
	
	return readServerConfig(lpLogicServer);
}

bool CLogicServerConfig::throwGmLevelCfg(CLogicServer *lpLogicServer, const char *pszFileName)
{
	wylib::stream::CMemoryStream ms;	
	if ( ms.loadFromFile(pszFileName) <= 0 )
	{
		OutputMsg(rmTip, "unable to load from %s", pszFileName);
		return false;
	}

	if ( !setScript((LPSTR)ms.getMemory()) )
		return false;

	if (!openGlobalTable("GMSet"))
		return false;

	int nVal = -1;
	int nGmLevel = getFieldInt("gmlevel", &nVal);
	GetLogicServer()->SetGmLevel(nGmLevel);
	closeTable();
	return true;
}

bool CLogicServerConfig::readServerConfig(CLogicServer *lpLogicServer)
{
	conf_logicsrv conf;
	read_config_new(&conf);

	SYSTEMTIME startSysTime; //服务器的开启时间
	CMiniDateTime nStart;

	BOOL IsValid =TRUE;
	//LPCTSTR sName = getFieldString("ServerOpenTime", "",&IsValid);
	LPCTSTR sName = conf.zone_open_time;
	if( IsValid &&  sName != NULL && strlen(sName) >10) //是个基本合法的名字
	{
		sscanf(sName, "%d-%d-%d %d:%d:%d", &startSysTime.wYear, &startSysTime.wMonth, &startSysTime.wDay, &startSysTime.wHour, &startSysTime.wMinute, &startSysTime.wSecond);
	}
	else
	{
		GetLocalTime(&startSysTime);
	}
	nStart.encode(startSysTime);

	SYSTEMTIME combineTime; //服务器的合服时间
	CMiniDateTime nCombine;

	BOOL boValid =TRUE;
	//LPCTSTR sTime = getFieldString("ServerCombineTime", "",&boValid);
	LPCTSTR sTime = conf.zone_merge_time;
	if( boValid &&  sTime != NULL && strlen(sTime) >10) //是个基本合法的名字
	{
		sscanf(sTime, "%d-%d-%d %d:%d:%d", &combineTime.wYear, &combineTime.wMonth, &combineTime.wDay, &combineTime.wHour, &combineTime.wMinute, &combineTime.wSecond);
		nCombine.encode(combineTime);
	}
	else
	{
		nCombine = 0;
	}

	char strFilePath[64] = "./data/runtime";

	lpLogicServer->SetServerIndex(conf.srv_index);
	lpLogicServer->SetServerName(conf.srv_name);
	lpLogicServer->SetServerOpenTime(nStart); //设置开区时间
	lpLogicServer->SetServerCombineTime(nCombine);	// 设置合区时间
	lpLogicServer->SetStrFilePath(strFilePath);	

	lpLogicServer->SetSpid(conf.spguid);

	//INT nVal;
	//nVal = CLogicServer::ChuanQiZhiRen; // 默认游戏索引
	//lpLogicServer->SetGameIndex(getFieldInt("GameIndex", &nVal)); //设置游戏索引
	lpLogicServer->SetGameIndex(CLogicServer::ChuanQiZhiRen);

	CLogicGateManager *pLogicManager = lpLogicServer->GetGateManager();
	if (pLogicManager)
	{
		pLogicManager->SetServiceHost(conf.gate_srv_ip);
		pLogicManager->SetServicePort(conf.gate_srv_port);
	}
	
	CLogicSSClient * pSessionClient = lpLogicServer->GetSessionClient();
	if (pSessionClient)
	{
		pSessionClient->SetServerHost(conf.sesn_srv_ip);
		pSessionClient->SetServerPort(conf.sesn_srv_port);
	}

	LogSender * pLogClient  =lpLogicServer->GetLogClient(); 
	if (pLogClient)
	{
		pLogClient->SetServerHost(conf.log_srv_ip);
		pLogClient->SetServerPort(conf.log_srv_port);
		pLogClient->SetServerIndex(conf.srv_index);
		pLogClient->SetServerName(conf.srv_name);
		pLogClient->SetServerType(jxSrvDef::GameServer);
	}

	CLocalSender * pLocalLogClient  =lpLogicServer->GetLocalClient(); 
	if (pLocalLogClient)
	{
		pLocalLogClient->SetServerHost(conf.locallog_srv_ip);
		pLocalLogClient->SetServerPort(conf.locallog_srv_port);
		pLocalLogClient->SetServerIndex(conf.srv_index);
		pLocalLogClient->SetServerName(conf.srv_name);
		pLocalLogClient->SetServerType(jxSrvDef::GameServer);
	}

	CDataClient * pDbClient  =lpLogicServer->GetDbClient(); 
	if (pDbClient)
	{
		pDbClient->SetServerHost(conf.db_srv_ip);
		pDbClient->SetServerPort(conf.db_srv_port);
	}

	CBackStageSender * pBackClient  =lpLogicServer->GetBackStageSender();
	if (pBackClient)
	{
		pBackClient->SetServerHost(conf.back_srv_ip);
		//pBackClient->SetServerHost("127.0.0.1");
		pBackClient->SetServerPort(conf.back_srv_port);
		pBackClient->SetServerIndex(conf.srv_index);
		pBackClient->SetServerName(conf.srv_name);
		pBackClient->SetServerType(jxSrvDef::GameServer);
	}

	world_client* pworld_client = lpLogicServer->NewWorldClient();
	if (pworld_client)
	{
		pworld_client->SetServerHost(conf.world_srv_ip); 
		pworld_client->SetServerPort(conf.world_srv_port);
	}

	return true;
	/*
	if ( !openGlobalTable("LogicServer") ) return false;

	LPCSTR sVal;
	INT nVal;
	char sServerName[64];

	//服务器名字和index
	getFieldStringBuffer("ServerName", sServerName, ArrayCount(sServerName)); //name
	//MoveMemory(sServerName, conf.srv_name, sizeof(conf.srv_name));

	SYSTEMTIME startSysTime; //服务器的开启时间
	CMiniDateTime nStart;

	BOOL IsValid =TRUE;
	LPCTSTR sName = getFieldString("ServerOpenTime", "",&IsValid);
	//LPCTSTR sName = conf.zone_open_time;
	if( IsValid &&  sName != NULL && strlen(sName) >10) //是个基本合法的名字
	{
		sscanf(sName, "%d-%d-%d %d:%d:%d", &startSysTime.wYear, &startSysTime.wMonth, &startSysTime.wDay, &startSysTime.wHour, &startSysTime.wMinute, &startSysTime.wSecond);
	}
	else
	{
		GetLocalTime(&startSysTime);
	}
	nStart.encode(startSysTime);


	SYSTEMTIME combineTime; //服务器的合服时间
	CMiniDateTime nCombine;

	BOOL boValid =TRUE;
	LPCTSTR sTime = getFieldString("ServerCombineTime", "",&boValid);
	if( boValid &&  sTime != NULL && strlen(sTime) >10) //是个基本合法的名字
	{
		sscanf(sTime, "%d-%d-%d %d:%d:%d", &combineTime.wYear, &combineTime.wMonth, &combineTime.wDay, &combineTime.wHour, &combineTime.wMinute, &combineTime.wSecond);
		nCombine.encode(combineTime);
	}
	else
	{
		nCombine = 0;
	}
	
	nVal = getFieldInt("ServerIndex"); //serverindex
	INT nServerIndex = nVal;

	char strFilePath[64] = "./data/runtime";
	//sprintf_s(strFilePath,sizeof(strFilePath),"./data/runtime",nServerIndex);
	if (lpLogicServer)
	{
		lpLogicServer->SetServerIndex(nServerIndex);
		lpLogicServer->SetServerName(sServerName);
		lpLogicServer->SetServerOpenTime(nStart); //设置开区时间
		lpLogicServer->SetServerCombineTime(nCombine);	// 设置合区时间
		lpLogicServer->SetStrFilePath(strFilePath);		
	}

	BOOL fiValid = TRUE;
	LPCTSTR sSPID = getFieldString("SPID", "", &fiValid);
	if (fiValid)
	{
		if (sSPID == NULL)
			fiValid = FALSE;
		else
		{
			int slen = (int)strlen(sSPID);
			if (slen <= 0 || slen > 10)
				fiValid = FALSE;
		}
	}
	if (fiValid)
	{
		lpLogicServer->SetSpidString(sSPID);
	}
	else
	{
		lpLogicServer->SetSpidString("vst");
		OutputMsg(rmError, "config error, SPID is invalid");
		//return false;
	}
	
	nVal = 0;
	INT_PTR nSpid = getFieldInt("spguid",&nVal); //读取spid
	lpLogicServer->SetSpid(nSpid); //设置spid

	nVal = CLogicServer::ChuanQiZhiRen; // 默认游戏索引
	lpLogicServer->SetGameIndex(getFieldInt("GameIndex", &nVal)); //设置游戏索引
	nVal = 0;

	
	//bool bStartCommonServer = true;	
	//bStartCommonServer = getFieldBoolean("IsStartCommonServer", &bStartCommonServer);	

	//nVal = 0;
	//nVal = getFieldInt("CommonServerId", &nVal);
	//if (lpLogicServer)lpLogicServer->SetCommonServerId(nVal);
	//if ((!isCommonServer) && (nVal == 0 || bStartCommonServer == false))lpLogicServer->SetStartCommonServer(false);

	

	
	//网关服务配置
	if ( openFieldTable("GateService") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		if (NULL != sVal)
		{
			CLogicGateManager *pLogicManager = lpLogicServer->GetGateManager();
			if (pLogicManager)
			{
				pLogicManager->SetServiceHost(sVal);
				pLogicManager->SetServicePort(nVal);
			}
		}
		closeTable();
	}

	//会话服务配置
	if ( openFieldTable("SessionServer") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		
		CLogicSSClient * pSessionClient = lpLogicServer->GetSessionClient();
		if (pSessionClient)
		{
			pSessionClient->SetServerHost(sVal);
			pSessionClient->SetServerPort(nVal);
		}
		closeTable();
	}
	
	//日志服务器地址配置
	if ( openFieldTable("LogServer") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		
		LogSender * pLogClient  =lpLogicServer->GetLogClient(); 
		if (pLogClient)
		{
			pLogClient->SetServerHost(sVal);
			pLogClient->SetServerPort(nVal);
			pLogClient->SetServerIndex(nServerIndex);
			pLogClient->SetServerName(sServerName);
			pLogClient->SetServerType(jxSrvDef::GameServer);
		}
		closeTable();
	}
	

	
	//公共日志服务器地址配置
	if ( openFieldTable("LocalLogServer") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");

		CLocalSender * pLocalLogClient  =lpLogicServer->GetLocalClient(); 
		if (pLocalLogClient)
		{
			pLocalLogClient->SetServerHost(sVal);
			pLocalLogClient->SetServerPort(nVal);
			pLocalLogClient->SetServerIndex(nServerIndex);
			pLocalLogClient->SetServerName(sServerName);
			pLocalLogClient->SetServerType(jxSrvDef::GameServer);
		}
		closeTable();
	}

	//数据client配置
	if (openFieldTable("DbServer"))
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		CDataClient * pDbClient  =lpLogicServer->GetDbClient(); 
		if (pDbClient)
		{
			pDbClient->SetServerHost(sVal);
			pDbClient->SetServerPort(nVal);
		}
		closeTable();
	}
	//好友服务器配置
	//if (openFieldTable("FriendServer"))
	//{
	//	CFriendClient *pClient = lpLogicServer->GetFriendClient();
	//	//内部服务器地址配置
	//	if (openFieldTable("Server"))
	//	{
	//		sVal = getFieldString("Host");
	//		nVal = getFieldInt("Port");
	//		pClient->SetServerHost(sVal);
	//		pClient->SetServerPort(nVal);
	//		closeTable();
	//	}
	//	//用户连接的好友服务器网关地址配置
	//	if (openFieldTable("Gate"))
	//	{
	//		sVal = getFieldString("Host");
	//		nVal = getFieldInt("Port");
	//		pClient->SetFriendGateHost(sVal);
	//		pClient->SetFriendGatePort(nVal);
	//		closeTable();
	//	}
	//	closeTable();
	//}
	if (openFieldTable("MgrServer"))
	{
		sVal = getFieldString("Host");
		nVal = getFieldInt("Port");
		//CMgrServClient *pClient = lpLogicServer->GetMgrClient();
		//pClient->SetServerHost(sVal);
		//pClient->SetServerPort(nVal);

		CBackStageSender * pBackClient  =lpLogicServer->GetBackStageSender();
		if (pBackClient)
		{
			pBackClient->SetServerHost(sVal);
			//pBackClient->SetServerHost("127.0.0.1");
			pBackClient->SetServerPort(nVal);
			pBackClient->SetServerIndex(nServerIndex);
			pBackClient->SetServerName(sServerName);
			pBackClient->SetServerType(jxSrvDef::GameServer);
		}
		closeTable();
	}

	//世界服务器
	if (feildTableExists("WorldServer") && openFieldTable("WorldServer"))
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");

		closeTable();

		// 跳过空白符
		while (*sVal == ' ' || *sVal == '\t')
			++sVal;
		
		if (*sVal != 0)
		{
			world_client* pworld_client = lpLogicServer->NewWorldClient();
			if (pworld_client)
			{
				pworld_client->SetServerHost(sVal);
				pworld_client->SetServerPort(nVal);
			}
			else
			{
				OutputMsg(rmError, _T("error,NewWorldClient() == NULL"));
			}		
		}		
	}

	//if (feildTableExists("CommonServer") && openFieldTable("CommonServer"))
	//{
	//	char szServerIP[32];
	//	WORD wPort;
	//	LPCSTR szField[2] = {"Server", "Client"};
	//	int nIndex = isCommonServer ? 0 : 1;
	//	if (feildTableExists(szField[nIndex]) && openFieldTable(szField[nIndex]))
	//	{
	//		getFieldStringBuffer("Address", szServerIP, ArrayCount(szServerIP));
	//		wPort = (WORD)getFieldInt("Port");
	//		GetLogicServer()->SetCommonServerAddr(szServerIP, wPort);
	//		closeTable();
	//	}
	//	closeTable();
	//}

	closeTable();
	*/
	return true;
}

/*
bool CLogicServerConfig::readServerConfig(CLogicServer *lpLogicServer)
{
	conf_logicsrv conf;
	read_config_new(&conf);
	
	if ( !openGlobalTable("LogicServer") ) return false;

	LPCSTR sVal;
	INT nVal;
	char sServerName[64];

	//服务器名字和index
	getFieldStringBuffer("ServerName", sServerName, ArrayCount(sServerName)); //name
	//MoveMemory(sServerName, conf.srv_name, sizeof(conf.srv_name));

	SYSTEMTIME startSysTime; //服务器的开启时间
	CMiniDateTime nStart;

	BOOL IsValid =TRUE;
	LPCTSTR sName = getFieldString("ServerOpenTime", "",&IsValid);
	//LPCTSTR sName = conf.zone_open_time;
	if( IsValid &&  sName != NULL && strlen(sName) >10) //是个基本合法的名字
	{
		sscanf(sName, "%d-%d-%d %d:%d:%d", &startSysTime.wYear, &startSysTime.wMonth, &startSysTime.wDay, &startSysTime.wHour, &startSysTime.wMinute, &startSysTime.wSecond);
	}
	else
	{
		GetLocalTime(&startSysTime);
	}
	nStart.encode(startSysTime);


	SYSTEMTIME combineTime; //服务器的合服时间
	CMiniDateTime nCombine;

	BOOL boValid =TRUE;
	LPCTSTR sTime = getFieldString("ServerCombineTime", "",&boValid);
	if( boValid &&  sTime != NULL && strlen(sTime) >10) //是个基本合法的名字
	{
		sscanf(sTime, "%d-%d-%d %d:%d:%d", &combineTime.wYear, &combineTime.wMonth, &combineTime.wDay, &combineTime.wHour, &combineTime.wMinute, &combineTime.wSecond);
		nCombine.encode(combineTime);
	}
	else
	{
		nCombine = 0;
	}
	
	nVal = getFieldInt("ServerIndex"); //serverindex
	INT nServerIndex = nVal;

	char strFilePath[64] = "./data/runtime";
	//sprintf_s(strFilePath,sizeof(strFilePath),"./data/runtime",nServerIndex);
	if (lpLogicServer)
	{
		lpLogicServer->SetServerIndex(nServerIndex);
		lpLogicServer->SetServerName(sServerName);
		lpLogicServer->SetServerOpenTime(nStart); //设置开区时间
		lpLogicServer->SetServerCombineTime(nCombine);	// 设置合区时间
		lpLogicServer->SetStrFilePath(strFilePath);		
	}

	BOOL fiValid = TRUE;
	LPCTSTR sSPID = getFieldString("SPID", "", &fiValid);
	if (fiValid)
	{
		if (sSPID == NULL)
			fiValid = FALSE;
		else
		{
			int slen = (int)strlen(sSPID);
			if (slen <= 0 || slen > 10)
				fiValid = FALSE;
		}
	}
	if (fiValid)
	{
		lpLogicServer->SetSpidString(sSPID);
	}
	else
	{
		lpLogicServer->SetSpidString("vst");
		OutputMsg(rmError, "config error, SPID is invalid");
		//return false;
	}
	
	nVal = 0;
	INT_PTR nSpid = getFieldInt("spguid",&nVal); //读取spid
	lpLogicServer->SetSpid(nSpid); //设置spid

	nVal = CLogicServer::ChuanQiZhiRen; // 默认游戏索引
	lpLogicServer->SetGameIndex(getFieldInt("GameIndex", &nVal)); //设置游戏索引
	nVal = 0;

	
	//bool bStartCommonServer = true;	
	//bStartCommonServer = getFieldBoolean("IsStartCommonServer", &bStartCommonServer);	

	//nVal = 0;
	//nVal = getFieldInt("CommonServerId", &nVal);
	//if (lpLogicServer)lpLogicServer->SetCommonServerId(nVal);
	//if ((!isCommonServer) && (nVal == 0 || bStartCommonServer == false))lpLogicServer->SetStartCommonServer(false);

	

	
	//网关服务配置
	if ( openFieldTable("GateService") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		if (NULL != sVal)
		{
			CLogicGateManager *pLogicManager = lpLogicServer->GetGateManager();
			if (pLogicManager)
			{
				pLogicManager->SetServiceHost(sVal);
				pLogicManager->SetServicePort(nVal);
			}
		}
		closeTable();
	}

	//会话服务配置
	if ( openFieldTable("SessionServer") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		
		CLogicSSClient * pSessionClient = lpLogicServer->GetSessionClient();
		if (pSessionClient)
		{
			pSessionClient->SetServerHost(sVal);
			pSessionClient->SetServerPort(nVal);
		}
		closeTable();
	}
	
	//日志服务器地址配置
	if ( openFieldTable("LogServer") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		
		LogSender * pLogClient  =lpLogicServer->GetLogClient(); 
		if (pLogClient)
		{
			pLogClient->SetServerHost(sVal);
			pLogClient->SetServerPort(nVal);
			pLogClient->SetServerIndex(nServerIndex);
			pLogClient->SetServerName(sServerName);
			pLogClient->SetServerType(jxSrvDef::GameServer);
		}
		closeTable();
	}
	

	
	//公共日志服务器地址配置
	if ( openFieldTable("LocalLogServer") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");

		CLocalSender * pLocalLogClient  =lpLogicServer->GetLocalClient(); 
		if (pLocalLogClient)
		{
			pLocalLogClient->SetServerHost(sVal);
			pLocalLogClient->SetServerPort(nVal);
			pLocalLogClient->SetServerIndex(nServerIndex);
			pLocalLogClient->SetServerName(sServerName);
			pLocalLogClient->SetServerType(jxSrvDef::GameServer);
		}
		closeTable();
	}

	//数据client配置
	if (openFieldTable("DbServer"))
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		CDataClient * pDbClient  =lpLogicServer->GetDbClient(); 
		if (pDbClient)
		{
			pDbClient->SetServerHost(sVal);
			pDbClient->SetServerPort(nVal);
		}
		closeTable();
	}
	//好友服务器配置
	//if (openFieldTable("FriendServer"))
	//{
	//	CFriendClient *pClient = lpLogicServer->GetFriendClient();
	//	//内部服务器地址配置
	//	if (openFieldTable("Server"))
	//	{
	//		sVal = getFieldString("Host");
	//		nVal = getFieldInt("Port");
	//		pClient->SetServerHost(sVal);
	//		pClient->SetServerPort(nVal);
	//		closeTable();
	//	}
	//	//用户连接的好友服务器网关地址配置
	//	if (openFieldTable("Gate"))
	//	{
	//		sVal = getFieldString("Host");
	//		nVal = getFieldInt("Port");
	//		pClient->SetFriendGateHost(sVal);
	//		pClient->SetFriendGatePort(nVal);
	//		closeTable();
	//	}
	//	closeTable();
	//}
	if (openFieldTable("MgrServer"))
	{
		sVal = getFieldString("Host");
		nVal = getFieldInt("Port");
		//CMgrServClient *pClient = lpLogicServer->GetMgrClient();
		//pClient->SetServerHost(sVal);
		//pClient->SetServerPort(nVal);

		CBackStageSender * pBackClient  =lpLogicServer->GetBackStageSender();
		if (pBackClient)
		{
			pBackClient->SetServerHost(sVal);
			//pBackClient->SetServerHost("127.0.0.1");
			pBackClient->SetServerPort(nVal);
			pBackClient->SetServerIndex(nServerIndex);
			pBackClient->SetServerName(sServerName);
			pBackClient->SetServerType(jxSrvDef::GameServer);
		}
		closeTable();
	}

	//世界服务器
	if (feildTableExists("WorldServer") && openFieldTable("WorldServer"))
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");

		closeTable();

		// 跳过空白符
		while (*sVal == ' ' || *sVal == '\t')
			++sVal;
		
		if (*sVal != 0)
		{
			world_client* pworld_client = lpLogicServer->NewWorldClient();
			if (pworld_client)
			{
				pworld_client->SetServerHost(sVal);
				pworld_client->SetServerPort(nVal);
			}
			else
			{
				OutputMsg(rmError, _T("error,NewWorldClient() == NULL"));
			}		
		}		
	}

	//if (feildTableExists("CommonServer") && openFieldTable("CommonServer"))
	//{
	//	char szServerIP[32];
	//	WORD wPort;
	//	LPCSTR szField[2] = {"Server", "Client"};
	//	int nIndex = isCommonServer ? 0 : 1;
	//	if (feildTableExists(szField[nIndex]) && openFieldTable(szField[nIndex]))
	//	{
	//		getFieldStringBuffer("Address", szServerIP, ArrayCount(szServerIP));
	//		wPort = (WORD)getFieldInt("Port");
	//		GetLogicServer()->SetCommonServerAddr(szServerIP, wPort);
	//		closeTable();
	//	}
	//	closeTable();
	//}

	closeTable();
	
	return true;
}
*/

//读取跨服配置表
bool CLogicServerConfig::LoadCrossServerConfig(CLogicServer *lpLogicServer)
{
	int i;
	int len = 0;
	conf_logicxsrv *clxs = 0;
	read_xconfig_new(&clxs, &len);

	int nCurrentServerIndex = GetLogicServer()->GetServerIndex();

	for( i = 0;i< len;++i)
	{
		if(nCurrentServerIndex == clxs[i].id){
			GetLogicServer()->SetCommonServer(true);
			GetLogicServer()->SetCommonServerId(clxs[i].id);
			GetLogicServer()->SetCommonServerAddr(clxs[i].login_ip,clxs[i].host, clxs[i].port,clxs[i].login_port,clxs[i].login_port);
			break;
		}
	}
	if(i == len){
		GetLogicServer()->SetCommonServerId(clxs[len-1].id);
		GetLogicServer()->SetCommonServerAddr(clxs[len-1].login_ip,clxs[len-1].host, clxs[len-1].port,clxs[len-1].login_port,clxs[len-1].login_port);
	}

	return true;

	/*
	if(!lpLogicServer) return false;

	bool result = false;
	CMemoryStream ms;
	try
	{
		if ( ms.loadFromFile("crossserver.txt") <= 0 )
		{
			showError(_T("unable to load config from file crossserver.txt"));
			return result;
		}
		if ( !setScript((LPCSTR)ms.getMemory()) )
		{
			showError(_T("parse config script failed"));
			return false;
		}
		if ( openGlobalTable("crossserver"))
		{
			 
			int nCurrentServerIndex = GetLogicServer()->GetServerIndex();
			//int nCrossServerIndex = 0;

			//if ( openFieldTable("map"))
			//{		
			//	size_t nCount = lua_objlen(m_pLua,-1);

			//	if ( enumTableFirst() )
			//	{

			//		INT_PTR nIdx = 0;
			//		do 
			//		{	
			//			int nStartId = getFieldInt("startid");
			//			int nEndId =getFieldInt("endid");
			//			int nCrossId = getFieldInt("crossid");

			//			if (nCurrentServerIndex >= nStartId && nCurrentServerIndex <= nEndId)
			//			{
			//				nCrossServerIndex = nCrossId;
			//				endTableEnum();
			//				break;
			//			}

			//			nIdx++;
			//		}while (enumTableNext());
			//	}

			//	closeTable();
			//}

			if ( openFieldTable("center"))
			{		
				size_t nCount = lua_objlen(m_pLua,-1);

				if ( enumTableFirst() )
				{

					INT_PTR nIdx = 0;
					do 
					{	
						int nServerId = getFieldInt("id");
						char szClientServerIP[64]; //客户端连接的ip，给客户端用的
						char szCommonServerIp[64]; //服务器连接的ip	
						getFieldStringBuffer("loginIP", szClientServerIP, ArrayCount(szClientServerIP));      //连接的公共服的域名
						getFieldStringBuffer("host", szCommonServerIp, ArrayCount(szCommonServerIp));   //连接公共数据服的域名
						int nPort =getFieldInt("port");
						int nMinPort = getFieldInt("loginPort");
						int nMaxPort = getFieldInt("loginPort");
						
						if(nServerId == nCurrentServerIndex)
						{	
							GetLogicServer()->SetCommonServer(true);
							GetLogicServer()->SetCommonServerId(nServerId);
							GetLogicServer()->SetCommonServerAddr(szClientServerIP,szCommonServerIp, nPort,nMinPort,nMaxPort);
							endTableEnum();
							break;
						}
						else if (nIdx == nCount - 1 && nPort > 0 && nCurrentServerIndex >= 0 && nCurrentServerIndex < CNewCrossMgr::SERVER_INDEX_MAX)
						{
							GetLogicServer()->SetCommonServerId(nServerId);
							GetLogicServer()->SetCommonServerAddr(szClientServerIP,szCommonServerIp, nPort,nMinPort,nMaxPort);
						}

						nIdx++;
					}while (enumTableNext());
				}

				closeTable();
			}

			closeTable();
		}
	}
	catch(RefString &s)
	{
		OutputMsg( rmNormal, s.rawStr() );
	}
	catch(...)
	{
		OutputMsg( rmNormal, _T("unexpected error on load config") );
	}
	return true;
	*/
}