#include <vcl.h>
#include <windows.h>
#pragma hdrstop

#include <tchar.h>
#include <stdio.h>

#include <WinBase.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include "compon.h"

#include <string>
#include <sstream>
#include <iostream>

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

struct conf_namesrv {
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

struct conf_logicsrv {
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

#define BUFLEN 1024
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ 2
#define CONFCMDARYLEN

#define CLOSESOCKET 0
#define RETRY 1
#define UNKNOWNCOMMAND 2
#define SUCCESS 3
#define NOCONF 4
#define BACKSRV_SEC_NUM 4

int len_cmdhead;
const AnsiString cmdhead = "MOC.DAEBTOB.XYHH";
char *port_to_listen = "65000";
/*
 CMDHEADlen(unsigned int)cmd0
 0 after CMDHEAD is char '\0'.
 */

// ---------------------------------------------------------------------------
const char * inet_ntop4(const unsigned char *, char *, socklen_t);
const char * inet_ntop6(const unsigned char *, char *, socklen_t);
char * inet_ntop_why(int, const void *, char *, socklen_t);
void print_dbg_msg(const char * const , const int);
void *get_in_addr(sockaddr *);
int handle_cmd(int, char *, int);

const char *retry = "retry";

enum confcmd_ord {
	session, name, logic, logger, locallog, gate, db, back, am, logic_x, other
};

const AnsiString confcmd[] = {"session", "name", "logic", "logger", "locallog", "gate", "db", "back", "am", "logic_x"};

// enum backsrv_conf_ord {
// bksrv_common, bksrv_db, bksrv_srv, bksrv_backsrv, bksrv_conf_end
// };

// const AnsiString backsrv_conf[BACKSRV_SEC_NUM] = {"common", "db", "srv", "backsrv"};

#pragma argsused

int _tmain(int argc, _TCHAR* argv[]) {
	unsigned short version;
	int fdmax;
	int listener;
	int newfd;
	int i, rv;
	int yes;
	int nbytes;
	socklen_t addrlen;
	struct addrinfo hints, *ai, *p;
	WSADATA wsadata;

	fd_set master;
	fd_set read_fds;
	struct sockaddr_storage remoteaddr;
	std::stringstream ss;
	std::string dbg_str;
	AnsiString info;

	char remoteIP[INET6_ADDRSTRLEN];
	char buf[BUFLEN];

	// #ifdef _DEBUG
	// int q;
	// conf_backsrv x;
	// AnsiString ppppp;
	// TMemIniFile *ini = new TMemIniFile(L"BackServer.ini");
	// TStringList *s = new TStringList;
	// for (q = bksrv_common; q < bksrv_conf_end; ++q) {
	// switch (q) {
	// case bksrv_common:
	// if (ini->SectionExists(backsrv_conf[bksrv_common])) {
	// x.spguid = ini->ReadInteger(backsrv_conf[bksrv_common], "spguid", 0);
	// std::cout << x.spguid << std::endl;
	// }
	// break;
	// case bksrv_db:
	// if(ini->SectionExists(backsrv_conf[bksrv_db])){
	// ppppp=ini->ReadString(backsrv_conf[bksrv_db], "host", "localhost");
	// //MoveMemory();
	// }
	// break;
	// default:
	// break;
	// }
	// }
	//
	// /*
	// s->Clear();
	// // ini->ReadSectionValues(backsrv_conf[i], s);
	// ini->ReadSection(backsrv_conf[i], s);
	// switch (i) {
	// case bksrv_common:
	// for (q = 0; q < s->Count; ++q) {
	// std::cout << AnsiString(s->operator[](q)).c_str() << std::endl;
	// }
	//
	// break;
	// default:
	// break;
	// }
	// */
	//
	// delete s;
	// delete ini;
	// #endif

	len_cmdhead = cmdhead.Length();
	exe_path = ExtractFilePath(AnsiString(argv[0]));
	DataModule1 = new TDataModule1(0);

	version = MAKEWORD(2, 2);
	WSAStartup(version, &wsadata);

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	SecureZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(0, port_to_listen, &hints, &ai)) != 0) {
		print_dbg_msg("getaddrinfo failed: ", WSAGetLastError());
		WSACleanup();
		exit(1);
	}

	yes = 1;
	for (p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			continue;
		}

		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			closesocket(listener);
			continue;
		}

		break;
	}

	if (!p) {
		print_dbg_msg("failed to bind: ", WSAGetLastError());
		WSACleanup();
		exit(2);
	}

	freeaddrinfo(ai);

	if (SOCKET_ERROR == listen(listener, SOMAXCONN)) {
		print_dbg_msg("listen failed: ", WSAGetLastError());
		closesocket(listener);
		WSACleanup();
		exit(3);
	}

	FD_SET(listener, &master);

	fdmax = listener;

	for (; ;) {
		read_fds = master;
		if (SOCKET_ERROR == select(fdmax + 1, &read_fds, 0, 0, 0)) {
			print_dbg_msg("select failed: ", WSAGetLastError());
			goto COMPLETED;
		}

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener) {
					addrlen = sizeof remoteaddr;
					newfd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);
					if (INVALID_SOCKET == (unsigned)newfd) {
						print_dbg_msg("accept failed: ", WSAGetLastError());
					}
					else {
						FD_SET((unsigned)newfd, &master);
						if (newfd > fdmax) {
							fdmax = newfd;
						}
						ss.str("");
						dbg_str.operator = ("new connection from ");
						dbg_str.operator +=
							(inet_ntop_why(remoteaddr.ss_family, get_in_addr((sockaddr*)&remoteaddr), remoteIP,
							INET6_ADDRSTRLEN));
						dbg_str.operator += (" on socket ");
						ss << newfd;
						dbg_str.operator += (ss.str());
						OutputDebugStringA(dbg_str.c_str());
					}
				}
				else {
					SecureZeroMemory(buf, BUFLEN);
					nbytes = recv(i, buf, BUFLEN, 0);
					if (nbytes > 0) {
						print_dbg_msg("bytes recieved: ", nbytes);
						handle_cmd(i, buf, nbytes);
					}
					else {
						if (!nbytes) {
							print_dbg_msg("connection closed on socket: ", i);
						}
						else {
							info = "recv failed on socket ";
							info.operator += (IntToStr(i));
							info.operator += (" : ");
							print_dbg_msg(info.c_str(), WSAGetLastError());
						}
						closesocket(i);
						FD_CLR((unsigned)i, &master);
					}
				}
			}
		}
	}
COMPLETED:
	closesocket(listener);
	FD_CLR((unsigned)listener, &master);
	for (i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &master)) {
			closesocket(i);
		}
	}
	FD_ZERO(&master);
	WSACleanup();
	delete DataModule1;
	return 0;
}

void print_dbg_msg(const char * const msg, const int value) {
	std::string dbg_str;
	std::stringstream ss;
	ss.str("");
	dbg_str.operator = (msg);
	ss << value;
	dbg_str.operator += (ss.str());
	OutputDebugStringA(dbg_str.c_str());
}

// ---------------------------------------------------------------------------

char * inet_ntop_why(int af, const void * src, char * dst, socklen_t cnt) {
	switch (af) {

	case AF_INET:
		return (char *)(inet_ntop4((const unsigned char *)src, dst, cnt));

	case AF_INET6:
		return (char *)(inet_ntop6((const unsigned char *)src, dst, cnt));

	default:
		return (0);
	}
}

// ---------------------------------------------------------------------------

const char * inet_ntop4(const unsigned char *src, char *dst, socklen_t size) {
	char tmp[sizeof("255.255.255.255")];
	int len;

	len = sprintf(tmp, "%u.%u.%u.%u", src[0], src[1], src[2], src[3]);
	if (len < 0)
		return 0;

	if (len > size) {
		errno = ENOSPC;
		return 0;
	}

	return strcpy(dst, tmp);
}

// ---------------------------------------------------------------------------

const char * inet_ntop6(const unsigned char *src, char *dst, socklen_t size) {

	char tmp[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")], *tp;
	struct {
		int base, len;
	} best, cur;
	unsigned int words[NS_IN6ADDRSZ / NS_INT16SZ];
	int i;

	memset(words, '\0', sizeof(words));
	for (i = 0; i < NS_IN6ADDRSZ; i += 2)
		words[i / 2] = (src[i] << 8) | src[i + 1];
	best.base = -1;
	cur.base = -1;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
		if (words[i] == 0) {
			if (cur.base == -1)
				cur.base = i, cur.len = 1;
			else
				cur.len++;
		}
		else {
			if (cur.base != -1) {
				if (best.base == -1 || cur.len > best.len)
					best = cur;
				cur.base = -1;
			}
		}
	}
	if (cur.base != -1) {
		if (best.base == -1 || cur.len > best.len)
			best = cur;
	}
	if (best.base != -1 && best.len < 2)
		best.base = -1;

	tp = tmp;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {

		if (best.base != -1 && i >= best.base && i < (best.base + best.len)) {
			if (i == best.base)
				* tp++ = ':';
			continue;
		}

		if (i != 0)
			* tp++ = ':';

		if (i == 6 && best.base == 0 && (best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
			if (!inet_ntop4(src + 12, tp, sizeof tmp - (tp - tmp)))
				return (0);
			tp += strlen(tp);
			break;
		} {
			int len = sprintf(tp, "%x", words[i]);
			if (len < 0)
				return 0;
			tp += len;
		}
	}

	if (best.base != -1 && (best.base + best.len) == (NS_IN6ADDRSZ / NS_INT16SZ))
		* tp++ = ':';
	*tp++ = '\0';

	if ((socklen_t)(tp - tmp) > size) {
		errno = ENOSPC;
		return 0;
	}

	return strcpy(dst, tmp);
}

// ---------------------------------------------------------------------------

void *get_in_addr(sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((sockaddr_in*)sa)->sin_addr);
	}
	return &(((sockaddr_in6*)sa)->sin6_addr);
}

int handle_cmd(int fd, char *buf, int buf_len) {
	int i, len, r;
	// TMemIniFile *ini = 0;
	// TStringList *s;
	AnsiString tmp0("");
	AnsiString tmp1("");
	UnicodeString str;

	buf += len_cmdhead;
	len = *(int *)buf;
	if (len != (buf_len - (len_cmdhead +sizeof(int)))) {
		send(fd, retry, strlen(retry), 0);
		r = RETRY;
	}
	else {
		buf += sizeof(int);
		tmp0 = buf;
		for (i = session; i < other; ++i) {
			if (tmp0.operator == (confcmd[i]))
				break;
		}
		if (other == i) {
			tmp1 = "unknown command";
			send(fd, tmp1.c_str(), tmp1.Length(), 0);
			r = UNKNOWNCOMMAND;
		}
		else {

			DataModule1->UniQuery1->SQL->Clear();
			DataModule1->UniQuery1->SQL->Add(Sysutils::Format("select * from %s;", ARRAYOFCONST((tmp0))));
			if (DataModule1->UniConnection1->InTransaction);
			else {
				DataModule1->UniConnection1->StartTransaction();
				try {
					DataModule1->UniQuery1->Execute();
					DataModule1->UniConnection1->Commit();
				}
				catch (...) {
					DataModule1->UniConnection1->Rollback();
				}
			}
			if (DataModule1->UniQuery1->RecordCount) {
				DataModule1->UniQuery1->First();

				if (tmp0 == confcmd[back]) {
					conf_backsrv *c = new conf_backsrv;
					SecureZeroMemory(c, sizeof *c);
					c->utf8 = (bool)DataModule1->UniQuery1->FieldByName("utf8")->AsInteger;
					c->spguid = DataModule1->UniQuery1->FieldByName("spguid")->AsInteger;
					c->db_port = DataModule1->UniQuery1->FieldByName("db_port")->AsInteger;
					c->srv_port = DataModule1->UniQuery1->FieldByName("srv_port")->AsInteger;
					c->srv_httpport = DataModule1->UniQuery1->FieldByName("srv_httpport")->AsInteger;
					c->backsrv_port = DataModule1->UniQuery1->FieldByName("srv_httpport")->AsInteger;
					str = DataModule1->UniQuery1->FieldByName("db_usr")->AsString;
					MoveMemory(c->db_usr, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("db_name")->AsString;
					MoveMemory(c->db_name, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("db_pwd")->AsString;
					MoveMemory(c->db_pwd, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("db_domain")->AsString;
					MoveMemory(c->db_domain, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("srv_domain")->AsString;
					MoveMemory(c->srv_domain, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("backsrv_domain")->AsString;
					MoveMemory(c->backsrv_domain, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("srv_name")->AsString;
					MoveMemory(c->srv_name, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("backsrv_name")->AsString;
					MoveMemory(c->backsrv_name, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("backip")->AsString;
					MoveMemory(c->backip, AnsiString(str).c_str(), AnsiString(str).Length());
					send(fd, (const char *)c, sizeof *c, 0);
					delete c;
				}
				else if (tmp0 == confcmd[db]) {
					conf_dbsrv * c = new conf_dbsrv;
					SecureZeroMemory(c, sizeof *c);

					c->utf8 = (bool) DataModule1->UniQuery1->FieldByName("utf8")->AsInteger;
					c->srv_index = DataModule1->UniQuery1->FieldByName("srv_index")->AsInteger;
					c->gate_srv_port = DataModule1->UniQuery1->FieldByName("gate_srv_port")->AsInteger;
					c->db_srv_port = DataModule1->UniQuery1->FieldByName("db_srv_port")->AsInteger;
					c->log_srv_port = DataModule1->UniQuery1->FieldByName("log_srv_port")->AsInteger;
					c->sesn_srv_port = DataModule1->UniQuery1->FieldByName("sesn_srv_port")->AsInteger;
					c->name_srv_port = DataModule1->UniQuery1->FieldByName("name_srv_port")->AsInteger;
					c->mysql_port = DataModule1->UniQuery1->FieldByName("mysql_port")->AsInteger;
					str = DataModule1->UniQuery1->FieldByName("srv_name")->AsString;
					MoveMemory(c->srv_name, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("boy_name_file")->AsString;
					MoveMemory(c->boy_name_file, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("girl_name_file")->AsString;
					MoveMemory(c->girl_name_file, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("gate_srv_ip")->AsString;
					MoveMemory(c->gate_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("db_srv_ip")->AsString;
					MoveMemory(c->db_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("log_srv_ip")->AsString;
					MoveMemory(c->log_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("sesn_srv_ip")->AsString;
					MoveMemory(c->sesn_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("name_srv_ip")->AsString;
					MoveMemory(c->name_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("mysql_host")->AsString;
					MoveMemory(c->mysql_host, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("mysql_usr")->AsString;
					MoveMemory(c->mysql_usr, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("mysql_pwd")->AsString;
					MoveMemory(c->mysql_pwd, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("mysql_db")->AsString;
					MoveMemory(c->mysql_db, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("esqltool_path")->AsString;
					MoveMemory(c->esqltool_path, AnsiString(str).c_str(), AnsiString(str).Length());

					send(fd, (const char *)c, sizeof *c, 0);
					delete c;
				}
				else if (tmp0 == confcmd[name]) {
					conf_namesrv * c = new conf_namesrv;
					SecureZeroMemory(c, sizeof *c);

					c->utf8 = (bool) DataModule1->UniQuery1->FieldByName("utf8")->AsInteger;
					c->spguid = DataModule1->UniQuery1->FieldByName("spguid")->AsInteger;
					c->name_srv_port = DataModule1->UniQuery1->FieldByName("name_srv_port")->AsInteger;
					c->mysql_port = DataModule1->UniQuery1->FieldByName("mysql_port")->AsInteger;
					str = DataModule1->UniQuery1->FieldByName("srv_name")->AsString;
					MoveMemory(c->srv_name, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("name_srv_ip")->AsString;
					MoveMemory(c->name_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("mysql_host")->AsString;
					MoveMemory(c->mysql_host, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("mysql_usr")->AsString;
					MoveMemory(c->mysql_usr, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("mysql_db")->AsString;
					MoveMemory(c->mysql_db, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("mysql_pwd")->AsString;
					MoveMemory(c->mysql_pwd, AnsiString(str).c_str(), AnsiString(str).Length());

					send(fd, (const char *)c, sizeof *c, 0);
					delete c;
				}
				else if (tmp0 == confcmd[logic]) {
					conf_logicsrv * c = new conf_logicsrv;
					SecureZeroMemory(c, sizeof *c);

					c->srv_index = DataModule1->UniQuery1->FieldByName("srv_index")->AsInteger;
					c->spguid = DataModule1->UniQuery1->FieldByName("spguid")->AsInteger;
					c->gate_srv_port = DataModule1->UniQuery1->FieldByName("gate_srv_port")->AsInteger;
					c->sesn_srv_port = DataModule1->UniQuery1->FieldByName("sesn_srv_port")->AsInteger;
					c->log_srv_port = DataModule1->UniQuery1->FieldByName("log_srv_port")->AsInteger;
					c->db_srv_port = DataModule1->UniQuery1->FieldByName("db_srv_port")->AsInteger;
					c->am_srv_port = DataModule1->UniQuery1->FieldByName("am_srv_port")->AsInteger;
					c->back_srv_port = DataModule1->UniQuery1->FieldByName("back_srv_port")->AsInteger;
					c->locallog_srv_port = DataModule1->UniQuery1->FieldByName("locallog_srv_port")->AsInteger;
					c->world_srv_port = DataModule1->UniQuery1->FieldByName("world_srv_port")->AsInteger;
					str = DataModule1->UniQuery1->FieldByName("spid")->AsString;
					MoveMemory(c->spid, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("zone_open_time")->AsString;
					MoveMemory(c->zone_open_time, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("zone_merge_time")->AsString;
					MoveMemory(c->zone_merge_time, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("srv_name")->AsString;
					MoveMemory(c->srv_name, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("gate_srv_ip")->AsString;
					MoveMemory(c->gate_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("sesn_srv_ip")->AsString;
					MoveMemory(c->sesn_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("log_srv_ip")->AsString;
					MoveMemory(c->log_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("db_srv_ip")->AsString;
					MoveMemory(c->db_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("am_srv_ip")->AsString;
					MoveMemory(c->am_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("back_srv_ip")->AsString;
					MoveMemory(c->back_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("locallog_srv_ip")->AsString;
					MoveMemory(c->locallog_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());
					str = DataModule1->UniQuery1->FieldByName("world_srv_ip")->AsString;
					MoveMemory(c->world_srv_ip, AnsiString(str).c_str(), AnsiString(str).Length());

					send(fd, (const char *)c, sizeof *c, 0);
					delete c;
				}
				else if (tmp0 == confcmd[logic_x]) {
					conf_logicxsrv *c = 0;
					conf_logicxsrv *ca = new conf_logicxsrv[DataModule1->UniQuery1->RecordCount];
					i = 0;
					while (i < DataModule1->UniQuery1->RecordCount) {
						c = &(ca[i]);
						SecureZeroMemory(c, sizeof *c);

						c->id = DataModule1->UniQuery1->FieldByName("id")->AsInteger;
						c->port = DataModule1->UniQuery1->FieldByName("port")->AsInteger;
						c->login_port = DataModule1->UniQuery1->FieldByName("login_port")->AsInteger;
						str = DataModule1->UniQuery1->FieldByName("host")->AsString;
						MoveMemory(c->host, AnsiString(str).c_str(), AnsiString(str).Length());
						str = DataModule1->UniQuery1->FieldByName("login_ip")->AsString;
						MoveMemory(c->login_ip, AnsiString(str).c_str(), AnsiString(str).Length());

						++i;
						DataModule1->UniQuery1->Next();
					}

					send(fd, (const char *)ca, sizeof(conf_logicxsrv) * DataModule1->UniQuery1->RecordCount, 0);
					delete ca;
				}
			}
			r = SUCCESS;
		}
	}
	return r;
}
