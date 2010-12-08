// Common.h
// Program common header file, contains declarations worth most other files

// RPN calculator

// Sébastien Millet
// August 2009 - May 2010

// A faire
//   *OK* NEG pour complexes et matrices
//   *OK* NEG qui ne stocke pas de signe sur une valeur nulle (real)
//   *OK* RE, IM, C->R et R->C pour matrices
//   VarFile::get_si(): quid si le si attaché est égal à 0 ?
//   Binaries : écrire plan de test et opérations * et /
//   *OK* opération + doit fonctionner entre une liste et n'importe quel élément
//   Remplacer string par wstring ?
//   *OK* { nom_variable } possible pour créer un tableau
//   Trouver une solution à des erreurs d'arrondi provoqués par la librairie "math". Ainsi le nombre
//     50000000000.15 une fois arrondi à 12 décimales vaut 50000000000.2 alors que le nombre
//     90000000000.15 une fois arrondi à 12 décimales vaut 90000000000.1
//     Une solution serait d'arrondir à 13 décimales puis d'enlever la dernière... Mais je cherche
//     une solution plus élégante.
//     Autre exemple : 10000.15946805 devient 10000.1594681 alors que
//                     50000.15946805 devient 50000.159468
//   Traiter le cas des références circulaires. Exemple :
//   << 'A' 'B' STO 'B' 'A' STO A >> EVAL
//     La séquence d'instructions ci-dessus provoque une boucle infinie...
//
// Programme de test :
// 	 *OK* Intitulé de l'erreur si pas assez d'argument lors de l'exécution de << -> ... << >> >>

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

#ifdef DEBUG
#define DEBUG_VERSION_STRING		"d"
#else
#define DEBUG_VERSION_STRING		""
#endif

#define BIN_NAME					"prpn"
#define MY_HELP_HTML_PREFIX			"prpn"
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
#define PORTABLE_DECIMAL_SEPARATOR	'.'
#define PORTABLE_STRING_DECIMAL_SEPARATOR	"."
#define ALTERN_DECIMAL_SEPARATOR	','
#define PORTABLE_COMPLEX_SEPARATOR	','
#define ALTERN_COMPLEX_SEPARATOR	';'

#define G_HARD_MAX_NB_BITS	64
extern int g_max_nb_bits;

extern std::string actual_locale;
extern std::string actual_country_code;

size_t my_get_string_length(const char *);
void my_append_padl(std::string&, const std::string&, const size_t&);
void my_append_padr(std::string&, const std::string&, const size_t&);

  // Commands that do not exist as per standard start with this string
#ifndef CMD_PREFIX_NOSTD
#define CMD_PREFIX_NOSTD "_"
#endif

void my_sleep_seconds(const int&);
const std::string integer_to_string(const int&);

int identify_builtin_command(const std::string& cmd);
const char *get_builtin_command_short_description(const int&);

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

typedef enum {REALDISP_STD, REALDISP_SCI, REALDISP_FIX, REALDISP_ENG} realdisp_t;

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

typedef enum {ANGLE_RAD, ANGLE_DEG} angle_t;

  // State of flags, referred to with FL_ enumeration
struct Flag1 {
	bool value;
	const char* description;
	bool default_value;
};

class Flags {

	  // FIXME - Temporary stuff
	  // Unused.
	  // Defined to hide the global "F" and avoid using F-> in the object implementation
	  // I keep it so long as Flags is being modified regularly...
	  // You may wonder why there's an issue with this object in particular?
	  // It has been created late and most of the code (get_ and set_ member-functions) are
	  // copy-paste of code that used to seat outside of the class. In such a code,
	  // F-> appear. As F is global, it works perfectly inside the Flags class implementation...
	static char F;

	Flag1 flags[FL_TAG_IT_END + 1];
	void set_bin_base_flags(const bool&, const bool&);
public:
	Flags();
	~Flags();
	bool get(const int&) const;
	void set(const int&, const bool&);
	bool get_default(const int&) const;
	const char *get_description(const int&) const;
	char get_decimal_separator(const bool&) const;
	char get_complex_separator(const bool&) const;
	void get_realdisp(const bool&, realdisp_t&, int&) const;
	void set_realdisp(const realdisp_t&, int);
	int get_binary_format() const;
	void set_binary_format(const int&);
	int get_bin_size() const;
	void set_bin_size(int);
	angle_t get_angle_mode() const;
	void set_angle_mode(const angle_t&);
};

extern Flags *F;

typedef enum {TOSTRING_DISPLAY, TOSTRING_EDIT, TOSTRING_PORTABLE} tostring_t;

class DisplayStackLayout;

int string_to_integer(const std::string&);


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

