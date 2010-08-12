// platform/os_generic.h
// Guess OS type and list declarations found in platform specific files

// RPN calculator

// Sébastien Millet
// August 2009 - April 2010


//
// VARIABLES DEFINED IN os_{systtype}.h FILE
//

#include "../Common.h"
#include <string>

#include "../MyString.h"

extern MyEncoding* E;

void os_init();
void os_terminate();

extern const char *os_to_be_continued;
extern const int os_to_be_continued_length;
int os_get_size_of_newline();
const std::string os_concatene(const std::string&, const std::string&);
bool os_dir_exists(const char *);
bool os_file_exists(const char *);
int os_dir_create(const char *);
int os_get_default_encoding();

const int OS_BEGIN = 0;
enum {OSF_RT_ARGV0 = 0,	// Ex.: /usr/local/bin/prpn
	OSF_RT_EXE,			// Ex.: prpn
	OSD_RT_EXE,			// EX.: /usr/local/bin
	OSD_HC_PREFIX,		// Ex.: /usr/local
	OSD_HC_BIN,			// EX.: /usr/local/bin
	OSD_HC_DOC,			// Ex.: /usr/local/share/doc/prpn
	OSD_HC_HTML,		// Ex.: /usr/local/share/doc/prpn
	OSD_USER_HOME,		// Ex.: ~
	OSD_USER_CONF,		// Ex.: ~/.prpn
	OSD_PKG_LOCALEDIR,	// Ex.: /usr/local/share/locale
	OSF_VARSRC,			// Ex.: /home/sebastien/.prpn/vars
	OSF_SMALL_VARSRC,	// Ex.: ~/.prpn/vars
	OSF_STACKRC,		// Ex.: /home/sebastien/.prpn/stack
	OSF_SMALL_STACKRC,	// Ex.: ~/.prpn/stack
	OS_END
};

class OS_Dirs {
	std::string dirs[OS_END];
	OS_Dirs(const OS_Dirs&);
public:
	OS_Dirs(const char *);
	virtual ~OS_Dirs();
	virtual const std::string get_dir(const int& d) {
		if (d >= OS_BEGIN && d < OS_END)
			return dirs[d];
		else
			throw(CalcFatal(__FILE__, __LINE__, "OS_Dirs::get_dir(): bad argument value"));
	}
};

extern OS_Dirs *osd;

//
// GUESS OS
//

// Check for Linux

#ifdef unix
#define PROG_UNIXLIKE
#endif
#ifdef __unix
#define PROG_UNIXLIKE
#endif
#ifdef linux
#define PROG_UNIXLIKE
#endif
#ifdef __linux
#define PROG_UNIXLIKE
#endif
#ifdef __unix__
#define PROG_UNIXLIKE
#endif
#ifdef __linux__
#define PROG_UNIXLIKE
#endif

// Check for Windows

#ifdef _WIN32
#define PROG_WINDOWS
#endif
#ifdef __WIN32__
#define PROG_WINDOWS
#endif
#ifdef _WIN64
#define PROG_WINDOWS
#endif
#ifdef __WIN64__
#define PROG_WINDOWS
#endif

  // Uncomment to test what happens if the OS is not identified
//#undef PROG_WINDOWS
//#undef PROG_UNIXLIKE


//
// Use guessed OS
//

#ifdef PROG_WINDOWS
#ifdef _MSC_VER

// Microsoft Visual C++ 2010 Express specific stuff

#define snprintf(buf, sizeof_buf, format, ...) \
	_snprintf_s(buf, sizeof_buf, _TRUNCATE, format, __VA_ARGS__)
#define strncpy(dest, source, count) \
	strncpy_s(dest, count, source, _TRUNCATE)

#endif
#endif
