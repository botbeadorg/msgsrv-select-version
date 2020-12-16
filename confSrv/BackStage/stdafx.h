#pragma once
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
	#include <crtdbg.h>
	#include <tchar.h>
	#include <winsock2.h>
	#include <Windows.h>
#endif

#include <_ast.h>
#include <_memchk.h>
#include <Thread.h>
#include <CustomSocket.h>
#include <Lock.h>
#include <Tick.h>
#include <RefString.hpp>
#include <Stream.h>
#include <wrand.h>
#include <bzhash.h>
#include <vector>
using namespace std;

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <stdlib.h>
#include <_ast.h>
#include <_memchk.h>
#include <Tick.h>
#include <Lock.h>
#include <Stream.h>
#include <QueueList.h>
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "ObjectAllocator.hpp"
//#include "SingleObjectAllocator.hpp"
#include "AppItnMsg.h"
#include <RefClass.hpp>
#include <RefString.hpp>
#include "EDPass.h"
#include <CustomSocket.h>
#include "DataPacket.hpp"
#include "DataPacketReader.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomServerSocket.h"
#include "CustomServerClientSocket.h"
#include "ServerDef.h"
#include "CustomJXServerClientSocket2.h"
#include "SQL.h"
#include "CustomLuaScript.h"
#include "CustomLuaConfig.h"

#include "SrvConfig.h"
#include "BackHttpUtility.h"
#include "BackHttpServer.h"
#include "BackClientSocket.h"
#include "BackServerSocket.h"
#include "FileLogger.h"

#include "LogType.h"
#include "DefExceptHander.h"

#include "BackServerClient.h"
#include "BackServer.h"

