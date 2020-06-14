// ---------------------------------------------------------------------------

#ifndef dm_mainH
#define dm_mainH
// ---------------------------------------------------------------------------\

#include <System.Classes.hpp>
#include "DBAccess.hpp"
#include "MemDS.hpp"
#include "MySQLUniProvider.hpp"
#include "Uni.hpp"
#include "UniProvider.hpp"
#include <Data.DB.hpp>

#include <windows.h>
#include <winbase.h>
#include <process.h>
#define FD_SETSIZE 1024
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <Mswsock.h>

#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <algorithm>

#include <boost/pool/singleton_pool.hpp>

// ---------------------------------------------------------------------------
struct spin_lock {
	long a;

	spin_lock() {
		a = 0;
	}

	void lock() {
		while (InterlockedCompareExchange(&a, 1, 0))
			Sleep(0);
	}

	void unlock() {
		a = 0;
	}
};

const int MIN_BUF_SIZE = 32;
const int BUF_SIZE = 128;
const int MAX_BUF_SIZE = 20480;

typedef char min_buf[MIN_BUF_SIZE];
typedef char max_buf[MAX_BUF_SIZE];
typedef char buf[BUF_SIZE];

// memory pool for small block
typedef boost::singleton_pool < struct min_buf_tag {
}, sizeof(min_buf) > min_buf_pool;

// memory pool for midsize memory block
typedef boost::singleton_pool < struct buf_tag {
}, sizeof(buf) > buf_pool;

// memory pool for big block
typedef boost::singleton_pool < struct max_buf_tag {
}, sizeof(max_buf) > max_buf_pool;

// sturcture for recving client's data
struct data_received_info {
	int fd;
	buf data;
	int data_len;

	void init_myself() {
		fd = INVALID_SOCKET;
		ZeroMemory(data, BUF_SIZE);
		data_len = -1;
	}
};

typedef boost::singleton_pool < struct data_received_info_tag {
}, sizeof(data_received_info) > dri_pool;

struct thread_Atype {
	unsigned long handle;
	int terminator_beginning, terminator_ending;

	thread_Atype() {
		handle = 0;
		terminator_beginning = terminator_ending = INVALID_SOCKET;
	}
};

struct thread_Btype {
	unsigned long handle;
	void *event_loop, *event_exit;
	spin_lock exit_lock;

	thread_Btype() {
		handle = 0;
		event_loop = event_exit = NULL;
	}
};

struct thread_Ctype {
	unsigned long handle;
	void *event_exit;

	thread_Ctype() {
		handle = 0;
		event_exit = NULL;
	}
};

struct thread_group {
	// select thread£¬for recving the client's data
	thread_Atype thread_selecting;
	// thread for sending data to the clients
	thread_Btype thread_sending;
	// thread for sending data to the Serial Port
	thread_Btype thread_writtingCOM;
	// thread for reading data from the Serial Port
	thread_Ctype thread_readingCOM;
	// thread for resetting command index
	thread_Ctype thread_reset_countor;
};

struct info_parsed {
	// socket
	int socket;
	// device id ,from client
	min_buf device_id;
	// phone ,query by device id
	min_buf phone;
	// command ,from client
	buf cmd_content;
	// data sented to client
	char *data_to_send;
	// memory pool type
	int pool_type;
	// via type
	int type;
	// the state of sending message
	// -1: not sending, 0: sended, 2: failed
	int sending_msg_state;
	// recived the response from devices?
	bool feedback_successfully;
	// the order of sending messages
	long sending_order;
	// the length of data sended to clients
	int data_len_to_send;
	// command order from the clients
	long cmd_index;
	// the delay of response
	long feedback_delay;
	// the delay of message's receipt
	long sending_msg_delay;

	void init_myself() {
		socket = INVALID_SOCKET;
		ZeroMemory(device_id, MIN_BUF_SIZE);
		ZeroMemory(phone, MIN_BUF_SIZE);
		ZeroMemory(cmd_content, BUF_SIZE);
		data_to_send = NULL;
		type = -1;
		pool_type = 0;
		sending_msg_state = -1;
		feedback_successfully = false;
		sending_order = -1;
		data_len_to_send = 0;
		cmd_index = -1;
		feedback_delay = sending_msg_delay = 0;
	}
};

typedef boost::singleton_pool < struct info_parsed_tag {
}, sizeof(info_parsed) > iparsed_pool;

// charge-time pair, from database
struct elec_time_pair {
	min_buf elecq;
	min_buf time;

	void init_myself() {
		ZeroMemory(elecq, sizeof(min_buf));
		ZeroMemory(time, sizeof(min_buf));
	}
};

// memory pool for charge-time pair
typedef boost::singleton_pool < struct elec_time_pair_tag {
}, sizeof(elec_time_pair) > etp_pool;

struct mylist {
	TList *list;
	spin_lock lock;

	mylist() {
		list = new TList;
	}

	~mylist() {
		delete list;
	}
};

#define MSG_SENDING_FLOW 0
#define HANDLING_DB_FLOW 1
#define MSG_REPORTING_FLOW 2
#define MSG_FEEDBACK_FLOW 3
#define SEDNING_FLOW 4

extern mylist workflow[5];
extern thread_group threads;
extern const int VIA_DB;
extern const int VIA_MSG_WITH_FB;
extern const int VIA_MSG_WITHOUT_FB;

extern long countor;

extern spin_lock countor_lock;
extern spin_lock r_w_msg_lock;

void print_dbg_msg(const char * const msg, const int value);
// start messsage server
void start_msgsrv(const char * const port, unsigned long baudrate);
// close message server
void end_msgsrv();

class TController : public TDataModule {
__published: // IDE-managed Components

	TUniQuery *UniQuery2;
	TUniQuery *UniQuery4;
	TUniQuery *UniQuery3;
	TUniDataSource *UniDataSource1;
	TUniQuery *UniQuery1;
	TUniConnection *uniconn;
	TMySQLUniProvider *mysql_provider;
	TUniQuery *UniQuery5;

private: // User declarations
public: // User declarations

	__fastcall TController(TComponent* Owner);
};

// ---------------------------------------------------------------------------
extern PACKAGE TController *Controller;
// ---------------------------------------------------------------------------
#endif
