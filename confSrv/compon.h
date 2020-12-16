// ---------------------------------------------------------------------------

#ifndef componH
#define componH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include "DBAccess.hpp"
#include "SQLiteUniProvider.hpp"
#include "Uni.hpp"
#include "UniProvider.hpp"
#include <Data.DB.hpp>
#include "DAScript.hpp"
#include "UniScript.hpp"
#include "MemDS.hpp"

// ---------------------------------------------------------------------------
class TDataModule1 : public TDataModule {
__published: // IDE-managed Components
	TSQLiteUniProvider *SQLiteUniProvider1;
	TUniConnection *UniConnection1;
	TUniScript *UniScript1;
	TUniQuery *UniQuery1;

	void __fastcall DataModuleCreate(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TDataModule1(TComponent* Owner);
};

// ---------------------------------------------------------------------------
extern PACKAGE TDataModule1 *DataModule1;
extern String exe_path;
// ---------------------------------------------------------------------------
#endif
