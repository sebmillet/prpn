// platform/os_unix.h
// Definitions for unix-line systems

// RPN calculator

// Sébastien Millet
// August 2009 - April 2010

#include "Common.h"
#include <cstdlib>
#include <sys/stat.h>

#define CODE_BIN		"bin"
#define CODE_DOC		"share/doc"
#define CODE_HTML		"share/doc"
#define SEP				'/'
#define USER_CONF		".prpn"
#define VARSRC_FILE		"varsrc"
#define STACKRC_FILE	"stackrc"

const char *os_to_be_continued = "┄";
const int os_to_be_continued_length = 1;
int os_get_size_of_newline() { return 1; }

void os_init() { }

int os_get_default_encoding() { return ENCODING_UTF8; }

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

OS_Dirs::OS_Dirs(const char *argv0) {
	dirs[OSF_RT_ARGV0] = argv0;
	my_split(argv0, SEP, dirs[OSD_RT_EXE], dirs[OSF_RT_EXE]);
	dirs[OSD_HC_PREFIX] = PKG_PREFIX;
	dirs[OSD_HC_BIN] = PKG_BINDIR;
	dirs[OSD_HC_DOC] = PKG_DOCDIR;
	dirs[OSD_HC_HTML] = PKG_HTMLDIR;
	dirs[OSD_PKG_LOCALEDIR] = PKG_LOCALEDIR;

	const char *h = getenv("HOME");
	string p;
	if (h != NULL) {
		dirs[OSD_USER_HOME] = h;
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

