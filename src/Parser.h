// Parser.h
// Convert a stream into StackItem*'s

// RPN calculator

// Sébastien Millet
// August-September 2009

#ifndef PARSER_H
#define PARSER_H

#include "Common.h"
#include <string>

class Binary;
class StackItem;
class VarDirectory;

void prepare_arith();
int get_bin_size_from_flags();

class TransStack;
st_err_t read_rc_file(TransStack*, const tostring_t&, const std::string&, const std::string&, const bool&, std::string&, std::string&, const int& = -1);


//
// ERRORS
//

struct ParserError {
	bool set;
	size_t col_start;
	size_t col_end;
	size_t line;
	size_t c_index;
};


//
// TOKENS
//

class Tokeniser {
	tostring_t t;
	std::istream* iss;
	size_t line;
	size_t column;
	size_t c_index;
	bool c_exists;
	char actual_c;
	char prev_c;
public:
	Tokeniser(std::istream* s, const tostring_t& tt) : t(tt), iss(s),
		line(1), column(0), c_index(0), c_exists(false) { }
	virtual ~Tokeniser() { }
	bool get_token(ParserError&, std::string&);
	size_t get_column() const { return column; }
	size_t get_line() const { return line; }
};


//
// ELEMENTS
//

typedef enum {EL_NUL, EL_ERROR, EL_BINARY, EL_REAL, EL_COMPLEX, EL_WORD, EL_EXPRESSION, EL_STRING, EL_OTHER} element_t;

element_t string_to_real(std::string, real&);

class Element {
public:
	element_t type;
	std::string s;
	real r1;
	real r2;
	Binary* pb;
	Element();
	~Element();
};

class Elementiser {
	tostring_t t;
	Tokeniser tokenise;
	bool tok_exists;
	std::string actual_tok;
	ParserError pe;
	ParserError prev_pe;
public:
	Elementiser(std::istream* s, const tostring_t& tt) : t(tt), tokenise(s, tt), tok_exists(false) { }
	virtual ~Elementiser() { }
	bool get_element(ParserError&, Element&);
};


//
// ITEMS
//

class Itemiser {
	Elementiser elementise;
public:
	Itemiser(std::istream* s, const tostring_t& t) : elementise(s, t) { }
	virtual ~Itemiser() { }
	bool get_simple_item(ParserError&, StackItem*&, item_t&);
	bool get_item_recursive(ParserError&, StackItem*&, VarDirectory*& vars0, int&);
	bool get_item(ParserError&, StackItem*&);
};

#endif // PARSER_H

