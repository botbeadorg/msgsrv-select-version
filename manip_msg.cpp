// ---------------------------------------------------------------------------

#pragma hdrstop

#include "manip_msg.h"

// ---------------------------------------------------------------------------
#pragma package(smart_init)

bool start_msg_device(bool * const result, int com_num, unsigned long baudrate)
{
	bool corr_proc = true;
	if (corr_proc = (result && com_num > 0))
		* result = SMSStartService(com_num, baudrate);
	if (!*result)
		print_msg_error();
	return corr_proc;
}

bool stop_msg_device(bool * const result) {
	bool corr_proc = true;
	if (corr_proc = result)
		* result = SMSStopSerice();
	if (!*result)
		print_msg_error();
	return corr_proc;
}

bool send_msg(unsigned long * const msg_num, const AnsiString * const msg,
	const AnsiString * const phone) {
	bool corr_proc = true;
	int status_sended_msg = -2;
	String dbg_str;
	if (corr_proc = (msg_num && msg && phone))
		* msg_num = SMSSendMessage(msg->c_str(), phone->c_str());
	return corr_proc;
}

void print_msg_error() {
	char err_info[256];
	ZeroMemory(err_info, 256);
	SMSGetLastError(err_info);
	OutputDebugStringA(err_info);
}

bool has_new_message(bool * const yes_or_not, SMSMessageStruct * const new_msg)
{
	bool corr_proc = true;
	if (corr_proc = (yes_or_not && new_msg))
		* yes_or_not = SMSGetNextMessage(new_msg);
	return corr_proc;
}

bool has_sending_feedback(bool * const yes_or_not,
	SMSReportStruct * const feedback) {
	bool corr_proc = true;
	if (corr_proc = yes_or_not && feedback)
		* yes_or_not = SMSReport(feedback);
	return corr_proc;
}
