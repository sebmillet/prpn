// platform/os_win.h
// Definitions for unknown systems - useful to have the source compile
// on unexpected OSes. Should contain conservative choices.

// RPN calculator

// Sébastien Millet
// August 2009 - April 2010

#include "Common.h"
#include <sys/stat.h>
#include <cstdio>

#define SEP			'/'

#define VARSRC_FILE			"varsrc"
#define VARSRC_FILE_ALT		"varsrc~"
#define STACKRC_FILE		"stackrc"
#define STACKRC_FILE_ALT	"stackrc~"

const char *os_to_be_continued = "...";
const int os_to_be_continued_length = 3;

int os_get_size_of_newline() { return 1; }

void os_init() { }

int os_get_default_encoding() { return ENCODING_UTF8; }

const string os_get_install_language() { return ""; }

const string os_concatene(const string& base, const string& added) { return concatene(SEP, base, added); }

static bool os_e_exists(const char *sz, const unsigned int& my_flag) {
	struct stat st;

	if (stat(sz, &st) != 0)
		return false;
    return ((st.st_mode & S_IFMT) == my_flag);
}

bool os_dir_exists(const char *sz) { return os_e_exists(sz, S_IFDIR); }
bool os_file_exists(const char *sz) { return os_e_exists(sz, S_IFREG); }

int os_dir_create(const char *sz) {
	return mkdir(sz, S_IRWXU | S_IRWXG);
}

int os_rename(const char *actual_name, const char* new_name) {
	return rename(actual_name, new_name);
}

OS_Dirs::OS_Dirs(const char *argv0) {
	dirs[OSF_RT_ARGV0] = argv0;
	my_split(argv0, SEP, dirs[OSD_RT_EXE], dirs[OSF_RT_EXE]);
	dirs[OSD_HC_PREFIX] = ".";
	dirs[OSD_HC_BIN] = ".";
	dirs[OSD_HC_DOC] = ".";
	dirs[OSD_HC_HTML] = ".";
	dirs[OSD_PKG_LOCALEDIR] = ".";
	dirs[OSD_USER_HOME] = ".";
	dirs[OSD_USER_CONF] = "./.prpn";
	dirs[OSF_SMALL_VARSRC] = os_concatene(dirs[OSD_USER_CONF], VARSRC_FILE);
	dirs[OSF_VARSRC] = os_concatene(dirs[OSD_USER_CONF], VARSRC_FILE);
	dirs[OSF_VARSRC_ALT] = os_concatene(dirs[OSD_USER_CONF], VARSRC_FILE_ALT);
	dirs[OSF_SMALL_STACKRC] = os_concatene(dirs[OSD_USER_CONF], STACKRC_FILE);
	dirs[OSF_STACKRC] = os_concatene(dirs[OSD_USER_CONF], STACKRC_FILE);
	dirs[OSF_STACKRC_ALT] = os_concatene(dirs[OSD_USER_CONF], STACKRC_FILE_ALT);

	/*for (int i = OS_BEGIN; i != OS_END; i++) {
		debug_write_i("dir entry #%i", i);
		debug_write(dirs[i].c_str());
	}*/
}

OS_Dirs::~OS_Dirs() { }

