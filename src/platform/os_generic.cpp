// platform/os_generic.cpp
// Guess OS with os_guess.h and and import appropriate definitions

// RPN calculator

// Sébastien Millet
// August 2009 - April 2010

#include "os_generic.h"

using namespace std;
  // Uncomment to test what happens if the OS is not identified
//#undef PROG_WINDOWS
//#undef PROG_UNIXLIKE

MyEncoding *E = NULL;

void os_terminate() {
	if (E != NULL)
		delete E;
}

const string concatene(const char& sep, const string& base, const string& added) {
	if (base.empty())
		return added;
	else if (base.at(base.length() - 1) == sep) {
		if (!added.empty() && added.at(0) == sep) {
			return base + added.substr(1);
		} else {
			return base + added;
		}
	} else if (!added.empty() && added.at(0) == sep) {
		return base + added;
	} else {
		return base + sep + added;
	}
}

void my_split(const string& f, const char& sep, string& left, string& right) {
	size_t pos = f.find_last_of(sep);
	if (pos == string::npos) {
		left = "";
		right = f;
	} else {
		left = f.substr(0, pos);
		right = f.substr(pos + 1);
	}
}

const std::string os_concatene(const std::string&, const std::string&);

#ifdef PROG_UNIXLIKE
#include "os_unix.h"
#define PROG_OS_OK
#else
#ifdef PROG_WINDOWS
#include "os_win.h"
#define PROG_OS_OK
#endif
#endif

#ifndef PROG_OS_OK
#include "os_unknown.h"
#endif

