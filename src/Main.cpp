// Main.cpp
// Main program body

// RPN calculator

// Sébastien Millet
// August 2009, May 2010

#include "Common.h"
#include "Parser.h"
#include "Stack.h"
#include "Variable.h"
#include "Scalar.h"
#include "PGetOpt.h"
#include <string>
#include <limits>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>

#include "platform/os_generic.h"

OS_Dirs *osd;

#include "MyIntl.h"

using namespace std;


//
// GLOBAL VARIABLES
//

int g_min_int;
int g_max_int;
  // Should be lower than, or equal to, G_HARD_MAX_NB_BITS
int g_max_nb_bits = 64;


//
// COMMAND-LINE OPTIONS
//

static bool opt_help = false;
static bool opt_version = false;

bool opt_console = false;
bool opt_batch = false;
static string opt_s_width;
static string opt_s_height;
static string opt_s_min_stack_height;
int opt_width = -1;
int opt_height = -1;
int opt_min_stack_height = -1;
bool debug_tokeniser = false;
bool debug_itemiser = false;
static string opt_debug;
bool has_gui = false;
bool opt_dry_run = false;

string html_help_file = "";
bool html_help_found = false;


//
// MAIN ROUTINES
//

static int check_class_count();

extern void ui_init();
extern void ui_terminate();

void debug_test_tokeniser();
void debug_test_itemiser();

char get_decimal_separator(const bool& user_setting) {
	return user_setting ? (flags[FL_DECIMAL_SEP].value ? ',' : '.') : '.';
}

char get_complex_separator(const bool& user_setting) {
	return user_setting ? (flags[FL_DECIMAL_SEP].value ? ';' : ',') : ',';
}

const string CalcFatal::get_description() const {
	std::ostringstream o;
	o << "Error caught, file: " << file << ", line: " << line << ", message: " << message;
	return o.str();
}

void usage() {
	std::cerr << _("Try `") << BIN_NAME << _(" --help' for more information.") << std::endl;
}

void help() {
	std::cerr << _("Usage: ") << BIN_NAME << _(" [options]...") << std::endl;
	std::cerr << "  -h  --help    Print this usage and exit" << std::endl;
	std::cerr << "  -v  --version Print version information and exit" << std::endl;
	std::cerr << "  -c  --console Run in text mode" << std::endl;
	std::cerr << "  -w  --width   Width of the display, in characters (default: 23)" << std::endl;
	std::cerr << "      --height  Height of the display, in lines (default: 5)" << std::endl;
	std::cerr << "      --min-stack-height Minimum height of the stack display, in lines" << std::endl;
	std::cerr << "                         Only in text mode" << std::endl;
	std::cerr << "  -b  --batch   Batch mode = display the stack once before exiting" << std::endl;
	std::cerr << "  -z  --dry-run Ignore rc files" << std::endl;
	std::cerr << "  -d  --debug <debug options> Turn debugging on" << std::endl;
}

void version() {
	std::cerr << PACKAGE_STRING << std::endl;
	std::cerr << "Copyright (C) 2010 Sébastien Millet" << std::endl;
	std::cerr << "This program is free software; you may redistribute it under the terms of" << std::endl;
	std::cerr << "the GNU General Public License version 3 or (at your option) any later version." << std::endl;
	std::cerr << "This program has absolutely no warranty." << std::endl;


}

static int string_to_integer(const string& s) { return atoi(s.c_str()); }

const string integer_to_string(const int& i) {
	ostringstream oss;
	oss << i;
	return oss.str();
}

static void read_opt_string_to_integer(const string& opt, int& val, const bool& enforce_min_value = false, const int& min_value = 0) {
	if (!opt.empty()) {
		val = string_to_integer(opt);
		if (enforce_min_value && val < min_value)
			val = min_value;
	}
}

int prog_init(int argc, char **argv) {
	osd = new OS_Dirs(argv[0]);

// Initialize gettext part
#if ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, PKG_LOCALEDIR);
	textdomain(PACKAGE);
#endif

	html_help_file = os_concatene(osd->get_dir(OSD_HC_HTML), MY_HELP_HTML);
	html_help_found = os_file_exists(html_help_file.c_str());

	if (!html_help_found) {
		debug_write("html file not found");
	} else {
		debug_write("html file found");
		debug_write(html_help_file.c_str());
	}

	if (!os_dir_exists(osd->get_dir(OSD_USER_CONF).c_str())) {
		debug_write("Creating personal conf directory of the following name:");
		debug_write(osd->get_dir(OSD_USER_CONF).c_str());
		if (os_dir_create(osd->get_dir(OSD_USER_CONF).c_str()) != 0) {
			debug_write("Could not create personal config directory");
		}
	} else {
		debug_write("Personal conf directory exists");
	}

	g_min_int = numeric_limits<int>::min();
	g_max_int = numeric_limits<int>::max();

	pgetopt::Options opt;
	const char *format;
	string item_error;
	int nb_trailing_args;
	string* trailing_args;

	opt.add("h", "help", &opt_help);
	opt.add("v", "version", &opt_version);
	opt.add("c", "console", &opt_console);
	opt.add("w", "width", &opt_s_width);
	opt.add("", "height", &opt_s_height);
	opt.add("", "min-stack-height", &opt_s_min_stack_height);
	opt.add("b", "batch", &opt_batch);
	opt.add("z", "dry-run", &opt_dry_run);
	opt.add("d", "debug", &opt_debug);

	int r = opt.parse(argc, argv, nb_trailing_args, trailing_args, format, item_error, 0);
	if (r != pgetopt::RET_OK) {
		char buf[200];
		snprintf(buf, sizeof(buf), _(format), item_error.c_str());
		std::cerr << BIN_NAME << ": " << buf << std::endl;
		usage();
		return PROG_INIT_EXIT;
	}

	if (opt_help) {
		help();
		return PROG_INIT_EXIT;
	} else if (opt_version) {
		version();
		return PROG_INIT_EXIT;
	}

	if (opt_debug != "") {
		cerr << "Debug variable set to " << opt_debug << endl;
		debug_itemiser = (opt_debug.find_first_of('i') != string::npos);
		debug_tokeniser = (opt_debug.find_first_of('t') != string::npos);
	}

	has_gui = (!opt_batch && !opt_console && !debug_itemiser && !debug_tokeniser);

	if (!opt_s_min_stack_height.empty() && has_gui) {
		std::cerr << BIN_NAME << ": --min_stack_height option allowed in console mode only" << std::endl;
		usage();
		return PROG_INIT_EXIT;
	}

	read_opt_string_to_integer(opt_s_height, opt_height, true, -1);
	read_opt_string_to_integer(opt_s_min_stack_height, opt_min_stack_height, true, -1);
	read_opt_string_to_integer(opt_s_width, opt_width, true, -1);
	if (opt_batch) {
		opt_height = 0;
		opt_min_stack_height = 0;
	}

	os_init();

	ui_init();

	if (!has_gui)
		return PROG_INIT_START_CONSOLE;
	else
		return PROG_INIT_START_GUI;
}

int prog_terminate() {
	ui_terminate();
	if (osd != NULL)
		delete osd;
	return check_class_count();
}

static int check_class_count() {
#ifdef DEBUG_CLASS_COUNT
	if (StackItem::get_class_count() + Stack::get_class_count() + NodeStack::get_class_count() + Real::get_class_count()
			+ Cplx::get_class_count() + ReadMatrixCell::get_class_count() + CCMatrix::get_class_count()
			+ ToString::get_class_count() + Var::get_class_count() + Tree::get_class_count() + Binary::get_class_count() + TransStack::get_class_count() == 0) {
		std::cout << "get_class_count() = 0 for watched classes" << endl;
		return 0;
	} else {
		std::cout << "TransStack class_count: " << TransStack::get_class_count() << std::endl;
		std::cout << "StackItem class_count: " << StackItem::get_class_count() << std::endl;
		std::cout << "Stack class_count: " << Stack::get_class_count() << std::endl;
		std::cout << "NodeStack class_count: " << NodeStack::get_class_count() << std::endl;
		std::cout << "Real class_count: " << Real::get_class_count() << std::endl;
		std::cout << "Cplx class_count: " << Cplx::get_class_count() << std::endl;
		std::cout << "ReadMatrixCell class_count: " << ReadMatrixCell::get_class_count() << std::endl;
		std::cout << "CCMatrix class_count: " << CCMatrix::get_class_count() << std::endl;
		std::cout << "ToString class_count: " << ToString::get_class_count() << std::endl;
		std::cout << "Var class_count: " << Var::get_class_count() << std::endl;
		std::cout << "Tree class_count: " << Tree::get_class_count() << std::endl;
		std::cout << "Binary class_count: " << Binary::get_class_count() << std::endl;
		return 99;
	}
#else	// !DEBUG_CLASS_COUNT
	return 0;
#endif	// DEBUG_CLASS_COUNT
}

#ifndef FOREIGN_MAIN
extern void console_loop();
int main(int argc, char** argv) {
	int c = 1;
	if (prog_init(argc, argv) != PROG_INIT_EXIT) {
		console_loop();
		c = prog_terminate();
	}
	return c;
}
#endif


//
// DEBUGGING CODE
//

void debug_test_tokeniser() {
	Tokeniser tokenise(&cin, TOSTRING_EDIT);
	string tok;
	ParserError par;
	bool r = true;
	int i;
	i = 0;
	while (r) {

		if (!(r = tokenise.get_token(par, tok)))
			break;

		cout << ":" << tok << ":  \t";
		if (++i % 4 == 0)
			cout << endl;
	}
	cout << endl;
}

void debug_test_itemiser() {
	bool r = true;
	Itemiser itemise(&cin, TOSTRING_EDIT);
	ParserError par;
	StackItem* si;
	while (r) {
		r = itemise.get_item(par, si);
		if (par.set)
			cout << "Syntax error: " << par.col_start << "-" << par.col_end << "," << par.line << endl;
		else if (r) {
			cout << "Item: " << simple_string(si) << endl;
			delete si;
		} else
			cout << "par.set = false and r = false" << endl;
	}
}

#ifdef DEBUG

#ifdef DEBUG_FILE
#include <fstream>

std::fstream fdebug;

static void my_debug_write(const char *s, const ios_base::openmode& m) {
	fdebug.open(DEBUG_OUTPUT_FILE, m);
	fdebug << s << std::endl;
	fdebug.close();
}

void debug_write(const char *s) {
	my_debug_write(s, fstream::out | fstream::app);
}

void debug_write_truncate(const char *s) {
	my_debug_write(s, fstream::out | fstream::app | fstream::trunc);
}

#else	// !DEBUG_FILE

void debug_write(const char *s) {
	std::cout << s << std::endl;
}

void debug_write_truncate(const char *s) {
	debug_write(s);
}


#endif	// DEBUG_FILE

#include <string.h>
void debug_write_i(const char *s, const int& i) {
	char *cz;
	int n = strlen(s) + 30;
	cz = new char[n + 1];
	snprintf(cz, n, s, i);
	debug_write(cz);
	delete []cz;
}

#endif	// DEBUG

