#ifndef SMSDLL_H
#define SMSDLL_H

typedef struct _sms_report_t_ {
	unsigned long index; // 短消息编号:index，从0开始递增
	char Msg[256]; // 短信内容
	int Success; // 是否发送成功 0为失败，非0为成功
	char PhoneNo[32]; // 目标手机号码
} SMSReportStruct;

typedef struct _sms_msg_t_ {
	char Msg[256]; // 短信内容
	char PhoneNo[32]; // 对方手机号码
	char ReceTime[32]; // 接收时间
} SMSMessageStruct;

// 启动服务,打开串口，初始化Modem, 0为失败，非0为成功
// 校验位， EvenParity :0,MarkParity:1,NoParity:2,OddParity:3,SpaceParity,4
// 停止位 OneStopBit 0,OnePointFiveStopBits:1,TwoStopBits 2
// 流控:NoFlowControl:0,    CtsRtsFlowControl:1,    CtsDtrFlowControl:2,    DsrRtsFlowControl:3,    DsrDtrFlowControl:4,    XonXoffFlowControl:5
extern "C" __declspec(dllimport) int _stdcall SMSStartService(int nPort,
	unsigned long BaudRate = 57600, int Parity = 2, int DataBits = 8,
	int StopBits = 0, int FlowControl = 0, char* csca = "card");

// 停止服务，并关闭串口,0为失败，非0为成功
extern "C" __declspec(dllimport) int _stdcall SMSStopSerice();

// 发送短消息,返回短消息编号:index，从0开始递增，该函数不会阻塞，立既返回，请用函数SMSQuery(unsigned long index)来查询是否发送成功
extern "C" __declspec(dllimport) unsigned long _stdcall SMSSendMessage
	(char* Msg, char* PhoneNo);

// 报告短信发送壮态(成功与否)0为有报告，非0为无
extern "C" __declspec(dllimport) int _stdcall SMSReport(SMSReportStruct* rept);

// 查询指定序号的短信是否发送成功(该序号由SMSSendMessage返回)
// 返回 0 表示发送失败
// 1 表示发送成功
// -1 表示没有查询到该序号的短信,可能仍在发送中。
extern "C" __declspec(dllimport) int _stdcall SMSQuery(unsigned long index);

// 接收短信,0为有短信，非0为无
extern "C" __declspec(dllimport) int _stdcall SMSGetNextMessage
	(SMSMessageStruct* Msg);

// 返回错误内容的长度
extern "C" __declspec(dllimport) int _stdcall SMSGetLastError(char* err);

#endif
