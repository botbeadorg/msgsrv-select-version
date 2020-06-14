// ---------------------------------------------------------------------------

#pragma hdrstop

#include "manip_ini_file.h"

// ---------------------------------------------------------------------------
bool open_ini_file(TMemIniFile* * const file_obj,
	const String * const file_name) {
	if (!file_name || !file_obj)
		return false;
	*file_obj = new TMemIniFile(*file_name);
	if (!*file_obj)
		return false;
	return true;
}

bool read_string_value_from_ini(String * const str_gotten,
	TMemIniFile * const file_obj, const String * const section,
	const String * const key, const String & default_value) {
	if (!file_obj || !section || !key)
		return false;
	*str_gotten = file_obj->ReadString(*section, *key, default_value);
	return true;
}

bool write_string_value_to_ini(TMemIniFile * const file_obj,
	const String * const section, const String * const key,
	const String & value) {
	if (!file_obj || !section || !key)
		return false;
	file_obj->WriteString(*section, *key, value);
	return true;
}

bool close_ini_file(TMemIniFile* *file_obj) {
	if (!file_obj)
		return false;
	delete *file_obj;
	*file_obj = NULL;
	return true;
}
