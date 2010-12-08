// Parser.cpp
// Convert a stream into StackItem*'s

// RPN calculator

// Sébastien Millet
// August-September 2009

#include "Common.h"
#include "Parser.h"
#include "Scalar.h"
#include "Stack.h"
#include "Variable.h"
#include <sstream>
#include <fstream>

#ifdef DEBUG
#include <iostream>
#endif

using namespace std;

//extern BuiltinCommandDescriptor builtinCommands[];
//extern const unsigned int sizeof_builtinCommands;
extern int os_get_size_of_newline();

const bool late_eof = true;


//
// TOKENS
//

bool Tokeniser::get_token(ParserError& par, string& tok) {

	// Automat definition
	typedef enum {TK_NUL, TK_WRD, TK_STR, TK_EXP, TK_INF, TK_SUP, TK_BIN, TK_END}
		status_t;
	enum {CT_BLANK, CT_WORD, CT_INF, CT_SUP, CT_APOST, CT_DBLQT, CT_DIESE, CT_DEFAULT, CT_END}
		ct;
	struct automat_cell {
		status_t next_status;
		bool rec_char;
	};
	static const automat_cell automat[7][9] = {
		//CT_BLANK   CT_WORD    CT_INF     CT_SUP     CT_APOST   CT_DBLQT   CT_DIESE   CT_DEFAULT CT_END
		{{TK_NUL,0},{TK_WRD,1},{TK_INF,1},{TK_SUP,1},{TK_EXP,1},{TK_STR,1},{TK_BIN,1},{TK_END,1},{TK_END,0}}, // TK_NUL
		{{TK_END,0},{TK_WRD,1},{TK_WRD,1},{TK_WRD,1},{TK_END,0},{TK_END,0},{TK_END,0},{TK_END,0},{TK_END,0}}, // TK_WRD
		{{TK_STR,1},{TK_STR,1},{TK_STR,1},{TK_STR,1},{TK_STR,1},{TK_END,1},{TK_STR,1},{TK_STR,1},{TK_END,0}}, // TK_STR
		{{TK_EXP,1},{TK_EXP,1},{TK_EXP,1},{TK_EXP,1},{TK_END,1},{TK_EXP,1},{TK_EXP,1},{TK_EXP,1},{TK_END,0}}, // TK_EXP
		{{TK_END,0},{TK_WRD,1},{TK_END,1},{TK_END,1},{TK_END,0},{TK_END,0},{TK_END,0},{TK_END,0},{TK_END,0}}, // TK_INF
		{{TK_END,0},{TK_WRD,1},{TK_END,0},{TK_END,1},{TK_END,0},{TK_END,0},{TK_END,0},{TK_END,0},{TK_END,0}}, // TK_SUP
		{{TK_WRD,1},{TK_WRD,1},{TK_END,0},{TK_END,0},{TK_END,0},{TK_END,0},{TK_END,0},{TK_END,0},{TK_END,0}}  // TK_BIN
	};
	// End of automat definition

	char decimal_separator = F->get_decimal_separator(t != TOSTRING_PORTABLE);
	int size_of_newline = os_get_size_of_newline();

	//cout << "Tokeniser::get_token(): entering function\n";

	if (!c_exists)
		if (!iss->eof()) {
			//cout << "Tokeniser::get_token(): reading one character\n";
			iss->get(actual_c);
			column++;
			//if (late_eof)
				c_exists = !iss->eof();
			//else
			//	c_exists = true;
		}

	bool rc;
	bool nl;
	par.col_start = string::npos;
	par.set = false;
	ostringstream oss("");
	status_t status = TK_NUL;
	while (status != TK_END && c_exists) {

		//cout << "T1: Tokeniser::get_token(): analyzing {" << static_cast<int>(actual_c) << "}\n";

		if (isalnum(actual_c) || actual_c == '+' || actual_c == '-' || actual_c == '_' || actual_c == decimal_separator ||
				actual_c == '?' || actual_c == '=' || actual_c == '%')
			ct = CT_WORD;
		else if (actual_c == '\'')
			ct = CT_APOST;
		else if (actual_c == '"')
			ct = CT_DBLQT;
		else if (actual_c == ' ' || actual_c == '\t' || actual_c == '\n' || actual_c == '\r')
			ct = CT_BLANK;
		else if (actual_c == '<')
			ct = CT_INF;
		else if (actual_c == '>')
			ct = CT_SUP;
		else if (actual_c == '#')
			ct = CT_DIESE;
		else
			ct = CT_DEFAULT;
		rc = automat[status][ct].rec_char;
		status = automat[status][ct].next_status;
		if (rc) {
			oss << actual_c;
			if (par.col_start == string::npos) {
				par.col_start = column;
				par.line = line;
				par.c_index = c_index;
			}
			par.col_end = column;
		}
		if (rc || status != TK_END) {
			if (iss->eof()) {
				//cout << "Tokeniser::get_token(): iss->eof() is true\n";
				c_exists = false;
			} else {
				//cout << "Tokeniser::get_token(): iss->eof() is false, will read another char\n";
				nl = ((actual_c == '\n' && prev_c != '\r') || actual_c == '\r');
				prev_c = actual_c;
				iss->get(actual_c);
				//if (late_eof)
					c_exists = !iss->eof();
				if (c_exists) {
					if (nl) {
						column = 0;
						line++;
						c_index += size_of_newline - 1;
					}
					column++;
					c_index++;
				}
			}
		}

		//debug_print_perror("Tokeniser::get_token() [1]", par);

	}
	tok = oss.str();
	if (par.col_start == string::npos) {
		par.col_start = column + 1;
		par.col_end = column + 1;
		par.line = line;
		par.c_index = c_index;
	}

	//cout << "Tokeniser::get_token(): leaving, return value = " << tok << "\n";
	//debug_print_perror("Tokeniser::get_token() [2]", par);

	return (tok != "");
}


//
// ELEMENTS
//

Element::Element() : pb(0) { }

Element::~Element() {
	if (pb)
		delete pb;
}

element_t string_to_real(string tok, real& r, const tostring_t& t) {
	int m_sign = 1;
	int e_sign = 1;
	string m_bc;
	string m_ac;
	string e = "";
	size_t p;

	char decimal_separator = F->get_decimal_separator(t != TOSTRING_PORTABLE);

	if (tok == "")
		return EL_ERROR;

	  // Signe
	if (tok.at(0) == '-') {
		m_sign = -1;
		tok.erase(0, 1);
	} else if (tok.at(0) == '+')
		tok.erase(0, 1);

	  // Mantisse
	if ((p = tok.find_first_of(decimal_separator)) != string::npos) {
		m_bc = tok.substr(0, p);
		m_ac = tok.substr(p + 1);
		if ((p = m_ac.find_first_of("eE")) != string::npos) {
			if ((e = m_ac.substr(p + 1)) == "")
				return EL_ERROR;
			m_ac = m_ac.substr(0, p);
		}
	} else {
		m_bc = tok;
		m_ac = "";
		if ((p = m_bc.find_first_of("eE")) != string::npos) {
			if ((e = m_bc.substr(p + 1)) == "")
				return EL_ERROR;
			m_bc = m_bc.substr(0, p);
		}
	}
	if (m_bc == "" && m_ac == "")
		return EL_ERROR;

	if (e != "") {
		if (e.at(0) == '-') {
			e_sign = -1;
			e.erase(0, 1);
		} else if (e.at(0) == '+')
			e.erase(0, 1);
		if (e == "")
			return EL_ERROR;
	}

	if (m_bc.find_first_not_of("0123456789") != string::npos ||
			m_ac.find_first_not_of("0123456789") != string::npos ||
			e.find_first_not_of("0123456789") != string::npos)
		return EL_ERROR;

	bool can_be_zero = (m_bc.find_first_not_of("0") == string::npos && m_ac.find_first_not_of("0") == string::npos);
	if (m_ac == "")
		m_ac = "0";

	if (e != "") {
		if (e_sign < 0)
			e = "-" + e;
		e = "E" + e;
	}
	string x = m_bc + PORTABLE_STRING_DECIMAL_SEPARATOR + m_ac + e;
	istringstream iss(x);
	prepare_arith();
	real r_tmp;
	iss >> r_tmp;
	if (real_check_bounds(can_be_zero, 1, r_tmp, r, true) != ST_ERR_OK)
		return EL_ERROR;
	if (m_sign < 0 && r != 0)
		r = -r;
	return EL_REAL;
}

int base_letter_to_int(const char& c) {
	switch (c) {
		case 'b':
			return 2;
		case 'o':
			return 8;
		case 'd':
			return 10;
		case 'h':
			return 16;
		default:
			return -1;
	}
}

void divide_by_2(string& n, const int& base) {
	int v;
	int carry_back = 0;

	//cout << "P2: Dividing by 2 the Digits: " << n << endl;

	for (string::iterator it = n.begin(); it != n.end(); it++) {
		v = digit_to_int(*it);
		*it = int_to_digit(v / 2 + carry_back);
		if (v % 2)
			carry_back = base / 2;
		else
			carry_back = 0;
	}
	//cout << "P2:  Result: " << n << endl;
	remove_leading_zeros(n);
	//cout << "P2:  Result: " << n << endl;
}

element_t string_to_binary(string tok, Binary*& pb, const int& nb_bits) {

	//cout << "P1: Will now analyse \"" << tok << "\"\n";

	bitset<G_HARD_MAX_NB_BITS>* pbits = new bitset<G_HARD_MAX_NB_BITS>;
	pb = 0;
	if (tok.length() < 2) {
		delete pbits;
		return EL_ERROR;
	}
	if (tok.at(0) != '#') {
		delete pbits;
		return EL_ERROR;
	}
	if (tok.at(1) == ' ')
		tok.erase(0, 2);
	else
		tok.erase(0, 1);
	if (tok.empty()) {
		delete pbits;
		return EL_ERROR;
	}

	int base = base_letter_to_int(tok.at(tok.length() - 1));
	if (base == -1)
		base = F->get_binary_format();
	else
		tok.erase(tok.length() - 1, 1);
	if (tok.empty()) {
		delete pbits;
		return EL_ERROR;
	}

	int v;
	for (string::iterator it = tok.begin(); it != tok.end(); it++) {
		v = digit_to_int(*it);
		if (v < 0 || v >= base) {
			delete pbits;
			return EL_ERROR;
		}
	}
	int p = 0;

	//cout << "P2: Base: " << base_int_to_letter(base) << endl;
	//cout << "P2: Digits: " << tok << endl;

	while (tok != "0") {
		//cout << "tok = \"" << tok << "\"" << endl;
		v = digit_to_int(tok.at(tok.length() - 1));
		(*pbits)[p++] = (v % 2 != 0);
		if (p >= nb_bits)
			break;
		divide_by_2(tok, base);
	}

	pb = new Binary(*pbits);

	//b.debug_cout_litteral("P2xxxx: ");

	return EL_BINARY;
}

bool Elementiser::get_element(ParserError& par, Element& element) {

	enum {COMP_NONE, COMP_WAITING_R1, COMP_WAITING_R2, COMP_WAITING_CLOSURE} comp_status;
	real r0 = 0.0;
	Binary* pb0 = 0;
	char c;

	char decimal_separator = F->get_decimal_separator(t != TOSTRING_PORTABLE);
	char complex_separator = F->get_complex_separator(t != TOSTRING_PORTABLE);

	if (!tok_exists)
		tok_exists = tokenise.get_token(pe, actual_tok);

	element.type = EL_NUL;
	element_t el;
	comp_status = COMP_NONE;

	//if (tok_exists) cout << "actual_tok = \"" << actual_tok << "\"\n";
	//if (!tok_exists) cout << "no actual_tok!\n";

	while (tok_exists && !pe.set && element.type == EL_NUL) {
		c = actual_tok.at(0);
		if ((c == '+' || c == '-') && actual_tok.length() >= 2) {
			c = actual_tok.at(1);
		}
		if (isdigit(c) || c == decimal_separator)
			el = string_to_real(actual_tok, r0, t);
		else if (c == '#') {
			el = string_to_binary(actual_tok, pb0, F->get_bin_size());
		} else if (isalpha(c) || c == '_' || c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '<' || c == '>' || c == '?' ||
				c == '{' || c == '}' || c == '%' || c == '=') {
			el = EL_WORD;
		} else if (c == '\'')
			el = EL_EXPRESSION;
		else if (c == '"')
			el = EL_STRING;
		else if (c == complex_separator && comp_status != COMP_NONE && actual_tok.length() == 1)
			el = EL_NUL;
		else
			el = EL_OTHER;
		switch (comp_status) {
			case COMP_NONE:
				if (actual_tok == "(")
					comp_status = COMP_WAITING_R1;
				else {
					element.s = actual_tok;
					element.type = el;
					element.r1 = r0;
					element.pb = pb0;
				}
				break;
			case COMP_WAITING_R1:
				if (el == EL_REAL) {
					element.r1 = r0;
					comp_status = COMP_WAITING_R2;
				} else if (el != EL_NUL)
					element.type = EL_ERROR;
				break;
			case COMP_WAITING_R2:
				if (el == EL_REAL) {
					element.r2 = r0;
					comp_status = COMP_WAITING_CLOSURE;
				} else if (el != EL_NUL)
					element.type = EL_ERROR;
				break;
			case COMP_WAITING_CLOSURE:
				if (actual_tok == ")")
					element.type = EL_COMPLEX;
				else if (el != EL_NUL)
					element.type = EL_ERROR;
				break;
		}
		prev_pe = pe;
		tok_exists = tokenise.get_token(pe, actual_tok);

		//if (tok_exists) cout << "actual_tok = \"" << actual_tok << "\"\n";
		//if (!tok_exists) cout << "no actual_tok!\n";

	}
	if (element.type == EL_NUL) {
		if (comp_status == COMP_WAITING_R1 || comp_status == COMP_WAITING_R2 ||
				(comp_status == COMP_WAITING_CLOSURE && tok_exists)) {
			element.type = EL_ERROR;
			if (!tok_exists)
				prev_pe = pe;
		} else if (comp_status == COMP_WAITING_CLOSURE)
			element.type = EL_COMPLEX;
	}
	par = prev_pe;
	if (element.type == EL_ERROR)
		par.set = true;
	return (element.type != EL_NUL);
}


//
// ITEMS
//

string branch_str[] = {
	"",			// IT_NUL
	"",			// IT_SI
	"->",		// IT_GET_VARS
	"{",		// IT_OPEN_LIST
	"<<",		// IT_OPEN_PROGRAM
	"}",		// IT_CLOSE_LIST
	">>",		// IT_CLOSE_PROGRAM
	"IF",		// IT_IF
	"THEN",		// IT_THEN
	"ELSE",		// IT_ELSE
//	"IFERR",	// IT_IFERR
	"WHILE",	// IT_WHILE
	"REPEAT",	// IT_REPEAT
	"DO",		// IT_DO
	"UNTIL",	// IT_UNTIL
	"FOR",		// IT_FOR
	"START",	// IT_START
	"NEXT",		// IT_NEXT
	"STEP",		// IT_STEP
	"END"		// IT_END
};
const unsigned int sizeof_branch_str = sizeof(branch_str) / sizeof(*branch_str);

  // Produces items made that can't contain other items
bool Itemiser::get_simple_item(ParserError& par, StackItem*& si, item_t& itt) {

	  // ITS for ITemiser State
	typedef enum {ITS_ATOMIC, ITS_VECTOR, ITS_MATRIX, ITS_MATRIX_INTERLINE} its_t;
	its_t its = ITS_ATOMIC;
	its_t what_is_it = ITS_ATOMIC;

	Element el;
	bool r;

	bool complex_seen = false;
	mat_read_t *pm = 0;
	int cur_line = -1000;
	int cur_column = -1000;
	int nb_columns = -1000;
	ReadMatrixCell rmc;

	itt = IT_SI;

	do {
		r = elementise.get_element(par, el);

		//cout << "** READING: el.s = " << el.s << endl;

		if (!r || par.set)
			break;

		if (its == ITS_ATOMIC) {
			if (el.type == EL_REAL)
				si = new StackItemReal(Real(el.r1));
			else if (el.type == EL_COMPLEX)
				si = new StackItemCplx(Cplx(el.r1, el.r2));
			else if (el.type == EL_BINARY)
				si = new StackItemBinary(*(el.pb));
			else if (el.type == EL_STRING || el.type == EL_EXPRESSION) {
				el.s.erase(0, 1);
				if (el.s.length() >= 1 &&
						((el.s.at(el.s.length() - 1) == '"' && el.type == EL_STRING) ||
						(el.s.at(el.s.length() - 1) == '\'' && el.type == EL_EXPRESSION)))
					el.s.erase(el.s.length() - 1, 1);
				if (el.type == EL_STRING)
					si = new StackItemString(el.s);
				else if (el.type == EL_EXPRESSION) {
					if (el.s.empty())
						par.set = true;
					else
						si = new StackItemExpression(el.s, true);
				}
			} else if (el.type == EL_WORD) {
				int builtin_id;
				size_t x;
				string uc_s = el.s;
				upper_case(uc_s);
				for (x = 0; x < sizeof_branch_str; x++) {
					if (branch_str[x] != "" && branch_str[x] == uc_s)
						break;
				}
				if (x < sizeof_branch_str) {
					itt = static_cast<item_t>(x);
				} else {
					builtin_id = identify_builtin_command(el.s);
					if (builtin_id >= 0)  {
						si = new StackItemBuiltinCommand(builtin_id);
					} else {
						si = new StackItemExpression(el.s, false);
					}
				}
			} else if (el.type == EL_OTHER && el.s == "[") {
				its = ITS_VECTOR;
				what_is_it = ITS_VECTOR;
				pm = new vector< vector<ReadMatrixCell>* >;
				cur_line = -1;
				cur_column = -1;
				nb_columns = 0;
				complex_seen = false;
			} else if (el.type != EL_NUL) {
				par.set = true;
			}
		} else {
			  // its != ITS_ATOMIC
			bool item_eaten = false;

			//cout << "Reading matrix... " << el.type << ": " << el.s << ", " << el.r1 << ", " << el.r2 << endl;

			if (its == ITS_VECTOR && el.type == EL_OTHER && el.s == "[" && cur_line == -1) {
				its = ITS_MATRIX;
				what_is_it = ITS_MATRIX;
				item_eaten = true;
			} else if (el.type == EL_OTHER && el.s == "]") {
				if (its == ITS_VECTOR && cur_line >= 0) {
					its = ITS_ATOMIC;
					item_eaten = true;
				} else if (its == ITS_MATRIX && cur_line >= 0 && cur_column >= 0) {
					if (cur_line == 0)
						nb_columns = cur_column + 1;
					if (cur_column == nb_columns -1) {
						its = ITS_MATRIX_INTERLINE;
						cur_column = -1;
						item_eaten = true;
					}
				} else if (its == ITS_MATRIX_INTERLINE && cur_line >= 0) {
					its = ITS_ATOMIC;
					item_eaten = true;
				}
			} else if ((its == ITS_VECTOR || its == ITS_MATRIX) && (el.type == EL_REAL || el.type == EL_COMPLEX)) {
				if (cur_line == -1 || cur_column == -1) {
					pm->push_back(new vector<ReadMatrixCell>);
					cur_line++;
					cur_column = -1;
				}
				cur_column++;
				if (nb_columns == 0 || cur_column <= nb_columns - 1) {
					if (el.type == EL_COMPLEX || complex_seen) {
						rmc.type = SCALAR_COMPLEX;
						if (el.type == EL_REAL)
							rmc.s = new Cplx(el.r1, 0);
						else {
							rmc.s = new Cplx(el.r1, el.r2);
							complex_seen = true;
						}
					} else if (el.type == EL_REAL) {
						rmc.type = SCALAR_REAL;
						rmc.s = new Real(el.r1);
					}
					pm->at(cur_line)->push_back(rmc);
					item_eaten = true;
				}
			} else if ((its == ITS_MATRIX || its == ITS_MATRIX_INTERLINE) && el.type == EL_OTHER && el.s == "[" && cur_line >= 0) {
				if (nb_columns == 0)
					nb_columns = cur_column + 1;
				if (its == ITS_MATRIX_INTERLINE || cur_column == nb_columns - 1) {
					its = ITS_MATRIX;
					cur_column = -1;
					item_eaten = true;
				}
			}
			if (!item_eaten)
				par.set = true;
		}
	} while (r && !par.set && its != ITS_ATOMIC);

	if (!r)
		par.set = false;

	if (!par.set && what_is_it != ITS_ATOMIC) {

		//cout << "Finalizing matrix building, cur_column = " << cur_column << ", cur_line = " << cur_line << ", nb_columns = " << nb_columns << endl;

		if ((its == ITS_VECTOR || its == ITS_MATRIX) && nb_columns >= 1 && cur_column != nb_columns - 1)
			par.set = true;
		else if (cur_line == -1)
			par.set = true;
		else {
			r = true;
			if (nb_columns < cur_column + 1)
				nb_columns = cur_column + 1;
			if (complex_seen) {
				si = new StackItemMatrixCplx(pm, what_is_it == ITS_VECTOR ? DIM_VECTOR : DIM_MATRIX, cur_line + 1, nb_columns);
			} else {

				//std::cout << "Creating matrix!!!" << endl;
				//std::cout << "nb_lines: " << cur_line + 1 << ", nb_columns: " << nb_columns << endl;

				si = new StackItemMatrixReal(pm, what_is_it == ITS_VECTOR ? DIM_VECTOR : DIM_MATRIX, cur_line + 1, nb_columns);
			}
		}
	}

	if (pm != 0) {
		Real* pr;
		Cplx* pc;
		ReadMatrixCell* pe;
		for (size_t i = 0; i < pm->size(); i++) {
			if (par.set) {
				for (size_t j = 0; j < pm->at(i)->size(); j++) {
					pe = &(pm->at(i)->at(j));
					if (pe->type == SCALAR_REAL) {
						pr = static_cast<Real*>(pe->s);
						delete pr;
					} else if (pe->type == SCALAR_COMPLEX) {
						pc = static_cast<Cplx*>(pe->s);
						delete pc;
					}
				}
			}
			delete pm->at(i);
		}
	}

	if (par.set || !r) {
		itt = IT_NUL;
		si = 0;
	}
	return (par.set ? true : r);
}

enum {GET_SI = 0,
	GET_VARS,					// -> ...
	GET_LIST,					// { }
	GET_PROGRAM,				// << >> or inside a branch command (if then, loop, etc.)
	GET_IF, GET_THEN, GET_ELSE,	// IF THEN ELSE END
	GET_WHILE, GET_REPEAT,		// WHILE REPEAT END
	GET_DO, GET_UNTIL,			// DO UNTIL END
	GET_LOOP,					// FOR/START NEXT/STEP
	GET_FOR_VARIABLE,			// Following the 'FOR' instruction
	GET_END,
	GET_ERROR};

  // Produces items of any type, including, meta-items (lists & programs & program structures) = those
  // that can contain items
bool Itemiser::get_item_recursive(ParserError& par, StackItem*& si0, VarDirectory*& vars0, int& status) {
	enum {AC_NUL, AC_RECORD, AC_NEW_VARS, AC_NEW_LIST, AC_NEW_PROGRAM, AC_NEW_IF, AC_CREATE_ELSE,
		AC_NEW_WHILE, AC_NEW_DO, AC_NEW_FOR, AC_NEW_START, AC_RECORD_NEXT, AC_RECORD_STEP, AC_RECORD_FOR_VARIABLE};
	struct automat_cell {
		int next_status;
		int action;
	};

	static const automat_cell automat[13][19] = {
																	// **** GET_SI
//IT_NUL             IT_SI                 IT_GET_VARS
{{GET_END, AC_NUL}, {GET_END, AC_RECORD}, {GET_VARS, AC_NEW_VARS},
   //IT_OPEN_LIST             IT_OPEN_PROGRAM                IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_LIST, AC_NEW_LIST}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                IT_THEN              IT_ELSE
	{GET_IF, AC_NEW_IF}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE                   IT_REPEAT
	{GET_WHILE, AC_NEW_WHILE}, {GET_ERROR, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_DO, AC_NEW_DO}, {GET_ERROR, AC_NUL},
   //IT_FOR                  IT_START
	{GET_FOR_VARIABLE, AC_NEW_FOR}, {GET_LOOP, AC_NEW_START},
   //IT_NEXT              IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_ERROR, AC_NUL}},
																	// **** GET_VARS
//IT_NUL               IT_SI                  IT_GET_VARS
{{GET_ERROR, AC_NUL}, {GET_VARS, AC_RECORD}, {GET_ERROR, AC_NUL},
   //IT_OPEN_LIST         IT_OPEN_PROGRAM                IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_ERROR, AC_NUL}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                IT_THEN              IT_ELSE
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE             IT_REPEAT
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_FOR                  IT_START
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_NEXT                 IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_ERROR, AC_NUL}},
																	// **** GET_LIST
//IT_NUL             IT_SI                  IT_GET_VARS
{{GET_END, AC_NUL}, {GET_LIST, AC_RECORD}, {GET_ERROR, AC_NUL},
   //IT_OPEN_LIST             IT_OPEN_PROGRAM                IT_CLOSE_LIST      IT_CLOSE_PROGRAM
	{GET_LIST, AC_NEW_LIST}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_END, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                    IT_THEN              IT_ELSE
	{GET_IF, AC_NEW_IF}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE                   IT_REPEAT
	{GET_WHILE, AC_NEW_WHILE}, {GET_ERROR, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_DO, AC_NEW_DO}, {GET_ERROR, AC_NUL},
   //IT_FOR                  IT_START
	{GET_FOR_VARIABLE, AC_NEW_FOR}, {GET_LOOP, AC_NEW_START},
   //IT_NEXT              IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_ERROR, AC_NUL}},
																	// **** GET_PROGRAM
//IT_NUL             IT_SI                     IT_GET_VARS
{{GET_END, AC_NUL}, {GET_PROGRAM, AC_RECORD}, {GET_VARS, AC_NEW_VARS},
   //IT_OPEN_LIST             IT_OPEN_PROGRAM                IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_LIST, AC_NEW_LIST}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_ERROR, AC_NUL}, {GET_END, AC_NUL},
   //IT_IF                    IT_THEN              IT_ELSE
	{GET_IF, AC_NEW_IF}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE                   IT_REPEAT
	{GET_WHILE, AC_NEW_WHILE}, {GET_ERROR, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_DO, AC_NEW_DO}, {GET_ERROR, AC_NUL},
   //IT_FOR                  IT_START
	{GET_FOR_VARIABLE, AC_NEW_FOR}, {GET_LOOP, AC_NEW_START},
   //IT_NEXT              IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_ERROR, AC_NUL}},
																	// **** GET_IF
//IT_NUL               IT_SI                IT_GET_VARS
{{GET_ERROR, AC_NUL}, {GET_IF, AC_RECORD}, {GET_VARS, AC_NEW_VARS},
   //IT_OPEN_LIST             IT_OPEN_PROGRAM                IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_LIST, AC_NEW_LIST}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                IT_THEN             IT_ELSE
	{GET_IF, AC_NEW_IF}, {GET_THEN, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE                   IT_REPEAT
	{GET_WHILE, AC_NEW_WHILE}, {GET_ERROR, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_DO, AC_NEW_DO}, {GET_ERROR, AC_NUL},
   //IT_FOR                  IT_START
	{GET_FOR_VARIABLE, AC_NEW_FOR}, {GET_LOOP, AC_NEW_START},
   //IT_NEXT              IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_ERROR, AC_NUL}},
																	// **** GET_THEN
//IT_NUL             IT_SI                  IT_GET_VARS
{{GET_END, AC_NUL}, {GET_THEN, AC_RECORD}, {GET_VARS, AC_NEW_VARS},
   //IT_OPEN_LIST             IT_OPEN_PROGRAM                IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_LIST, AC_NEW_LIST}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                IT_THEN              IT_ELSE
	{GET_IF, AC_NEW_IF}, {GET_ERROR, AC_NUL}, {GET_ELSE, AC_CREATE_ELSE},
   //IT_WHILE                   IT_REPEAT
	{GET_WHILE, AC_NEW_WHILE}, {GET_ERROR, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_DO, AC_NEW_DO}, {GET_ERROR, AC_NUL},
   //IT_FOR                  IT_START
	{GET_FOR_VARIABLE, AC_NEW_FOR}, {GET_LOOP, AC_NEW_START},
   //IT_NEXT              IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_END, AC_NUL}},
																	// **** GET_ELSE
//IT_NUL             IT_SI                  IT_GET_VARS
{{GET_END, AC_NUL}, {GET_ELSE, AC_RECORD}, {GET_VARS, AC_NEW_VARS},
   //IT_OPEN_LIST             IT_OPEN_PROGRAM                IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_LIST, AC_NEW_LIST}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                IT_THEN              IT_ELSE
	{GET_IF, AC_NEW_IF}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE                   IT_REPEAT
	{GET_WHILE, AC_NEW_WHILE}, {GET_ERROR, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_DO, AC_NEW_DO}, {GET_ERROR, AC_NUL},
   //IT_FOR                  IT_START
	{GET_FOR_VARIABLE, AC_NEW_FOR}, {GET_LOOP, AC_NEW_START},
   //IT_NEXT              IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_END, AC_NUL}},
																	// **** GET_WHILE
//IT_NUL               IT_SI                   IT_GET_VARS
{{GET_ERROR, AC_NUL}, {GET_WHILE, AC_RECORD}, {GET_VARS, AC_NEW_VARS},
   //IT_OPEN_LIST             IT_OPEN_PROGRAM                IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_LIST, AC_NEW_LIST}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                IT_THEN              IT_ELSE
	{GET_IF, AC_NEW_IF}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE                   IT_REPEAT
	{GET_WHILE, AC_NEW_WHILE}, {GET_REPEAT, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_DO, AC_NEW_DO}, {GET_ERROR, AC_NUL},
   //IT_FOR                  IT_START
	{GET_FOR_VARIABLE, AC_NEW_FOR}, {GET_LOOP, AC_NEW_START},
   //IT_NEXT              IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_ERROR, AC_NUL}},
																	// **** GET_REPEAT
//IT_NUL             IT_SI                    IT_GET_VARS
{{GET_END, AC_NUL}, {GET_REPEAT, AC_RECORD}, {GET_VARS, AC_NEW_VARS},
   //IT_OPEN_LIST             IT_OPEN_PROGRAM                IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_LIST, AC_NEW_LIST}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                IT_THEN              IT_ELSE
	{GET_IF, AC_NEW_IF}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE                   IT_REPEAT
	{GET_WHILE, AC_NEW_WHILE}, {GET_ERROR, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_DO, AC_NEW_DO}, {GET_ERROR, AC_NUL},
   //IT_FOR                  IT_START
	{GET_FOR_VARIABLE, AC_NEW_FOR}, {GET_LOOP, AC_NEW_START},
   //IT_NEXT              IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_END, AC_NUL}},
																	// **** GET_DO
//IT_NUL               IT_SI                IT_GET_VARS
{{GET_ERROR, AC_NUL}, {GET_DO, AC_RECORD}, {GET_VARS, AC_NEW_VARS},
   //IT_OPEN_LIST             IT_OPEN_PROGRAM                IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_LIST, AC_NEW_LIST}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                IT_THEN              IT_ELSE
	{GET_IF, AC_NEW_IF}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE                   IT_REPEAT
	{GET_WHILE, AC_NEW_WHILE}, {GET_ERROR, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_DO, AC_NEW_DO}, {GET_UNTIL, AC_NUL},
   //IT_FOR                  IT_START
	{GET_FOR_VARIABLE, AC_NEW_FOR}, {GET_LOOP, AC_NEW_START},
   //IT_NEXT              IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_ERROR, AC_NUL}},
																	// **** GET_UNTIL
//IT_NUL             IT_SI                   IT_GET_VARS
{{GET_END, AC_NUL}, {GET_UNTIL, AC_RECORD}, {GET_VARS, AC_NEW_VARS},
   //IT_OPEN_LIST             IT_OPEN_PROGRAM                IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_LIST, AC_NEW_LIST}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                IT_THEN              IT_ELSE
	{GET_IF, AC_NEW_IF}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE                   IT_REPEAT
	{GET_WHILE, AC_NEW_WHILE}, {GET_ERROR, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_DO, AC_NEW_DO}, {GET_ERROR, AC_NUL},
   //IT_FOR                          IT_START
	{GET_FOR_VARIABLE, AC_NEW_FOR}, {GET_LOOP, AC_NEW_START},
   //IT_NEXT              IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_END, AC_NUL}},
																	// **** GET_LOOP
//IT_NUL               IT_SI                  IT_GET_VARS
{{GET_ERROR, AC_NUL}, {GET_LOOP, AC_RECORD}, {GET_VARS, AC_NEW_VARS},
   //IT_OPEN_LIST             IT_OPEN_PROGRAM                IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_LIST, AC_NEW_LIST}, {GET_PROGRAM, AC_NEW_PROGRAM}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                IT_THEN              IT_ELSE
	{GET_IF, AC_NEW_IF}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE                   IT_REPEAT
	{GET_WHILE, AC_NEW_WHILE}, {GET_REPEAT, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_DO, AC_NEW_DO}, {GET_ERROR, AC_NUL},
   //IT_FOR                          IT_START
	{GET_FOR_VARIABLE, AC_NEW_FOR}, {GET_LOOP, AC_NEW_START},
   //IT_NEXT                    IT_STEP
	{GET_END, AC_RECORD_NEXT}, {GET_END, AC_RECORD_STEP},
   //IT_END
	{GET_ERROR, AC_NUL}},
																	// **** GET_FOR_VARIABLE
//IT_NUL               IT_SI                               IT_GET_VARS
{{GET_ERROR, AC_NUL}, {GET_LOOP, AC_RECORD_FOR_VARIABLE}, {GET_ERROR, AC_NUL},
   //IT_OPEN_LIST         IT_OPEN_PROGRAM      IT_CLOSE_LIST        IT_CLOSE_PROGRAM
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_IF                IT_THEN              IT_ELSE
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_WHILE             IT_REPEAT
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_DO                IT_UNTIL
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_FOR               IT_START
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_NEXT              IT_STEP
	{GET_ERROR, AC_NUL}, {GET_ERROR, AC_NUL},
   //IT_END
	{GET_ERROR, AC_NUL}},
};

	bool r = true;
	par.set = false;
	item_t itt;
	int action;
	StackItem* si = 0;
	VarDirectory* vars = 0;
	while (r && !par.set && status != GET_END && status != GET_ERROR) {

		r = get_simple_item(par, si, itt);
		action = automat[status][itt].action;

		//cout << "1: status = " << status << ", itt = " << itt << ", action = " << action << endl;

		if (action == AC_NEW_VARS) {
			vars = new VarDirectory(0, 0);
			si = 0;
		} else if (action == AC_NEW_LIST)
			si = new StackItemList();
		else if (action == AC_NEW_PROGRAM) {
			if (vars0 != 0)
				if (vars0->get_size() == 0)
					par.set = true;
			if (!par.set) {
				si = new StackItemProgram(true, vars0);
				vars0 = 0;
			}
		} else if (action == AC_NEW_IF)
			si = new StackItemBranchIf();
		else if (action == AC_CREATE_ELSE)
			dynamic_cast<StackItemBranch*>(si0)->add_item(2, 0);
		else if (action == AC_NEW_WHILE)
			si = new StackItemBranchWhile();
		else if (action == AC_NEW_DO)
			si = new StackItemBranchDo();
		else if (action == AC_NEW_FOR || action == AC_NEW_START) {
			si = new StackItemBranchLoop(action == AC_NEW_FOR ? LOOP_OPEN_FOR : LOOP_OPEN_START);
			if (action == AC_NEW_START) {
				dynamic_cast<StackItemBranchLoop*>(si)->set_loop_varname("");
			}
		} else if (action != AC_NUL && action != AC_RECORD && action != AC_RECORD_FOR_VARIABLE &&
			action != AC_RECORD_NEXT && action != AC_RECORD_STEP) {
			debug_write_i("action = %i", action);
			throw(CalcFatal(__FILE__, __LINE__, "Itemiser::get_item_recursive(): unknown action"));
		}

		if (!par.set && (action == AC_NEW_VARS || action == AC_NEW_LIST || action == AC_NEW_PROGRAM || action == AC_NEW_IF || action == AC_NEW_WHILE || action == AC_NEW_DO || action == AC_NEW_FOR || action == AC_NEW_START)) {
			int new_status = automat[status][itt].next_status;

			//cout << "1.5: entering reading of " << si << " (status = " << status << ", new_status = " << new_status << ")" << endl;

			get_item_recursive(par, si, vars, new_status);

			itt = IT_SI;
			r = true;
		}

		//cout << "2: status = " << status << ", itt = " << itt << ", action = " << action << endl;

		action = automat[status][itt].action;
		status = automat[status][itt].next_status;

		//cout << "forwarded: 3: status = " << status << ", itt = " << itt << ", action = " << action << endl;
		//cout << "si = " << si << ", si0 = " << si0 << endl;

		if (par.set)
			break;

		if (action == AC_RECORD) {
			if (status == GET_VARS) {
				if (si->is_unquoted_varname()) {
					VarFile* e = vars0->new_varfile(si->get_varname());
					if (e == 0)
						status = GET_ERROR;
					delete si;
				} else if (si->get_type() == SITYPE_PROGRAM) {
					si0 = si;
					status = GET_END;
				} else {
					status = GET_ERROR;
					delete si;
				}
			} else if (si0 != 0) {
				if (status == GET_LIST || status == GET_PROGRAM)
					dynamic_cast<StackItemMeta*>(si0)->append_item(si);
				else if (status == GET_IF)
					dynamic_cast<StackItemBranch*>(si0)->add_item(0, si);
				else if (status == GET_THEN)
					dynamic_cast<StackItemBranch*>(si0)->add_item(1, si);
				else if (status == GET_ELSE)
					dynamic_cast<StackItemBranch*>(si0)->add_item(2, si);
				else if (status == GET_WHILE)
					dynamic_cast<StackItemBranch*>(si0)->add_item(0, si);
				else if (status == GET_REPEAT)
					dynamic_cast<StackItemBranch*>(si0)->add_item(1, si);
				else if (status == GET_DO)
					dynamic_cast<StackItemBranch*>(si0)->add_item(0, si);
				else if (status == GET_UNTIL)
					dynamic_cast<StackItemBranch*>(si0)->add_item(1, si);
				else if (status == GET_LOOP)
					dynamic_cast<StackItemBranch*>(si0)->add_item(0, si);
				else
					throw(CalcFatal(__FILE__, __LINE__, "bad status while action == AC_RECORD"));
			} else
				si0 = si;
			si = 0;
		} else if (action == AC_RECORD_FOR_VARIABLE) {
			if (si->is_unquoted_varname()) {
				dynamic_cast<StackItemBranchLoop*>(si0)->set_loop_varname(si->get_varname());
			} else {
				status = GET_ERROR;
			}
			delete si;
			si = 0;
		} else if (action == AC_RECORD_NEXT || action == AC_RECORD_STEP) {
			dynamic_cast<StackItemBranchLoop*>(si0)->set_loop_close(
				action == AC_RECORD_NEXT ? LOOP_CLOSE_NEXT : LOOP_CLOSE_STEP);
		}
	} 

	//cout << "4: status = " << status << ", itt = " << itt << ", action = " << action << endl;
	//cout << "par.set = " << par.set << endl;
	//cout << "si = " << si << ", si0 = " << si0 << endl;

	if (status == GET_ERROR)
		par.set = true;
	if (par.set && si0 != 0) {
		delete si0;
		si0 = 0;
	}
	if (par.set && si != 0) {
		delete si;
		si = 0;
	}
	if (vars0 != 0)
		delete vars0;

	return (par.set ? true : r);
}

bool Itemiser::get_item(ParserError& par, StackItem*& si) {
	int status = GET_SI;
	si = 0;
	VarDirectory* vars = 0;
	bool r = get_item_recursive(par, si, vars, status);
	//cout << "status = " << status << endl;
	return (par.set ? true : r);
}

//
// FUNCTIONS
//

st_err_t parser_str_to(TransStack& ts, string str, string& cmd_err) {
	ParserError par;
	SIO s;

	istringstream* iss = new istringstream(str);
	Itemiser* itemise = new Itemiser(iss, TOSTRING_EDIT);

	st_err_t c = ST_ERR_OK;
	bool r;
	do {
		r = itemise->get_item(par, s.si);
		if (par.set)
			c = ST_ERR_SYNTAX_ERROR;
		else if (!r)
			break;
		else {
			s.ownership = TSO_OWNED_BY_TS;
			c = ts.do_push_eval(s, true, cmd_err);
		}
	} while (c == ST_ERR_OK);

	delete iss;
	delete itemise;

	return c;
}

st_err_t read_rc_file(TransStack* ts, const tostring_t& tostring, istream& is, const char *filename_display,
		const bool& do_eval, string& error_l1, string& error_l2, const int& limit_number_of_items) {
	st_err_t c = ST_ERR_OK;
	error_l1 = "";
	error_l2 = "";
	if (is.good()) {

// 1. Read items

		debug_write("Reading:");
		debug_write(filename_display);

		vector<SIO> vs;
		Itemiser* itemise = new Itemiser(&is, tostring);
		ParserError par = {false, 0, 0, 0, 0};
		SIO s;
		bool r = true;
		string cmd_err;
		int n = 0;
		while (!par.set && r && (limit_number_of_items < 0 || n < limit_number_of_items)) {
			r = itemise->get_item(par, s.si);
			if (!par.set && r) {
				vs.push_back(SIO(TSO_OUTSIDE_TS, s.si));

				debug_write_i("Item #%i", n);
				debug_write(simple_string(s.si).c_str());
			}
			n++;
		}
		delete itemise;

		debug_write_i("number of items read: %i", n);

		if (par.set) {
			cmd_err = "Syntax";
			c = ST_ERR_SYNTAX_ERROR;
		}
		if (par.set || (c != ST_ERR_OK && c != ST_ERR_EXIT)) {
			error_l1 = filename_display;
			error_l1.append(":");
			error_l2 = cmd_err + " Error, line " + integer_to_string(par.line);
		}

// 2. Send items in the stack, and evaluate them if do_eval is true

		if (c == ST_ERR_OK) {
			bool inside_undo_sequence = false;
			for (vector<SIO>::iterator it = vs.begin(); it != vs.end() && c == ST_ERR_OK; it++) {
				(*it).ownership = TSO_OWNED_BY_TS;
				if (do_eval) {
					c = ts->do_push_eval(*it, inside_undo_sequence, cmd_err);
					inside_undo_sequence = true;
				} else {
					ts->transstack_push((*it).si);
				}
			}
		}

		for (vector<SIO>::iterator it = vs.begin(); it != vs.end(); it++)
			(*it).cleanup();

	} else {
		c = ST_ERR_FILE_READ_ERROR;
	}
	return c;
}

