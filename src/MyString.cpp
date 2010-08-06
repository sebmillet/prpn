//
// MyString.cpp
//
// Sébastien Millet, 2010
//
// Provides functions to manage either 1-byte or UTF-8 encoding with string objects
// 1-byte stands for any encoding that codes characters using a single byte: ASCII,
// latinx encodings...
//

#include "MyString.h"
#include <iostream>
#include <string.h>

using namespace std;


//
// MYENCODING CLASS
//

MyEncoding::MyEncoding(const myencoding_t& e) : actual_encoding(e) { }
MyEncoding::~MyEncoding() { }

int MyEncoding::utf8_get_char_size(const char *it) {
	if (it == NULL)
		return 0;
	if (*it == '\0')
		return 0;
	unsigned char code = static_cast<unsigned char>(0xff & static_cast<unsigned char>(*it));
	if (code < 0x80)
		return 1;
	else if ((code >> 5) == 0x6)
		return 2;
	else if ((code >> 4) == 0xe)
		return 3;
	else if ((code >> 3) == 0x1e)
		return 4;
	else
		  // Error, unknown code... we just keep going, silently
		return 1;
}

void MyEncoding::utf8_move_next_char(const char **pit) {
	int l = utf8_get_char_size(*pit);
	int a;
	for (a = 0; a < l; a++) {
		if ((*pit)[a] == '\0')
			break;
	}
	*pit += a;
}

size_t MyEncoding::utf8_get_string_length(const char *sz) {
	const char *it = sz;
	size_t l = 0;
	while (*it != '\0') {
		utf8_move_next_char(&it);
		l += 1;
	}
	return l;
}

void MyEncoding::utf8_append_padl(string& s, const string& a, const size_t& pad_length) {
	long nb_spaces = static_cast<long>(pad_length) - static_cast<long>(utf8_get_string_length(a.c_str()));
	if (nb_spaces >= 1)
		s.append(nb_spaces, ' ');
	s.append(a);
}

void MyEncoding::utf8_append_padr(string& s, const string& a, const size_t& pad_length) {
	long nb_spaces = static_cast<long>(pad_length) - static_cast<long>(utf8_get_string_length(a.c_str()));
	s.append(a);
	if (nb_spaces >= 1)
		s.append(nb_spaces, ' ');
}

void MyEncoding::utf8_transform_start_length(const string& s, size_t& start, size_t& length) {
	size_t l;
	int n;
	int a;

	  // Find real start according to UTF-8 encoding
	const char *it = s.c_str();
	size_t char_pos = 0;
	l = 0;
	while (*it != '\0' && char_pos < start) {
		n = utf8_get_char_size(it);
		for (a = 0; a < n; a++) {
			if (it[a] == '\0')
				break;
		}
		l += a;
		it += a;
		char_pos++;
	}
	size_t utf8_byte_start = l;

	  // Find real length according to UTF-8 encoding
	size_t nb_chars = 0;
	l = 0;
	while (*it != '\0' && nb_chars < length) {
		n = utf8_get_char_size(it);
		for (a = 0; a < n; a++) {
			if (it[a] == '\0')
				break;
		}
		l += a;
		it += a;
		nb_chars++;
	}

	  // "Return" values
	start = utf8_byte_start;
	length = l;
}

const string MyEncoding::utf8_substr(const string& s, size_t start, size_t length) {
	utf8_transform_start_length(s, start, length);
	return s.substr(start, length);
}

void MyEncoding::utf8_erase(string& s, size_t start, size_t length) {
	utf8_transform_start_length(s, start, length);
	s.erase(start, length);
}

void MyEncoding::ascii_append_padl(string& s, const string& a, const size_t& pad_length) {
	long nb_spaces = static_cast<long>(pad_length) - static_cast<long>(a.length());
	if (nb_spaces >= 1)
		s.append(nb_spaces, ' ');
	s.append(a);
}

void MyEncoding::ascii_append_padr(string& s, const string& a, const size_t& pad_length) {
	long nb_spaces = static_cast<long>(pad_length) - static_cast<long>(a.length());
	s.append(a);
	if (nb_spaces >= 1)
		s.append(nb_spaces, ' ');
}


//
// HELPER FUNCTIONS
// my_ functions package utf8_ functions, so that further developments
// (that manage other character sets) can be easily integrated.
//

size_t MyEncoding::get_string_length(const char *sz) {
	if (actual_encoding == MYENCODING_UTF8)
		return utf8_get_string_length(sz);
	 // actual_encoding == MYENCODING_1BYTE
	return strlen(sz);
}

void MyEncoding::append_padl(string& s, const string& a, const size_t& pad_length) {
	if (actual_encoding == MYENCODING_UTF8)
		return utf8_append_padl(s, a, pad_length);
	else if (actual_encoding == MYENCODING_1BYTE)
		return ascii_append_padl(s, a, pad_length);
}

void MyEncoding::append_padr(string& s, const string& a, const size_t& pad_length) {
	if (actual_encoding == MYENCODING_UTF8)
		return utf8_append_padr(s, a, pad_length);
	else if (actual_encoding == MYENCODING_1BYTE)
		return ascii_append_padr(s, a, pad_length);
}

const string MyEncoding::substr(const string& s, size_t start, size_t length) {
	if (actual_encoding == MYENCODING_UTF8)
		return utf8_substr(s, start, length);
	// actual_encoding == MYENCODING_1BYTE
	if (start > s.length())
		start = s.length();
	if (length > s.length())
		length = s.length();
	return s.substr(start, length);
}

void MyEncoding::erase(string& s, size_t start, size_t length) {
	if (actual_encoding == MYENCODING_UTF8)
		utf8_erase(s, start, length);
	else if (actual_encoding == MYENCODING_1BYTE) {
		if (start > s.length())
			start = s.length();
		if (length > s.length())
			length = s.length();
		s.erase(start, length);
	}
}

