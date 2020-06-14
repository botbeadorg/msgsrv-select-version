// ---------------------------------------------------------------------------

#ifndef manip_ini_fileH
#define manip_ini_fileH
// ---------------------------------------------------------------------------
#include <vcl.h>
#include <inifiles.hpp>

bool open_ini_file(TMemIniFile* * const file_obj,
	const String * const file_name);
bool read_string_value_from_ini(String * const str_gotten,
	TMemIniFile * const file_obj, const String * const section,
	const String * const key, const String & default_value = "");
bool write_string_value_to_ini(TMemIniFile * const file_obj,
	const String * const section, const String * const key,
	const String & value);
bool close_ini_file(TMemIniFile* *file_obj);
#endif
