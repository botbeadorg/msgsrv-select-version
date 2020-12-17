// ---------------------------------------------------------------------------

#pragma hdrstop

#include "compon.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma classgroup "Vcl.Controls.TControl"
#pragma link "DBAccess"
#pragma link "SQLiteUniProvider"
#pragma link "Uni"
#pragma link "UniProvider"
#pragma link "DAScript"
#pragma link "UniScript"
#pragma link "MemDS"
#pragma resource "*.dfm"
TDataModule1 *DataModule1;
String exe_path;

// ---------------------------------------------------------------------------
__fastcall TDataModule1::TDataModule1(TComponent* Owner) : TDataModule(Owner) {
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule1::DataModuleCreate(TObject *Sender) {
        UniConnection1->ProviderName = L"SQLite";
        UniConnection1->SpecificOptions->Values[L"Direct"] = L"True";
        UniConnection1->SpecificOptions->Values[L"ForceCreateDatabase"] = L"True";
        UniConnection1->Database = exe_path + L"srvConf.db";
        UniConnection1->Connect();
        UniScript1->Connection = UniConnection1;
        UniScript1->SQL->Clear();
        UniScript1->SQL->Add("CREATE TABLE IF NOT EXISTS back ( \
spguid INTEGER NOT NULL,  \
utf8 INTEGER NOT NULL CHECK(utf8 = 1 or utf8 = 0), \
db_port INTEGER DEFAULT 3306, \
srv_port INTEGER NOT NULL CHECK(1 <= srv_port and srv_port <= 65535), \
srv_httpport INTEGER NOT NULL CHECK(1 <= srv_httpport and srv_httpport <= 65535), \
backsrv_port INTEGER NOT NULL CHECK(1 <= backsrv_port and backsrv_port <= 65535), \
db_usr TEXT DEFAULT 'root', \
db_name TEXT NOT NULL, \
db_pwd TEXT NOT NULL, \
db_domain TEXT DEFAULT 'localhost', \
srv_domain TEXT DEFAULT '0.0.0.0', \
backsrv_domain TEXT DEFAULT '0.0.0.0', \
srv_name TEXT NOT NULL, \
backsrv_name TEXT NOT NULL, \
backip TEXT DEFAULT '', \
rowid_2th INTEGER PRIMARY KEY AUTOINCREMENT );create table if not exists db( \
srv_name text not null, \
srv_index integer not null, \
boy_name_file text not null, \
girl_name_file text not null, \
gate_srv_ip text default '0.0.0.0', \
gate_srv_port integer not null, \
db_srv_ip text default '0.0.0.0', \
db_srv_port integer not null, \
log_srv_ip text default '127.0.0.1', \
log_srv_port integer not null, \
sesn_srv_ip text default '127.0.0.1', \
sesn_srv_port integer not null, \
name_srv_ip text default '127.0.0.1', \
name_srv_port integer not null, \
mysql_host text default 'localhost', \
mysql_port integer default 3306, \
mysql_usr text default 'root', \
mysql_pwd text not null, \
mysql_db text not null, \
utf8 integer default 1 check(utf8 = 1 or utf8 = 0), \
esqltool_path text not null, \
rowid_2th integer primary key autoincrement \
);create table if not exists name(\
spguid integer not null,\
srv_name text not null,\
name_srv_ip text default '0.0.0.0',\
name_srv_port integer not null check(1 <= name_srv_port and name_srv_port <= 65535),\
mysql_host text default 'localhost',\
mysql_port integer default 3306,\
mysql_usr text default 'root',\
mysql_pwd text not null,\
mysql_db text not null,\
utf8 integer default 1,\
rowid_2th integer primary key autoincrement\
);CREATE TABLE if not exists logic (\
srv_name TEXT NOT NULL,\
srv_index INTEGER NOT NULL,\
zone_open_time TEXT NOT NULL,\
zone_merge_time TEXT NOT NULL DEFAULT '',\
spid TEXT NOT NULL,\
spguid INTEGER NOT NULL DEFAULT 1,\
gate_srv_ip TEXT NOT NULL DEFAULT '127.0.0.1',\
gate_srv_port INTEGER NOT NULL,\
sesn_srv_ip TEXT NOT NULL,\
sesn_srv_port INTEGER NOT NULL,\
log_srv_ip TEXT NOT NULL,\
log_srv_port INTEGER NOT NULL,\
db_srv_ip TEXT NOT NULL DEFAULT '127.0.0.1',\
db_srv_port INTEGER NOT NULL,\
am_srv_ip TEXT NOT NULL DEFAULT '127.0.0.1',\
am_srv_port INTEGER NOT NULL,\
mgr_srv_ip TEXT NOT NULL DEFAULT '',\
mgr_srv_port INTEGER NOT NULL,\
locallog_srv_ip TEXT NOT NULL DEFAULT '127.0.0.1',\
locallog_srv_port INTEGER NOT NULL,\
CHECK (-1 < spguid and spguid < 256)\
);");
        UniScript1->Execute();
        UniQuery1->Connection = UniConnection1;
}
// ---------------------------------------------------------------------------
