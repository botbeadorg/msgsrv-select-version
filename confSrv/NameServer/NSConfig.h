#pragma once

class CNameServer;

class CNameServerConfig
	: public CCustomLuaConfig
{
public:
	typedef CCustomLuaConfig Inherited;
protected:
	void showError(LPCTSTR sError);
	bool readConfig(CNameServer *lpNameServer);
public:
	bool loadConfig(CNameServer *lpNameServer);
	void setup(CNameServer *);
};