// ---------------------------------------------------------------------------

#ifndef manip_msgH
#define manip_msgH
// ---------------------------------------------------------------------------
#include <vcl.h>
#include "SMSDLL.h"

bool start_msg_device(bool * const result, int com_num = 1,
	unsigned long baudrate = 115200);
bool stop_msg_device(bool * const result);
bool send_msg(unsigned long * const msg_num, const AnsiString * const msg,
	const AnsiString * const phone);
void print_msg_error();
bool has_sending_feedback(bool * const yes_or_not,
	SMSReportStruct * const feedback);
bool has_new_message(bool * const yes_or_not, SMSMessageStruct * const new_msg);

#endif
