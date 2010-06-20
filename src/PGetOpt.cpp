// vim:encoding=utf-8:

//
// PGetOpt.cpp
//
// v1.0.20100319
//

//
// Parse command-line options, in a portable way
// => no use of getopt.h, that's part of glibc. glibc' getopt
// is perfect... But it is specific to glibc.
//
// Sébastien Millet
// February, March 2010
//

#include <stdio.h>
#include "PGetOpt.h"

  // Use in conjunction with xgettext: invoque like this:
  // 	xgettext --keyword=_N ...
#define _N(s)	s

const char* err_strings[] = {
	"",
	_N("invalid option -- %s"),
	_N("unrecognized option `%s'"),
	_N("option requires an argument -- %s"),
	_N("option `%s' is ambiguous"),
	_N("trailing arguments%s")
};

using namespace pgetopt;

//
// POptions
//

Options::Options(const int& q) : opts(0), actual_entry(-1), opts_quantity(q) {
	if (opts_quantity != 0) {
		opts = new OptionEntry[opts_quantity];
	}
}

Options::~Options() {
	if (opts != 0)
		delete []opts;
}

void Options::next_actual_entry() {
	if (++actual_entry >= opts_quantity) {
		int previous_quantity = opts_quantity;
		opts_quantity += OPTS_STEP_INCREASE;
		OptionEntry *new_opts = new OptionEntry[opts_quantity];
		for (int i = 0; i < previous_quantity; i++)
			new_opts[i] = opts[i];
		if (previous_quantity >= 1)
			delete []opts;
		opts = new_opts;
	}
}

void Options::inner_add(const pgetopt::option_t& type, const char* short_name, const char* long_name, void* value) {
	next_actual_entry();
	OptionEntry *po = &opts[actual_entry];
	po->type = static_cast<option_t>(type);
	po->short_name = short_name;
	po->long_name = long_name;
	po->allow_no_prefix = false;
	po->append = false;
	if (po->long_name.length() >= 5 && po->long_name.at(0) == '[' && po->long_name.at(3) == ']') {
		po->allow_no_prefix = true;
		po->long_name.erase(0, 4);
	}
	po->pvalue = static_cast<void*>(value);
	po->count = 0;
}

void Options::add(const char* short_name, const char* long_name, bool* value) {
	inner_add(T_BOOL, short_name, long_name, static_cast<void*>(value));
}

void Options::add(const char* short_name, const char* long_name, std::string* value, const bool& append) {
	inner_add(T_STRING, short_name, long_name, static_cast<void*>(value));
	opts[actual_entry].append = append;
}

void Options::OptionEntry::set_T_STRING(const std::string& s) {
	if (type != T_STRING)
		throw(int(THROW_INTERNAL_001));
	if (append) {
		if (!static_cast<std::string*>(pvalue)->empty())
			static_cast<std::string*>(pvalue)->append(",");
		static_cast<std::string*>(pvalue)->append(s);
	} else
		*static_cast<std::string*>(pvalue) = s;
	count++;
}

void Options::OptionEntry::set_T_BOOL(const bool& b) {
	*static_cast<bool*>(pvalue) = b;
	count++;
}

int Options::parse(int argc, char** argv,
		int& nb_trailing_args, std::string*& trailing_args, const char*& format, std::string& item_error,
		const int& max_nb_trailing_args) {

	int nb_options = actual_entry + 1;

	nb_trailing_args = 0;
	trailing_args = 0;
	item_error = "";
	std::string param;
	std::string left_to_parse;
	std::string a;
	bool is_long = false;
	bool all_trailing = false;
	bool waiting_for_value = false;
	int waiting_for_value_on = -1;

	int trailing_args_quantity = 0;
	int c = RET_OK;
	item_error = "";

	if (nb_options == 0)
		return 0;

	int best_i = -1, best_n = -1;
	for (int ii = 1; ii < argc && c == RET_OK; ii++) {
		param = argv[ii];
		bool is_trailing = false;
		if (waiting_for_value) {
			opts[waiting_for_value_on].set_T_STRING(argv[ii]);
			waiting_for_value = false;
			continue;
		}
		if (all_trailing)
			is_trailing = true;
		else if (param.length() >= 3 && param.substr(0, 2) == "--") {
			param.erase(0, 2);
			left_to_parse = param;
			is_long = true;
		} else if (param == "--") {
			all_trailing = true;
			left_to_parse = "";
		} else if (param.length() >= 2 && param.at(0) == '-') {
			param.erase(0, 1);
			left_to_parse = param;
			is_long = false;
		} else {
			is_trailing = true;
		}
		if (is_trailing) {
			nb_trailing_args++;
			if (nb_trailing_args > trailing_args_quantity) {
				int previous_quantity = trailing_args_quantity;
				trailing_args_quantity += TRAILING_ARGS_STEP_INCREASE;
				std::string* new_trailing_args = new std::string[trailing_args_quantity];
				for (int i = 0; i < previous_quantity; i++)
					new_trailing_args[i] = trailing_args[i];
				if (previous_quantity >= 1)
					delete []trailing_args;
				trailing_args = new_trailing_args;
			}
			trailing_args[nb_trailing_args - 1] = argv[ii];
		} else {
			int na, n;
			while (!left_to_parse.empty()) {
				if (is_long) {
					a = left_to_parse;
					left_to_parse = "";
				} else {
					a = left_to_parse.substr(0, 1);
					left_to_parse.erase(0, 1);
				}
				na = a.length();
				best_i = -1;
				best_n = -1;
				bool bv = true;
				for	(	int double_loop = 0;
						double_loop == 0 || (best_i < 0 && double_loop == 1 && is_long && na >=3 && a.substr(0, 2) == "no");
						double_loop++) {
					bool is_neg = (double_loop == 1 && is_long && na >= 3 && a.substr(0, 2) == "no");
					bool exact_match = false;
					for (int i = 0; i < nb_options; i++) {
						n = opts[i].long_name.length();
						if (!is_long && opts[i].short_name == a) {
							best_i = i;
							best_n = 1;
							bv = true;
							break;
						} else if (is_long && n >= 1 &&
								(	opts[i].long_name.substr(0, na) == a
								 		||
									(is_neg && n >= 1 && opts[i].allow_no_prefix && opts[i].long_name.substr(0, na - 2) == a.substr(2))
								)) {
							if (na > best_n || (double_loop == 0 && na == n) ||
									(double_loop == 1 && na - 2 == n)) {
								best_i = i;
								best_n = na;
								bv = (double_loop == 0);
								if ((double_loop == 0 && na == n) || (double_loop == 1 && na == n + 2))
									exact_match = true;
							} else if (na == best_n && !exact_match) {
								best_i = -1;
							}
						}
					}
				}

				//std::cout << "best_i = " << best_i << ", best_n = " << best_n << std::endl;

				if (best_n >= 1 && best_i < 0) {
					c = RET_AMBIGUOUS_OPTION;
					item_error = argv[ii];
					break;
				}
				if (!is_long && best_n == 1 && best_i >= 0 && opts[best_i].type == T_STRING && !left_to_parse.empty()) {
					opts[best_i].set_T_STRING(left_to_parse);
					left_to_parse.erase();
				} else if (best_n >= 1 && best_i >= 0 && opts[best_i].type == T_STRING) {
					waiting_for_value = true;
					waiting_for_value_on = best_i;
					item_error = a;
				} else if (best_n >= 1 && best_i >= 0 && opts[best_i].type == T_BOOL) {
					opts[best_i].set_T_BOOL(bv);
				} else {
					if (is_long) {
						c = RET_UNKNOWN_LONG_OPTION;
						item_error = argv[ii];
					} else {
						c = RET_UNKNOWN_SHORT_OPTION;
						item_error = a;
					}
					break;
				}
			}
		}
	}
	if (waiting_for_value) {
		c = RET_OPTION_REQUIRES_AN_ARGUMENT;
		item_error = (opts[best_i].short_name.empty() ? opts[best_i].long_name : opts[best_i].short_name);
	}

	if (c == RET_OK) {
		if (max_nb_trailing_args >= 0 && nb_trailing_args > max_nb_trailing_args) {
			c = RET_TRAILING_ARGS;
			item_error = "";
		}
	}
	if (c != RET_OK) {
		format = err_strings[c];
	}

	return c;
}

