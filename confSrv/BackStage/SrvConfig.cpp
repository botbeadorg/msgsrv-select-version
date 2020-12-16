#include "stdafx.h"
#include "SrvConfig.h"
#include <ws2tcpip.h>

#pragma pack(1)
struct conf_backsrv {
	bool utf8;
	int spguid;
	unsigned db_port;
	unsigned srv_port;
	unsigned srv_httpport;
	unsigned backsrv_port;
	char db_usr[33];
	char db_name[65];
	char db_pwd[65];
	char db_domain[65];
	char srv_domain[65];
	char backsrv_domain[65];
	char srv_name[65];
	char backsrv_name[65];
	char backip[257];
};
#pragma pack()

using namespace wylib::stream;

void read_config_new(conf_backsrv *cbs)
{
	int r, len;
	unsigned short version;
	addrinfo hints, *res;
	char *port_to_listen = "65000";
	const char *cmdhead = "MOC.DAEBTOB.XYHH";
	const char *cmd = "back";
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

CSrvConfig::CSrvConfig(void)
{
	memset(&backServerConf, 0, sizeof(backServerConf));
	backIp = 0;
}


CSrvConfig::~CSrvConfig(void)
{
}

void CSrvConfig::ReadConfig()
{
	char szIP[100];
	conf_backsrv conf;
	try{
		read_config_new(&conf);
		CBackClientSocket::g_nSpid = conf.spguid;
		DbConf.bUtf8 = conf.utf8;
		MoveMemory(DbConf.szHost,conf.db_domain,strlen(conf.db_domain));
		DbConf.nPort = conf.db_port;
		MoveMemory(DbConf.szUser,conf.db_usr,strlen(conf.db_usr));
		MoveMemory(DbConf.szDbName,conf.db_name,strlen(conf.db_name));
		MoveMemory(DbConf.szPassWord,conf.db_pwd,strlen(conf.db_pwd));
		SrvConf.szAddr;
		MoveMemory(SrvConf.szAddr,conf.srv_domain,strlen(conf.srv_domain));
		SrvConf.nPort = conf.srv_port;
		SrvConf.nHttpPort = conf.srv_httpport;
		MoveMemory(SrvConf.szServiceName,conf.srv_name,strlen(conf.srv_name));
		MoveMemory(backServerConf.szAddr,conf.backsrv_domain,strlen(conf.backsrv_domain));
		backServerConf.nPort = conf.backsrv_port;
		MoveMemory(backServerConf.szServiceName,conf.backsrv_name,strlen(conf.backsrv_name));
	
		szIP[0] = 0;
		MoveMemory(szIP,conf.backip,strlen(conf.backip));
		if (szIP[0] != 0)
		{
			backIp = inet_addr(szIP);
			if (backIp != 0 && backIp != INADDR_NONE && backIp != INADDR_ANY)
			{
				backIp = ntohl(backIp);
			}
		}
	}catch(...){
		OutputMsg( rmNormal, _T("unexpected error on load config") );
	}



	/*
#ifdef WIN32
	try
#endif
	{
		
		CMemoryStream ms;
		ms.loadFromFile("backserver.txt");
		setScript((LPCSTR)ms.getMemory());

		if ( openGlobalTable("BackServer"))
		{
			int nDefault = 0;
			nSpid = getFieldInt("spguid");
			CBackClientSocket::g_nSpid = nSpid;

			if ( openFieldTable("SQL"))
			{				
				getFieldStringBuffer(("Host"), DbConf.szHost,sizeof(DbConf.szHost));	
				DbConf.nPort =  getFieldInt("Port");	
				getFieldStringBuffer(("DBName"), DbConf.szDbName,sizeof(DbConf.szDbName));
				getFieldStringBuffer(("DBUser"), DbConf.szUser,sizeof(DbConf.szUser));
				char szTemp[100];
				getFieldStringBuffer(("DBPass"), szTemp,sizeof(szTemp));
				DecryptPassword(DbConf.szPassWord,sizeof(DbConf.szPassWord),szTemp,"123456abc123456a");
				
				int nDef = 0;
				DbConf.bUtf8 = getFieldInt("utf8", &nDef) ? true : false; //数据库的编码是不是utf8

				closeTable();//DB
			}

			if ( openFieldTable(("Server")))
			{
				getFieldStringBuffer(("BindAddress"), SrvConf.szAddr,sizeof(SrvConf.szAddr));	
				SrvConf.nPort = getFieldInt("Port");			
				SrvConf.nHttpPort = getFieldInt("HttpPort", &nDefault);
				getFieldStringBuffer(("ServiceName"), SrvConf.szServiceName,sizeof(SrvConf.szServiceName));	
				closeTable();//DB
			}

			if (feildTableExists("Back_Server") && openFieldTable("Back_Server"))
			{
				getFieldStringBuffer(("BindAddress"), backServerConf.szAddr, sizeof(backServerConf.szAddr));
				backServerConf.nPort = getFieldInt("Port");
				getFieldStringBuffer(("ServiceName"), backServerConf.szServiceName, sizeof(backServerConf.szServiceName));

				// 白名单
				char szIP[100];
				szIP[0] = 0;
				getFieldStringBuffer(("BackIP"), szIP, sizeof(szIP));
				if (szIP[0] != 0)
				{
					backIp = inet_addr(szIP);
					if (backIp != 0 && backIp != INADDR_NONE && backIp != INADDR_ANY)
					{
						backIp = ntohl(backIp);
					}
				}

				closeTable();
			}

			closeTable();//关闭Config
		}
	}
#ifdef WIN32
	catch(RefString &s)
	{
		OutputMsg( rmNormal, s.rawStr() );
	}
	catch(...)
	{
		OutputMsg( rmNormal, _T("unexpected error on load config") );
	}
#endif
	*/
}

void CSrvConfig::ShowError(const LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString sErr;
	sErr = _T("[Configuration Error]");
	sErr += sError;
	//集中处理错误，为了简单起见，此处直接抛出异常。异常会在readConfig中被捕获从而立刻跳出对配置的循环读取。
	throw sErr;
}