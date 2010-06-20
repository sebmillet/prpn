// vim:encoding=utf-8:

//
// PGetOpt.h
//
// v1.0.20100319
//

//
// Parse command-line options, in a portable way
// => no use of getopt.h, that's part of glibc. glibc' getopt
// is perfect... But it is specific to glibc.
// 'P' like Portable...
//
// Sébastien Millet
// February, March 2010
//

#include <string>

const int TRAILING_ARGS_STEP_INCREASE = 8;
const int OPTS_STEP_INCREASE = 8;

namespace pgetopt {
	enum {RET_OK,
		RET_UNKNOWN_SHORT_OPTION,
		RET_UNKNOWN_LONG_OPTION,
		RET_OPTION_REQUIRES_AN_ARGUMENT,
		RET_AMBIGUOUS_OPTION,
		RET_TRAILING_ARGS};
	typedef enum {T_BOOL, T_STRING} option_t;
	enum {THROW_INTERNAL_001 = 30000};
	class Options;
}

class pgetopt::Options {
	class OptionEntry;
	OptionEntry *opts;
	int actual_entry;
	int opts_quantity;

	void next_actual_entry();

	class OptionEntry {
		OptionEntry(const OptionEntry&);
	public:
		option_t type;
		std::string short_name;
		std::string long_name;
		bool allow_no_prefix;
		bool append;
		void *pvalue;
		int count;
		OptionEntry() { }
		~OptionEntry() { }
		void set_T_BOOL(const bool&);
		void set_T_STRING(const std::string&);
	};

	Options(const Options&);
	void inner_add(const pgetopt::option_t&, const char*, const char*, void*);

public:
	Options(const int& = 0);
	~Options();
	void add(const char*, const char*, bool*);
	void add(const char*, const char*, std::string*, const bool& = false);
	int parse(int argc, char** argv, int&, std::string*&, const char*& , std::string&, const int& = -1);
};

