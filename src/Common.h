// Common.h
// Program common header file, contains declarations worth most other files

// RPN calculator

// Sébastien Millet
// August 2009 - May 2010

// A faire
//   NEG pour complexes et matrices
//   NEG qui ne stocke pas de signe sur une valeur nulle (real)
//   RE, IM, C->R et R->C pour matrices
//   VarFile::get_si(): quid si le si attaché est égal à 0 ?
//   Binaries : écrire plan de test et opérations * et /
//   opération + doit fonctionner entre une liste et n'importe quel élément
//   Remplacer string par wstring ?
//   Localisation ?
//
// Programme de test :
// 	 Intitulé de l'erreur si pas assez d'argument lors de l'exécution de << -> ... << >> >>

#ifndef COMMON_H
#define COMMON_H

#ifdef HAVE_CONFIG_H
#include "../config.h"
#else
#include "../extracfg.h"
#endif

#include <string>

#ifdef DEBUG
#include <iostream>
#endif

#define BIN_NAME					"prpn"
#define MY_HELP_HTML_PREFIX			"pRPN"
#define MY_HELP_HTML_PEXTENSION		".html"

enum {ENCODING_UNKNOWN, ENCODING_1BYTE, ENCODING_UTF8};

#define ENV_LANG					"LANG"
#define DEFAULT_ACTUAL_COUNTRY_CODE	"en"

#ifdef DEBUG
  // Uncomment the below to hard-code language or encoding
  // I surround it with #ifdef DEBUG so you don't forget it while
  // compiling for production... :-)
//#define DEBUG_ENFORCE_ACTUAL_COUNTRY_CODE	"en"
//#define DEBUG_ENFORCE_ENCODING			ENCODING_1BYTE
#endif

  // Count objects creations and destruction, to detect leaks
#define DEBUG_CLASS_COUNT
  // Debug the transactional stack (that manages undo levels)
//#define DEBUG_TRANSSTACK

typedef double real;

#define G_HARD_MAX_NB_BITS	64
extern int g_max_nb_bits;

extern std::string actual_locale;
extern std::string actual_country_code;

size_t my_get_string_length(const char *);
void my_append_padl(std::string&, const std::string&, const size_t&);
void my_append_padr(std::string&, const std::string&, const size_t&);

void my_sleep_seconds(const int&);
const std::string integer_to_string(const int&);

void upper_case(std::string&);
void lower_case(std::string&);

enum {
	PROG_INIT_START_GUI,
	PROG_INIT_START_CONSOLE,
	PROG_INIT_EXIT
};
int prog_init(int, char **);
int prog_terminate();

typedef enum {
	ST_ERR_OK,
	ST_ERR_TOO_FEW_ARGUMENTS,
	ST_ERR_BAD_ARGUMENT_TYPE,
	ST_ERR_INFINITE_RESULT,
	ST_ERR_OVERFLOW,
	ST_ERR_UNDERFLOW_NEG,
	ST_ERR_UNDERFLOW_POS,
	ST_ERR_BAD_ARGUMENT_VALUE,
	ST_ERR_UNDEFINED_RESULT,
	ST_ERR_SYNTAX_ERROR,
	ST_ERR_STR_TO_BAD_ARGUMENT_TYPE,
	ST_ERR_INVALID_DIMENSION,
	ST_ERR_DIRECTORY_NOT_ALLOWED,
	ST_ERR_NON_EMPTY_DIRECTORY,
	ST_ERR_UNDEFINED_NAME,
	ST_ERR_FILE_READ_ERROR,
	ST_ERR_FILE_WRITE_ERROR,
	ST_ERR_NOT_IMPLEMENTED,
	ST_ERR_INTERNAL,
	ST_ERR_EXIT
} st_err_t;

  // Help display type
enum {DH_MAIN, DH_FLAGS};

  // State of flags, referred to with FL_ enumeration
struct Flag {
	bool value;
	const char* description;
	bool default_value;
};
extern Flag flags[];
enum {
	FL_TAG_IT_BEGIN = 31,
	FL_LAST = 31,
	FL_AUTO_PRINT = 32,
	FL_CR_AUTOMATIC = 33,
	FL_MAIN_VALUE = 34,
	FL_CONST_EVAL = 35,
	FL_FUNC_EVAL = 36,
	FL_BIN_SIZE_6 = 37,
	FL_BIN_BASE_2 = 43,
	FL_DISPLAY_L1 = 45,
	FL_RESERVED1 = 46,
	FL_RESERVED2 = 47,
	FL_DECIMAL_SEP = 48,
	FL_REAL_FORMAT_2 = 49,
	FL_TONE = 51,
	FL_FAST_PRINT = 52,
	FL_REAL_NB_DECS_4 = 53,
	FL_UNDERFLOW_OK = 57,
	FL_OVERFLOW_OK = 58,
	FL_INFINITE = 59,
	FL_ANGLE = 60,
	FL_UNDERFLOW_NEG_EXCEPT = 61,
	FL_UNDERFLOW_POS_EXCEPT = 62,
	FL_OVERFLOW_EXCEPT = 63,
	FL_INFINITE_EXCEPT = 64,
	FL_TAG_IT_END = 64
};

typedef enum {TOSTRING_DISPLAY, TOSTRING_EDIT, TOSTRING_PORTABLE} tostring_t;

class DisplayStackLayout;

char get_decimal_separator(const bool&);
char get_complex_separator(const bool&);

  //
  // WARNING
  //
  // The values below are not meaningless -> the elements of the array
  //   string branch_str[]
  // are referred to using IT_* constants so they must start at zero
  // and be enumerated in the right order, that's why I explicited the values.
  // This array is defined and initialized in Parser.cpp.
  //
  // It is also used in the array
  //   automat
  // defined in the Parser.cpp, member-function Itemiser::get_item_recursive().
  //
typedef enum {IT_NUL = 0, IT_SI = 1, IT_GET_VARS = 2,
	IT_OPEN_LIST = 3, IT_OPEN_PROGRAM = 4, IT_CLOSE_LIST = 5, IT_CLOSE_PROGRAM = 6,
	IT_IF = 7, IT_THEN = 8, IT_ELSE = 9, //IT_IFERR,
	IT_WHILE = 10, IT_REPEAT = 11, IT_DO = 12, IT_UNTIL = 13,
	IT_FOR = 14, IT_START = 15, IT_NEXT = 16, IT_STEP = 17,
	IT_END = 18} item_t;

  // Used in try/catch/throw sequences, for an immediate program stop upon internal bug detection
class CalcFatal {
	std::string file;
	unsigned int line;
	std::string message;
public:
	CalcFatal(const char* f, const unsigned int& l, const char* m) : file(f), line(l), message(m) { }
	virtual ~CalcFatal() { }
	virtual const std::string get_description() const;
};

#ifdef DEBUG

#ifndef DEBUG_OUTPUT_FILE
#define DEBUG_OUTPUT_FILE	(BIN_NAME "-dbg.txt")
#endif	// DEBUG_OUTPUT_FILE

void debug_write(const char *);
void debug_write_truncate(const char *);
void debug_write_i(const char *, const int&);

#else	// !DEBUG

#ifndef debug_write
#define debug_write(x)
#endif
#ifndef debug_write_i
#define debug_write_i(x,y)
#endif
#ifndef debug_write_truncate
#define debug_write_truncate(x)
#endif

#endif	// DEBUG

#endif // COMMON_H

