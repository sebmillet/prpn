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
#include <cstdarg>

#include "platform/os_generic.h"

OS_Dirs *osd = NULL;

Flags *F = NULL;

#include "MyIntl.h"

using namespace std;


//
// GLOBAL VARIABLES
//

int g_min_int;
int g_max_int;
  // Should be lower than, or equal to, G_HARD_MAX_NB_BITS
int g_max_nb_bits = 64;

string actual_locale;
string actual_country_code;
int encoding;


//
// COMMAND-LINE OPTIONS
//

static bool opt_help = false;
static bool opt_version = false;

bool opt_console = false;
bool opt_batch = false;
bool opt_class_count = false;
static string opt_s_width;
static string opt_s_height;
static string opt_s_min_stack_height;
int opt_width = -1;
int opt_height = -1;
int opt_min_stack_height = -1;
bool debug_tokeniser = false;
bool debug_itemiser = false;
bool debug_locale = false;
static string opt_debug;
bool has_gui = false;
bool opt_dry_run = false;

string html_help_file = "";
bool html_help_found = false;


//
// MAIN ROUTINES
//

void upper_case(string& s) {
	for (string::iterator it = s.begin(); it != s.end(); it++)
		if (*it >= 'a' && *it <= 'z')
			*it = static_cast<char>(toupper(*it));
}

void lower_case(string& s) {
	for (string::iterator it = s.begin(); it != s.end(); it++)
		if (*it >= 'A' && *it <= 'Z')
			*it = static_cast<char>(tolower(*it));
}

static int check_class_count();

extern void ui_init();
extern void ui_terminate();

void debug_test_tokeniser();
void debug_test_itemiser();

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
	std::cerr << "  -h  --help    " << _("Print this usage and exit") << std::endl;
	std::cerr << "  -v  --version " << _("Print version information and exit") << std::endl;
	std::cerr << "  -c  --console " << _("Run in text mode") << std::endl;
	std::cerr << "  -w  --width   " << _("Width of the display, in characters (default: 23)") << std::endl;
	std::cerr << "      --height  " << _("Height of the display, in lines (default: 5)") << std::endl;
	std::cerr << "      --min-stack-height " << _("Minimum height of the stack display, in lines") << std::endl;
	std::cerr << "                         " << _("Only in text mode") << std::endl;
	std::cerr << "  -b  --batch   " << _("Batch mode = display the stack once before exiting") << std::endl;
	std::cerr << "  -z  --dry-run " << _("Ignore rc files") << std::endl;
	std::cerr << "  -a  --class " << _("Display class count before exiting") << std::endl;
	std::cerr << "  -d  --debug " << _("<debug options> Turn debugging on") << std::endl;
}

void version() {
	std::cerr << PACKAGE_STRING DEBUG_VERSION_STRING << std::endl;
	std::cerr << "Copyright (C) 2010, 2013 Sébastien Millet" << std::endl;
	std::cerr << "This program is free software; you may redistribute it under the terms of" << std::endl;
	std::cerr << "the GNU General Public License version 3 or (at your option) any later version." << std::endl;
	std::cerr << "This program has absolutely no warranty." << std::endl;


}

int string_to_integer(const string& s) { return atoi(s.c_str()); }

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

static int get_encoding_from_locale(const string& l) {
	string encoding = l;
	if (l.length() >= 3) {
		size_t p = l.find_last_of('.');
		if (p != string::npos) {
			encoding = l.substr(p + 1);
		}
	}
	upper_case(encoding);
	debug_write("encoding to parse = ");
	debug_write(encoding.c_str());
	if (encoding == "UTF8" || encoding == "UTF-8")
		return ENCODING_UTF8;
	if (encoding.substr(0, 5) == "LATIN" || encoding.substr(0, 9) == "ISO-8859-")
		return ENCODING_1BYTE;
	return ENCODING_UNKNOWN;
}

static const string get_country_code_from_locale(const string& l) {
	string r = "";
	if (l.length() >= 2)
		r = l.substr(0, 2);
	return r;
}

  // Define actual_country_code
  //
  // * IMPORTANT *
  //   osd must be instanciated before calling work_out_locale()
  //
static void work_out_locale() {
	if (osd == NULL)
		throw(CalcFatal(__FILE__, __LINE__, "work_out_locale(): osd not yet created!!!?"));

	actual_locale = "";
	actual_country_code = "";

// Initialize gettext part
#if ENABLE_NLS

#ifdef PRPN_WX_GUI
    const string get_wxlocale_canonicalname();
	const string wxs = get_wxlocale_canonicalname();
	const char *sz = wxs.c_str();
#else
	const char *sz = setlocale(LC_ALL, "");
#endif

	if (sz != NULL)
		actual_locale = sz;

	debug_write("actual_locale =");
	debug_write(actual_locale.c_str());

	actual_country_code = get_country_code_from_locale(actual_locale);
	encoding = get_encoding_from_locale(actual_locale);

	bindtextdomain(PACKAGE, osd->get_dir(OSD_PKG_LOCALEDIR).c_str());

	debug_write("text domain =");
	debug_write(PACKAGE);
	debug_write("OSD_PKG_LOCALEDIR =");
	debug_write(osd->get_dir(OSD_PKG_LOCALEDIR).c_str());

	textdomain(PACKAGE);
#else
	actual_locale = "";
	actual_country_code = "";
#endif

	  // Uncomment the two instructions below to see what happens when language
	  // or encoding is still unknown after call to setlocale(). Note this
	  // is what happens if ENABLE_NLS is set to 0.
	//actual_country_code = "";
	//encoding = ENCODING_UNKNOWN;

	if (actual_country_code.empty() || encoding == ENCODING_UNKNOWN) {
		  // setlocale() returned an empty string OR ENABLE_NLS is set to 0
		  // OR no way to work out the encoding used from the locale
		  //   => we are now trying to guess language and encoding from the environment
		char *e = getenv(ENV_LANG);
		if (e != NULL) {
			string env_lang = e;
			debug_write("* USING LANG ENVIRONMENT VARIABLE");
			debug_write(env_lang.c_str());
			if (actual_country_code.empty())
				actual_country_code = get_country_code_from_locale(env_lang);
			if (encoding == ENCODING_UNKNOWN)
				encoding = get_encoding_from_locale(env_lang);
		}
	}

#ifdef DEBUG_ENFORCE_ACTUAL_COUNTRY_CODE
	actual_country_code = DEBUG_ENFORCE_ACTUAL_COUNTRY_CODE;
#endif
#ifdef DEBUG_ENFORCE_ENCODING
	encoding = DEBUG_ENFORCE_ENCODING;
#endif

#ifdef DEBUG
	debug_write("* GUESSED:");
	if (encoding == ENCODING_UNKNOWN)
		debug_write("encoding = UNKNOWN");
	else if (encoding == ENCODING_1BYTE)
		debug_write("encoding = 1BYTE");
	else if (encoding == ENCODING_UTF8)
		debug_write("encoding = UTF-8");
	debug_write("actual country code = ");
	debug_write(actual_country_code.c_str());
#endif

	string t = os_get_install_language();
	if (!t.empty())
		actual_country_code = t;
	lower_case(actual_country_code);

	if (encoding == ENCODING_UNKNOWN)
		encoding = os_get_default_encoding();
	if (actual_country_code == "")
		actual_country_code = DEFAULT_ACTUAL_COUNTRY_CODE;

#ifdef DEBUG
	debug_write("* DEFINITIVE:");
	if (encoding == ENCODING_1BYTE)
		debug_write("encoding = 1BYTE");
	else if (encoding == ENCODING_UTF8)
		debug_write("encoding = UTF-8");
	debug_write("actual country code = ");
	debug_write(actual_country_code.c_str());
#endif

}

int prog_init(int argc, char **argv) {
	osd = new OS_Dirs(argv[0]);

// STAGE 1 -> from now on, we can rely on osd (standard directories)

	work_out_locale();

	html_help_found = false;
	string h;
	for (int i = 0; i < (actual_country_code != DEFAULT_ACTUAL_COUNTRY_CODE ? 2 : 1) && !html_help_found; i++) {
		if (i == 0)
			h = MY_HELP_HTML_PREFIX + actual_country_code + MY_HELP_HTML_PEXTENSION;
		else if (i == 1)
			h = MY_HELP_HTML_PREFIX DEFAULT_ACTUAL_COUNTRY_CODE MY_HELP_HTML_PEXTENSION;
		else
			throw(CalcFatal(__FILE__, __LINE__, "prog_init(): internal error 001"));
		html_help_file = os_concatene(osd->get_dir(OSD_HC_HTML), h);
		html_help_found = os_file_exists(html_help_file.c_str());
	}

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
	opt.add("a", "class", &opt_class_count);
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
		string ddesc = "Debug variable set to " + opt_debug;
		debug_write(ddesc.c_str());
		debug_itemiser = (opt_debug.find_first_of('i') != string::npos);
		debug_tokeniser = (opt_debug.find_first_of('t') != string::npos);
		debug_locale = (opt_debug.find_first_of('l') != string::npos);
	}

	has_gui = (!opt_batch && !opt_console && !debug_itemiser && !debug_tokeniser && !debug_locale);

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

	myencoding_t myenc;
	if (encoding == ENCODING_UTF8)
		myenc = MYENCODING_UTF8;
	else if (encoding == ENCODING_1BYTE)
		myenc = MYENCODING_1BYTE;
	else
		throw(CalcFatal(__FILE__, __LINE__, "prog_init(): unknown encoding to use!!!?"));
	E = new MyEncoding(myenc);

// STAGE 2 -> from now on, we can rely on E (encoding management)

	F = new Flags();

// STAGE 3 -> from now on, we can rely on F (calculator flags)

	os_init();

	ui_init();

	if (!has_gui)
		return PROG_INIT_START_CONSOLE;
	else
		return PROG_INIT_START_GUI;
}

int prog_terminate() {
	ui_terminate();
	if (E != NULL)
		delete E;
	if (osd != NULL)
		delete osd;
	return check_class_count();
}

static int check_class_count() {
#ifdef DEBUG_CLASS_COUNT
	if (StackItem::get_class_count() + Stack::get_class_count() + NodeStack::get_class_count() + Real::get_class_count()
			+ Cplx::get_class_count() - 1 + ReadMatrixCell::get_class_count() + CCMatrix::get_class_count()
			+ ToString::get_class_count() + Var::get_class_count() + Tree::get_class_count() + Binary::get_class_count() + TransStack::get_class_count() == 0) {
		if (opt_class_count)
			std::cout << "get_class_count() = 0 for watched classes" << endl;
		return 0;
	} else {
		std::cout << "TransStack class_count: " << TransStack::get_class_count() << std::endl;
		std::cout << "StackItem class_count: " << StackItem::get_class_count() << std::endl;
		std::cout << "Stack class_count: " << Stack::get_class_count() << std::endl;
		std::cout << "NodeStack class_count: " << NodeStack::get_class_count() << std::endl;
		std::cout << "Real class_count: " << Real::get_class_count() << std::endl;
		std::cout << "Cplx class_count: " << Cplx::get_class_count() - 1 << std::endl;
		std::cout << "ReadMatrixCell class_count: " << ReadMatrixCell::get_class_count() << std::endl;
		std::cout << "CCMatrix class_count: " << CCMatrix::get_class_count() << std::endl;
		std::cout << "ToString class_count: " << ToString::get_class_count() << std::endl;
		std::cout << "Var class_count: " << Var::get_class_count() << std::endl;
		std::cout << "Tree class_count: " << Tree::get_class_count() << std::endl;
		std::cout << "Binary class_count: " << Binary::get_class_count() << std::endl;
		std::cout << "Exec class_count: " << Exec::get_class_count() << std::endl;
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

		r = tokenise.get_token(par, tok);
		if (!r)
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
			cout << "Item: " << simple_string(const_cast<const StackItem *>(si)) << endl;
			delete si;
		} else
			cout << "par.set = false and r = false" << endl;
	}
}

void debug_display_locale() {
	cout << actual_country_code << endl;
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

void debug_write_v(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *cz;
  int l = strlen(fmt) * 2 + 100;
  cz = new char[l];
  vsnprintf(cz, l, fmt, args);
	debug_write(cz);
  va_end(args);
	delete []cz;
}

#endif	// DEBUG

