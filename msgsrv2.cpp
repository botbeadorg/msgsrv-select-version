// ---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
#include <registry.hpp>
// ---------------------------------------------------------------------------

#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>
USEFORM("user_interface.cpp", Form1);
USEFORM("dm_main.cpp", Controller); /* TDataModule: File Type */
void start_with_windows();

// ---------------------------------------------------------------------------
WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
	try {
		Application->Initialize();
		UnicodeString title = "Message Server";
		Application->Title = title;
		CreateMutex(NULL, false, title.c_str());
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			return 0;
		}
		start_with_windows();
		Application->MainFormOnTaskBar = true;
		Application->CreateForm(__classid(TForm1), &Form1);
		Application->CreateForm(__classid(TController), &Controller);
		Application->ShowMainForm = false;
		Application->Run();
	}
	catch (Exception &exception) {
		Application->ShowException(&exception);
	}
	catch (...) {
		try {
			throw Exception("");
		}
		catch (Exception &exception) {
			Application->ShowException(&exception);
		}
	}
	return 0;
}

// ---------------------------------------------------------------------------
void start_with_windows() {
	TRegistry *reg = new TRegistry();
	reg->RootKey = HKEY_LOCAL_MACHINE;
	reg->OpenKey("\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", true);
	if (!reg->ValueExists("MsgSrv")) {
		reg->WriteString("MsgSrv", GetCurrentDir() + "\\" + ExtractFileName
			(Application->ExeName));
	}
	reg->CloseKey();
	delete reg;
}
