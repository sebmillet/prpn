// Compat.cpp
// Compatibility functions

// RPN calculator

// Sébastien Millet
// August 2009, May 2010

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <string>

// Portable sleep function...
// FIXME: raises the CPU at a 100%!!!

#ifdef HAVE_SLEEP
#include <unistd.h>
void my_sleep_seconds(const int& n) { sleep(n); }
#else // ! HAVE_SLEEP
#include <time.h>
void my_sleep_seconds(const int& n) {
	clock_t endsleep = endsleep = clock() + n * CLOCKS_PER_SEC;
	while (clock() < endsleep) {}
}
#endif

// String management functions, to avoid being messed up with non-ASCII characters.
// Assume UTF8 for now, but the helper functions my_* can package other
// underlying encodings if need be.

static size_t utf8_get_string_length(const char *sz) {
	const char *it = sz;
	size_t l = 0;
	unsigned char code;
	int code_sequence_size;
	while (*it != '\0') {
		code = static_cast<unsigned char>(0xff & static_cast<unsigned char>(*it));
		//cout << "code = " << code << endl;
		if (code < 0x80)
			code_sequence_size = 1;
		else if ((code >> 5) == 0x6)
			code_sequence_size = 2;
		else if ((code >> 4) == 0xe)
			code_sequence_size = 3;
		else if ((code >> 3) == 0x1e)
			code_sequence_size = 4;
		else
			  // Error, unknown code... we just keep going, silently
			code_sequence_size = 1;
		it += code_sequence_size;
		l++;
	}
	return l;
}

static void utf8_append_padl(std::string& s, const std::string& a, const size_t& pad_length) {
	long nb_spaces = static_cast<long>(pad_length) - static_cast<long>(utf8_get_string_length(a.c_str()));
	if (nb_spaces >= 1)
		s.append(nb_spaces, ' ');
	s.append(a);
}

static void utf8_append_padr(std::string& s, const std::string& a, const size_t& pad_length) {
	long nb_spaces = static_cast<long>(pad_length) - static_cast<long>(utf8_get_string_length(a.c_str()));
	s.append(a);
	if (nb_spaces >= 1)
		s.append(nb_spaces, ' ');
}

size_t my_get_string_length(const char *sz) { return utf8_get_string_length(sz); }
void my_append_padl(std::string& s, const std::string& a, const size_t& pad_length) {
	utf8_append_padl(s, a, pad_length);
}
void my_append_padr(std::string& s, const std::string& a, const size_t& pad_length) {
	utf8_append_padr(s, a, pad_length);
}
