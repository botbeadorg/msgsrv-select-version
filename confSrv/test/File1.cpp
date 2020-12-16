#include <vcl.h>
#include <windows.h>

#pragma hdrstop
#pragma argsused

#include <tchar.h>

#include <stdio.h>

#include <WinBase.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

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
#pragma pack()

#define MAXDATASIZE 100

char *port_to_listen = "65000";
const AnsiString cmdhead = "MOC.DAEBTOB.XYHH";
const AnsiString cmd = "back";

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int _tmain(int argc, _TCHAR* argv[]) {
	int r, len;
	unsigned short version;
	struct addrinfo hints, *res;
	conf_backsrv cbs;
	WSADATA wsadata;

	char buf[2048];
	char *buf_t = buf;
	version = MAKEWORD(2, 2);
	WSAStartup(version, &wsadata);

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
	MoveMemory(buf, cmdhead.c_str(), cmdhead.Length());
	buf_t = buf + cmdhead.Length();
	*(int *)buf_t = cmd.Length() + 1;
	buf_t += sizeof(int);
	MoveMemory(buf_t, cmd.c_str(), cmd.Length());
	len = cmdhead.Length()+sizeof(int) + cmd.Length() + 1;
	send(c, buf, len, 0);

	do {
		SecureZeroMemory(buf, 2048);
		r = recv(c, buf, 2048, 0);
		if (r > 0) {
			printf("Bytes received: %d\n", r);
			cbs = *(conf_backsrv*)buf;
		}
		else if (r == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());
	}
	while (r > 0);

	WSACleanup();
	return 0;
}
