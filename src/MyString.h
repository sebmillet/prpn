//
// MyString.h
//
// Sébastien Millet, 2010
//
// Provides functions to manage UTF-8 encoding with string objects
//

#ifndef MYSTRING_H
#define MYSTRING_H

#include <string>

typedef enum {MYENCODING_1BYTE, MYENCODING_UTF8} myencoding_t;

class MyEncoding {
	myencoding_t actual_encoding;
	inline int utf8_get_char_size(const char *);
	void utf8_move_next_char(const char **);
	size_t utf8_get_string_length(const char *);
	void utf8_append_padl(std::string&, const std::string&, const size_t&);
	void utf8_append_padr(std::string&, const std::string&, const size_t&);
	void utf8_transform_start_length(const std::string&, size_t&, size_t&);
	const std::string utf8_substr(const std::string&, size_t, size_t);
	void utf8_erase(std::string&, size_t, size_t);
	void ascii_append_padl(std::string&, const std::string&, const size_t&);
	void ascii_append_padr(std::string&, const std::string&, const size_t&);
public:
	MyEncoding(const myencoding_t&);
	virtual ~MyEncoding();
	myencoding_t get_actual_encoding() const;
	size_t get_string_length(const char *);
	void append_padl(std::string&, const std::string&, const size_t&);
	void append_padr(std::string&, const std::string&, const size_t&);
	const std::string substr(const std::string&, size_t = 0, size_t = std::string::npos);
	void erase(std::string&, size_t = 0, size_t = std::string::npos);
};

#endif
