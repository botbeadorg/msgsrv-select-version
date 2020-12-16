#include "StdAfx.h"
#include <ws2tcpip.h>


#pragma pack(1)
struct conf_dbsrv {
	bool utf8;
	int srv_index;
    int gate_srv_port;
	int db_srv_port;
	int log_srv_port;
	int sesn_srv_port;
	int name_srv_port;
	int mysql_port;
	char srv_name[33];
    char gate_srv_ip[33];
	char db_srv_ip[33];
	char log_srv_ip[33];
	char sesn_srv_ip[33];
	char name_srv_ip[33];
	char mysql_host[33];
	char mysql_usr[33];
	char mysql_db[33];
	char mysql_pwd[65];
	char boy_name_file[129];
	char girl_name_file[129];
    char esqltool_path[129];
};
#pragma pack()

CDBConfig::CDBConfig(void)
{
	ServerIndex = 1;
}


CDBConfig::~CDBConfig(void)
{
}


//读取跨服配置表
bool CDBConfig::LoadCrossServerConfig()
{
	/*
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
			int nCenterServerIndex = 0;
			if ( openFieldTable("map"))
			{	
				size_t nCount = lua_objlen(m_pLua,-1);

				if ( enumTableFirst() )
				{
					do 
					{
						int nStart = getFieldInt("start");
						int nEnd = getFieldInt("idend");
						int nCenterid = getFieldInt("centerid");

						if(ServerIndex >= nStart && ServerIndex <= nEnd && ServerIndex != nCenterid)
						{
							nCenterServerIndex = nCenterid;
							//break;
						}
					}while (enumTableNext());
				}

				closeTable();
			}

			if(nCenterServerIndex > 0)
			{
				if ( openFieldTable("center"))
				{		
					size_t nCount = lua_objlen(m_pLua,-1);

					if ( enumTableFirst() )
					{
						ZeroMemory(&DBCenterAddr, sizeof(DBCenterAddr));

						INT_PTR nIdx = 0;
						do 
						{		
							// 读取DBCenter服务器配置
							int nServerId = getFieldInt("id");
							if(nServerId == nCenterServerIndex)
							{
								getFieldStringBuffer("commondbsrv", DBCenterAddr.szAddr, sizeof(DBCenterAddr.szAddr));
								DBCenterAddr.nPort = getFieldInt("dbport");
							//	break;
							}

							nIdx++;
						}while (enumTableNext());
					}
					closeTable();//GateService
				}
			}
			
			closeTable();//关闭DBServer
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
	*/
	//数据服不再读取配置了
	return true;
}

void read_config_new(conf_dbsrv *cbs)
{
	int r, len;
	unsigned short version;
	addrinfo hints, *res;
	char *port_to_listen = "65000";
	const char *cmdhead = "MOC.DAEBTOB.XYHH";
	const char *cmd = "db";
	char buf[2048];
	char *buf_t = buf;
	SecureZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (0 != getaddrinfo("localhost", port_to_listen, &hints, &res)) {
		r = WSAGetLastError();
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

	SecureZeroMemory(cbs, sizeof(*cbs));
	r = recv(c, (char *)cbs, sizeof(*cbs), 0);
	if (r > 0) {
		printf("bytes received: %d\n", r);
	}
	else if (r == 0)
		printf("Connection closed\n");
	else
		printf("recv failed: %d\n", WSAGetLastError());
	closesocket(c);
}

bool CDBConfig::ReadConfig()
{
	conf_dbsrv cdbs;
	try{
		read_config_new(&cdbs);
		MoveMemory(ServerName, cdbs.srv_name, sizeof(ServerName));
		ServerIndex = cdbs.srv_index;
		MoveMemory(m_sBoyFileName, cdbs.boy_name_file, sizeof(m_sBoyFileName));
		MoveMemory(m_sGirlFileName, cdbs.girl_name_file, sizeof(m_sGirlFileName));
		MoveMemory(m_EsqlToolPath, cdbs.esqltool_path, sizeof(m_EsqlToolPath));
		MoveMemory(GateAddr.szAddr, cdbs.gate_srv_ip, sizeof(GateAddr.szAddr));
		GateAddr.nPort = cdbs.gate_srv_port;
		MoveMemory(DataAddr.szAddr, cdbs.db_srv_ip, sizeof(DataAddr.szAddr));
		DataAddr.nPort = cdbs.db_srv_port;
		MoveMemory(LogAddr.szAddr, cdbs.log_srv_ip, sizeof(LogAddr.szAddr));
		LogAddr.nPort = cdbs.log_srv_port;
		MoveMemory(NameAddr.szAddr, cdbs.name_srv_ip, sizeof(NameAddr.szAddr));
		NameAddr.nPort = cdbs.name_srv_port;
		MoveMemory(DbConf.szHost, cdbs.mysql_host, sizeof(DbConf.szHost));
		DbConf.nPort = cdbs.mysql_port;
		MoveMemory(DbConf.szDbName, cdbs.mysql_db, sizeof(DbConf.szDbName));
		MoveMemory(DbConf.szUser, cdbs.mysql_usr, sizeof(DbConf.szUser));
		MoveMemory(DbConf.szPassWord, cdbs.mysql_pwd, sizeof(DbConf.szPassWord));
		
	}
	catch(...){
		OutputMsg( rmNormal, _T("unexpected error on load config") );
	}
	return true;

	/*
	bool result = false;
	static LPCTSTR szConfigFileName = _T("DBServer.txt");
	

	wylib::stream::CMemoryStream ms;
	//bool result = false;
	//CMemoryStream ms;
	try
	{
		//if ( ms.loadFromFile("DBServer.txt") <= 0 )
		if ( ms.loadFromFile(szConfigFileName) <= 0 )
		{
			showError(_T("unable to load config from file DBServer.txt"));
			return result;
		}
		if ( !setScript((LPCSTR)ms.getMemory()) )
		{
			showError(_T("parse config script failed"));
			return false;
		}
		if ( openGlobalTable("DBServer"))
		{
			getFieldStringBuffer(("ServerName"), ServerName,sizeof(ServerName));	
			ServerIndex = getFieldInt("ServerIndex");	

			getFieldStringBuffer(("BoyNameFile"), m_sBoyFileName,sizeof(m_sBoyFileName));	
			getFieldStringBuffer(("GirlNameFile"), m_sGirlFileName,sizeof(m_sGirlFileName));	

			LPCTSTR stemp = getFieldString("EsqlToolPath","D:\\cqAdmin\\ESQL\\ESQLTool.exe");

			_asncpytA(m_EsqlToolPath,stemp);
			m_EsqlToolPath[sizeof(m_EsqlToolPath)-1] = 0;

			if ( openFieldTable("GateService"))
			{				
				getFieldStringBuffer(("Address"), GateAddr.szAddr,sizeof(GateAddr.szAddr));	
				GateAddr.nPort = getFieldInt("Port");	
				closeTable();//GateService
			}
				
			if ( openFieldTable("DBService"))
			{				
				getFieldStringBuffer(("Address"), DataAddr.szAddr,sizeof(DataAddr.szAddr));	
				DataAddr.nPort = getFieldInt("Port");	
				closeTable();//DBService 
			}
			if ( openFieldTable("LogServer"))
			{				
				getFieldStringBuffer(("Address"), LogAddr.szAddr,sizeof(LogAddr.szAddr));	
				LogAddr.nPort = getFieldInt("Port");	
				closeTable();//LogServer 
			}
						
			//if ( openFieldTable("SessionServer"))
			//{				
				//getFieldStringBuffer(("Address"), SessionAddr.szAddr,sizeof(SessionAddr.szAddr));	
				//SessionAddr.nPort = getFieldInt("Port");	
				//closeTable();//SessionServer  
			//}
			
			if ( openFieldTable("NameServer"))
			{				
				getFieldStringBuffer(("Address"), NameAddr.szAddr,sizeof(NameAddr.szAddr));	
				NameAddr.nPort = getFieldInt("Port");	
				closeTable();//NameServer  
			}

			// 读取DBCenter服务器配置
			
			//去掉了dbsencter的读取
			//ZeroMemory(&DBCenterAddr, sizeof(DBCenterAddr));
			//if (feildTableExists("DBCenter") && openFieldTable("DBCenter"))
			//{
				//getFieldStringBuffer("Address", DBCenterAddr.szAddr, sizeof(DBCenterAddr.szAddr));
				//DBCenterAddr.nPort = getFieldInt("Port");
				//closeTable();
			//}
			

			if ( openFieldTable("SQL"))
			{				
				getFieldStringBuffer(("Host"), DbConf.szHost,sizeof(DbConf.szHost));	
				DbConf.nPort = getFieldInt("Port");	
				getFieldStringBuffer(("DBName"), DbConf.szDbName,sizeof(DbConf.szDbName));
				getFieldStringBuffer(("DBUser"), DbConf.szUser,sizeof(DbConf.szUser));
				char szTemp[100] = {0};
				getFieldStringBuffer(("DBPass"), szTemp,sizeof(szTemp));
			//	getFieldStringBuffer(("Key"), DbConf.szKey,sizeof(DbConf.szKey));	
				//密码要解密
				DecryptPassword(DbConf.szPassWord,sizeof(DbConf.szPassWord),szTemp,"123456abc123456a");
				closeTable();//DB
				result = true;
			}
			closeTable();//关闭DBServer
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

	if(result)
	{
		result = LoadCrossServerConfig();
	}
	return result;
	*/
}

void CDBConfig::ShowError(const LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString sErr;
	sErr = _T("[Configuration Error]");
	sErr += sError;
	//集中处理错误，为了简单起见，此处直接抛出异常。异常会在readConfig中被捕获从而立刻跳出对配置的循环读取。
	throw sErr;
}