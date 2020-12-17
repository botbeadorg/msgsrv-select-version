#include "StdAfx.h"
#include <ws2tcpip.h>

#pragma pack(1)
struct conf_namesrv{
	bool utf8;
	int spguid;
	int name_srv_port;
    int mysql_port;
	char srv_name[33];
    char name_srv_ip[33];
	char mysql_host[33];
	char mysql_usr[33];
	char mysql_db[33];
    char mysql_pwd[65];
};
#pragma pack()

void read_config_new(conf_namesrv *cbs)
{
	int r, len;
	unsigned short version;
	addrinfo hints, *res;
	char *port_to_listen = "65000";
	const char *cmdhead = "MOC.DAEBTOB.XYHH";
	const char *cmd = "name";
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

void CNameServerConfig::setup(CNameServer *lpNameServer)
{
	conf_namesrv conf;
	try{
		read_config_new(&conf);
		lpNameServer->SetSpId(conf.spguid);
		lpNameServer->SetServerName(conf.srv_name);
		lpNameServer->SetServiceHost(conf.name_srv_ip);
		lpNameServer->SetServicePort(conf.name_srv_port);
		lpNameServer->SetSQLConfig(conf.mysql_host, conf.mysql_port, conf.mysql_db, conf.mysql_usr, conf.mysql_pwd);
	}catch(...){
		OutputMsg( rmError, _T("unexpected error on load config") );
	}
}

void CNameServerConfig::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

bool CNameServerConfig::readConfig(CNameServer *lpNameServer)
{
	if ( !openGlobalTable("NameServer") )
	{
		OutputMsg(rmError,"Can not find table NameServer, load fail");
		return false;
	}
	LPCSTR sDefalut="";
	LPCSTR sVal;
	INT nVal,nDefault =0;

	//服务名称
	sVal = getFieldString("ServerName",sDefalut);
	

	nVal = getFieldInt("spguid",&nDefault); 
	if(nVal >= 256)
	{
		OutputMsg(rmError,"spid=%d超过了范围,必须是0-255之间的数",(int)nVal);
		return false;
	}
	else
	{
		OutputMsg(rmTip,"spid=%d",(int)nVal);
	}

	//设置spid编号
	lpNameServer->SetSpId(nVal);
	lpNameServer->SetServerName(sVal);

	OutputMsg(rmTip,"spid=%s,spguid=%d",sVal,nVal);
	//名称服务配置
	if ( openFieldTable("NameService") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		lpNameServer->SetServiceHost(sVal);
		lpNameServer->SetServicePort(nVal);
		closeTable();
	}
	else
	{
		OutputMsg(rmError,"Can not find  table NameService, load fail");
		return false;
		
	}
	//数据库配置
	if ( openFieldTable("SQL") )
	{
		LPCSTR sDBName, sDBUser, sDBPass;
		CHAR sPlantPass[128];

		sVal = getFieldString("Host");
		nVal = getFieldInt("Port");
		sDBName = getFieldString("DBName");
		sDBUser = getFieldString("DBUser");
		sDBPass = getFieldString("DBPass");

		DecryptPassword(sPlantPass, ArrayCount(sPlantPass), sDBPass, "123456abc123456a");
		lpNameServer->SetSQLConfig(sVal, nVal, sDBName, sDBUser, sPlantPass);
		closeTable();
	}
	else
	{
		OutputMsg(rmError,"Can not find  table SQL, load fail");
		return false;
		
	}
	closeTable();
	return true;
}

bool CNameServerConfig::loadConfig(CNameServer *lpNameServer)
{
	/*
	static LPCTSTR szConfigFileName = _T("nameserver.txt");
	bool result = false;

	wylib::stream::CMemoryStream ms;
	try
	{
		//加载配置文件
		if ( ms.loadFromFile(szConfigFileName) <= 0 )
		{
			OutputMsg(rmError,_T("unabled to load config from %s"), szConfigFileName);
			return false;
		}
		setScript((LPCSTR)ms.getMemory());
		//读取配置文件
		result = readConfig(lpNameServer);
	}
	catch (RefString& s)
	{
		OutputMsg( rmError, (LPCTSTR)s );
	}
	catch (...)
	{
		OutputMsg( rmError, _T("unexpected error on load config") );
	}
	return result;
	*/
	setup(lpNameServer);
	return 1;
}
