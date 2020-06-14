// ---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "user_interface.h"
#include "dm_main.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

// ---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner) {
	close_flag = false;
	offduty = true;
	by_hand = false;
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::Button1Click(TObject *Sender) {
	char *port = "12345";
	start_msgsrv("12345", 115200);
	offduty = false;
	by_hand = true;
	N1->Checked = true;
	N1->Enabled = false;
	N2->Checked = false;
	N2->Enabled = true;
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::Button2Click(TObject *Sender) {
	end_msgsrv();
	offduty = true;
	by_hand = true;
	N2->Checked = true;
	N2->Enabled = false;
	N1->Checked = false;
	N1->Enabled = true;
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::N4Click(TObject *Sender) {
	close_flag = true;
	Close();
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::FormCloseQuery(TObject *Sender, bool &CanClose) {
	CanClose = close_flag;
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::Timer1Timer(TObject *Sender) {
	//
	if (!by_hand) {
		if (offduty) {
			Button1Click((TObject *)(Button1));
			offduty = false;
		}
	}
}
// ---------------------------------------------------------------------------
