// platform/os_win.h
// Definitions for unknown systems - useful to have the source compile
// on unexpected OSes. Should contain conservative choices.

// RPN calculator

// Sébastien Millet
// August 2009 - April 2010

#define SEP			'/'

const char *os_to_be_continued = "...";
const int os_to_be_continued_length = 3;

int os_get_size_of_newline() { return 1; }

void os_init() { E = new MyEncoding(MYENCODING_1BYTE); }

const string os_concatene(const string& base, const string& added) { return concatene(SEP, base, added); }

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
	dirs[OSF_SMALL_STACKRC] = os_concatene(dirs[OSD_USER_CONF], STACKRC_FILE);
	dirs[OSF_STACKRC] = os_concatene(dirs[OSD_USER_CONF], STACKRC_FILE);

	/*for (int i = OS_BEGIN; i != OS_END; i++) {
		debug_write_i("dir entry #%i", i);
		debug_write(dirs[i].c_str());
	}*/
}

OS_Dirs::~OS_Dirs() { }

