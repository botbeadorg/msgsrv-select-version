// ---------------------------------------------------------------------------

#pragma hdrstop

#include "dm_main.h"
#include "manip_ini_file.h"
#include "manip_msg.h"
using namespace std;
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma classgroup "System.Classes.TPersistent"
#pragma link "DBAccess"
#pragma link "MemDS"
#pragma link "myprovider160.lib"
#pragma link "Uni"
#pragma link "UniProvider"
#pragma resource "*.dfm"

// from database
const int VIA_DB = 0;
// from message
const int VIA_MSG_WITH_FB = 1;
// from message without sending to client
const int VIA_MSG_WITHOUT_FB = 2;

long countor = 0;
int serial_port_number = 0;

mylist workflow[5];

spin_lock countor_lock;
spin_lock r_w_msg_lock;

thread_group threads;

static String server, name, password, database, sp_number;
static std::vector<elec_time_pair*>etps;
static const String srv_name = "msgsrv";
static const String file_name = "conn_params.ini";
static const int NS_IN6ADDRSZ = 16;
static const int NS_INT16SZ = 2;
static char port_to_listen[8] = {0};
static const String query_phone_cmd =
	"select phone4device from device where device_ID = '";
static const String query_table_cmd = "show tables like '";

bool use_winsock(const int low_byte, const int high_byte);
void drop_winsock();
int sendall(int s, char *buf, int *len);
const char * inet_ntop_why(int af, const void * src, char * dst, socklen_t cnt);
static const char * inet_ntop4(const unsigned char *src, char *dst,
	socklen_t size);
static const char * inet_ntop6(const unsigned char *src, char *dst,
	socklen_t size);

static void *get_in_addr(sockaddr *sa);
void num_str(AnsiString * const str, const int num);
bool parse_data_received(const data_received_info * const dri);
bool query_phone_by_device_id(AnsiString * const phone,
	const AnsiString * const device_id);

void str_num(int * const num, AnsiString * const str);
bool record_eq_or_def_state(char * const device_id, const char * const phone,
	const char * const msg, const char * const msg_time = "");
bool be_numberic_string(bool * const result, const AnsiString * const string);
bool dev_send_to_client_directly(const char * const msg_content,
	const char * const phone);

bool open_conn();
bool close_conn();

bool query_electric_quantity_from_table(const AnsiString * const device_id);
bool format_time_str(AnsiString * const str_formatted,
	const AnsiString * const org_str);
bool format_electric_quantity_feedback(char * const eq_str,
	int * const len_content, const AnsiString * const head);
int get_lastest_state_record(char * const defending_state,
	const char * const dev_id);
void deal_with_info_parsed_via_db(info_parsed * const one_info);
int set_msgsrv_phone(const char * const setting_phone_cmd);
bool get_srv_self_path(String * const path, const String * const srv_name);

// start message server
void start_msgsrv(const char * const port, unsigned long baudrate);
// close message server
void end_msgsrv();

// thread for sending via network
unsigned __stdcall sending_func(void *param);
bool start_sending_thread();
void end_sending_thread();
void active_sending_thread();

// thread for zero counter
bool start_reset_countor_thread();
unsigned __stdcall reset_countor_func(void *param);
void end_reset_countor_thread();

// thread for sending message
bool start_msg_sending_thread();
void end_msg_sending_thread();
unsigned __stdcall msg_sending_func(void *param);
void active_msg_sending_thread();

// thread for receiving message
bool start_msg_receiving_thread();
void end_msg_receiving_thread();
unsigned __stdcall msg_receiving_func(void *param);

// thread for recving via network
unsigned __stdcall selecting_func(void *param);
unsigned __stdcall setup_terminator(void *);
bool start_selecting_thread(const char * const port);
void end_selecting_thread();

TController *Controller;

// ---------------------------------------------------------------------------
__fastcall TController::TController(TComponent* Owner) : TDataModule(Owner) {
}
// ---------------------------------------------------------------------------

unsigned __stdcall reset_countor_func(void *param) {
LOOP:
	Sleep(86400000);
	countor_lock.lock();
	countor = 0;
	countor_lock.unlock();
	goto LOOP;
	return 0;
}

bool start_reset_countor_thread() {
	unsigned int thread_id;
	threads.thread_reset_countor.handle =
		_beginthreadex(NULL, 0, reset_countor_func, NULL, 0, &thread_id);

	if (0 == threads.thread_reset_countor.handle) {
		print_dbg_msg("create thread_reset_countor thread failed: ",
			GetLastError());
		return false;
	}
	return true;
}

void print_dbg_msg(const char * const msg, const int value) {
	string dbg_str;
	stringstream ss;
	ss.str("");
	dbg_str.operator = (msg);
	ss << value;
	dbg_str.operator += (ss.str());
	OutputDebugStringA(dbg_str.c_str());
}

void end_reset_countor_thread() {
	TerminateThread((void *)threads.thread_reset_countor.handle, 0);
	CloseHandle((void *)threads.thread_reset_countor.handle);
}

// db connection params
bool set_conn_params() {
	bool corr_proc = true;
	TMemIniFile *ini_file = NULL;
	String path;
	String full_name;
	String section = "conn_params";
	String key_server = "server";
	String default_value_key_server = "localhost";
	String key_database = "database";
	String default_value_key_database = "msgsrv_db";
	String key_name = "name";
	String default_value_key_name = "root";
	String key_pwd = "password";
	String key_sp_number = "serial_port_number";
	String default_sp_number = "1";
	if (corr_proc = (
#ifdef SRV
		get_srv_self_path(&path, &srv_name)
#else
		(path = ExtractFilePath(Application->ExeName), path.Length())
#endif
		&& (full_name = path + file_name, FileExists(full_name))
		&& open_ini_file(&ini_file, &full_name) && read_string_value_from_ini
		(&server, ini_file, &section, &key_server, default_value_key_server)
		&& read_string_value_from_ini(&database, ini_file, &section,
		&key_database, default_value_key_database) && read_string_value_from_ini
		(&name, ini_file, &section, &key_name, default_value_key_name)
		&& read_string_value_from_ini(&password, ini_file, &section,
		&key_pwd) && read_string_value_from_ini(&sp_number, ini_file, &section,
		&key_sp_number, default_sp_number))) {
		Controller->uniconn->Server = server;
		Controller->uniconn->Database = database;
		Controller->uniconn->Username = name;
		Controller->uniconn->Password = password;
		serial_port_number = sp_number.ToInt();
	}
	if (ini_file)
		close_ini_file(&ini_file);
	return corr_proc;
}

// open db connection
bool open_conn() {
	if (set_conn_params()) {
		Controller->uniconn->Connect();
		return true;
	}
	return false;
}

// close db connection
bool close_conn() {
	if (Controller->uniconn->Connected) {
		Controller->uniconn->Close();
		return true;
	}
	return false;
}

bool use_winsock(const int low_byte, const int high_byte) {
	unsigned short version = MAKEWORD(low_byte, high_byte);
	WSADATA wsadata;
	if (WSAStartup(version, &wsadata))
		return false;
	if (LOBYTE(wsadata.wVersion) != low_byte || HIBYTE(wsadata.wVersion)
		!= high_byte) {
		WSACleanup();
		return false;
	}
	return true;
}

void drop_winsock() {
	WSACleanup();
}

int sendall(int s, char *buf, int *len) {
	int total = 0;
	int bytesleft = *len;
	int n;

	while (total < *len) {
		n = send(s, buf + total, bytesleft, 0);
		if (n == -1) {
			break;
		}
		total += n;
		bytesleft -= n;
	}

	*len = total;

	return n == -1 ? -1 : 0;
}

const char * inet_ntop_why(int af, const void * src, char * dst, socklen_t cnt)
{
	switch (af) {

	case AF_INET:
		return (inet_ntop4((const unsigned char *)src, dst, cnt));

	case AF_INET6:
		return (inet_ntop6((const unsigned char *)src, dst, cnt));

	default:
		return (NULL);
	}
}

static const char * inet_ntop4(const unsigned char *src, char *dst,
	socklen_t size) {
	char tmp[sizeof("255.255.255.255")];
	int len;

	len = sprintf(tmp, "%u.%u.%u.%u", src[0], src[1], src[2], src[3]);
	if (len < 0)
		return NULL;

	if (len > size) {
		errno = ENOSPC;
		return NULL;
	}

	return strcpy(dst, tmp);
}

static const char * inet_ntop6(const unsigned char *src, char *dst,
	socklen_t size) {

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

		if (i == 6 && best.base == 0 && (best.len == 6 ||
			(best.len == 5 && words[5] == 0xffff))) {
			if (!inet_ntop4(src + 12, tp, sizeof tmp - (tp - tmp)))
				return (NULL);
			tp += strlen(tp);
			break;
		} {
			int len = sprintf(tp, "%x", words[i]);
			if (len < 0)
				return NULL;
			tp += len;
		}
	}

	if (best.base != -1 && (best.base + best.len) ==
		(NS_IN6ADDRSZ / NS_INT16SZ))
		* tp++ = ':';
	*tp++ = '\0';

	if ((socklen_t)(tp - tmp) > size) {
		errno = ENOSPC;
		return NULL;
	}

	return strcpy(dst, tmp);
}

unsigned __stdcall selecting_func(void *param) {
	fd_set master;
	fd_set read_fds;
	int fdmax;

	int listener;
	int newfd;
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;

	int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

	bool yes = true;
	int i, rv;

	struct addrinfo hints, *ai, *p;

	unsigned int thread_id;

	string dbg_str;
	stringstream ss;

	data_received_info *dri;

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, port_to_listen, &hints, &ai)) != 0) {
		print_dbg_msg("getaddrinfo failed: ", WSAGetLastError());
		exit(1);
	}

	for (p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			continue;
		}

		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes,
			sizeof(bool));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			closesocket(listener);
			continue;
		}

		break;
	}

	if (NULL == p) {
		print_dbg_msg("failed to bind: ", WSAGetLastError());
		exit(2);
	}

	freeaddrinfo(ai);

	if (SOCKET_ERROR == listen(listener, SOMAXCONN)) {
		print_dbg_msg("listen failed: ", WSAGetLastError());
		exit(3);
	}

	FD_SET((unsigned)listener, &master);

	fdmax = listener;

	CloseHandle((void *)_beginthreadex(NULL, 0, setup_terminator, NULL, 0,
		&thread_id));

	addrlen = sizeof(remoteaddr);
	threads.thread_selecting.terminator_ending =
		accept(listener, (sockaddr*)&remoteaddr, &addrlen);

	FD_SET((unsigned)threads.thread_selecting.terminator_ending, &master);

	if (threads.thread_selecting.terminator_ending > fdmax) {
		fdmax = threads.thread_selecting.terminator_ending;
	}

	for (; ;) {
		read_fds = master;
		if (SOCKET_ERROR == select(fdmax + 1, &read_fds, NULL, NULL, NULL)) {
			print_dbg_msg("select failed: ", WSAGetLastError());
			exit(4);
		}

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener) {

					addrlen = sizeof remoteaddr;
					newfd = accept(listener, (struct sockaddr*)&remoteaddr,
						&addrlen);

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
							(inet_ntop_why(remoteaddr.ss_family,
							get_in_addr((sockaddr*)&remoteaddr), remoteIP,
							INET6_ADDRSTRLEN));
						dbg_str.operator += (" on socket ");
						ss << newfd;
						dbg_str.operator += (ss.str());
						OutputDebugStringA(dbg_str.c_str());
					}
				}
				else {
					dri = (data_received_info*)dri_pool::malloc();
					dri->init_myself();
					dri->fd = i;

					if (NULL == dri) {
						OutputDebugStringA(
							"malloc() from singleton_pool failed");
					}

					if ((nbytes = recv(i, dri->data, BUF_SIZE, 0)) <= 0) {

						if (0 == nbytes) {
							print_dbg_msg(
								"remote peer has close the socket: ", i);
							if (threads.thread_selecting.terminator_ending == i)
							{
								closesocket(i);
								goto END_THREAD;
							}
						}
						else {
							print_dbg_msg("recv failed: ", WSAGetLastError());
						}
						closesocket(i);
						FD_CLR((unsigned)i, &master);

						dri_pool::free(dri);
					}
					else {
						dri->data_len = nbytes;
						parse_data_received(dri);
						dri_pool::free(dri);
					}
				}
			}
		}
	}
END_THREAD:
	closesocket(listener);
	FD_CLR((unsigned)listener, &master);
	for (i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &master)) {
			closesocket(i);
		}
	}
	return 0;
}

unsigned __stdcall setup_terminator(void *param) {
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (0 != getaddrinfo("localhost", port_to_listen, &hints, &res)) {
		print_dbg_msg("getaddrinfo failed: ", WSAGetLastError());
		exit(1);
	}

	threads.thread_selecting.terminator_beginning =
		socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (INVALID_SOCKET == (unsigned)
		threads.thread_selecting.terminator_beginning) {
		print_dbg_msg("get fd failed: ", WSAGetLastError());
		exit(2);
	}

	if (SOCKET_ERROR == connect(threads.thread_selecting.terminator_beginning,
		res->ai_addr, res->ai_addrlen)) {
		print_dbg_msg("connect failed: ", WSAGetLastError());
		closesocket(threads.thread_selecting.terminator_beginning);
		exit(3);
	}

	freeaddrinfo(res);
	return 0;
}

void *get_in_addr(sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((sockaddr_in*)sa)->sin_addr);
	}
	return &(((sockaddr_in6*)sa)->sin6_addr);
}

bool parse_data_received(const data_received_info * const dri) {
	bool corr_proc = true;
	int k, i, j;
	info_parsed *one_iparsed = NULL;
	AnsiString dev_id, phone, num_suffix;
	k = j = i = 0;
	one_iparsed = (info_parsed*)iparsed_pool::malloc();
	one_iparsed->init_myself();
	if (corr_proc = (dri && (INVALID_SOCKET != (unsigned)(dri->fd)) &&
		(dri->data_len > 0))) {
		countor_lock.lock();
		one_iparsed->cmd_index = InterlockedIncrement(&(countor));
		countor_lock.unlock();
		num_str(&num_suffix, one_iparsed->cmd_index);
	LOOP:
		if ('\0' == dri->data[j]) {
			if (0 == k) {
				i = j;
				k++;
				goto BOOST;
			}
			if (1 == k) {
				MoveMemory(one_iparsed->device_id, &(dri->data[i + 1]), j - i);
				i = j;
				k++;
				goto BOOST;
			}
			if (2 == k) {
				MoveMemory(one_iparsed->cmd_content, &(dri->data[i + 1]),
				j - i);
				MoveMemory(&(one_iparsed->cmd_content[0]) +
					strlen(one_iparsed->cmd_content), num_suffix.c_str(),
					num_suffix.Length());
				k++;
			}
		}
	BOOST:
		if (++j < dri->data_len)
			goto LOOP;
		if ((0 == strncmp(one_iparsed->cmd_content, "SYDL", 4)) ||
			(0 == strncmp(one_iparsed->cmd_content, "TQZT", 4))) {
			one_iparsed->type = VIA_DB;
			one_iparsed->socket = dri->fd;
			deal_with_info_parsed_via_db(one_iparsed);
			workflow[SEDNING_FLOW].lock.lock();
			workflow[SEDNING_FLOW].list->Add(one_iparsed);
			workflow[SEDNING_FLOW].lock.unlock();
			active_sending_thread();
			goto END_PROC1;
		}
		else {
			dev_id.operator = (one_iparsed->device_id);
			if (dev_id.operator == ("whosyourdaddy")) {
				set_msgsrv_phone(one_iparsed->cmd_content);
				iparsed_pool::free(one_iparsed);
				goto END_PROC;
			}
			if (corr_proc = query_phone_by_device_id(&phone, &dev_id)) {
				MoveMemory(one_iparsed->phone, phone.c_str(), phone.Length());
			}
			if (('D' == one_iparsed->cmd_content[0]
				&& 'S' == one_iparsed->cmd_content[1]) ||
				('D' == one_iparsed->cmd_content[0]
				&& 'H' == one_iparsed->cmd_content[1]) ||
				('J' == one_iparsed->cmd_content[0]
				&& 'S' == one_iparsed->cmd_content[1])) {
				one_iparsed->type = VIA_MSG_WITHOUT_FB;
			}
			else {
				one_iparsed->type = VIA_MSG_WITH_FB;
				one_iparsed->socket = dri->fd;
			}
			workflow[MSG_SENDING_FLOW].lock.lock();
			workflow[MSG_SENDING_FLOW].list->Add(one_iparsed);
			workflow[MSG_SENDING_FLOW].lock.unlock();
		}
	END_PROC:
		active_msg_sending_thread();
	}
END_PROC1:
	return corr_proc;
}

void num_str(AnsiString * const str, const int num) {
	char * zero_strs[4] = {"~", "~0", "~00", "~000"};
	int zero_len = 0;
	if (str && (-1 < num && num < 10000)) {
		str->operator = (IntToStr(num));
		zero_len = 4 - str->Length();
		str->Insert(zero_strs[zero_len], 0);
	}
}

bool query_phone_by_device_id(AnsiString * const phone,
	const AnsiString * const device_id) {
	bool corr_proc = true;
	AnsiString sql_cmd = "";
	if (corr_proc = (device_id && Controller->uniconn->Connected)) {
		Controller->UniQuery1->SQL->Clear();
		sql_cmd.operator = (query_phone_cmd);
		sql_cmd.operator += (*device_id);
		sql_cmd.operator += ("'");
		Controller->UniQuery1->SQL->Add(sql_cmd);
		Controller->UniQuery1->Open();
		if (corr_proc = (1 == Controller->UniQuery1->RecordCount)) {
			*phone = Controller->UniQuery1->FieldByName("phone4device")
				->AsString;
		}
		Controller->UniQuery1->Close();
	}
	return corr_proc;
}

unsigned __stdcall msg_sending_func(void *param) {
	info_parsed *one_info = NULL;
	TList *list = NULL;
	int list_count = 0;
LOOP:

	if (WAIT_OBJECT_0 == WaitForSingleObject
		(threads.thread_writtingCOM.event_loop, INFINITE)) {
		threads.thread_writtingCOM.exit_lock.lock();
		if (WAIT_OBJECT_0 == WaitForSingleObject
			(threads.thread_writtingCOM.event_exit, 0)) {
			threads.thread_writtingCOM.exit_lock.unlock();
			goto END_THREAD;
		}
		threads.thread_writtingCOM.exit_lock.unlock();
		workflow[MSG_SENDING_FLOW].lock.lock();
		list = workflow[MSG_SENDING_FLOW].list;
		list_count = workflow[MSG_SENDING_FLOW].list->Count;
		for (int i = 0; i < list_count; i++) {
			one_info = (info_parsed*)(list->Items[i]);
			r_w_msg_lock.lock();
			one_info->sending_order = SMSSendMessage(one_info->cmd_content,
				one_info->phone);
			r_w_msg_lock.unlock();
			AnsiString dbg_str("send msg content: ");
			dbg_str.operator += (one_info->cmd_content);
			OutputDebugStringA(dbg_str.c_str());
			one_info->sending_msg_state = 0;
			workflow[MSG_REPORTING_FLOW].lock.lock();
			workflow[MSG_REPORTING_FLOW].list->Add(one_info);
			workflow[MSG_REPORTING_FLOW].lock.unlock();
			list->Items[i] = NULL;
		}
		workflow[MSG_SENDING_FLOW].list->Pack();
		workflow[MSG_SENDING_FLOW].lock.unlock();
		goto LOOP;
	}
END_THREAD:
	return 0;
}

bool start_msg_sending_thread() {
	unsigned int thread_id;
	threads.thread_writtingCOM.event_loop =
		CreateEvent(NULL, false, false, NULL);
	if (NULL == threads.thread_writtingCOM.event_loop) {
		print_dbg_msg("create threads.thread_writtingCOM.event_loop failed: ",
			GetLastError());
		return false;
	}
	threads.thread_writtingCOM.event_exit =
		CreateEvent(NULL, false, false, NULL);
	if (NULL == threads.thread_writtingCOM.event_exit) {
		print_dbg_msg("create threads.thread_writtingCOM.event_exit failed: ",
			GetLastError());
		return false;
	}
	threads.thread_writtingCOM.handle =
		_beginthreadex(NULL, 0, msg_sending_func, NULL, 0, &thread_id);

	if (0 == threads.thread_writtingCOM.handle) {
		print_dbg_msg("create thread_writtingCOM thread failed: ",
			GetLastError());
		return false;
	}
	return true;
}

void end_msg_sending_thread() {
	bool evnt1, evnt2;
	threads.thread_writtingCOM.exit_lock.lock();
	evnt1 = SetEvent(threads.thread_writtingCOM.event_loop);
	evnt2 = SetEvent(threads.thread_writtingCOM.event_exit);
	threads.thread_writtingCOM.exit_lock.unlock();
	if (WAIT_OBJECT_0 != WaitForSingleObject
		((void *)threads.thread_writtingCOM.handle, INFINITE)) {
		TerminateThread((void *)threads.thread_writtingCOM.handle, 0);
	}
	CloseHandle((void *)threads.thread_writtingCOM.handle);
}

void active_msg_sending_thread() {
	SetEvent(threads.thread_writtingCOM.event_loop);
}

unsigned __stdcall msg_receiving_func(void *param) {
	char temp_char;
	bool has_report, has_feedback;
	bool append_to_sending_flow;
	int number;
	int i, j, k;
	int index_report;
	info_parsed *info = NULL;
	char str_style_number[5] = {0};
	AnsiString temp_str;
	SMSReportStruct rept;
	char *unused_dev_id = NULL;
	TList *list = NULL;
	int list_count = 0;
	SMSMessageStruct smg;

	unused_dev_id = (char *)buf_pool::malloc();

LOOP:
	Sleep(200);

	if (WAIT_OBJECT_0 == WaitForSingleObject
		(threads.thread_readingCOM.event_exit, 0))
		goto END_THREAD;
	has_report = has_feedback = false;
	r_w_msg_lock.lock();
	if (0 != SMSReport(&rept)) {
		has_report = true;
		index_report = rept.index;
	}
	else {
		has_report = false;
	}
	r_w_msg_lock.unlock();
	append_to_sending_flow = false;
	workflow[MSG_REPORTING_FLOW].lock.lock();
	list = workflow[MSG_REPORTING_FLOW].list;
	list_count = list->Count;
	for (int m = 0; m < list_count; m++) {
		info = (info_parsed*)(list->Items[m]);
		if (has_report) {
			if (index_report == info->sending_order) {
				if (0 != rept.Success) {
					info->sending_msg_state = 1;
					if (VIA_MSG_WITHOUT_FB == info->type) {
						iparsed_pool::free(info);
					}
					else {
						workflow[MSG_FEEDBACK_FLOW].lock.lock();
						workflow[MSG_FEEDBACK_FLOW].list->Add(info);
						workflow[MSG_FEEDBACK_FLOW].lock.unlock();
					}

					list->Items[m] = NULL;
				}
				else {
					info->sending_msg_state = 2;
					info->data_to_send = (char *)min_buf_pool::malloc();
					info->pool_type = 1;
					MoveMemory(info->data_to_send, "failed",
						info->data_len_to_send = strlen("failed"));
					info->data_len_to_send += 2;
					workflow[SEDNING_FLOW].lock.lock();
					workflow[SEDNING_FLOW].list->Add(info);
					workflow[SEDNING_FLOW].lock.unlock();
					append_to_sending_flow = true;
					list->Items[m] = NULL;
				}
			}
			else {
				if (info->sending_msg_delay > 60000) {
					iparsed_pool::free(info);
					info->data_to_send = (char *)min_buf_pool::malloc();
					info->pool_type = 1;
					MoveMemory(info->data_to_send, "failed",
						info->data_len_to_send = strlen("failed"));
					info->data_len_to_send += 2;
					workflow[SEDNING_FLOW].lock.lock();
					workflow[SEDNING_FLOW].list->Add(info);
					workflow[SEDNING_FLOW].lock.unlock();
					append_to_sending_flow = true;
					list->Items[m] = NULL;
				}
				else {
					info->sending_msg_delay += 200;
				}
			}
		}
		else {
			if (info->sending_msg_delay > 60000) {
				iparsed_pool::free(info);
				info->data_to_send = (char *)min_buf_pool::malloc();
				info->pool_type = 1;
				MoveMemory(info->data_to_send, "failed",
					info->data_len_to_send = strlen("failed"));
				info->data_len_to_send += 2;
				workflow[SEDNING_FLOW].lock.lock();
				workflow[SEDNING_FLOW].list->Add(info);
				workflow[SEDNING_FLOW].lock.unlock();
				append_to_sending_flow = true;
				list->Items[m] = NULL;
			}
			else {
				info->sending_msg_delay += 200;
			}
		}
	}
	list->Pack();
	workflow[MSG_REPORTING_FLOW].lock.unlock();
	if (append_to_sending_flow)
		active_sending_thread();

	r_w_msg_lock.lock();
	if (0 != SMSGetNextMessage(&smg)) {
		has_feedback = true;
		AnsiString dbg_str("receive smg content: ");
		dbg_str.operator += (smg.Msg);
		OutputDebugStringA(dbg_str.c_str());
	}
	else {
		has_feedback = false;
	}
	r_w_msg_lock.unlock();
	append_to_sending_flow = false;
	if (has_feedback) {
		if (('G' == smg.Msg[0] && 'J' == smg.Msg[1]) ||
			('Q' == smg.Msg[0] && 'Y' == smg.Msg[1])) {
			dev_send_to_client_directly(smg.Msg, smg.PhoneNo);
			goto LOOP;
		}
		ZeroMemory(str_style_number, 5);
		MoveMemory(str_style_number, smg.Msg + 2, 4);
		temp_str.operator = (str_style_number);
		str_num(&number, &temp_str);
		ZeroMemory(unused_dev_id, BUF_SIZE);
		record_eq_or_def_state(unused_dev_id, smg.PhoneNo, smg.Msg);
		workflow[MSG_FEEDBACK_FLOW].lock.lock();
		list = workflow[MSG_FEEDBACK_FLOW].list;
		list_count = list->Count;
		for (int n = 0; n < list_count; n++) {
			info = (info_parsed*)(list->Items[n]);
			if (number == info->cmd_index) {
				info->data_to_send = (char *)buf_pool::malloc();
				info->pool_type = 2;
				ZeroMemory(info->data_to_send, BUF_SIZE);
				MoveMemory(info->data_to_send, smg.Msg, 2);
				j = info->data_len_to_send += 2;
				i = 6;
				while ((temp_char = smg.Msg[i]) != 'V') {
					info->data_to_send[j] = temp_char;
					j++;
					i++;
				}
				info->data_to_send[j] = temp_char;
				info->data_len_to_send += (i - 6 + 1) + 2;
				info->feedback_successfully = true;
				workflow[SEDNING_FLOW].lock.lock();
				workflow[SEDNING_FLOW].list->Add(info);
				workflow[SEDNING_FLOW].lock.unlock();
				list->Items[n] = NULL;
				append_to_sending_flow = true;
			}
			else {
				if (info->feedback_delay > 60000) {
					info->data_to_send = (char *)min_buf_pool::malloc();
					info->pool_type = 1;
					ZeroMemory(info->data_to_send, MIN_BUF_SIZE);
					MoveMemory(info->data_to_send, "failed",
						info->data_len_to_send = strlen("failed"));
					info->data_len_to_send += 2;
					workflow[SEDNING_FLOW].lock.lock();
					workflow[SEDNING_FLOW].list->Add(info);
					workflow[SEDNING_FLOW].lock.unlock();
					list->Items[n] = NULL;
					append_to_sending_flow = true;
				}
				else {
					info->feedback_delay += 200;
				}
			}
		}
		list->Pack();
		workflow[MSG_FEEDBACK_FLOW].lock.unlock();
	}
	else {
		workflow[MSG_FEEDBACK_FLOW].lock.lock();
		list = workflow[MSG_FEEDBACK_FLOW].list;
		list_count = list->Count;
		for (int n = 0; n < list_count; n++) {
			info = (info_parsed*)(list->Items[n]);
			if (info->feedback_delay >= 60000) {
				info->data_to_send = (char *)min_buf_pool::malloc();
				info->pool_type = 1;
				ZeroMemory(info->data_to_send, MIN_BUF_SIZE);
				MoveMemory(info->data_to_send, "failed",
					info->data_len_to_send = strlen("failed"));
				info->data_len_to_send += 2;
				workflow[SEDNING_FLOW].lock.lock();
				workflow[SEDNING_FLOW].list->Add(info);
				workflow[SEDNING_FLOW].lock.unlock();
				list->Items[n] = NULL;
				append_to_sending_flow = true;
			}
			else {
				info->feedback_delay += 200;
			}
		}
		list->Pack();
		workflow[MSG_FEEDBACK_FLOW].lock.unlock();
	}
	if (append_to_sending_flow)
		active_sending_thread();
	goto LOOP;

END_THREAD:
	buf_pool::free(unused_dev_id);
	return 0;
}

void str_num(int * const num, AnsiString * const str) {
	bool is_numberic_style;
	if (str && (be_numberic_string(&is_numberic_style, str), is_numberic_style))
	{
		*num = str->ToInt();
	}
}

bool record_eq_or_def_state(char * const device_id, const char * const phone,
	const char * const msg, const char * const msg_time) {
	bool need_to_write_state = false;
	int pos = -1;
	AnsiString sql_cmd("");
	AnsiString eq_table_name(""), ds_table_name("");
	AnsiString msgstr("");
	AnsiString eq_str("");
	AnsiString def_state("");
	AnsiString curr_time_str("");
	AnsiString temp_str("");
	AnsiString dev_id("");
	if (device_id && phone && msg && msg_time) {
		sql_cmd = "select device_ID from device where phone4device = '";
		sql_cmd += phone;
		sql_cmd += "'";
		Controller->UniQuery2->SQL->Clear();
		Controller->UniQuery2->SQL->Add(sql_cmd);
		Controller->UniQuery2->Open();
		if (1 == Controller->UniQuery2->RecordCount) {
			dev_id.operator =
				(Controller->UniQuery2->FieldByName("device_ID")->AsString);
			MoveMemory(device_id, dev_id.c_str(), dev_id.Length());
		}
		else {
			goto ERROR_END;
		}
		msgstr.operator = (msg);
		temp_str = msgstr.SubString(0, 2);
		if (temp_str.operator == ("BF") || temp_str.operator == ("CF")) {
			need_to_write_state = true;
			sql_cmd.operator = ("show tables like '");
			ds_table_name.operator = (device_id);
			ds_table_name.operator += ("_defencing_state");
			sql_cmd.operator += (ds_table_name);
			sql_cmd.operator += ("'");

			Controller->UniQuery2->SQL->Clear();
			Controller->UniQuery2->SQL->Add(sql_cmd);
			Controller->UniQuery2->Open();
			if (0 == Controller->UniQuery2->RecordCount) {
				goto ERROR_END;
			}
			def_state.operator += (temp_str);
			pos = msgstr.Pos("*");
			temp_str = msgstr.SubString(pos, msgstr.Length() - pos + 1);
			def_state.operator += (temp_str);
		}
		sql_cmd.operator = ("show tables like '");
		eq_table_name.operator = (device_id);
		eq_table_name.operator += ("_eq");
		sql_cmd.operator += (eq_table_name);
		sql_cmd.operator += ("'");
		Controller->UniQuery2->SQL->Clear();
		Controller->UniQuery2->SQL->Add(sql_cmd);
		Controller->UniQuery2->Open();
		if (0 == Controller->UniQuery2->RecordCount) {
			goto ERROR_END;
		}

		pos = msgstr.Pos("DC");
		eq_str.operator =
			(msgstr.SubString(pos + 2, msgstr.Length() - (pos + 2)));
		curr_time_str.operator = (FormatDateTime("yyyy-mm-dd hh:nn:ss", Now()));
		Controller->UniQuery2->SQL->Clear();
		sql_cmd.operator = ("INSERT INTO ");
		sql_cmd.operator += (eq_table_name);
		sql_cmd.operator += (" (electric_quantity, time) VALUES ('");
		sql_cmd.operator += (eq_str);
		sql_cmd.operator += ("', '");
		sql_cmd.operator += (curr_time_str);
		sql_cmd.operator += ("')");
		Controller->UniQuery2->SQL->Add(sql_cmd);
		Controller->UniQuery2->Execute();
		if (need_to_write_state) {
			sql_cmd.operator = ("INSERT INTO ");
			sql_cmd.operator += (ds_table_name);
			sql_cmd.operator += (" (defencing_state, time) VALUES ('");
			sql_cmd.operator += (def_state);
			sql_cmd.operator += ("', '");
			sql_cmd.operator += (curr_time_str);
			sql_cmd.operator += ("')");
			Controller->UniQuery2->SQL->Clear();
			Controller->UniQuery2->SQL->Add(sql_cmd);
			Controller->UniQuery2->Execute();
		}
		Controller->UniQuery2->Close();
		return true;
	}
ERROR_END:
	Controller->UniQuery2->Close();
	return false;
}

bool dev_send_to_client_directly(const char * const msg_content,
	const char * const phone) {

	bool append_to_msg_sending_flow = false;
	int i = 0;
	info_parsed *temp_info = NULL;
	char *unused_dev_id = NULL;
	AnsiString temp_phone("");
	AnsiString temp_field_name("forwarding_phone_");
	AnsiString sqlcmd("select forwarding_phone_1,forwarding_phone_2,forwarding_phone_3,forwarding_phone_4,\
forwarding_phone_5,forwarding_phone_6,forwarding_phone_7,forwarding_phone_8,forwarding_phone_9,\
forwarding_phone_10 from device where phone4device = '");
	TStringList *phone_list = new TStringList;
	if (!msg_content || !phone) {
		return false;
	}
	unused_dev_id = (char *)buf_pool::malloc();
	record_eq_or_def_state(unused_dev_id, phone, msg_content);
	sqlcmd.operator += (phone);
	sqlcmd.operator += ("'");

	if (!Controller->uniconn->Connected) {
		open_conn();
	}
	Controller->UniQuery5->SQL->Clear();
	Controller->UniQuery5->SQL->Add(sqlcmd);
	Controller->UniQuery5->Open();
	if (!(1 == Controller->UniQuery5->RecordCount)) {
		return false;
	}

	for (; i < 10; i++) {
		temp_phone.operator =
			(Controller->UniQuery5->FieldByName(temp_field_name.operator +
			(IntToStr(i + 1)))->AsString);
		if (temp_phone.operator == (""))
			continue;
		temp_info = (info_parsed*)iparsed_pool::malloc();
		temp_info->init_myself();
		temp_info->type = VIA_MSG_WITHOUT_FB;
		MoveMemory(temp_info->phone, temp_phone.c_str(), temp_phone.Length());
		MoveMemory(temp_info->cmd_content, msg_content, strlen(msg_content));
		workflow[MSG_SENDING_FLOW].lock.lock();
		workflow[MSG_SENDING_FLOW].list->Add(temp_info);
		workflow[MSG_SENDING_FLOW].lock.unlock();
		append_to_msg_sending_flow = true;
	}

	if (append_to_msg_sending_flow)
		active_msg_sending_thread();
	if (phone_list)
		delete phone_list;
	buf_pool::free(unused_dev_id);
	return true;
}

bool start_msg_receiving_thread() {
	unsigned int thread_id;
	threads.thread_readingCOM.event_exit =
		CreateEvent(NULL, false, false, NULL);
	if (NULL == threads.thread_readingCOM.event_exit) {
		print_dbg_msg("create threads.thread_readingCOM.event_exit failed: ",
			GetLastError());
		return false;
	}
	threads.thread_readingCOM.handle =
		_beginthreadex(NULL, 0, msg_receiving_func, NULL, 0, &thread_id);

	if (0 == threads.thread_readingCOM.handle) {
		print_dbg_msg("create thread_readingCOM thread failed: ",
		GetLastError());
		return false;
	}
	return true;
}

void end_msg_receiving_thread() {
	bool evnt2;
	unsigned long wait_state;
	evnt2 = SetEvent(threads.thread_readingCOM.event_exit);
	if (WAIT_OBJECT_0 != (wait_state =
		WaitForSingleObject((void *)threads.thread_readingCOM.handle,
		INFINITE))) {
		TerminateThread((void *)threads.thread_readingCOM.handle, 0);
	}
	CloseHandle((void *)threads.thread_readingCOM.handle);
}

bool be_numberic_string(bool * const result, const AnsiString * const string) {
	int i, j, k;
	String decimal_value_str[10] = {
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
	String reusable_str;
	if (!result || !string)
		return false;
	j = string->Length() + 1;
	for (i = 1; i < j; i++) {
		reusable_str = string->SubString(i, 1);
		for (k = 0; k < 10; k++) {
			if (reusable_str.operator == (decimal_value_str[k]))
				break;
		}
		if (10 == k) {
			*result = false;
			goto PROC_END;
		}
	}
	if (i == j)
		* result = true;
PROC_END:
	return true;
}

unsigned __stdcall sending_func(void *param) {
	int byetes_number_sended;
	std::vector<info_parsed*>::iterator bi;
	std::vector<info_parsed*>::iterator ei;
	info_parsed *reusable_info_parsed = NULL;
	TList *list = NULL;
	int list_count = 0;
LOOP:
	if (WAIT_OBJECT_0 == WaitForSingleObject(threads.thread_sending.event_loop,
		INFINITE)) {
		threads.thread_sending.exit_lock.lock();
		if (WAIT_OBJECT_0 == WaitForSingleObject
			(threads.thread_sending.event_exit, 0)) {
			threads.thread_sending.exit_lock.unlock();
			goto END_THREAD;
		}
		threads.thread_sending.exit_lock.unlock();
		workflow[SEDNING_FLOW].lock.lock();
		list = workflow[SEDNING_FLOW].list;
		list_count = list->Count;
		for (int i = 0; i < list_count; i++) {
			reusable_info_parsed = (info_parsed*)list->Items[i];
			byetes_number_sended = reusable_info_parsed->data_len_to_send;
			if (-1 == sendall(reusable_info_parsed->socket,
				reusable_info_parsed->data_to_send, &byetes_number_sended)) {
				print_dbg_msg("sending failed: ", WSAGetLastError());
			}
			AnsiString dbg_str("send content via net: ");
			dbg_str.operator += (reusable_info_parsed->data_to_send);
			OutputDebugStringA(dbg_str.c_str());
			switch (reusable_info_parsed->pool_type) {
			case 1:
				min_buf_pool::free(reusable_info_parsed->data_to_send);
				break;
			case 2:
				buf_pool::free(reusable_info_parsed->data_to_send);
				break;
			case 3:
				max_buf_pool::free(reusable_info_parsed->data_to_send);
				break;
			default: ;
			}
			iparsed_pool::free(reusable_info_parsed);
			list->Items[i] = NULL;
		}
		list->Pack();
		workflow[SEDNING_FLOW].lock.unlock();
		goto LOOP;
	}
END_THREAD:
	return 0;
}

bool start_sending_thread() {
	unsigned int thread_id;
	threads.thread_sending.event_loop = CreateEvent(NULL, false, false, NULL);
	if (NULL == threads.thread_sending.event_loop) {
		print_dbg_msg("create threads.thread_sending.event_loop failed: ",
			GetLastError());
		return false;
	}
	threads.thread_sending.event_exit = CreateEvent(NULL, false, false, NULL);
	if (NULL == threads.thread_sending.event_exit) {
		print_dbg_msg("create threads.thread_sending.event_exit failed: ",
			GetLastError());
		return false;
	}
	threads.thread_sending.handle = _beginthreadex(NULL, 0, sending_func, NULL,
		0, &thread_id);

	if (0 == threads.thread_sending.handle) {
		print_dbg_msg("create sending thread failed: ", GetLastError());
		return false;
	}
	return true;
}

void end_sending_thread() {
	bool evnt1, evnt2;
	threads.thread_sending.exit_lock.lock();
	evnt1 = SetEvent(threads.thread_sending.event_loop);
	evnt2 = SetEvent(threads.thread_sending.event_exit);
	threads.thread_sending.exit_lock.unlock();
	if (WAIT_OBJECT_0 != WaitForSingleObject
		((void *)threads.thread_sending.handle, INFINITE)) {
		TerminateThread((void *)threads.thread_sending.handle, 0);
	}
	CloseHandle((void *)threads.thread_sending.handle);
}

void active_sending_thread() {
	SetEvent(threads.thread_sending.event_loop);
}

bool start_selecting_thread(const char * const port) {
	unsigned int thread_id;
	if (port && 0 != strcmp(port, "")) {
		MoveMemory(port_to_listen, port, strlen(port));
		if (false == use_winsock(2, 2)) {
			print_dbg_msg("use winsock dll failed: ", GetLastError());
			return false;
		}
		threads.thread_selecting.handle =
			_beginthreadex(NULL, 0, selecting_func, NULL, 0, &thread_id);
		if (0 == threads.thread_selecting.handle) {
			print_dbg_msg("create selecting thread failed: ", GetLastError());
			return false;
		}
		return true;
	}
	return false;
}

void end_selecting_thread() {
	unsigned long wait_state;
	closesocket(threads.thread_selecting.terminator_beginning);

	if (WAIT_OBJECT_0 != WaitForSingleObject
		((void *)threads.thread_selecting.handle, INFINITE)) {
		TerminateThread((void *)threads.thread_selecting.handle, 0);
	}
	CloseHandle((void *)threads.thread_selecting.handle);
	drop_winsock();
}

void start_msgsrv(const char * const port, unsigned long baudrate) {
	open_conn();
	bool start_msg_modem_ok;
	if (false == start_reset_countor_thread()) {
		OutputDebugStringA("start reseting cmd countor thread failed");
		return;
	}

	if (start_msg_device(&start_msg_modem_ok, serial_port_number, baudrate),
		!start_msg_modem_ok) {
		OutputDebugStringA("start msg modem failed");
		return;
	}
	if (false == start_msg_receiving_thread()) {
		OutputDebugStringA("start message receiving thread failed");
		return;
	}

	if (false == start_msg_sending_thread()) {
		OutputDebugStringA("start message sending thread failed");
		return;
	}

	if (false == start_sending_thread()) {
		OutputDebugStringA("start net sending thread failed");
		return;
	}

	if (false == start_selecting_thread(port)) {
		OutputDebugStringA("start net receiving thread failed");
		return;
	}
}

void end_msgsrv() {
	bool end_msg_modem;
	end_selecting_thread();
	end_msg_receiving_thread();
	end_msg_sending_thread();
	end_sending_thread();
	end_reset_countor_thread();
	close_conn();
	stop_msg_device(&end_msg_modem);
	dri_pool::purge_memory();
	etp_pool::purge_memory();
	iparsed_pool::purge_memory();
	min_buf_pool::purge_memory();
	buf_pool::purge_memory();
	max_buf_pool::purge_memory();
}

void deal_with_info_parsed_via_db(info_parsed * const one_info) {
	int result = -1;
	info_parsed *reusable_info = NULL;
	AnsiString device_id;
	AnsiString head;
	if (one_info) {
		device_id.operator = (one_info->device_id);
		if (0 == strncmp(one_info->cmd_content, "SYDL", 4)) {
			if (query_electric_quantity_from_table(&device_id)) {
				head.operator = ("ok");
				one_info->data_to_send = (char *)max_buf_pool::malloc();

				ZeroMemory(one_info->data_to_send, MAX_BUF_SIZE);
				one_info->pool_type = 3;
				format_electric_quantity_feedback(one_info->data_to_send,
					&(one_info->data_len_to_send), &head);
			}
			else {
				head.operator = ("failed");
				one_info->data_to_send = (char *)min_buf_pool::malloc();
				ZeroMemory(one_info->data_to_send, sizeof(MIN_BUF_SIZE));
				one_info->pool_type = 1;
				MoveMemory(one_info->data_to_send, head.c_str(), head.Length());
				one_info->data_len_to_send = head.Length() + 2;
			}
		}
		else if (0 == strncmp(one_info->cmd_content, "TQZT", 4)) {
			one_info->data_to_send = (char *)min_buf_pool::malloc();
			ZeroMemory(one_info->data_to_send, sizeof(MIN_BUF_SIZE));
			one_info->pool_type = 1;
			if (0 == (result =
				get_lastest_state_record((one_info->data_to_send + 3),
				device_id.c_str()))) {
				(one_info->data_to_send)[0] = 'o';
				(one_info->data_to_send)[1] = 'k';

			}
			else {
				MoveMemory(one_info->data_to_send, "failed",
					one_info->data_len_to_send = strlen("failed"));
				one_info->data_len_to_send += 2;
			}
		}
	}

}

bool query_electric_quantity_from_table(const AnsiString * const device_id) {
	bool corr_proc = true;
	elec_time_pair *etp = NULL;
	AnsiString sql_cmd = "";
	TDateTime time;
	AnsiString inter_str, time_formatted;
	if (corr_proc = (device_id && Controller->uniconn->Connected)) {
		Controller->UniQuery1->SQL->Clear();
		sql_cmd.operator = (query_phone_cmd);
		sql_cmd.operator += (*device_id);
		sql_cmd.operator += ("'");
		Controller->UniQuery1->SQL->Add(sql_cmd);
		Controller->UniQuery1->Open();
		if (corr_proc = (1 == Controller->UniQuery1->RecordCount)) {
			Controller->UniQuery1->SQL->Clear();
			sql_cmd.operator = ("select electric_quantity,time from ");
			sql_cmd.operator += (*device_id);
			sql_cmd.operator += ("_eq");
			sql_cmd.operator +=
				(" where DATE_SUB(CURDATE(), INTERVAL 1 MONTH) <= date(time)");
			Controller->UniQuery1->SQL->Clear();
			Controller->UniQuery1->SQL->Add(sql_cmd);
			Controller->UniQuery1->Open();
			if (corr_proc = (Controller->UniQuery1->RecordCount > 0)) {
				Controller->UniQuery1->First();
			LOOP:
				if (!Controller->UniQuery1->Eof) {
					etp = (elec_time_pair*)etp_pool::malloc();
					etp->init_myself();
					inter_str.operator =
						(Controller->UniQuery1->FieldByName("electric_quantity")
						->AsString);
					MoveMemory(etp->elecq, inter_str.c_str(),
						inter_str.Length());
					time.operator =
						(Controller->UniQuery1->FieldByName("time")
						->AsDateTime);
					inter_str.operator =
						(FormatDateTime("yyyy-mm-dd hh:nn:ss", time));
					time_formatted.operator = ("");
					if (format_time_str(&time_formatted, &inter_str)) {
						MoveMemory(etp->time, time_formatted.c_str(),
							time_formatted.Length());
					}
					etps.push_back(etp);
					Controller->UniQuery1->Next();
					goto LOOP;
				}
			}
		}
		Controller->UniQuery1->Close();
	}
	return corr_proc;
}

bool format_time_str(AnsiString * const str_formatted,
	const AnsiString * const org_str) {
	bool corr_proc = true;
	if (corr_proc = (str_formatted && org_str && org_str->Length() == 19)) {
		(*str_formatted).operator = ("");
		str_formatted->operator += (org_str->SubString(3, 2));
		str_formatted->operator += (org_str->SubString(6, 2));
		str_formatted->operator += (org_str->SubString(9, 2));
		str_formatted->operator += (org_str->SubString(12, 2));
		str_formatted->operator += (org_str->SubString(15, 2));
	}
	return corr_proc;
}

bool format_electric_quantity_feedback(char * const eq_str,
	int * const len_content, const AnsiString * const head) {
	bool corr_proc = true;
	int index = 0;
	int temp_len = 0;
	elec_time_pair *reusable_etp = NULL;
	std::vector<elec_time_pair*>::iterator first;
	std::vector<elec_time_pair*>::iterator last;
	if (corr_proc = (eq_str && len_content && head && etps.size())) {
		first = etps.begin();
		last = etps.end();
		MoveMemory(eq_str, head->c_str(), index += head->Length());
		index++;
	LOOP:
		if (last != first) {
			reusable_etp = *first;
			MoveMemory(&(eq_str[index]), reusable_etp->time, temp_len =
				strlen(reusable_etp->time));
			index += temp_len;
			eq_str[index++] = '=';
			MoveMemory(&(eq_str[index]), reusable_etp->elecq, temp_len =
				strlen(reusable_etp->elecq));
			index += (temp_len + 1);
			etp_pool::free(reusable_etp);
			first++;
			goto LOOP;
		}
		*len_content = index + 1;
		etps.clear();
	}
	return corr_proc;
}

int get_lastest_state_record(char * const defending_state,
	const char * const dev_id) {
	AnsiString sql_cmd("");
	AnsiString table_name("");
	AnsiString vcl_defending_state("");
	if (!defending_state || !dev_id) {
		return 1;
	}
	table_name.operator = (dev_id);
	table_name.operator = ("_defencing_state");
	sql_cmd.operator = ("show tables like '");
	sql_cmd.operator += (table_name);
	sql_cmd.operator += ("'");
	if (!Controller->uniconn->Connected) {
		open_conn();
	}
	Controller->UniQuery3->SQL->Clear();
	Controller->UniQuery3->SQL->Add(sql_cmd);
	Controller->UniQuery3->Open();
	if (0 >= Controller->UniQuery3->RecordCount) {
		return 2;
	}
	Controller->UniQuery3->SQL->Clear();
	sql_cmd.operator = ("select defencing_state from ");
	sql_cmd.operator += (table_name);
	sql_cmd.operator += (" order by time desc limit 1");
	Controller->UniQuery3->SQL->Add(sql_cmd);
	Controller->UniQuery3->Open();
	if (0 >= Controller->UniQuery3->RecordCount) {
		return 3;
	}
	vcl_defending_state.operator =
		(Controller->UniQuery3->FieldByName("defencing_state")->AsString);
	MoveMemory(defending_state, vcl_defending_state.c_str(),
		vcl_defending_state.Length());
	return 0;
}

int set_msgsrv_phone(const char * const setting_phone_cmd) {
	info_parsed *one_info = NULL;
	AnsiString temp_phone("");
	const AnsiString sql_cmd("select phone4device from device");
	if (setting_phone_cmd) {
		if (!Controller->uniconn->Connected) {
			open_conn();
		}
		Controller->UniQuery4->SQL->Clear();
		Controller->UniQuery4->SQL->Add(sql_cmd);
		Controller->UniQuery4->Open();
		Controller->UniQuery4->First();
	LOOP:
		if (!Controller->UniQuery4->Eof) {
			temp_phone.operator =
				(Controller->UniQuery4->FieldByName("phone4device")->AsString);
			one_info = (info_parsed*)iparsed_pool::malloc();
			one_info->init_myself();
			one_info->type = VIA_MSG_WITHOUT_FB;
			MoveMemory(one_info->phone, temp_phone.c_str(),
				temp_phone.Length());
			MoveMemory(one_info->cmd_content, setting_phone_cmd,
				strlen(setting_phone_cmd));
			workflow[MSG_SENDING_FLOW].lock.lock();
			workflow[MSG_SENDING_FLOW].list->Add(one_info);
			workflow[MSG_SENDING_FLOW].lock.unlock();
			Controller->UniQuery4->Next();
			goto LOOP;
		}
	}
	return 0;
}

bool get_srv_self_path(String * const path, const String * const srv_name) {
	bool corr_proc;
	TRegistry *reg = NULL;

	if (corr_proc = (path && srv_name) && (reg = new TRegistry)) {
		reg->RootKey = HKEY_LOCAL_MACHINE;
		if (corr_proc =
			reg->OpenKeyReadOnly("SYSTEM\\CurrentControlSet\\Services\\" +
			*srv_name)) {
			if (corr_proc = reg->ValueExists("ImagePath")) {
				*path = reg->ReadString("ImagePath");
				reg->CloseKey();
			}
			else {
				reg->CloseKey();
			}
		}
	}
	if (reg)
		delete reg;
	return corr_proc;
}
