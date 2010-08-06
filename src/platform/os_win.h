// platform/os_win.h
// Definitions for Windows systems

// RPN calculator

// Sébastien Millet
// August 2009 - April 2010

#include "../Common.h"
#include <windows.h>
#include <stdlib.h>
#include <shlobj.h>

  // FIXME - BCC55 include files don't know this constant...
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

#define SEP					'\\'
#define MAX_REGISTRY_KEY_VALUE_LENGTH	MAX_PATH
#define INSTDIR_HKEY		HKEY_LOCAL_MACHINE
#define INSTDIR_KEY_PATH	"SOFTWARE\\pRPN"
#define INSTDIR_KEY_NAME	"InstallationDirectory"
#define USER_CONF			"pRPN"
#define VARSRC_FILE			"varsrc"
#define STACKRC_FILE		"stackrc"

const char *os_to_be_continued = "...";
const int os_to_be_continued_length = 3;
int os_get_size_of_newline() { return 2; }

void os_init() { E = new MyEncoding(MYENCODING_1BYTE); }

extern const string os_concatene(const string& base, const string& added) {
	return concatene('\\', base, added);
}

static bool os_e_exists(const char *sz, const unsigned int& my_flag, const bool& reverse = false) {
	DWORD dwAttrs = GetFileAttributes(sz);
	if (dwAttrs == static_cast<DWORD>(INVALID_FILE_ATTRIBUTES))
		return false;
	return ((dwAttrs & my_flag) != my_flag ? reverse : !reverse);
}

bool os_dir_exists(const char *sz) { return os_e_exists(sz, FILE_ATTRIBUTE_DIRECTORY); }

bool os_file_exists(const char *sz) { return os_e_exists(sz, FILE_ATTRIBUTE_DIRECTORY, true); }

int os_dir_create(const char *sz) {
	int r = !CreateDirectory(sz, NULL);
	if (r != 0) {
		debug_write_i("Could not create dir: %i", GetLastError());
	}
	return r;
}

static LONG read_key_string(const HKEY& key_base, const string& key_path, const string& key_name,
		string& key_value) {
	char sz[MAX_REGISTRY_KEY_VALUE_LENGTH];
	DWORD size_sz = sizeof(sz) / sizeof(*sz);
	HKEY hKey;
	LONG res  = RegOpenKeyEx(key_base, key_path.c_str(), 0L, KEY_QUERY_VALUE, &hKey);
	if (res == ERROR_SUCCESS) {
		res = RegQueryValueEx(hKey, key_name.c_str(), NULL, NULL, (BYTE*)&sz[0], &size_sz);
		if (res == ERROR_SUCCESS)
			key_value = sz;
		RegCloseKey(hKey);
	}
	return res;
}

OS_Dirs::OS_Dirs(const char *argv0) {
	TCHAR szPath[MAX_PATH];

	dirs[OSF_RT_ARGV0] = argv0;

	if (GetModuleFileName(NULL, szPath, MAX_PATH)) {
		my_split(szPath, SEP, dirs[OSD_RT_EXE], dirs[OSF_RT_EXE]);
	} else {
		my_split(argv0, SEP, dirs[OSD_RT_EXE], dirs[OSF_RT_EXE]);
	}
	string inst_dir;
	if (read_key_string(INSTDIR_HKEY, INSTDIR_KEY_PATH, INSTDIR_KEY_NAME, inst_dir) == ERROR_SUCCESS) {
		dirs[OSD_HC_PREFIX] = inst_dir;
		dirs[OSD_HC_BIN] = inst_dir;
		dirs[OSD_HC_DOC] = inst_dir;
		dirs[OSD_HC_HTML] = inst_dir;
		dirs[OSD_PKG_LOCALEDIR] = inst_dir;
	} else {
		dirs[OSD_HC_PREFIX] = dirs[OSD_RT_EXE];
		dirs[OSD_HC_BIN] = dirs[OSD_RT_EXE];
		dirs[OSD_HC_DOC] = dirs[OSD_RT_EXE];
		dirs[OSD_HC_HTML] = dirs[OSD_RT_EXE];
		dirs[OSD_PKG_LOCALEDIR] = dirs[OSD_RT_EXE];
	}

	string p;
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath))) {
		dirs[OSD_USER_HOME] = szPath;
		p = "~";
	} else {
		dirs[OSD_USER_HOME] = ".";
		p = dirs[OSD_USER_HOME];
	}
	dirs[OSD_USER_CONF] = os_concatene(dirs[OSD_USER_HOME], USER_CONF);
	p = os_concatene(p, USER_CONF);
	dirs[OSF_SMALL_VARSRC] = os_concatene(p, VARSRC_FILE);
	dirs[OSF_VARSRC] = os_concatene(dirs[OSD_USER_CONF], VARSRC_FILE);
	dirs[OSF_SMALL_STACKRC] = os_concatene(p, STACKRC_FILE);
	dirs[OSF_STACKRC] = os_concatene(dirs[OSD_USER_CONF], STACKRC_FILE);

	/*for (int i = OS_BEGIN; i != OS_END; i++) {
		debug_write_i("dir entry #%i", i);
		debug_write(dirs[i].c_str());
	}*/
}

OS_Dirs::~OS_Dirs() { }

