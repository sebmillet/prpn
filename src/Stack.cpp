// Stack.cpp
// Stack implementation and StackItems

// RPN calculator

// Sébastien Millet
// August-September 2009

#include "Common.h"
#include "Stack.h"
#include "Variable.h"
#include "Ui.h"
#include "MyIntl.h"
#include "platform/os_generic.h"
#include <sstream>

#ifdef DEBUG
#include <iostream>
#endif














#include <cstdlib>















using namespace std;

const int DEFAULT_PORTABLE_BIN_BASE = 16;

extern const real MINR;
extern const real MAXR;

static string st_errors[] = {
	"",						// ST_ERR_OK
	"Too Few Arguments",	// ST_ERR_TOO_FEW_ARGUMENTS
	"Bad Argument Type",	// ST_ERR_BAD_ARGUMENT_TYPE
	"Infinite Result",		// ST_ERR_INFINITE_RESULT
	"Overflow",				// ST_ERR_OVERFLOW
	"Negative Underflow",	// ST_ERR_UNDERFLOW_NEG
	"Positive Underflow",	// ST_ERR_UNDERFLOW_POS
	"Bad Argument Value",	// ST_ERR_BAD_ARGUMENT_VALUE
	"Undefined Result",		// ST_ERR_UNDEFINED_RESULT
	"Syntax Error",			// ST_ERR_SYNTAX_ERROR
	"Bad Argument Type",	// ST_ERR_STR_TO_BAD_ARGUMENT_TYPE
	"Invalid Dimension",	// ST_ERR_INVALID_DIMENSION
	"Directory Not Allowed",// ST_ERR_DIRECTORY_NOT_ALLOWED
	"Non-Empty Directory",	// ST_ERR_NON_EMPTY_DIRECTORY
	"Undefined Name",		// ST_ERR_UNDEFINED_NAME
	"File Read Error",		// ST_ERR_FILE_READ_ERROR
	"File Write Error",		// ST_ERR_FILE_WRITE_ERROR
	"Not Yet Implemented",	// ST_ERR_NOT_IMPLEMENTED
	"Internal Error",		// ST_ERR_INTERNAL
	"Quitting"				// ST_ERR_EXIT
};

enum {RDM_HP, RDM_TABLE};
static const bool DEFAULT_RDM_BEHAVIOR = RDM_HP;
static bool cfg_rdm_behavior = DEFAULT_RDM_BEHAVIOR;

//
// Functions
//

extern const BuiltinCommandDescriptor builtinCommands[];
extern const unsigned int sizeof_builtinCommands;

  // Return ID of builtin command, if recognized, return -1 otherwise.
  // Built-in command IDs are in the range [0..sizeof_builtinCommands - 1].
int identify_builtin_command(const string& cmd) {
	int builtin_id;
	int up = static_cast<int>(sizeof_builtinCommands);
	string uc_cmd = cmd;
	upper_case(uc_cmd);
	for (builtin_id = 0; builtin_id < up; builtin_id++)
		if (uc_cmd == builtinCommands[builtin_id].command)
			break;
	if (builtin_id < up)
		return builtin_id;
	else
		return -1;
}
const char *get_builtin_command_short_description(const int& builtin_id) {
	return builtinCommands[builtin_id].short_description;
}

const string simple_string(const StackItem *si) {
	ToString tostr(TOSTRING_DISPLAY, 1, 0);
	si->to_string(tostr);
	string s;
	const vector<string> *p = tostr.get_value();
	return (*p)[0];
}

static void lvars_to_string(VarDirectory* const &lvars, ToString& tostr) {
	if (!lvars)
		return;
	vector<string> *by_order;
	lvars->get_vars(by_order);
	tostr.add_string("->");
	for (vector<string>::reverse_iterator it = by_order->rbegin(); it != by_order->rend(); it++)
		tostr.add_string(*it);
}

const string base_int_to_letter(const int& base) {
	switch (base) {
		case 2:
			return "b";
		case 8:
			return "o";
		case 10:
			return "d";
		case 16:
			return "h";
		default:
			return "?";
	}
}

  // Used by GET and similar commands
st_err_t check_coordinates(const Coordinates& bounds, const Coordinates& coord) {
	if (coord.d != DIM_ANY && bounds.d != coord.d)
		return ST_ERR_BAD_ARGUMENT_VALUE;
	if (coord.d != DIM_ANY) {
		for (int ii = 0; ii < (bounds.d == DIM_VECTOR ? 1 : 2); ii++) {
			int b = (ii == 0 ? bounds.i : bounds.j);
			int v = (ii == 0 ? coord.i : coord.j);
			if (v < 1 || v > b)
				return ST_ERR_BAD_ARGUMENT_VALUE;
		}
	} else {
		int n;
		if (bounds.d == DIM_VECTOR)
			n = bounds.i;
		else
			n = bounds.i * bounds.j;
		if (coord.i < 1 || coord.i > n)
				return ST_ERR_BAD_ARGUMENT_VALUE;
	}
	return ST_ERR_OK;
}

  // Used to push back an item into the stack after copying it if necessary.
  // Modify the ownership if need be (to avoid the item being cleaned up later.)
void get_ready_si(SIO& s, StackItem*& si) {
	if (s.ownership == TSO_OUTSIDE_TS) {
		si = s.si;
		s.ownership = TSO_OWNED_BY_TS;
	} else {
		si = s.si->dup();
	}
}

  // Get a copy of the item, if necessary (= if it is not owned by the transstack)
  // Leave the SIO object ownership untouched
void get_si_dup_if_necessary(const SIO& s, StackItem*& si) {
	si = s.si;
	if (s.ownership == TSO_OWNED_BY_TS)
		si = s.si->dup();
}

const string get_error_string(const st_err_t& c) { return st_errors[c]; }

  // Convert the output of a function from radians to degrees, if needed.
  // "I needed" = only if actual user settings are set to DEGREES.
void convert_to_degrees_if_needed(st_err_t& c, Real& x) {
	if (c == ST_ERR_OK && F->get_angle_mode() == ANGLE_DEG) {
		Real copy_x = x;
		copy_x.r_to_d(c, x);
	}
}

void convert_to_degrees_if_needed(st_err_t& c, Cplx& cplx) {
	if (c == ST_ERR_OK && F->get_angle_mode() == ANGLE_DEG && cplx.get_im() == 0) {
		Real re = Real(cplx.get_re());
		convert_to_degrees_if_needed(c, re);
		if (c == ST_ERR_OK)
			cplx = Cplx(re.get_value(), 0);
	}
}


  // Convert the input of a function from degrees to radians, if needed.
  // "I needed" = only if actual user settings are set to DEGREES.
void convert_to_radians_if_needed(st_err_t& c, Real& x) {
	if (F->get_angle_mode() == ANGLE_DEG) {
		Real copy_x = x;
		copy_x.d_to_r(c, x);
	}
}

  // Number of columns in a file containing StackItems
#define RC_FILE_WIDTH			78

void write_si(const tostring_t& tostring, const StackItem *csi, ostream& oss) {
	ToString tostr(tostring, 0, RC_FILE_WIDTH);
	csi->to_string(tostr);
	tostr.lock();
	const char *sz = tostr.get_first_line();
	while (sz != NULL) {
		oss << sz;
		if (sz != NULL)
			oss << "\n";
		sz = tostr.get_next_line();
	}
	tostr.unlock();
}


//
// Flags
//

static Flag1 default_flags[FL_TAG_IT_END + 1] = {
	{false, "", false}, {false, "", false}, {false, "", false}, {false, "", false}, {false, "", false},
	{false, "", false}, {false, "", false}, {false, "", false}, {false, "", false}, {false, "", false},
	{false, "", false}, {false, "", false}, {false, "", false}, {false, "", false}, {false, "", false},
	{false, "", false}, {false, "", false}, {false, "", false}, {false, "", false}, {false, "", false},
	{false, "", false}, {false, "", false}, {false, "", false}, {false, "", false}, {false, "", false},
	{false, "", false}, {false, "", false}, {false, "", false}, {false, "", false}, {false, "", false},
	{false, "", false},
	{true, _N("Allow LAST function"), true},	// FL_LAST			31
	{false, _N("Automatic print mode"), false},	// FL_AUTO_PRINT	32
	{false, _N("Automatic CR"), false},	// FL_CR_AUTOMATIC	33
	{false, _N("Principal value (definition range)"), false},	// FL_MAIN_VALUE	34
	{true, _N("Symbolic constants evaluation"), true},	// FL_CONST_EVAL	35
	{true, _N("Symbolic functions evaluation"), true},	// FL_FUNC_EVAL		36
	{true, "", true}, {true, "", true}, {true, "", true}, {true, "", true}, {true, "", true},
		{true, _N("Binary integers word size"), true},	// FL_BIN_SIZE_6		37-42
	{false, "", false}, {false, _N("Numeric base for binary integers"), false},		// FL_BIN_BASE_2		43-44
	{true, _N("Level 1 display"), true},	// FL_DISPLAY_L1	45
	{false, _N("Reserved"), false},	// FL_RESERVED1		46
	{false, _N("Reserved"), false},	// FL_RESERVED2		47
	{false, _N("Decimal separator"), false},	// FL_DECIMAL_SEP	48
	{false, "", false}, {false, _N("Real numbers format"), false},		// FL_REAL_FORMAT_2	49-50
	{false, _N("Tone"), false},	// FL_TONE			51
	{false, _N("Fast printing"), false},	// FL_FAST_PRINT	52
	{false, "", false}, {false, "", false}, {false, "", false}, {false, _N("Number of decimals"), false},			// FL_REAL_NB_DECS_4	53-56
	{false, _N("Underflow processed normally"), false},					// FL_UNDERFLOW_OK	57
	{false, _N("Overflow processed normally"), false},					// FL_OVERFLOW_OK	58
	{true, _N("Infinite Result processed normally"), true},				// FL_INFINITE		59
	{false, _N("Angle, unset for degrees, set for radians"), false},	// FL_ANGLE			60
	{false, _N("Underflow- processed as an exception"), false},			// FL_UNDERFLOW_NEG_EXCEPT	61
	{false, _N("Underdlow+ processed as an exception"), false},			// FL_UNDERFLOW_POS_EXCEPT	62
	{false, _N("Overflow processed as an exception"), false},			// FL_OVERFLOW_EXCEPT		63
	{false, _N("Infinite Result processed as an exception"), false}		// FL_INFINITE_EXCEPT		64
};

Flags::Flags() {
	for (int i = 0; static_cast<unsigned int>(i) < sizeof(flags) / sizeof(*flags); i++)
		flags[i] = default_flags[i];
}

Flags::~Flags() { }

bool Flags::get(const int& i) const { return flags[i].value; }

void Flags::set(const int& i, const bool& v) { flags[i].value = v; }

bool Flags::get_default(const int& i) const { return flags[i].default_value; }

const char* Flags::get_description(const int& i) const { return flags[i].description; }

char Flags::get_decimal_separator(const bool& user_setting) const {
	return user_setting ? (get(FL_DECIMAL_SEP) ?
				ALTERN_DECIMAL_SEPARATOR : PORTABLE_DECIMAL_SEPARATOR) : PORTABLE_DECIMAL_SEPARATOR;
}

char Flags::get_complex_separator(const bool& user_setting) const {
	return user_setting ? (get(FL_DECIMAL_SEP) ?
				ALTERN_COMPLEX_SEPARATOR : PORTABLE_COMPLEX_SEPARATOR) : PORTABLE_COMPLEX_SEPARATOR;
}

void Flags::set_bin_base_flags(const bool& flag1, const bool& flag2) {
	set(FL_BIN_BASE_2, flag1);
	set(FL_BIN_BASE_2 + 1, flag2);
}

void Flags::set_binary_format(const int& new_base) {
	switch (new_base) {
		case 2:
			set_bin_base_flags(false, true);
			break;
		case 8:
			set_bin_base_flags(true, false);
			break;
		case 10:
			set_bin_base_flags(false, false);
			break;
		case 16:
			set_bin_base_flags(true, true);
			break;
		default:
			throw(CalcFatal(__FILE__, __LINE__, "Flags::set_binary_format(): unknown base"));
	}
}

int Flags::get_binary_format() const {
	if (!get(FL_BIN_BASE_2) && !get(FL_BIN_BASE_2 + 1))
		return 10;
	else if (!get(FL_BIN_BASE_2) && get(FL_BIN_BASE_2 + 1))
		return 2;
	else if (get(FL_BIN_BASE_2) && !get(FL_BIN_BASE_2 + 1))
		return 8;
	else if (get(FL_BIN_BASE_2) && get(FL_BIN_BASE_2 + 1))
		return 16;
	  // Never happens ; written to avoid a warning
	return -1;
}

int Flags::get_bin_size() const {
	int r = 0;
	int m = 1;
	for (int i = FL_BIN_SIZE_6; i <= FL_BIN_SIZE_6 + 5; i++) {
		if (get(i))
			r += m;
		m *= 2;
	}
	return r + 1;
}

void Flags::set_bin_size(int n) {
	n--;
	if (n < 0)
		n = 0;
	if (n > 63)
		n = 63;
	for (int i = FL_BIN_SIZE_6; i < FL_BIN_SIZE_6 + 6; i++) {
		set(i, n % 2 != 0);
		n /= 2;
	}
}

void Flags::get_realdisp(const bool& user_setting, realdisp_t& rd, int& nb) const {

//
// The conversion from flags 49 and 50 to the formatting of reals is as follows.
// Status\Flag	49		50
//				unset	unset	STD
//				unset	set		SCI
//				set		unset	FIX
//				set		set		ENG
//
// The number of decimals to display is coded as follows.
// val(f53) + val(f54) * 2 + val(f55) * 4 + val(f56) * 8
// Where val(fxx) is 1 is flag number xx is set, 0 otherwise.
//

	rd = user_setting ? (get(FL_REAL_FORMAT_2) ?
			(get(FL_REAL_FORMAT_2 + 1) ? REALDISP_ENG : REALDISP_FIX) :
			(get(FL_REAL_FORMAT_2 + 1) ? REALDISP_SCI : REALDISP_STD)) :
		REALDISP_STD;
	if (rd == REALDISP_STD)
		nb = -1;
	else {
		nb = 0;
		int base_power = 1;
		for (int i = 0; i < 4; i++) {
			if (get(FL_REAL_NB_DECS_4 + i))
				nb += base_power;
			base_power += base_power;
		}
	}
}

void Flags::set_realdisp(const realdisp_t& rd, int nb) {
	switch (rd) {
		case REALDISP_STD:
			set(FL_REAL_FORMAT_2, false);
			set(FL_REAL_FORMAT_2 + 1, false);
			break;
		case REALDISP_SCI:
			set(FL_REAL_FORMAT_2, false);
			set(FL_REAL_FORMAT_2 + 1, true);
			break;
		case REALDISP_FIX:
			set(FL_REAL_FORMAT_2, true);
			set(FL_REAL_FORMAT_2 + 1, false);
			break;
		case REALDISP_ENG:
			set(FL_REAL_FORMAT_2, true);
			set(FL_REAL_FORMAT_2 + 1, true);
			break;
		default:
			throw(CalcFatal(__FILE__, __LINE__, "Flags::set_realdisp(): unknown display type!"));
	}

	if (nb < 0)
		nb = 0;
	if (nb > 11)
		nb = 11;
	for (int i = FL_REAL_NB_DECS_4; i < FL_REAL_NB_DECS_4 + 4; i++) {
		set(i, nb % 2 != 0);
		nb /= 2;
	}
}

angle_t Flags::get_angle_mode() const {
	return (get(FL_ANGLE) ? ANGLE_RAD : ANGLE_DEG);
}

void Flags::set_angle_mode(const angle_t& am) {
	if (am == ANGLE_RAD || am == ANGLE_DEG)
		set(FL_ANGLE, am == ANGLE_RAD);
	else
		throw(CalcFatal(__FILE__, __LINE__, "Flags::set_angle_mode(): unknown angle type"));
}


//
// ToString
//

#ifdef DEBUG_CLASS_COUNT
long ToString::class_count = 0;
#endif

ToString::ToString(const tostring_t& t, const size_t& h, const size_t& w) : locked(false), type(t), max_height(h), max_width(w) {
#ifdef DEBUG_CLASS_COUNT
	class_count++;
#endif
}

ToString::~ToString() {
#ifdef DEBUG_CLASS_COUNT
	class_count--;
#endif
}

void ToString::reset(const tostring_t& t, const size_t& h, const size_t& w) {
	type = t;
	max_height = h;
	max_width = w;
	v.clear();
}

bool ToString::add_string(const std::string& s, const bool& new_line) {
	if (locked)
		throw(CalcFatal(__FILE__, __LINE__, "*ToString::add_string(): object is locked"));
	bool v_updated = false;
	char complex_separator = F->get_complex_separator(type != TOSTRING_PORTABLE);
	if (v.empty()) {
		v.push_back("");
		if (max_height != 1 && max_width >= 1 && s.length() >= 2 && s.at(0) == '(' && s.at(s.length() - 1) == ')') {
			  // Complex numbers: split on 2 lines if necessary
			if (s.length() == max_width + 1) {
				v.back() = s.substr(0, s.length() - 1);
				v.push_back(")");
				v_updated = true;
			} else if (s.length() > max_width + 1) {
				size_t pos = s.find_first_of(complex_separator);
				if (pos != string::npos) {
					v.back() = s.substr(0, pos + 1);
					v.push_back(s.substr(pos + 1));
					v_updated = true;
				}
			}
		}
	} else {
		if (max_height != 1 && (new_line || (max_width >= 1 && v.back().size() + s.length() + 1 > max_width)))
			v.push_back("");
		else
			v.back().append(" ");
	}
	if (!v_updated)
		v.back().append(s);
	return (max_height <= 0 || v.size() < max_height || max_width <= 0 || (v.size() == max_height && v.back().length() < max_width));
}

void ToString::lock() {
	if (locked)
		throw(CalcFatal(__FILE__, __LINE__, "*ToString::lock(): object is already locked"));
	locked = true;
}

void ToString::unlock() {
	if (!locked)
		throw(CalcFatal(__FILE__, __LINE__, "*ToString::unlock(): object is not locked"));
	locked = false;
}

const char *ToString::get_line() {
	if (!locked)
		throw(CalcFatal(__FILE__, __LINE__, "*ToString::get_line(): object not locked"));
	if (actual_index == static_cast<int>(v.size()))
		return NULL;
	else if (actual_index > static_cast<int>(v.size()))
		throw(CalcFatal(__FILE__, __LINE__, "*ToString::get_line(): call beyond end of array of strings"));
	else
		return v[actual_index].c_str();
}

const char *ToString::get_first_line() {
	actual_index = 0;
	return get_line();
}

const char *ToString::get_next_line() {
	actual_index++;
	return get_line();
}


//
// Stack
//

#ifdef DEBUG_CLASS_COUNT
long Stack::class_count = 0;
#endif
#ifdef DEBUG_TRANSSTACK
int Stack::class_serial = 0;
#endif

#ifndef USE_VECTOR
const int stack_pre_alloc = 1;
Stack::Stack(const size_t&) : count(0), allocated(0), values(0), has_ownership(false) {
#else
Stack::Stack(const size_t&) : has_ownership(false) {
#endif

#ifdef DEBUG_TRANSSTACK
	object_serial = class_serial++;
#endif
#ifdef DEBUG_CLASS_COUNT
	class_count++;
#endif

#ifdef DEBUG_TRANSSTACK
	cout << "Stack::Stack(), creating from scratch" << endl;
#endif // DEBUG_TRANSSTACK

#ifndef USE_VECTOR
	re_alloc(stack_pre_alloc);
#endif
}

Stack::~Stack() {
#ifndef USE_VECTOR
	if (has_ownership)
		while (count)
			delete stack_pop();
	delete []values;
#else
	if (has_ownership)
		while (!values.empty())
			delete stack_pop();
#endif

#ifdef DEBUG_CLASS_COUNT
	class_count--;
#endif
}

#ifndef USE_VECTOR
Stack::Stack(const Stack& s) : count(s.count), allocated(s.count), values(0), has_ownership(false) {
#else
Stack::Stack(const Stack& s) : has_ownership(false) {
#endif

#ifdef DEBUG_TRANSSTACK
	object_serial = class_serial++;
#endif
#ifdef DEBUG_CLASS_COUNT
	class_count++;
#endif

#ifndef USE_VECTOR
#ifdef DEBUG_TRANSSTACK
	//cout << "Stack::Stack(Stack*), copying " << s.count <<
	//	" element(s) from [" << s.object_serial << "] to [" << object_serial << "]" << endl;
#endif
	if (s.count > 0) {
		values = new StackItem*[allocated];
		for (size_t i = 0; i < s.count; i++)
			values[i] = s.values[i];
	}
#else
	values = s.values;
#endif
}

#ifndef USE_VECTOR

void Stack::re_alloc(const size_t& new_alloc) {
	if (new_alloc <= allocated)
		return;
	StackItem **new_values = new StackItem*[new_alloc];
	for (size_t i = 0; i < allocated; i++)
		new_values[i] = values[i];
	if (allocated > 0)
		delete []values;
	values = new_values;
	allocated = new_alloc;
}

size_t Stack::stack_push(StackItem *si) {
	if (count >= allocated)
		re_alloc(allocated + stack_alloc_step);
	values[count++] = si;
	return count;
}

StackItem *Stack::stack_pop() {
	if (count == 0)
		return 0;
	StackItem *r = values[--count];
	return r;
}

#else // USE_VECTOR is defined

size_t Stack::stack_push(StackItem *si) {
	values.push_back(si);
	return values.size();
}

StackItem *Stack::stack_pop() {
	if (values.empty())
		return 0;
	StackItem *r = values.back();
	values.pop_back();
	return r;
}

#endif

const StackItem *Stack::stack_get_const_si(const int& stack_item_number) const {
	if (stack_item_number > static_cast<int>(get_count()))
		throw(CalcFatal(__FILE__, __LINE__, "*Stack::get_const_si(): wrong item number"));
	return const_cast<const StackItem*>(values[get_count() - stack_item_number]);
}

const string Stack::get_display_line(const DisplayStackLayout& dsl, const int& line_number,
						IntervalShift& ishift, bool& recalc, bool& no_more_lines, int& item_number) {
	static vector<string> dv;
	static vector<string> xv;
	static int e1;
	static int e_start;
	static int e_end;
	static int first_elem_height;
	static int n;

	if (recalc) {
		e1 = -1;
		first_elem_height = 0;
		n = get_count();
		ToString tostr(TOSTRING_DISPLAY,
						dsl.get_max_stack() >= 1 ?
							(F->get(FL_DISPLAY_L1) && dsl.get_max_stack() >= 1 ? dsl.get_max_stack()
							 :
							1)
						: 1,
						dsl.get_width() >= 4 ? dsl.get_width() - 3 : 0);

		dv.clear();
		xv.clear();

		if (n >= 1) {
			values[n - 1]->to_string(tostr);
			dv = *(tostr.get_value());
			first_elem_height = dv.size();
			e1 = 0;
		}
		ishift.max = first_elem_height - dsl.get_max_stack();
		if (ishift.max < 0)
			ishift.max = 0;
		ishift.min = dsl.get_max_stack() - n - first_elem_height + 1 + ishift.max;
		if (ishift.min > 0)
			ishift.min = 0;
		if (dsl.get_max_stack() < 1 || (dsl.get_max_stack() >= 1 && first_elem_height < dsl.get_max_stack())) {
			if (dsl.get_max_stack() >= 1)
				e1 = dsl.get_max_stack() - static_cast<int>(dv.size());
			else
				e1 = n - 1;
			e1 = (e1 > n - 1 ? n - 1 : e1);
			if (e1 < 0 && !dv.empty())
				e1 = 0;
			if (e1 >= 1) {
				dv.insert(dv.begin(), e1, "");
				for (int i = n - e1; i <= n - 1; i++) {
					tostr.reset(TOSTRING_DISPLAY, 1, dsl.get_width() - 3);
					values[i - 1]->to_string(tostr);
					dv[i - (n - e1)] = (*tostr.get_value())[0];
				}
			}
		}
		e_start = static_cast<int>(dv.size()) - dsl.get_min_stack();
		if (e_start > 0)
			e_start = 0;
		if (e_start == 0 && dsl.get_max_stack() >= 1) {
			e_start = static_cast<int>(dv.size()) - dsl.get_max_stack();
			e_start = e_start > e1 ? e1 : e_start;
		}
		if (e_start < 0) {
			e_start = e_start < e1 - dsl.get_min_stack() + 1 ? e1 - dsl.get_min_stack() + 1 : e_start;
			if (e_start > 0)
				e_start = 0;
		}
		e_end = static_cast<int>(dv.size()) - 1;
		if (dsl.get_max_stack() >= 1 && e_end - e_start + 1 > dsl.get_max_stack())
			e_end = e_start + dsl.get_max_stack() - 1;
		if (dsl.get_max_stack() < 1) {
			e_start = 0;
			e_end = dv.size() - 1;
		}
		recalc = false;
	}
	if (ishift.actual < ishift.min)
		ishift.actual = ishift.min;
	if (ishift.actual > ishift.max)
		ishift.actual = ishift.max;

	int e = e_start + line_number - 1 + ishift.actual;
	ostringstream o;
	item_number = e1 - e + 1;
	int w = dsl.get_width() - 3;
	for (int i = item_number; i >= 10; i /= 10)
		w--;
	if (dsl.get_max_stack() >= 1) {
		if (item_number >= 1)
			o << item_number << ":";
		else
			o << "  ";
	}

	no_more_lines = (e >= static_cast<int>(dv.size()) - 1);

	string s;
	if (e >= 0) {
		s = dv[e];
	} else if (n - item_number >= 0) {
		ToString tostr(TOSTRING_DISPLAY, 1, dsl.get_width() - 3);
		for (int i = xv.size(); i < -e; i++) {
			tostr.reset(TOSTRING_DISPLAY, 1, dsl.get_width() - 3);
			values[n - i - e1 - 2]->to_string(tostr);
			xv.push_back((*tostr.get_value())[0]);
		}
		s = xv[-e - 1];
	}

	ui_string_trim(s, static_cast<size_t>(w), &dsl);

	if (w >= 1 && dsl.get_max_stack() >= 1 && (e < e1 || first_elem_height == 1)) {
		o << " ";
		string tmp;
		E->append_padl(tmp, s.c_str(), w);
		o << tmp;
	} else {
		if (dsl.get_max_stack() >= 1)
			o << " ";
		o << s;
	}

	if (item_number < 1)
		item_number = 1;

	return o.str();
}


//
// NodeStack
//

#ifdef DEBUG_CLASS_COUNT
long NodeStack::class_count = 0;
#endif
#ifdef DEBUG_TRANSSTACK
int NodeStack::class_serial = 0;
#endif

#ifndef STACK_USE_MAP
NodeStack::NodeStack(NodeStack *p, NodeStack *n, Stack *s) : si_keeper(0), count(0), allocated(0), previous(p), next(n) {
#else
NodeStack::NodeStack(NodeStack *p, NodeStack *n, Stack *s) : previous(p), next(n) {
#endif

#ifdef DEBUG_TRANSSTACK
	object_serial = class_serial++;
#endif
#ifdef DEBUG_CLASS_COUNT
	class_count++;
#endif

	if (s != 0)
		st = new Stack(*s);
	else
		st = new Stack();
}

NodeStack::~NodeStack() {

#ifdef DEBUG_TRANSSTACK

		cout << "NodeStack::~NodeStack(), will delete part of keeper items" << endl;
		cout << "\tprevious = [" << (previous == 0 ? -1 : previous->object_serial) <<
			"], next = [" << (next == 0 ? -1 : next->object_serial) << "]" << endl;

#ifndef STACK_USE_MAP

		for (int i = 0; i < count; i++)
			cout << "\tkeeper{" << si_keeper[i].si->get_object_serial() << "} = " << debug_keeper_type_name(si_keeper[i].type) << endl;
#else
		for (map<StackItem*, si_keeper_t>::iterator it = si_map_keeper.begin(); it != si_map_keeper.end(); it++)
			cout << "\tkeeper{" << it->first->get_object_serial() << "} = " << debug_keeper_type_name(it->second) << endl;
#endif // DEBUG_TRANSSTACK

#endif

#ifndef STACK_USE_MAP
	for (int i = 0; i < count; i++) {
		si_keeper_t sit = si_keeper[i].type;
		if ((sit == SI_ADDED && next == 0) ||
			(sit == SI_DELETED && previous == 0)) {

#ifdef DEBUG_TRANSSTACK
			cout << "NodeStack::~NodeStack(), deleting item [" << si_keeper[i].si->get_object_serial() <<
				"] of type " << debug_keeper_type_name(si_keeper[i].type) << endl;
#endif // DEBUG_TRANSSTACK

			delete si_keeper[i].si;
		} else {

#ifdef DEBUG_TRANSSTACK
			cout << "NodeStack::~NodeStack(), ignoring item [" << si_keeper[i].si->get_object_serial() <<
				"] of type " << debug_keeper_type_name(si_keeper[i].type) << endl;
#endif // DEBUG_TRANSSTACK

		}
	}
	if (si_keeper != 0)
		delete []si_keeper;

#else // STACK_USE_MAP is defined

	for (map<StackItem*, si_keeper_t>::iterator it = si_map_keeper.begin(); it != si_map_keeper.end(); it++) {
		si_keeper_t sit = it->second;
		if ((sit == SI_ADDED && next == 0) ||
			(sit == SI_DELETED && previous == 0)) {

#ifdef DEBUG_TRANSSTACK
			cout << "NodeStack::~NodeStack(), deleting item [" << it->first->get_object_serial() <<
				"] of type " << debug_keeper_type_name(it->second) << endl;
#endif // DEBUG_TRANSSTACK

			delete it->first;
		} else {

#ifdef DEBUG_TRANSSTACK
			cout << "NodeStack::~NodeStack(), ignoring item [" << it->first->get_object_serial() <<
				"] of type " << debug_keeper_type_name(it->second) << endl;
#endif // DEBUG_TRANSSTACK

		}
	}

#endif	// STACK_USE_MAP

	delete st;

#ifdef DEBUG_TRANSSTACK
	cout << "NodeStack::~NodeStack(), (end)" << endl;
#endif // DEBUG_TRANSSTACK

#ifdef DEBUG_CLASS_COUNT
	class_count--;
#endif

}

#ifndef STACK_USE_MAP

void NodeStack::re_alloc(const int& new_alloc) {
	if (new_alloc <= allocated)
		return;
	ListSI *new_si_keeper = new ListSI[new_alloc];
	for (int i = 0; i < allocated; i++)
		new_si_keeper[i] = si_keeper[i];
	if (allocated > 0)
		delete []si_keeper;
	si_keeper = new_si_keeper;
	allocated = new_alloc;
}

int NodeStack::add(const ListSI& lsi) {
	if (count >= allocated)
		re_alloc(allocated + nodestack_alloc_step);
	si_keeper[count++] = lsi;
	return count;
}

int NodeStack::nodestack_push(StackItem *si) {
	int v = st->stack_push(si);
	int i;
	for (i = count - 1; i >= 0; i--)
		if (si_keeper[i].si == si) {
			if (si_keeper[i].type != SI_DELETED && si_keeper[i].type != SI_UNDEF)
				throw(CalcFatal(__FILE__, __LINE__, "inconsistent keeper item values in nodestack_push()"));
			else
				break;
		}
	if (i >= 0) {
		si_keeper[i].type = (si_keeper[i].type == SI_DELETED ? SI_UNDEF : SI_ADDED);

#ifdef DEBUG_TRANSSTACK
		cout << "NodeStack::nodestack_push(), marked [" << si->get_object_serial() <<
			"] as " << debug_keeper_type_name(si_keeper[i].type) << endl;
#endif // DEBUG_TRANSSTACK

	} else {
		ListSI lsi = {SI_ADDED, si};
		add(lsi);

#ifdef DEBUG_TRANSSTACK
		cout << "NodeStack::nodestack_push(), added [" << si->get_object_serial() <<
			"] as " << debug_keeper_type_name(SI_ADDED) << endl;
#endif // DEBUG_TRANSSTACK

	}
	return v;
}

SIO NodeStack::nodestack_pop() {
	SIO s;
	s.si = st->stack_pop();
	int i;
	for (i = count - 1; i >= 0; i--)
		if (si_keeper[i].si == s.si) {
			if (si_keeper[i].type != SI_ADDED && si_keeper[i].type != SI_UNDEF)
				throw(CalcFatal(__FILE__, __LINE__, "inconsistent keeper item values in nodestack_pop()"));
			else
				break;
		}
	if (i >= 0) {
		si_keeper[i].type = (si_keeper[i].type == SI_ADDED ? SI_UNDEF : SI_DELETED);

#ifdef DEBUG_TRANSSTACK
		cout << "NodeStack::nodestack_pop(), marked [" << s.si->get_object_serial() <<
			"] as " << debug_keeper_type_name(si_keeper[i].type) << endl;
#endif // DEBUG_TRANSSTACK
		s.ownership = (si_keeper[i].type == SI_UNDEF ? TSO_OUTSIDE_TS : TSO_OWNED_BY_TS);

	} else {
		ListSI lsi = {SI_DELETED, s.si};
		add(lsi);

#ifdef DEBUG_TRANSSTACK
		cout << "NodeStack::nodestack_pop(), added [" << s.si->get_object_serial() <<
			"] as " << debug_keeper_type_name(SI_DELETED) << endl;
#endif // DEBUG_TRANSSTACK
		s.ownership = TSO_OWNED_BY_TS;

	}
	return s;
}

#else // STACK_USE_MAP is defined

int NodeStack::nodestack_push(StackItem *si, const bool& transactional) {
	int v = st->stack_push(si);
	if (transactional) {
		si_keeper_t t;
		t = si_map_keeper[si];
		if (t == SI_DELETED) {

#ifdef DEBUG_TRANSSTACK
			cout << "NodeStack::nodestack_push(), removing [" << si->get_object_serial() <<
				"] of type " << debug_keeper_type_name(si_map_keeper[si]) << endl;
#endif // DEBUG_TRANSSTACK
			si_map_keeper.erase(si);

		} else if (t == SI_UNDEF) {

			si_map_keeper[si] = SI_ADDED;
#ifdef DEBUG_TRANSSTACK
			cout << "NodeStack::nodestack_push(), added [" << si->get_object_serial() <<
				"] as " << debug_keeper_type_name(si_map_keeper[si]) << endl;
#endif // DEBUG_TRANSSTACK

		} else {
			throw(CalcFatal(__FILE__, __LINE__, "inconsistent keeper item values in nodestack_push() (STACK_USE_MAP defined)"));
		}
	}

	return v;
}

SIO NodeStack::nodestack_pop(const bool& transactional) {
	SIO s;
	s.si = st->stack_pop();
	if (transactional || si_map_keeper.size() != 0) {
		si_keeper_t t;
		t = si_map_keeper[s.si];
		if (t == SI_ADDED) {

#ifdef DEBUG_TRANSSTACK
			cout << "NodeStack::nodestack_pop(), removing [" << s.si->get_object_serial() <<
				"] of type " << debug_keeper_type_name(si_map_keeper[s.si]) << endl;
#endif // DEBUG_TRANSSTACK
			si_map_keeper.erase(s.si);
			s.ownership = TSO_OUTSIDE_TS;

		} else if (t == SI_UNDEF) {

			si_map_keeper[s.si] = SI_DELETED;
#ifdef DEBUG_TRANSSTACK
			cout << "NodeStack::nodestack_pop(), added [" << s.si->get_object_serial() <<
				"] as " << debug_keeper_type_name(si_map_keeper[s.si]) << endl;
#endif // DEBUG_TRANSSTACK
			s.ownership = TSO_OWNED_BY_TS;

		} else {
			throw(CalcFatal(__FILE__, __LINE__, "inconsistent keeper item values in nodestack_pop() (STACK_USE_MAP defined)"));
		}
	} else {
		s.ownership = TSO_OUTSIDE_TS;
	}
	return s;
}

#endif // STACK_USE_MAP


//
// Exec
//

#ifdef DEBUG_CLASS_COUNT
long Exec::class_count = 0;
#endif

Exec::Exec(const SIO& ss, VarDirectory *l) : s(ss), lvars(l), ip(0) {
	//cout << "Creation of a new exec level: " << s.ownership << " / " << s.si << endl;
#ifdef DEBUG_CLASS_COUNT
	class_count++;
#endif
}

Exec::~Exec() {
	//cout << "Deleting one Exec: " << this << "\n";
#ifdef DEBUG_CLASS_COUNT
	class_count--;
#endif
}

Exec::Exec(const Exec& e) : s(e.s), lvars(e.lvars), ip(e.ip) {
	//cout << "Creating one Exec: " << this << "\n";
#ifdef DEBUG_CLASS_COUNT
	class_count++;
#endif
}


//
// TransStack::Path
//

TransStack::TSVars::TSVars() : tree(new Tree()), si_path(0) { }

TransStack::TSVars::~TSVars() {
	if (si_path != 0)
		delete si_path;
	delete tree;
}

void TransStack::TSVars::get_path(vector<string>*& directories) const { tree->get_path(directories); }

StackItemList *TransStack::TSVars::get_si_path() const {
	vector<string> *directories;
	tree->get_path(directories);
	StackItemList *l = new StackItemList();
	for (vector<string>::iterator it = directories->begin(); it != directories->end(); it++) {
		l->append_item(new StackItemExpression(*it, false));
	}
	delete directories;
	return l;
}

bool TransStack::TSVars::update_si_path() {
	if (si_path != NULL && !tree->get_pwd_has_changed())
		return false;
	if (si_path != NULL)
		delete si_path;
	si_path = get_si_path();
	tree->reset_pwd_has_changed();
	ToString tostr(TOSTRING_DISPLAY, 1, 0);
	si_path->to_string(tostr);
	const vector<string> *p = tostr.get_value();
	s = (*p)[0];
	return true;
}

const string& TransStack::TSVars::get_si_path_string() const { return s; }
void TransStack::TSVars::get_var_list(vector<string>*& by_order) const { tree->get_var_list(by_order); }
void TransStack::TSVars::home() { tree->home(); }
void TransStack::TSVars::up() { tree->up(); }
VarDirectory *TransStack::TSVars::save_pwd() const { return tree->save_pwd(); }
void TransStack::TSVars::recall_pwd(VarDirectory* d) { tree->recall_pwd(d); }


//
// TransStack
//

#ifdef DEBUG_CLASS_COUNT
long TransStack::class_count = 0;
#endif

TransStack::TransStack(const bool& tc, const bool& ah, vector<Exec> *ex, const int& ul) : count(0), modified_flag(true),
		exec_stack(ex), undo_levels(ul), exec_mode(EXECMODE_RUN), temporary_copy(tc), allow_halt(ah) {
#ifdef DEBUG_CLASS_COUNT
		class_count++;
#endif
	tail = new NodeStack(0, 0, 0);
	head = tail;
	if (exec_stack == NULL)
		exec_stack = new vector<Exec>;
	ground_level = exec_stack->size();
}

TransStack::~TransStack() {
#ifdef DEBUG_CLASS_COUNT
		class_count--;
#endif
#ifdef DEBUG_TRANSSTACK
	cout << "TransStack::~TransStack() (start), count = " << count << endl;
#endif // DEBUG_TRANSSTACK

	clear_exec_stack();

	if (!temporary_copy)
		delete exec_stack;

	forward_head();
	while (count > 0)
		forward_tail();
	if (tail != head)
		throw(CalcFatal(__FILE__, __LINE__, "inconsistent data in TransStack::~TransStack()"));
	head->st->take_ownership();
	delete head;

#ifdef DEBUG_TRANSSTACK
	cout << "TransStack::~TransStack() (end), count = " << count << endl;
#endif // DEBUG_TRANSSTACK

}

bool TransStack::get_modified_flag() const { return modified_flag; }
void TransStack::set_modified_flag(const bool& m) { modified_flag = m; }

void TransStack::forward_tail() {

#ifdef DEBUG_TRANSSTACK
	cout << "TransStack::forward_tail() (start), count = " << count << endl;
#endif // DEBUG_TRANSSTACK

	if (count <= 0)
		return;
	if (tail->next == 0)
		throw(CalcFatal(__FILE__, __LINE__, "inconsistent data in TransStack::forward_tail()"));
	NodeStack *old_tail = tail;
	tail = tail->next;
	tail->previous = 0;
	delete old_tail;
	count--;

#ifdef DEBUG_TRANSSTACK
	cout << "TransStack::forward_tail() (end), count = " << count << endl;
	cout << "\ttail = [" << tail->get_object_serial() << "], head = [" << head->get_object_serial() << "]" << endl;
	cout << "\ttail.count = " << tail->si_map_keeper.size() << ", head.count = " << head->si_map_keeper.size() << endl;
#endif // DEBUG_TRANSSTACK

}

void TransStack::control_undos_chain_size() {
	while ((undo_levels == 0 && count >= 1) || (undo_levels > 0 && count > undo_levels + 1))
		forward_tail();
}

void TransStack::forward_head() {

#ifdef DEBUG_TRANSSTACK
	cout << "TransStack::forward_head() (start), count = " << count << endl;
#endif // DEBUG_TRANSSTACK

	NodeStack *old_head = head;
	head = new NodeStack(old_head, 0, old_head->st);
	old_head->next = head;
	count++;
	control_undos_chain_size();
	set_modified_flag(false);

#ifdef DEBUG_TRANSSTACK
	cout << "TransStack::forward_head() (end), count = " << count << endl;
	cout << "\ttail = [" << tail->get_object_serial() << "], head = [" << head->get_object_serial() << "]" << endl;
	cout << "\ttail.count = " << tail->si_map_keeper.size() << ", head.count = " << head->si_map_keeper.size() << endl;
#endif // DEBUG_TRANSSTACK

}

void TransStack::backward_head() {

#ifdef DEBUG_TRANSSTACK
	cout << "TransStack::backward_head() (start), count = " << count << endl;
#endif // DEBUG_TRANSSTACK

	if (count <= 0)
		return;
	NodeStack *old_head = head;
	head = head->previous;
	delete old_head;
	head->next = 0;
	count--;
	set_modified_flag(true);

#ifdef DEBUG_TRANSSTACK
	cout << "TransStack::backward_head() (end), count = " << count << endl;
	cout << "\ttail = [" << tail->get_object_serial() << "], head = [" << head->get_object_serial() << "]" << endl;
	cout << "\ttail.count = " << tail->si_map_keeper.size() << ", head.count = " << head->si_map_keeper.size() << endl;
#endif // DEBUG_TRANSSTACK

}

int TransStack::get_undo_levels() const { return undo_levels; }

void TransStack::set_undo_levels(int ul) {
	if (ul < -1)
		ul = -1;
	  // z used to avoid a warning with Microsoft Visual C++ 2010 Express
	  // "warning C4127: conditional expression is constant"
	int z = 0;
	if (HARD_MAX_UNDO_LEVELS >= z && ul > HARD_MAX_UNDO_LEVELS)
		ul = HARD_MAX_UNDO_LEVELS;
	undo_levels = ul;
}

int TransStack::transstack_push(StackItem *si) {
	set_modified_flag(true);
	return head->nodestack_push(si, undo_levels != 0);
}

int TransStack::transstack_push_SIO(SIO& s) {
	s.ownership = TSO_OWNED_BY_TS;
	return transstack_push(s.si);
}

SIO TransStack::transstack_pop() {
	set_modified_flag(true);
	return head->nodestack_pop(undo_levels != 0);
}

st_err_t TransStack::read_integer(int& n) {
	if (stack_get_count() < 1)
		return ST_ERR_TOO_FEW_ARGUMENTS;
	SIO s = transstack_pop();
	st_err_t c = s.si->to_integer(n);
	if (c != ST_ERR_OK)
		transstack_push(s.si);
	else
		s.cleanup();
	return c;
}

inline const string TransStack::transstack_get_display_line(const DisplayStackLayout& dsl, const int& line_number,
						IntervalShift& ishift, bool& recalc, bool& no_more_lines, int& item_number) {
	return head->st->get_display_line(dsl, line_number, ishift, recalc, no_more_lines, item_number);
}

st_err_t TransStack::inner_push_eval(const eval_t& et, SIO& s, const bool& inside_undo_sequence, string& cmd_err) {
	manage_si_t msi;
	StackItem *si;

	if (!inside_undo_sequence && get_modified_flag())
		forward_head();

	//cout << "A1: evaluating \"" << simple_string(s.si) << "\"\n";
	//cout << "A1:   get_type() = " << s.si->get_type() << endl;

	st_err_t c = s.si->eval(et, *this, msi, cmd_err);

	if (c == ST_ERR_OK && msi == MANAGE_SI_PUSH) {
		si = s.si;
		if (s.ownership != TSO_OWNED_BY_TS)
			si = s.si->dup();
		transstack_push(si);
	} else {
		if (c == ST_ERR_OK && msi == MANAGE_SI_EXEC) {
			bool feed_with_stack;
			VarDirectory *v = s.si->get_lvars(feed_with_stack);
			if (s.ownership == TSO_OUTSIDE_TS)
				s.ownership = TSO_OWNED_BY_TS;
			exec_stack->push_back(Exec(s, v));
			if (feed_with_stack)
				c = read_lvars(v, cmd_err);
		} else if (msi != MANAGE_SI_EXEC && s.ownership == TSO_OWNED_BY_TS)
			delete s.si;
	}
	return c;
}

st_err_t TransStack::do_push_eval(SIO& s, const bool& inside_undo_sequence, string& cmd_err, const eval_t& et) {
	st_err_t c = inner_push_eval(et, s, inside_undo_sequence, cmd_err);
	while (c == ST_ERR_OK && exec_stack->size() > ground_level && exec_mode == EXECMODE_RUN) {
		c = exec1(cmd_err);
	}

//    debug_write_i("Size of exec_stack = %i", static_cast<int>(exec_stack->size()));

	if (c != ST_ERR_OK && !temporary_copy && allow_halt && exec_stack->size() >= 1)
		exec_mode = EXECMODE_HALT;
	if (exec_mode != EXECMODE_HALT)
		clear_exec_stack();

#ifdef DEBUG_TRANSSTACK
	cout << "TransStack::tail = [" << tail->get_object_serial() << "], ::head = [" << head->get_object_serial() <<
		"], ::count = " << count << ", ::modified_flag = " << modified_flag << endl;
#endif // DEBUG_TRANSSTACK

	return c;
}

void TransStack::halt() { exec_mode = EXECMODE_HALT; }
st_err_t TransStack::sst(string& cmd_err) { return exec1(cmd_err); }
void TransStack::abort() {
	if (exec_stack->size() > ground_level)
		pop_exec_stack();
}
void TransStack::kill() { clear_exec_stack(); }
void TransStack::cont() { exec_mode = EXECMODE_RUN; }
void TransStack::get_next_instruction(string& s) const {
	if (exec_stack->empty()) {
		s = "";
		return;
	}
	Exec& e = exec_stack->back();
	e.s.si->next_instruction(e.ip, s);
}

size_t TransStack::stack_get_count() const { return head->st->get_count(); }

const StackItem *TransStack::transstack_get_const_si(const int& stack_item_number) const {
	return head->st->stack_get_const_si(stack_item_number);
}

st_err_t TransStack::exec1(string& cmd_err) {
	if (exec_stack->empty()) {
		exec_mode = EXECMODE_RUN;
		return ST_ERR_OK;
	}
	Exec& e = exec_stack->back();

	int ip_backup = e.ip;
	st_err_t c = e.s.si->exec1(*this, e.ip, cmd_err);
	if (c != ST_ERR_OK)
		e.ip = ip_backup;

	if (c != ST_ERR_OK)
		return c;
	Exec& e2 = exec_stack->back();
	if (e2.ip == IP_END)
		pop_exec_stack();
	return ST_ERR_OK;
}

void TransStack::push_exec_stack(SIO s, VarDirectory *lvars) {
	exec_stack->push_back(Exec(s, lvars));
}

void TransStack::clear_exec_stack() {
	while (exec_stack->size() > ground_level)
		pop_exec_stack();
	exec_mode = EXECMODE_RUN;
}

size_t TransStack::pop_exec_stack() {
	if (exec_stack->size() <= ground_level) {
		exec_mode = EXECMODE_RUN;
		return exec_stack->size();
	}
	Exec& e = exec_stack->back();
	if (e.s.ownership == TSO_OWNED_BY_TS)
		delete e.s.si;
	else
		e.s.si->clear_lvars();
	exec_stack->pop_back();
	if (exec_stack->size() <= ground_level)
		exec_mode = EXECMODE_RUN;
	return exec_stack->size();
}

st_err_t TransStack::read_lvars(VarDirectory *v, string& cmd_err) {
	if (!v)
		return ST_ERR_OK;
	if (stack_get_count() < v->get_size()) {
		cmd_err = "->";
		return ST_ERR_TOO_FEW_ARGUMENTS;
	}
	SIO s;
	vector<string> *by_order;
	v->get_vars(by_order);
	StackItem *si;
	for (vector<string>::const_iterator it = by_order->begin(); it != by_order->end(); it++) {
		s = transstack_pop();
		get_si_dup_if_necessary(s, si);
		if (v->sto(*it, si) != ST_ERR_OK)
			throw(CalcFatal(__FILE__, __LINE__, "unable to execute lvars->sto()"));
	}
	return ST_ERR_OK;
}

VarFile *TransStack::get_local_var(const string& s) {
	Var *e;
	for (std::vector<Exec>::reverse_iterator it = exec_stack->rbegin(); it != exec_stack->rend(); it++) {
		if (it->lvars != NULL && (e = it->lvars->get_var(s)) != NULL) {
			if (e->get_type() == VAR_FILE)
				return dynamic_cast<VarFile*>(e);
			throw(CalcFatal(__FILE__, __LINE__, "TransStack::get_local_var(): found a local directory!!!??"));
		}
	}
	return NULL;
}

st_err_t TransStack::crdir(const string& s) { return vars.tree->crdir(s); }
st_err_t TransStack::sto(const string& s, StackItem* const si, const bool& allow_local) {
	if (allow_local) {
		VarFile *vf = get_local_var(s);
		if (vf) {
			vf->attach_si(si);
			return ST_ERR_OK;
		}
	}
	return vars.tree->sto(s, si);
}

st_err_t TransStack::purge(const string& s) { return vars.tree->purge(s); }

st_err_t TransStack::rcl(const string& s, StackItem*& si, Var*& e, const bool& allow_local) {
	if (allow_local) {
		VarFile *vf = get_local_var(s);
		if (vf) {
			si = vf->get_si_dup();
			return ST_ERR_OK;
		}
	}
	return vars.tree->rcl(s, si, e);
}

st_err_t TransStack::rcl_const(const string& s, const StackItem*& si, Var*& e) {
	return vars.tree->rcl_const(s, si, e);
}

st_err_t TransStack::rcl_for_eval(const string& s, StackItem*& si, const bool& allow_local) {
	if (allow_local) {
		VarFile *vf = get_local_var(s);
		if (vf) {
			si = vf->get_si_dup();
			return ST_ERR_OK;
		}
	}
	return vars.tree->rcl_for_eval(s, si);
}


//
// StackItem
//

#ifdef DEBUG_CLASS_COUNT
long StackItem::class_count = 0;
#endif
#ifdef DEBUG_TRANSSTACK
int StackItem::class_serial = 0;
#endif

string StackItem::empty_string = "";

StackItem::StackItem() {
	//cout << "Creating Z1: " << this << endl;
#ifdef DEBUG_TRANSSTACK
	object_serial = class_serial++;
#endif
#ifdef DEBUG_CLASS_COUNT
	class_count++;
#endif
}
StackItem::~StackItem() {
	//cout << "Deleting Z1: " << this << endl;
#ifdef DEBUG_CLASS_COUNT
	class_count--;
#endif
}

st_err_t StackItem::op_equal_generic(StackItem& arg2, StackItem*& ret) {
	ret = new StackItemReal(Real(arg2.same(this) ? 1 : 0));
	return ST_ERR_OK;
}
st_err_t StackItem::op_notequal_generic(StackItem& arg2, StackItem*& ret) {
	ret = new StackItemReal(Real(arg2.same(this) ? 0 : 1));
	return ST_ERR_OK;
}
st_err_t StackItem::op_equal(StackItemExpression* arg2, StackItem*& ret) {
	ret = new StackItemReal(Real(arg2->same(this) ? 1 : 0));
	return ST_ERR_OK;
}
st_err_t StackItem::op_notequal(StackItemExpression* arg2, StackItem*& ret) {
	ret = new StackItemReal(Real(arg2->same(this) ? 0 : 1));
	return ST_ERR_OK;
}

bool StackItem::is_varname() const { return false; }

bool StackItem::is_unquoted_varname() const { return false; }

const string& StackItem::get_varname() const { return empty_string; }

st_err_t StackItem::eval(const eval_t&, TransStack&, manage_si_t& msi, string& cmd_err) {
	msi = MANAGE_SI_PUSH;
	cmd_err = "";
	return ST_ERR_OK;
}

st_err_t StackItem::exec1(TransStack&, int& ip, string& cmd_err) {
	ip = IP_END;
	cmd_err = "";
	return ST_ERR_OK;
}

void StackItem::next_instruction(const int&, string& s) const {
	s = simple_string(this);
}

VarDirectory *StackItem::get_lvars(bool& feed_with_stack) const {
	feed_with_stack = false;
	return NULL;
}

void StackItem::clear_lvars() { }

StackItem* StackItem::forge_list_size(const Coordinates& coord) {
	StackItemList *sil = new StackItemList();
	sil->append_item(new StackItemReal(Real(coord.i)));
	if (coord.d == DIM_MATRIX)
		sil->append_item(new StackItemReal(Real(coord.j)));
	return sil;
}

st_err_t StackItem::op_get(TransStack& ts, StackItemList *sil, StackItem*& ret) {
	Coordinates coord;
	st_err_t c = get_coordinates(ts, sil, coord);
	if (c != ST_ERR_OK)
		return c;
	ret = sil->get_dup_nth_item(coord.i);
	return ST_ERR_OK;
}

#define IMPLEMENT_SCALAR_OP_GET(SCTYPE) \
st_err_t StackItem::op_get(TransStack& ts, StackItemMatrix##SCTYPE *sim, StackItem*& ret) { \
	Coordinates coord; \
	st_err_t c = get_coordinates(ts, sim, coord); \
	if (c != ST_ERR_OK) \
		return c; \
	ret = new StackItem##SCTYPE(sim->get_value(coord)); \
	return ST_ERR_OK; \
}
IMPLEMENT_SCALAR_OP_GET(Real)
IMPLEMENT_SCALAR_OP_GET(Cplx)


//
// StackItemBuiltinCommand
//

  // Base

static st_err_t bc_help(TransStack&, SIO*, string&) {
	ui_display_help(DH_MAIN);
	return ST_ERR_OK;
}
static st_err_t bc_help_flags(TransStack&, SIO*, string&) {
	ui_display_help(DH_FLAGS);
	return ST_ERR_OK;
}
static st_err_t bc_exit(TransStack&, SIO*, string&) { return ST_ERR_EXIT; }

static st_err_t bc_about(TransStack&, SIO*, string&) {
	ui_set_error(PACKAGE_STRING DEBUG_VERSION_STRING, "Sébastien Millet 2010");
	return ST_ERR_OK;
}

  // Arithmetic

void prepare_arith();

#define IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(OP) \
static st_err_t bc_##OP(StackItem& op1, StackItem& op2, StackItem*& ret, string&) { \
	prepare_arith(); \
	return op1.op_##OP##_generic(op2, ret); \
}
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(add)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(sub)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(mul)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(div)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(pow)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(percent)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(percent_ch)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(percent_t)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(mod)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(min)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(max)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(hms_add)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(hms_sub)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(cross)
IMPLEMENT_BC_TWO_ARGS_GENERIC_FUNC(dot)


  // Real functions

#define IMPLEMENT_BC_OP_FUNC(OP) \
static st_err_t bc_##OP(StackItem& op1, StackItem*& ret, string&) { \
	prepare_arith(); \
	return op1.op_##OP(ret); \
}
IMPLEMENT_BC_OP_FUNC(cos)
IMPLEMENT_BC_OP_FUNC(sin)
IMPLEMENT_BC_OP_FUNC(tan)
IMPLEMENT_BC_OP_FUNC(acos)
IMPLEMENT_BC_OP_FUNC(asin)
IMPLEMENT_BC_OP_FUNC(atan)
IMPLEMENT_BC_OP_FUNC(ln)
IMPLEMENT_BC_OP_FUNC(exp)
IMPLEMENT_BC_OP_FUNC(cosh)
IMPLEMENT_BC_OP_FUNC(sinh)
IMPLEMENT_BC_OP_FUNC(tanh)
IMPLEMENT_BC_OP_FUNC(acosh)
IMPLEMENT_BC_OP_FUNC(asinh)
IMPLEMENT_BC_OP_FUNC(atanh)
IMPLEMENT_BC_OP_FUNC(neg)
IMPLEMENT_BC_OP_FUNC(ip)
IMPLEMENT_BC_OP_FUNC(fp)
IMPLEMENT_BC_OP_FUNC(floor)
IMPLEMENT_BC_OP_FUNC(ceil)
IMPLEMENT_BC_OP_FUNC(abs)
IMPLEMENT_BC_OP_FUNC(sign)
IMPLEMENT_BC_OP_FUNC(mant)
IMPLEMENT_BC_OP_FUNC(xpon)
IMPLEMENT_BC_OP_FUNC(inv)
IMPLEMENT_BC_OP_FUNC(sq)
IMPLEMENT_BC_OP_FUNC(sqr)
IMPLEMENT_BC_OP_FUNC(re)
IMPLEMENT_BC_OP_FUNC(im)
IMPLEMENT_BC_OP_FUNC(conj)
IMPLEMENT_BC_OP_FUNC(arg)
IMPLEMENT_BC_OP_FUNC(p_to_r)
IMPLEMENT_BC_OP_FUNC(r_to_p)
IMPLEMENT_BC_OP_FUNC(to_hms)
IMPLEMENT_BC_OP_FUNC(hms_to)
IMPLEMENT_BC_OP_FUNC(d_to_r)
IMPLEMENT_BC_OP_FUNC(r_to_d)
IMPLEMENT_BC_OP_FUNC(log)
IMPLEMENT_BC_OP_FUNC(alog)
IMPLEMENT_BC_OP_FUNC(lnp1)
IMPLEMENT_BC_OP_FUNC(expm)

static st_err_t bc_minr(StackItem*& si, string&) { si = new StackItemReal(Real(MINR)); return ST_ERR_OK; }
static st_err_t bc_maxr(StackItem*& si, string&) { si = new StackItemReal(Real(MAXR)); return ST_ERR_OK; }

static st_err_t bc_rad(TransStack& ts, SIO* args, string&) {
	F->set_angle_mode(ANGLE_RAD);
	return ST_ERR_OK;
}

static st_err_t bc_deg(TransStack& ts, SIO* args, string&) {
	F->set_angle_mode(ANGLE_DEG);
	return ST_ERR_OK;
}

static st_err_t bc_r_to_c(StackItem& op1, StackItem& op2, StackItem*& ret, string&) {
	prepare_arith();
	return op1.op_r_to_c_generic(op2, ret);
}

static st_err_t bc_c_to_r(TransStack& ts, SIO *args, string&) {
	prepare_arith();
	return args[0].si->op_c_to_r(ts);
}

  // Comparison functions

static st_err_t bc_same(StackItem& op1, StackItem& op2, StackItem*& ret, string&) {
	int res = op1.same(&op2) ? 1 : 0;
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

#define IMPLEMENT_OP_LOGICAL(OP) \
static st_err_t bc_##OP(StackItem& op1, StackItem& op2, StackItem*& ret, string&) { \
	return op1.op_##OP##_generic(op2, ret); \
}
IMPLEMENT_OP_LOGICAL(equal)
IMPLEMENT_OP_LOGICAL(notequal)
IMPLEMENT_OP_LOGICAL(lower)
IMPLEMENT_OP_LOGICAL(lower_or_equal)
IMPLEMENT_OP_LOGICAL(greater)
IMPLEMENT_OP_LOGICAL(greater_or_equal)
IMPLEMENT_OP_LOGICAL(and)
IMPLEMENT_OP_LOGICAL(or)
IMPLEMENT_OP_LOGICAL(xor)

static st_err_t bc_not(StackItem& op1, StackItem*& ret, string&) {
	return op1.op_not(ret);
}

  // Stack manipulation commands

static st_err_t bc_dup(TransStack& ts, SIO *args, string&) {
	ts.transstack_push_SIO(args[0]);
	ts.transstack_push(args[0].si->dup());
	return ST_ERR_OK;
}

static st_err_t bc_swap(TransStack& ts, SIO *args, string&) {
	ts.transstack_push_SIO(args[1]);
	ts.transstack_push_SIO(args[0]);
	return ST_ERR_OK;
}

static st_err_t bc_drop(TransStack&, SIO*, string&) { return ST_ERR_OK; }

static st_err_t bc_clear(TransStack& ts, SIO*, string&) {
	while (ts.stack_get_count() >= 1)
		ts.transstack_pop().cleanup();
	return ST_ERR_OK;
}

static st_err_t bc_over(TransStack& ts, SIO *args, string&) {
	ts.transstack_push_SIO(args[0]);
	ts.transstack_push_SIO(args[1]);
	ts.transstack_push(args[0].si->dup());
	return ST_ERR_OK;
}

static st_err_t bc_dup2(TransStack& ts, SIO *args, string&) {
	ts.transstack_push_SIO(args[0]);
	ts.transstack_push_SIO(args[1]);
	for (int i = 0; i <= 1; i++)
		ts.transstack_push(args[i].si->dup());
	return ST_ERR_OK;
}

  // Identical of bc_drop(), yes, I know, but I find it cleaner to define a dedicated
  // function for drop2, instead of re-using bc_drop().
static st_err_t bc_drop2(TransStack&, SIO*, string&) { return ST_ERR_OK; }

static st_err_t bc_rot(TransStack& ts, SIO *args, string&) {
	ts.transstack_push_SIO(args[1]);
	ts.transstack_push_SIO(args[2]);
	ts.transstack_push_SIO(args[0]);
	return ST_ERR_OK;
}

  // Used by ROLLD, ROLL, PICK, DUPN and DROPN commands, to retrieve the first argument = the value
  // of "N", N being the number of items to operate on.
  // Also used by ->LIST command
static st_err_t stack_nwrapper(TransStack& ts, const StackItem& op, int& n, SIO*& items) {
	st_err_t c = op.to_integer(n);
	if (c != ST_ERR_OK)
		return c;
	  // HP-28S behavior: nul or negative values are silently eaten by ROLLD, ROLL, etc., functions,
	  // doing nothing else than just consuming the first numeric argument.
	if (n < 1)
		return ST_ERR_OK;
	else if (static_cast<size_t>(n) > ts.stack_get_count())
		return ST_ERR_TOO_FEW_ARGUMENTS;
	items = new SIO[n];
	for (int i = n - 1; i >= 0; i--)
		items[i] = ts.transstack_pop();
	return ST_ERR_EXIT;
}

static st_err_t bc_rolld(TransStack& ts, SIO *args, string&) {
	int n;
	SIO *items = 0;
	st_err_t c = stack_nwrapper(ts, *args[0].si, n, items);
	if (c != ST_ERR_EXIT)
		return c;
	ts.transstack_push(items[n - 1].si);
	for (int i = 0; i < n - 1; i++)
		ts.transstack_push(items[i].si);
	delete []items;
	return ST_ERR_OK;
}

static st_err_t bc_roll(TransStack& ts, SIO *args, string&) {
	int n;
	SIO *items = 0;
	st_err_t c = stack_nwrapper(ts, *args[0].si, n, items);
	if (c != ST_ERR_EXIT)
		return c;
	for (int i = 1; i < n; i++)
		ts.transstack_push(items[i].si);
	ts.transstack_push(items[0].si);
	delete []items;
	return ST_ERR_OK;
}

static st_err_t bc_pick(TransStack& ts, SIO *args, string&) {
	int n;
	SIO *items = 0;
	st_err_t c = stack_nwrapper(ts, *args[0].si, n, items);
	if (c != ST_ERR_EXIT)
		return c;
	for (int i = 0; i < n ; i++)
		ts.transstack_push(items[i].si);
	ts.transstack_push(items[0].si->dup());
	delete []items;
	return ST_ERR_OK;
}

static st_err_t bc_dupn(TransStack& ts, SIO *args, string&) {
	int n;
	SIO *items = 0;
	st_err_t c = stack_nwrapper(ts, *args[0].si, n, items);
	if (c != ST_ERR_EXIT)
		return c;
	for (int i = 0; i < n; i++)
		ts.transstack_push(items[i].si);
	for (int i = 0; i < n; i++)
		ts.transstack_push(items[i].si->dup());
	delete []items;
	return ST_ERR_OK;
}

static st_err_t bc_dropn(TransStack& ts, SIO *args, string&) {
	int n;
	SIO *items = 0;
	st_err_t c = stack_nwrapper(ts, *args[0].si, n, items);
	if (c != ST_ERR_EXIT)
		return c;
	for (int i = 0; i < n; i++)
		items[i].cleanup();
	delete []items;
	return ST_ERR_OK;
}

static st_err_t bc_depth(TransStack& ts, SIO*, string&) {
	ts.transstack_push(new StackItemReal(Real(ts.stack_get_count())));
	return ST_ERR_OK;
}

  // Flags

static st_err_t get_flag_number(const StackItem& op, int& n) {
	st_err_t c = op.to_integer(n);
	if (c != ST_ERR_OK)
		return c;
	if (n >= 1 && n <= FL_TAG_IT_END)
		return ST_ERR_OK;
	return ST_ERR_BAD_ARGUMENT_VALUE;
}

static st_err_t update_flag(const StackItem& op, bool value) {
	int n;
	st_err_t c = get_flag_number(op, n);
	if (c == ST_ERR_OK)
		F->set(n, value);
	return c;
}

static st_err_t get_flag_value(const StackItem& op, StackItem*& ret, const bool& value) {
	int n;
	st_err_t c = get_flag_number(op, n);
	if (c == ST_ERR_OK) {
		ret = new StackItemReal(Real(F->get(n) == value ? 1 : 0));
	}
	return c;
}

static st_err_t get_flag_value_and_update(const StackItem& op, StackItem*& ret, const bool& value) {
	int n;
	st_err_t c = get_flag_number(op, n);
	if (c == ST_ERR_OK) {
		ret = new StackItemReal(Real(F->get(n) == value ? 1 : 0));
		F->set(n, false);
	}
	return c;
}

static st_err_t bc_sf(StackItem& op1, StackItem*&, string&) { return update_flag(op1, true); }
static st_err_t bc_cf(StackItem& op1, StackItem*&, string&) { return update_flag(op1, false); }
static st_err_t bc_fs_get(StackItem& op1, StackItem*& ret, string&) { return get_flag_value(op1, ret, true); }
static st_err_t bc_fc_get(StackItem& op1, StackItem*& ret, string&) { return get_flag_value(op1, ret, false); }
static st_err_t bc_fs_get_clear(StackItem& op1, StackItem*& ret, string&) { return get_flag_value_and_update(op1, ret, true); }
static st_err_t bc_fc_get_clear(StackItem& op1, StackItem*& ret, string&) { return get_flag_value_and_update(op1, ret, false); }

  // String

static st_err_t bc_to_str(StackItem& op1, StackItem*& ret, string&) {
	ToString tostr(TOSTRING_DISPLAY, 1, 0);
	op1.to_string(tostr);
	const vector<string> *p = tostr.get_value();
	ret = new StackItemString((*p)[0]);
	return ST_ERR_OK;
}

static st_err_t bc_str_to(TransStack& ts, SIO*, string& cmd_err) {
	if (ts.stack_get_count() < 1)
		return ST_ERR_TOO_FEW_ARGUMENTS;

	SIO s = ts.transstack_pop();
	st_err_t c = s.si->op_str_to(ts, cmd_err);

	if (c == ST_ERR_SYNTAX_ERROR) {
		  // Normally, a simple ts.backward_head() is necessary here.
		  // I do an additional forward followed by a backward do reflect the behavior
		  // of my HP-28S calculator: when a syntax error occurs during a STR-> command,
		  // an UNDO executed right after won't recall the stack state as before the string
		  // was entered. Of course this is true only when undo levels is set to 1. Any way
		  // I do an additional forward/backward (whatever undo levels is set to)
		  // so as to decrease the number of experienced undo levels by 1.
		ts.forward_head();
		ts.backward_head();
		ts.backward_head();
		s.cleanup();
		cmd_err = "STR->";
	} else if (c == ST_ERR_STR_TO_BAD_ARGUMENT_TYPE && F->get(FL_LAST))
		ts.transstack_push(s.si);
	else
		s.cleanup();

	return c;
}

static st_err_t bc_cmdsub(StackItem& op1, StackItem& op2, StackItem& op3, StackItem*& ret, string&) {
	int n1, n2 = 0;
	st_err_t c = op2.to_integer(n1);
	if (c == ST_ERR_OK)
		c = op3.to_integer(n2);
	if (c != ST_ERR_OK)
		return c;
	if (n2 < n1) {
		n1 = 2;
		n2 = 1;
	}
	if (n1 < 1)
		n1 = 1;
	if (n2 < 1)
		n2 = 1;
	return op1.op_cmdsub(n1, n2, ret);
}

  // Lists

static st_err_t bc_to_list(TransStack& ts, SIO *args, string&) {
	int n;
	SIO *items = 0;
	st_err_t c = stack_nwrapper(ts, *args[0].si, n, items);
	if (c != ST_ERR_EXIT && c != ST_ERR_OK)
		return c;
	if (c == ST_ERR_OK && n >= 1)
		throw(CalcFatal(__FILE__, __LINE__, "bc_to_list(): inconsistent data"));
	StackItemList *sil = new StackItemList();
	StackItem *ready_si;
	for (int i = 0; i < n; i++) {
		get_ready_si(items[i], ready_si);
		sil->append_item(ready_si);
	}
	ts.transstack_push(sil);
	if (c == ST_ERR_EXIT)
		delete []items;
	return ST_ERR_OK;
}

static st_err_t bc_list_to(TransStack& ts, SIO *args, string&) { return args[0].si->op_list_to(ts, args[0].ownership); }

static st_err_t bc_get(TransStack& ts, SIO *args, string&) {
	StackItem *ret;
	st_err_t c = args[0].si->op_get_generic(ts, *(args[1].si), ret);
	if (c == ST_ERR_OK)
		ts.transstack_push(ret);
	return c;
}

static void pushback_incremented_bounds(TransStack& ts, SIO *args, const bool& push_args0) {
	Coordinates bounds;
	st_err_t c;
	if ((c = args[0].si->get_bounds(bounds)) != ST_ERR_OK)
		throw(CalcFatal(__FILE__, __LINE__, "pushback_incremented_bounds(): args[0].si should answer to get_bounds()!"));
	if (push_args0)
		ts.transstack_push_SIO(args[0]);
	StackItem *ready_si;
	get_ready_si(args[1], ready_si);
	c = ready_si->increment_coord(ts, bounds);
	if (c != ST_ERR_OK)
		throw(CalcFatal(__FILE__, __LINE__, "pushback_incremented_bounds(): increment_coord() returned an error code!!!?"));
	ts.transstack_push(ready_si);
}

static st_err_t bc_geti(TransStack& ts, SIO *args, string&) {
	StackItem *ret;
	st_err_t c = args[0].si->op_get_generic(ts, *(args[1].si), ret);
	if (c == ST_ERR_OK) {
		pushback_incremented_bounds(ts, args, true);
		ts.transstack_push(ret);
	}
	return c;
}

static st_err_t bc_put(TransStack& ts, SIO *args, string&) {
	StackItem *ready_si;
	get_ready_si(args[0], ready_si);
	st_err_t c = ready_si->op_put_generic(ts, *(args[1].si), args[2]);
	if (c == ST_ERR_OK)
		ts.transstack_push(ready_si);
	else if (args[0].si != ready_si)
		delete ready_si;
	return c;
}

static st_err_t bc_puti(TransStack& ts, SIO *args, string& cmd_err) {
	st_err_t c = bc_put(ts, args, cmd_err);
	if (c == ST_ERR_OK)
		pushback_incremented_bounds(ts, args, false);
	return c;
}

static st_err_t bc_size(StackItem& op1, StackItem*& ret, string&) { return op1.op_size(ret); }

static st_err_t bc_arry_to(TransStack& ts, SIO *args, string&) { return args[0].si->op_arry_to(ts); }

static st_err_t bc_to_arry(TransStack& ts, SIO *args, string&) { return args[0].si->op_to_arry(ts); }

static st_err_t bc_con(TransStack& ts, SIO *args, string&) {
	StackItem *ret;
	st_err_t c = args[0].si->op_con_generic(ts, *(args[1].si), ret);
	if (c == ST_ERR_OK)
		ts.transstack_push(ret);
	return c;
}
static st_err_t bc_trn(StackItem& op1, StackItem*& ret, string&) { return op1.op_trn(ret); }
static st_err_t bc_rdm(TransStack& ts, SIO *args, string&) {
	StackItem *ret;
	st_err_t c = args[0].si->op_rdm_generic(ts, *(args[1].si), ret);
	if (c == ST_ERR_OK)
		ts.transstack_push(ret);
	return c;
}
static st_err_t bc_idn(StackItem& op1, StackItem*& ret, string&) { return op1.op_idn(ret); }

  // Variables

static st_err_t bc_sto(TransStack& ts, SIO *args, string&) { return args[1].si->op_sto(ts, args[0]); }

static st_err_t bc_rcl(TransStack& ts, SIO *args, string&) { return args[0].si->op_rcl(ts); }

static st_err_t bc_purge(TransStack& ts, SIO *args, string&) { return args[0].si->op_purge(ts); }

static st_err_t bc_vars(TransStack& ts, SIO*, string&) {
	vector<string> *by_order;
	ts.vars.get_var_list(by_order);
	StackItemList *l = new StackItemList();
	for (vector<string>::iterator it = by_order->begin(); it != by_order->end(); it++) {
		l->append_item(new StackItemExpression(*it, true));
	}
	delete by_order;
	ts.transstack_push(l);
	return ST_ERR_OK;
}

static st_err_t bc_path(TransStack& ts, SIO*, string&) {
	ts.transstack_push(ts.vars.get_si_path());
	return ST_ERR_OK;
}

static st_err_t bc_home(TransStack& ts, SIO*, string&) {
	ts.vars.home();
	return ST_ERR_OK;
}

static st_err_t bc_up(TransStack& ts, SIO*, string&) {
	ts.vars.up();
	return ST_ERR_OK;
}

static st_err_t bc_crdir(TransStack& ts, SIO *args, string&) { return args[0].si->op_crdir(ts); }

static st_err_t bc_eval(TransStack& ts, SIO*, string& cmd_err) {
	if (ts.stack_get_count() < 1)
		return ST_ERR_TOO_FEW_ARGUMENTS;

	SIO s = ts.transstack_pop();
	if (s.ownership == TSO_OWNED_BY_TS)
		s.ownership = TSO_FROZEN;
	st_err_t c = ts.inner_push_eval(EVAL_HARD, s, true, cmd_err);
	s.cleanup();

	return c;
}

  // Binaries

static st_err_t bc_bin(TransStack&, SIO*, string&) {
	F->set_binary_format(2);
	return ST_ERR_OK;
}

static st_err_t bc_oct(TransStack&, SIO*, string&) {
	F->set_binary_format(8);
	return ST_ERR_OK;
}

static st_err_t bc_dec(TransStack&, SIO*, string&) {
	F->set_binary_format(10);
	return ST_ERR_OK;
}

static st_err_t bc_hex(TransStack&, SIO*, string&) {
	F->set_binary_format(16);
	return ST_ERR_OK;
}

static st_err_t bc_stws(StackItem& op1, StackItem*&, string&) {
	int n;
	st_err_t c = op1.to_integer(n);
	if (c != ST_ERR_OK)
		return c;
	if (n < 1)
		n = 1;
	if (n > g_max_nb_bits)
		n = g_max_nb_bits;
	F->set_bin_size(n);
	return ST_ERR_OK;
}

static st_err_t bc_rcws(StackItem*& si, string&) {
	si = new StackItemReal(Real(F->get_bin_size()));
	return ST_ERR_OK;
}

static st_err_t bc_b_to_r(StackItem& op1, StackItem*& ret, string&) { return op1.op_b_to_r(ret); }
static st_err_t bc_r_to_b(StackItem& op1, StackItem*& ret, string&) { return op1.op_r_to_b(ret); }

static st_err_t lowlevel_bc_rclf(StackItem*& si, const int& nb_bits) {
	bitset<G_HARD_MAX_NB_BITS> bits;

	for (int p = 0; p < nb_bits; p++)
		bits[p] = F->get(p + 1);

	si = new StackItemBinary(bits);
	return ST_ERR_OK;
}

static st_err_t bc_rclf(StackItem*& si, string&) {
	return lowlevel_bc_rclf(si, F->get_bin_size());
}

const string get_rclf_portable_string() {
	StackItem *si;
	st_err_t c = lowlevel_bc_rclf(si, g_max_nb_bits);
	if (c == ST_ERR_OK) {
		ToString tostr(TOSTRING_PORTABLE, 1, 0);
		si->to_string(tostr);
		delete si;

		const vector<string> *p = tostr.get_value();
		return (*p)[0];

		//string r = tostr.get_first_line();
		//return r;
	} else
		throw(CalcFatal(__FILE__, __LINE__, "get_rclf_portable_string(): lowlevel_bc_rclf() didn't return ST_ERR_OK!!!??"));
}

static st_err_t bc_stof(TransStack& ts, SIO *args, string&) { return args[0].si->op_stof(ts); }

  // Programs

static st_err_t bc_wait(StackItem& op1, StackItem*&, string&) {
	int n;
	st_err_t c = op1.to_integer(n);
	if (c != ST_ERR_OK)
		return c;
	if (n < 0)
		n = 0;
	my_sleep_seconds(n);
	return ST_ERR_OK;
}

static st_err_t bc_halt(TransStack& ts, SIO*, string&) { ts.halt(); return ST_ERR_OK; }
static st_err_t bc_sst(TransStack& ts, SIO*, string& cmd_err) { return ts.sst(cmd_err); }
static st_err_t bc_abort(TransStack& ts, SIO*, string&) { ts.abort(); return ST_ERR_OK; }
static st_err_t bc_kill(TransStack& ts, SIO*, string&) { ts.kill(); return ST_ERR_OK; }
static st_err_t bc_cont(TransStack& ts, SIO*, string&) { ts.cont(); return ST_ERR_OK; }

  // Misc

static st_err_t bc_std(TransStack&, SIO*, string&) {
	F->set_realdisp(REALDISP_STD, 0);
	return ST_ERR_OK;
}

static st_err_t set_realdisp_n(StackItem& op1, const realdisp_t& rd) {
	int n;
	st_err_t c = op1.to_integer(n);
	if (c != ST_ERR_OK)
		return c;
	F->set_realdisp(rd, n);
	return ST_ERR_OK;
}

static st_err_t bc_sci(StackItem& op1, StackItem*&, string&) { return set_realdisp_n(op1, REALDISP_SCI); }
static st_err_t bc_fix(StackItem& op1, StackItem*&, string&) { return set_realdisp_n(op1, REALDISP_FIX); }
static st_err_t bc_eng(StackItem& op1, StackItem*&, string&) { return set_realdisp_n(op1, REALDISP_ENG); }

static st_err_t bc_cllcd(TransStack&, SIO*, string&) {
	ui_cllcd();
	return ST_ERR_OK;
}

static st_err_t bc_clmf(TransStack&, SIO*, string&) {
	ui_clear_message_flag();
	return ST_ERR_OK;
}

static st_err_t bc_disp(StackItem& op1, StackItem& op2, StackItem*&, string&) { return op2.op_disp(op1); }

static st_err_t bc_read(TransStack& ts, SIO *args, string&) { return args[0].si->op_read(ts); }

static st_err_t bc_write(StackItem& op1, StackItem& op2, StackItem*&, string&) { return op2.op_write(op1); }

static st_err_t bc_hack_mgmt_std(StackItem& op1, StackItem*&, string&) {
	int n;
	st_err_t c = op1.to_integer(n);
	if (c != ST_ERR_OK)
		return c;
	cfg_realdisp_manage_std = (n != 0);
	return ST_ERR_OK;
}

static st_err_t bc_hack_remove_trailing_dot(StackItem& op1, StackItem*&, string&) {
	int n;
	st_err_t c = op1.to_integer(n);
	if (c != ST_ERR_OK)
		return c;
	cfg_realdisp_remove_trailing_dot = (n != 0);
	return ST_ERR_OK;
}

static st_err_t bc_undo(TransStack& ts, SIO*, string&) {
	if (!ts.get_modified_flag())
		ts.backward_head();
	ts.backward_head();
	return ST_ERR_OK;
}

static st_err_t bc_undo_levels(TransStack& ts, SIO *args, string&) {
	int n;
	st_err_t c = args[0].si->to_integer(n);
	if (c != ST_ERR_OK)
		return c;
	ts.set_undo_levels(n);
	return ST_ERR_OK;
}

static st_err_t bc_undo_levels_get(TransStack& ts, SIO*, string&) {
	ts.transstack_push(new StackItemReal(Real(ts.get_undo_levels())));
	return ST_ERR_OK;
}

  // Must be here for the sake of avoiding declarations of bc_ functions above
#include "Commands.h"

st_err_t StackItemBuiltinCommand::eval(const eval_t&, TransStack& ts, manage_si_t& msi, string& cmd_err) {
	const BuiltinCommandDescriptor& bc = builtinCommands[builtin_id];
	msi = MANAGE_SI_DONOTHING;

	cmd_err = bc.command;

	if (ts.stack_get_count() < bc.nb_args)
		return ST_ERR_TOO_FEW_ARGUMENTS;

	SIO *args;
	if (bc.nb_args >= 1)
		args = new SIO[bc.nb_args];
	else
		args = NULL;

	//debug_write_i("nb_args = %i", bc.nb_args);

	bool m = ts.get_modified_flag();
	for (int i = bc.nb_args - 1; i >= 0; i--)
		args[i] = ts.transstack_pop();

	st_err_t resp = ST_ERR_OK;

	if (bc.type == BC_FUNCTION_WRAPPER || bc.type == BC_COMMAND_WRAPPER) {
		StackItem *ret = 0;
		resp = ST_ERR_INTERNAL;
		switch (bc.nb_args) {
			case 0:
				if (bc.function0 != 0)
					resp = bc.function0(ret, cmd_err);
				break;
			case 1:
				if (bc.function1 != 0)
					resp = bc.function1(*args[0].si, ret, cmd_err);
				break;
			case 2:
				if (bc.function2 != 0)
					resp = bc.function2(*args[0].si, *args[1].si, ret, cmd_err);
				break;
			case 3:
				if (bc.function3 != 0)
					resp = bc.function3(*args[0].si, *args[1].si, *args[2].si, ret, cmd_err);
				break;
			default:
				  // Should never occur. If it does, ST_ERR_INTERNAL return value is fine
				;
		}
		if (resp == ST_ERR_OK && bc.type == BC_FUNCTION_WRAPPER)
			ts.transstack_push(ret);
	} else if (bc.type == BC_RAW)
		resp = bc.functionS(ts, args, cmd_err);

	if (resp != ST_ERR_OK && resp != ST_ERR_EXIT && F->get(FL_LAST)) {
		for (size_t i = 0; i < bc.nb_args; i++)
			ts.transstack_push(args[i].si);
	} else {
		for (int i = bc.nb_args - 1; i >= 0; i--) {
//            debug_write("A0");
			args[i].cleanup();
//            debug_write("A1");
		}
	}

	if (bc.type == BC_FUNCTION_WRAPPER || bc.type == BC_COMMAND_WRAPPER)
		if (resp != ST_ERR_OK && resp != ST_ERR_EXIT && F->get(FL_LAST))
			ts.set_modified_flag(m);
	if (resp == ST_ERR_OK || resp == ST_ERR_EXIT)
		cmd_err = "";

	if (args != NULL)
		delete []args;

	return resp;
}

StackItem *StackItemBuiltinCommand::dup() const {
	return new StackItemBuiltinCommand(builtin_id);
}

void StackItemBuiltinCommand::to_string(ToString& tostr, const bool&) const {
	tostr.add_string(builtinCommands[builtin_id].command);
}

sitype_t StackItemBuiltinCommand::get_type() const { return SITYPE_INTERNAL_BUILTIN_COMMAND; }
bool StackItemBuiltinCommand::same(StackItem* si) const {
	if (si->get_type() == SITYPE_INTERNAL_BUILTIN_COMMAND)
		return (builtin_id == dynamic_cast<StackItemBuiltinCommand*>(si)->builtin_id);
	return false;
}


//
// StackItemBinary
//

st_err_t bin_arith(st_err_t (*f)(const Binary&, const Binary&, Binary&), const Binary& lv, const Binary& rv, StackItem*& ret) {
	Binary res;
	st_err_t c = (*f)(lv, rv, res);

	if (c == ST_ERR_OK)
		ret = new StackItemBinary(res);
	return c;
}

StackItemBinary::StackItemBinary(const Binary& b) : bin(b) { }

StackItemBinary::~StackItemBinary() { }

StackItem *StackItemBinary::dup() const { return new StackItemBinary(bin); }

sitype_t StackItemBinary::get_type() const { return SITYPE_BINARY_INTEGER; }
bool StackItemBinary::same(StackItem* si) const {
	if (si->get_type() == SITYPE_BINARY_INTEGER)
		return bin.cmp(dynamic_cast<StackItemBinary*>(si)->bin) == 0;
	return false;
}

void StackItemBinary::to_string(ToString& tostr, const bool&) const {
	int b = (tostr.get_type() == TOSTRING_PORTABLE ? DEFAULT_PORTABLE_BIN_BASE : F->get_binary_format());
	tostr.add_string("# "  + bin.to_string(b, F->get_bin_size()) + base_int_to_letter(b));
}

st_err_t StackItemBinary::op_b_to_r(StackItem*& ret) {
	ret = new StackItemReal(Real(bin.to_real(F->get_bin_size())));
	return ST_ERR_OK;
}

st_err_t StackItemBinary::op_stof(TransStack&) {
	for (int p = 0; p < F->get_bin_size() && p + 1 <= FL_TAG_IT_END; p++)
		F->set(p + 1, bin.get_bit(p));
	for (int p = F->get_bin_size(); p + 1 <= FL_TAG_IT_END; p++)
		F->set(p + 1, false);
	return ST_ERR_OK;
}

st_err_t StackItemBinary::op_lower(StackItemBinary *arg1, StackItem*& ret) {
	real res = (arg1->bin.cmp(bin) == -1 ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

st_err_t StackItemBinary::op_lower_or_equal(StackItemBinary *arg1, StackItem*& ret) {
	real res = (arg1->bin.cmp(bin) <= 0 ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

st_err_t StackItemBinary::op_greater(StackItemBinary *arg1, StackItem*& ret) {
	real res = (arg1->bin.cmp(bin) == 1 ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

st_err_t StackItemBinary::op_greater_or_equal(StackItemBinary *arg1, StackItem*& ret) {
	real res = (arg1->bin.cmp(bin) >= 0 ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}


//
// StackItemReal and StackItemCplx
//

#define IMPLEMENT_SCALAR_OP(Scalar, SI, op, func) \
st_err_t SI::op(StackItem*& ret) { \
	Scalar res = sc; \
	res.func(); \
	ret = new SI(res); \
	return ST_ERR_OK; \
}
IMPLEMENT_SCALAR_OP(Real, StackItemReal, op_neg, neg)
IMPLEMENT_SCALAR_OP(Cplx, StackItemCplx, op_neg, neg)

#define IMPLEMENT_SCALAR_SQ(Scalar, SI) \
st_err_t SI::op_sq(StackItem*& ret) { \
	Scalar res; \
	st_err_t c = ST_ERR_OK; \
	Scalar##_mul(sc, sc, c, res); \
	if (c == ST_ERR_OK) \
		ret = new SI(Scalar(res)); \
	return c; \
}
IMPLEMENT_SCALAR_SQ(Real, StackItemReal)
IMPLEMENT_SCALAR_SQ(Cplx, StackItemCplx)

#define IMPLEMENT_INV(Scalar, SI, Unit) \
st_err_t SI::op_inv(StackItem*& ret) { \
	Scalar res; \
	st_err_t c = ST_ERR_OK; \
	Scalar##_div(Unit, sc, c, res); \
	if (c == ST_ERR_OK) \
		ret = new SI(res); \
	return c; \
}
IMPLEMENT_INV(Real, StackItemReal, Real(1))
IMPLEMENT_INV(Cplx, StackItemCplx, Cplx(1, 0))

template<class Scalar, class SI> st_err_t si_arith
		(void (*f)(const Scalar&, const Scalar&, st_err_t&, Scalar&), const Scalar& lv, const Scalar& rv, StackItem*& ret) {
	Scalar res;
	st_err_t c = ST_ERR_OK;
	(*f)(lv, rv, c, res);
	if (c == ST_ERR_OK)
		ret = new SI(res);
	return c;
}

template<class Scalar, class SI_Mat> st_err_t si_matrix_md(void (*f)(const Scalar&, const Scalar&, st_err_t&, Scalar&),
		Matrix<Scalar> *lv, const Scalar& rv, StackItem*& ret) {
	Matrix<Scalar> *mres = new Matrix<Scalar>(*lv);
	st_err_t c = mres->md(f, rv);
	if (c == ST_ERR_OK)
		ret = new SI_Mat(mres);
	else
		delete mres;
	return c;
}

Matrix<Cplx> *matrix_real_to_cplx(Matrix<Real> *m) {
	Matrix<Cplx> *mres = new Matrix<Cplx>(m->get_dimension(), m->get_nb_lines(), m->get_nb_columns(), Cplx(0, 0));
	for (int i = 0; i < m->get_nb_lines(); i++)
		for (int j = 0; j < m->get_nb_columns(); j++)
			mres->set_value(i, j, Cplx(m->get_value(i, j)));
	return mres;
}

st_err_t si_matrix_md2(void (*f)(const Cplx&, const Cplx&, st_err_t&, Cplx&), Matrix<Real> *lv, const Cplx& rv, StackItem*& ret) {
	Matrix<Cplx> *mres = matrix_real_to_cplx(lv);
	st_err_t c = mres->md(f, rv);
	if (c == ST_ERR_OK)
		ret = new StackItemMatrixCplx(mres);
	else
		delete mres;
	return c;
}

template<class Scalar, class SI_Mat> st_err_t si_matrix_as(void (*f)(const Scalar&, const Scalar&, st_err_t&, Scalar&), Matrix<Scalar> *lv, Matrix<Scalar> *rv, StackItem*& ret) {
	Matrix<Scalar> *mres = new Matrix<Scalar>(*lv);
	st_err_t c = mres->as(f, *rv);
	if (c == ST_ERR_OK)
		ret = new SI_Mat(mres);
	else
		delete mres;
	return c;
}

st_err_t si_matrix_as2(void (*f)(const Cplx&, const Cplx&, st_err_t&, Cplx&), Matrix<Real> *lv, Matrix<Cplx> *rv, StackItem*& ret) {
	Matrix<Cplx> *mres = matrix_real_to_cplx(lv);
	st_err_t c = mres->as(f, *rv);
	if (c == ST_ERR_OK)
		ret = new StackItemMatrixCplx(mres);
	else
		delete mres;
	return c;
}

st_err_t si_matrix_as3(void (*f)(const Cplx&, const Cplx&, st_err_t&, Cplx&), Matrix<Cplx> *lv, Matrix<Real> *rv, StackItem*& ret) {
	Matrix<Cplx> *new_rv = matrix_real_to_cplx(rv);
	st_err_t c = si_matrix_as<Cplx, StackItemMatrixCplx>(f, lv, new_rv, ret);
	delete new_rv;
	return c;
}

#define IMPLEMENT_SCALAR_OP_CON_LIST(MATSI, SI, SCALAR) \
st_err_t SI::op_con(TransStack& ts, StackItemList* sil, StackItem*& ret) { \
	Coordinates coord; \
	st_err_t c = sil->inner_get_coordinates(ts, coord); \
	if (c != ST_ERR_OK) \
		return c; \
	int i, j; \
	COORD_TO_MATRIX_IJ(coord, i, j); \
	Matrix<SCALAR> *pmat = new Matrix<SCALAR>(coord.d, i + 1, j + 1, sc); \
	ret = new MATSI(pmat); \
	return ST_ERR_OK; \
}
IMPLEMENT_SCALAR_OP_CON_LIST(StackItemMatrixReal, StackItemReal, Real)
IMPLEMENT_SCALAR_OP_CON_LIST(StackItemMatrixCplx, StackItemCplx, Cplx)

#define IMPLEMENT_SCALAR_OP_CON_MAT(MATSI, SI, SCALAR) \
st_err_t SI::op_con(TransStack&, MATSI* sim, StackItem*& ret) { \
	Matrix<SCALAR> *t = sim->get_matrix(); \
	Matrix<SCALAR> *pmat = new Matrix<SCALAR>(t->get_dimension(), t->get_nb_lines(), t->get_nb_columns(), SCALAR(sc)); \
	ret = new MATSI(pmat); \
	return ST_ERR_OK; \
}
IMPLEMENT_SCALAR_OP_CON_MAT(StackItemMatrixReal, StackItemReal, Real)
IMPLEMENT_SCALAR_OP_CON_MAT(StackItemMatrixCplx, StackItemReal, Cplx)
IMPLEMENT_SCALAR_OP_CON_MAT(StackItemMatrixCplx, StackItemCplx, Cplx)

//
// StackItemReal
//

st_err_t StackItemReal::op_add(StackItemCplx *arg1, StackItem*& ret) {
	return si_arith<Cplx, StackItemCplx>(Cplx_add, arg1->get_Cplx(), Cplx(sc), ret);
}
st_err_t StackItemReal::op_sub(StackItemCplx *arg1, StackItem*& ret) {
	return si_arith<Cplx, StackItemCplx>(Cplx_sub, arg1->get_Cplx(), Cplx(sc), ret);
}
st_err_t StackItemReal::op_mul(StackItemCplx *arg1, StackItem*& ret) {
	return si_arith<Cplx, StackItemCplx>(Cplx_mul, arg1->get_Cplx(), Cplx(sc), ret);
}
st_err_t StackItemReal::op_div(StackItemCplx *arg1, StackItem*& ret) {
	return si_arith<Cplx, StackItemCplx>(Cplx_div, arg1->get_Cplx(), Cplx(sc), ret);
}

StackItem *StackItemReal::dup() const { return new StackItemReal(sc); }

sitype_t StackItemReal::get_type() const { return SITYPE_REAL; }

bool StackItemReal::same(StackItem* si) const {
	if (si->get_type() == SITYPE_REAL)
		return sc.cmp(dynamic_cast<StackItemReal*>(si)->sc) == 0;
	return false;
}

Real StackItemReal::get_Real() const { return sc; }

void StackItemReal::set_Real(const Real& r) { sc = r; }

void StackItemReal::to_string(ToString& tostr, const bool&) const { tostr.add_string(sc.to_string(tostr.get_type())); }

st_err_t StackItemReal::to_integer(int& n) const {
	real r = sc.get_value();
	if (r < g_min_int + 1)
		n = g_min_int;
	else if (r > g_max_int - 1)
		n = g_max_int;
	else if (r < 0)
		n = static_cast<int>(real_ip(r - .5));
	else if (r > 0)
		n = static_cast<int>(real_ip(r + .5));
	else
		n = 0;
	return ST_ERR_OK;
}

#define IMPLEMENT_SIMPLE_REAL_FUNCTION(op, function) \
st_err_t StackItemReal::op(StackItem*& ret) { \
	ret = new StackItemReal(Real(function(sc.get_value()))); \
	return ST_ERR_OK; \
}

IMPLEMENT_SIMPLE_REAL_FUNCTION(op_ip, real_ip)
IMPLEMENT_SIMPLE_REAL_FUNCTION(op_floor, real_floor)
IMPLEMENT_SIMPLE_REAL_FUNCTION(op_ceil, real_ceil)
IMPLEMENT_SIMPLE_REAL_FUNCTION(op_abs, real_abs)
IMPLEMENT_SIMPLE_REAL_FUNCTION(op_sign, real_sign)
IMPLEMENT_SIMPLE_REAL_FUNCTION(op_mant, real_mant)
IMPLEMENT_SIMPLE_REAL_FUNCTION(op_xpon, real_xpon)

st_err_t StackItemReal::op_fp(StackItem*& ret) {
	size_t p;
	real r = 0;
	string s = sc.to_string(TOSTRING_PORTABLE);
	debug_write("pre s=");
	debug_write(s.c_str());
	if ((p = s.find_first_of('E')) != string::npos) {
		if (real_abs(sc.get_value()) < 1)
			r = sc.get_value();
	} else if ((p = s.find_first_of(PORTABLE_DECIMAL_SEPARATOR)) != string::npos) {
		s.erase(0, p);
		istringstream iss(s);
		iss >> r;
		if (sc.get_value() < 0)
			r = -r;
	}
	ret = new StackItemReal(Real(r));
	return ST_ERR_OK;
}

st_err_t StackItemReal::op_percent(StackItemReal* arg1, StackItem*& ret) {
	Real r, r2;
	st_err_t c = ST_ERR_OK;
	arg1->get_Real().mul(sc, c, r);
	r.div(Real(100.0), c, r2);
	if (c == ST_ERR_OK)
		ret = new StackItemReal(r2);
	return c;
}

st_err_t StackItemReal::op_percent_ch(StackItemReal* arg1, StackItem*& ret) {
	Real r, r2, r3;
	st_err_t c = ST_ERR_OK;
	sc.sub(arg1->get_Real(), c, r);
	r.mul(Real(100.0), c, r2);
	r2.div(arg1->get_Real(), c, r3);
	if (c == ST_ERR_OK)
		ret = new StackItemReal(r3);
	return c;
}

st_err_t StackItemReal::op_percent_t(StackItemReal* arg1, StackItem*& ret) {
	Real r, r2;
	st_err_t c = ST_ERR_OK;
	sc.mul(Real(100.0), c, r);
	r.div(arg1->get_Real(), c, r2);
	if (c == ST_ERR_OK)
		ret = new StackItemReal(r2);
	return c;
}

#define IMPLEMENT_TWO_ARGUMENTS_REAL_FUNCTION(OP) \
st_err_t StackItemReal::op_##OP(StackItemReal* arg1, StackItem*& ret) { \
	Real r; \
	st_err_t c = ST_ERR_OK; \
	arg1->get_Real().OP(sc, c, r); \
	if (c == ST_ERR_OK) \
		ret = new StackItemReal(r); \
	return c; \
}
IMPLEMENT_TWO_ARGUMENTS_REAL_FUNCTION(mod)
IMPLEMENT_TWO_ARGUMENTS_REAL_FUNCTION(hms_add)
IMPLEMENT_TWO_ARGUMENTS_REAL_FUNCTION(hms_sub)

st_err_t StackItemReal::op_min(StackItemReal* arg1, StackItem*& ret) {
	ret = new StackItemReal(arg1->get_Real().get_value() < sc.get_value() ? arg1->get_Real() : sc);
	return ST_ERR_OK;
}

st_err_t StackItemReal::op_max(StackItemReal* arg1, StackItem*& ret) {
	ret = new StackItemReal(arg1->get_Real().get_value() > sc.get_value() ? arg1->get_Real() : sc);
	return ST_ERR_OK;
}

void create_Real_or_Cplx_StackItem(const st_err_t& c, const Cplx& cplx, StackItem*& ret) {
	if (c == ST_ERR_OK) {
		if (cplx.get_im() != 0)
			ret = new StackItemCplx(cplx);
		else
			ret = new StackItemReal(Real(cplx.get_re()));
	}
}

#define IMPLEMENT_REAL_TO_REAL_OR_CPLX_FUNCTION(OP) \
st_err_t StackItemReal::op_##OP(StackItem*& ret) { \
	Cplx cplx; \
	st_err_t c = ST_ERR_OK; \
	sc.OP(c, cplx); \
	create_Real_or_Cplx_StackItem(c, cplx, ret); \
	return c; \
}

#define IMPLEMENT_REAL_TO_REAL_OR_CPLX_FUNCTION_ANGLE_OUTPUT(OP) \
st_err_t StackItemReal::op_##OP(StackItem*& ret) { \
	Cplx cplx; \
	st_err_t c = ST_ERR_OK; \
	sc.OP(c, cplx); \
	convert_to_degrees_if_needed(c, cplx); \
	create_Real_or_Cplx_StackItem(c, cplx, ret); \
	return c; \
}
IMPLEMENT_REAL_TO_REAL_OR_CPLX_FUNCTION(sqr)
IMPLEMENT_REAL_TO_REAL_OR_CPLX_FUNCTION(ln)
IMPLEMENT_REAL_TO_REAL_OR_CPLX_FUNCTION(log)
IMPLEMENT_REAL_TO_REAL_OR_CPLX_FUNCTION_ANGLE_OUTPUT(acos)
IMPLEMENT_REAL_TO_REAL_OR_CPLX_FUNCTION_ANGLE_OUTPUT(asin)
IMPLEMENT_REAL_TO_REAL_OR_CPLX_FUNCTION(acosh)
IMPLEMENT_REAL_TO_REAL_OR_CPLX_FUNCTION(atanh)

#define IMPLEMENT_REAL_TO_REAL_FUNCTION(OP) \
st_err_t StackItemReal::op_##OP(StackItem*& ret) { \
	st_err_t c = ST_ERR_OK; \
	Real real; \
	sc.OP(c, real); \
	if (c == ST_ERR_OK) \
		ret = new StackItemReal(real); \
	return c; \
}
#define IMPLEMENT_REAL_TO_REAL_FUNCTION_ANGLE_INPUT(OP) \
st_err_t StackItemReal::op_##OP(StackItem*& ret) { \
	st_err_t c = ST_ERR_OK; \
	Real real; \
	Real local_copy = sc; \
	convert_to_radians_if_needed(c, local_copy); \
	local_copy.OP(c, real); \
	if (c == ST_ERR_OK) \
		ret = new StackItemReal(real); \
	return c; \
}
IMPLEMENT_REAL_TO_REAL_FUNCTION_ANGLE_INPUT(cos)
IMPLEMENT_REAL_TO_REAL_FUNCTION_ANGLE_INPUT(sin)
IMPLEMENT_REAL_TO_REAL_FUNCTION_ANGLE_INPUT(tan)
IMPLEMENT_REAL_TO_REAL_FUNCTION(exp)
IMPLEMENT_REAL_TO_REAL_FUNCTION(alog)
IMPLEMENT_REAL_TO_REAL_FUNCTION(lnp1)
IMPLEMENT_REAL_TO_REAL_FUNCTION(expm)
IMPLEMENT_REAL_TO_REAL_FUNCTION(cosh)
IMPLEMENT_REAL_TO_REAL_FUNCTION(sinh)
IMPLEMENT_REAL_TO_REAL_FUNCTION(tanh)
IMPLEMENT_REAL_TO_REAL_FUNCTION(asinh)
IMPLEMENT_REAL_TO_REAL_FUNCTION(to_hms)
IMPLEMENT_REAL_TO_REAL_FUNCTION(hms_to)
IMPLEMENT_REAL_TO_REAL_FUNCTION(d_to_r)
IMPLEMENT_REAL_TO_REAL_FUNCTION(r_to_d)

st_err_t StackItemReal::op_atan(StackItem*& ret) {
	st_err_t c = ST_ERR_OK;
	Real res;
	sc.atan(c, res);
	convert_to_degrees_if_needed(c, res);
	if (c == ST_ERR_OK)
		ret = new StackItemReal(res);
	return c;
}

st_err_t StackItemReal::op_pow(StackItemReal *arg1, StackItem*& ret) {
	Cplx cplx;
	st_err_t c = ST_ERR_OK;
	arg1->get_Real().pow(sc, c, cplx);
	int n;
	create_Real_or_Cplx_StackItem(c, cplx, ret);
	return c;
}

st_err_t StackItemReal::op_r_to_c(StackItemReal *arg1, StackItem*& ret) {
	ret = new StackItemCplx(Cplx(arg1->sc.get_value(), sc.get_value()));
	return ST_ERR_OK;
}

st_err_t StackItemReal::op_mul(StackItemMatrixReal *arg1, StackItem*& ret) {
	return si_matrix_md<Real, StackItemMatrixReal>(Real_mul, arg1->get_matrix(), sc, ret);
}

st_err_t StackItemReal::op_div(StackItemMatrixReal *arg1, StackItem*& ret) {
	return si_matrix_md<Real, StackItemMatrixReal>(Real_div, arg1->get_matrix(), sc, ret);
}

st_err_t StackItemReal::op_mul(StackItemMatrixCplx *arg1, StackItem*& ret) {
	return si_matrix_md<Cplx, StackItemMatrixCplx>(Cplx_mul, arg1->get_matrix(), Cplx(sc), ret);
}

st_err_t StackItemReal::op_div(StackItemMatrixCplx *arg1, StackItem*& ret) {
	return si_matrix_md<Cplx, StackItemMatrixCplx>(Cplx_div, arg1->get_matrix(), Cplx(sc), ret);
}

st_err_t StackItemReal::op_lower(StackItemReal *arg1, StackItem*& ret) {
	real res = (arg1->sc.cmp(sc) == -1 ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

st_err_t StackItemReal::op_lower_or_equal(StackItemReal *arg1, StackItem*& ret) {
	real res = (arg1->sc.cmp(sc) <= 0 ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

st_err_t StackItemReal::op_greater(StackItemReal *arg1, StackItem*& ret) {
	real res = (arg1->sc.cmp(sc) == 1 ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

st_err_t StackItemReal::op_greater_or_equal(StackItemReal *arg1, StackItem*& ret) {
	real res = (arg1->sc.cmp(sc) >= 0 ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

#define REAL_LOGIC_PREFIX \
	int n1, n2; \
	st_err_t c = arg1->to_integer(n1); \
	if (c != ST_ERR_OK) \
		return c; \
	c = to_integer(n2); \
	if (c != ST_ERR_OK) \
		return c;

st_err_t StackItemReal::op_and(StackItemReal *arg1, StackItem*& ret) {
	REAL_LOGIC_PREFIX
	ret = new StackItemReal(Real(n1 != 0 && n2 != 0));
	return ST_ERR_OK;
}

st_err_t StackItemReal::op_or(StackItemReal *arg1, StackItem*& ret) {
	REAL_LOGIC_PREFIX
	ret = new StackItemReal(Real(n1 != 0 || n2 != 0));
	return ST_ERR_OK;
}

st_err_t StackItemReal::op_xor(StackItemReal *arg1, StackItem*& ret) {
	REAL_LOGIC_PREFIX
	ret = new StackItemReal(Real((n1 == 0 && n2 != 0) || n1 != 0 && n2 == 0));
	return ST_ERR_OK;
}

st_err_t StackItemReal::op_not(StackItem*& ret) {
	int n;
	st_err_t c = to_integer(n);
	if (c != ST_ERR_OK)
		return c;





	n = rand();
	debug_write_i("Rand = %i", n);
	ret = new StackItemReal(Real(RAND_MAX));





//    ret = new StackItemReal(Real(!n));
	return ST_ERR_OK;
}

st_err_t StackItemReal::op_r_to_b(StackItem*& ret) {
	real r = real_ip(sc.get_value() + .5);
	bitset<G_HARD_MAX_NB_BITS> bits;
	int max = F->get_bin_size();

	real real_max = get_max_real_from_bin_size(max);

	if (r > 0) {
		if (r >= real_max) {
			for (int p = 0; p < max; p++)
				bits[p] = true;
		} else {
			int p = 0;
			while (r > 0) {
				if (r >= (real)g_max_int)
					bits[p++] = false;
				else
					bits[p++] = ((int)r % 2 != 0);
				if (p >= max)
					break;
				r = real_ip(r / 2);
			}
		}
	}
	ret = new StackItemBinary(bits);
	return ST_ERR_OK;
}

#define IMPLEMENT_OP_PUT_MATRIX(MATSI, SI, Scalar) \
st_err_t SI::op_put_matrix(TransStack& ts, StackItem* si, MATSI* sim) { \
	Coordinates coord; \
	st_err_t c = si->get_coordinates(ts, sim, coord); \
	if (c != ST_ERR_OK) \
		return c; \
	sim->set_value(coord, Scalar(sc)); \
	return ST_ERR_OK; \
}
IMPLEMENT_OP_PUT_MATRIX(StackItemMatrixReal, StackItemReal, Real)
IMPLEMENT_OP_PUT_MATRIX(StackItemMatrixCplx, StackItemReal, Cplx)
IMPLEMENT_OP_PUT_MATRIX(StackItemMatrixCplx, StackItemCplx, Cplx)

st_err_t StackItemReal::op_idn(StackItem*& ret) {
	int n;
	st_err_t c = to_integer(n);
	if (c != ST_ERR_OK)
		return c;
	if (n < 1)
		return ST_ERR_BAD_ARGUMENT_VALUE;
	Matrix<Real> *pmat = new Matrix<Real>(DIM_MATRIX, n, n, Real(0));
	for (int i = 0; i < n; i++)
		pmat->set_value(i, i, Real(1));
	ret = new StackItemMatrixReal(pmat);
	return ST_ERR_OK;
}

st_err_t StackItemReal::op_disp(StackItem& arg1) {
	int n;
	st_err_t c = to_integer(n);
	if (c != ST_ERR_OK)
		return c;
	string s = simple_string(&arg1);
	if (s.length() >= 2 && s.at(0) == '"' && s.at(s.length() - 1) == '"')
		s = s.substr(1, s.length() - 2);
	ui_disp(n, s);
	return ST_ERR_OK;
}

st_err_t StackItemReal::increment_coord(TransStack&, const Coordinates& bounds) {
	int n;
	if (bounds.d == DIM_VECTOR)
		n = bounds.i;
	else
		n = bounds.i * bounds.j;
	int v;
	st_err_t c = to_integer(v);
	if (c != ST_ERR_OK)
		throw(CalcFatal(__FILE__, __LINE__, "StackItemReal::real_increment_coord(): to_integer() returned an error!!!?"));
	v++;
	if (v > n)
		v = 1;
	set_Real(Real(v));
	return ST_ERR_OK;
}

st_err_t StackItemReal::get_coordinates(TransStack&, StackItem* si, Coordinates& coord) {
	Coordinates bounds;
	coord.d = DIM_ANY;
	st_err_t c;
	if ((c = to_integer(coord.i)) != ST_ERR_OK)
		throw(CalcFatal(__FILE__, __LINE__, "StackItemReal::get_coordinates(): to_integer() returned an error!!!?"));
	if ((c = si->get_bounds(bounds)) != ST_ERR_OK)
		return c;
	if ((c = check_coordinates(bounds, coord)) != ST_ERR_OK)
		return c;
	return ST_ERR_OK;
}

st_err_t StackItem::op_put(TransStack& ts, StackItemList* sil, SIO& s) {
	Coordinates coord;
	st_err_t c = get_coordinates(ts, sil, coord);
	if (c != ST_ERR_OK)
		return c;
	sil->replace_nth_item(coord.i, s);
	return ST_ERR_OK;
}


//
// StackItemCplx
//

st_err_t si_cplx_arith(st_err_t (*f)(const Cplx&, const Cplx&, Cplx&), const Cplx& lv, const Cplx& rv, StackItem*& ret) {
	Cplx res;
	st_err_t c = (*f)(lv, rv, res);
	if (c == ST_ERR_OK)
		ret = new StackItemCplx(res);
	return c;
}

st_err_t StackItemCplx::op_sign(StackItem*& ret) {
	Cplx cplx = sc;
	st_err_t c = ST_ERR_OK;
	Cplx cplx2;
	cplx.sign(c, cplx2);
	if (c == ST_ERR_OK)
		ret = new StackItemCplx(cplx2);
	return c;
}

#define IMPLEMENT_CPLX_OPREAL(OP) \
st_err_t StackItemCplx::op_##OP(StackItem*& ret) { \
	Real r; \
	st_err_t c = ST_ERR_OK; \
	sc.OP(c, r); \
	if (c == ST_ERR_OK) \
		ret = new StackItemReal(r); \
	return c; \
}
IMPLEMENT_CPLX_OPREAL(abs)
IMPLEMENT_CPLX_OPREAL(arg)

#define IMPLEMENT_CPLX_OPCPLX(OP) \
st_err_t StackItemCplx::op_##OP(StackItem*& ret) { \
	Cplx cplx; \
	st_err_t c = ST_ERR_OK; \
	sc.OP(c, cplx); \
	if (c == ST_ERR_OK) \
		ret = new StackItemCplx(cplx); \
	return c; \
}
IMPLEMENT_CPLX_OPCPLX(ln)
IMPLEMENT_CPLX_OPCPLX(log)
IMPLEMENT_CPLX_OPCPLX(exp)
IMPLEMENT_CPLX_OPCPLX(alog)
IMPLEMENT_CPLX_OPCPLX(sqr)
IMPLEMENT_CPLX_OPCPLX(acos)
IMPLEMENT_CPLX_OPCPLX(asin)
IMPLEMENT_CPLX_OPCPLX(atan)
IMPLEMENT_CPLX_OPCPLX(cos)
IMPLEMENT_CPLX_OPCPLX(sin)
IMPLEMENT_CPLX_OPCPLX(tan)
IMPLEMENT_CPLX_OPCPLX(cosh)
IMPLEMENT_CPLX_OPCPLX(sinh)
IMPLEMENT_CPLX_OPCPLX(tanh)
IMPLEMENT_CPLX_OPCPLX(acosh)
IMPLEMENT_CPLX_OPCPLX(asinh)
IMPLEMENT_CPLX_OPCPLX(atanh)

st_err_t StackItemCplx::op_p_to_r(StackItem*& ret) {
	Cplx cplx;
	st_err_t c = ST_ERR_OK;
	Cplx local_copy = sc;
	Real im = Real(local_copy.get_im());
	convert_to_radians_if_needed(c, im);
	local_copy = Cplx(local_copy.get_re(), im.get_value());
	local_copy.p_to_r(c, cplx);
	if (c == ST_ERR_OK)
		ret = new StackItemCplx(cplx);
	return c;
}

st_err_t StackItemCplx::op_r_to_p(StackItem*& ret) {
	Cplx cplx;
	st_err_t c = ST_ERR_OK;
	sc.r_to_p(c, cplx);
	if (c == ST_ERR_OK) {
		Cplx cplx_copy = cplx;
		Real im = Real(cplx_copy.get_im());
		convert_to_degrees_if_needed(c, im);
		if (c == ST_ERR_OK)
			ret = new StackItemCplx(Cplx(cplx_copy.get_re(), im.get_value()));
	}
	return c;
}

st_err_t StackItemCplx::op_conj(StackItem*& ret) {
	Cplx cplx = sc;
	cplx.conj();
	ret = new StackItemCplx(cplx);
	return ST_ERR_OK;
}

st_err_t StackItemCplx::op_mul(StackItemMatrixCplx *arg1, StackItem*& ret) {
	return si_matrix_md<Cplx, StackItemMatrixCplx>(Cplx_mul, arg1->get_matrix(), sc, ret);
}

st_err_t StackItemCplx::op_div(StackItemMatrixCplx *arg1, StackItem*& ret) {
	return si_matrix_md<Cplx, StackItemMatrixCplx>(Cplx_div, arg1->get_matrix(), sc, ret);
}

st_err_t StackItemCplx::op_mul(StackItemMatrixReal *arg1, StackItem*& ret) {
	return si_matrix_md2(Cplx_mul, arg1->get_matrix(), sc, ret);
}

st_err_t StackItemCplx::op_div(StackItemMatrixReal *arg1, StackItem*& ret) {
	return si_matrix_md2(Cplx_div, arg1->get_matrix(), sc, ret);
}

st_err_t StackItemCplx::op_pow(StackItemCplx* arg1, StackItem*& ret) {
	Cplx cplx;
	st_err_t c = ST_ERR_OK;
	arg1->sc.pow(sc, c, cplx);
	if (c == ST_ERR_OK)
		ret = new StackItemCplx(cplx);
	return c;
}

st_err_t StackItemCplx::op_c_to_r(TransStack& ts) {
	ts.transstack_push(new StackItemReal(Real(sc.get_re())));
	ts.transstack_push(new StackItemReal(Real(sc.get_im())));
	return ST_ERR_OK;
}

st_err_t StackItemCplx::op_re(StackItem*& ret) {
	ret = new StackItemReal(Real(sc.get_re()));
	return ST_ERR_OK;
}

st_err_t StackItemCplx::op_im(StackItem*& ret) {
	ret = new StackItemReal(Real(sc.get_im()));
	return ST_ERR_OK;
}

StackItem *StackItemCplx::dup() const { return new StackItemCplx(sc); }

sitype_t StackItemCplx::get_type() const { return SITYPE_COMPLEX; }

bool StackItemCplx::same(StackItem* si) const {
	if (si->get_type() == SITYPE_COMPLEX)
		return sc.cmp(dynamic_cast<StackItemCplx*>(si)->sc) == 0;
	return false;
}

void StackItemCplx::to_string(ToString& tostr, const bool&) const { tostr.add_string(sc.to_string(tostr.get_type())); }


//
// StackItemMatrix*
//

template<class Scalar> void matrix_to_string(Matrix<Scalar> *pmat, ToString& tostr) {
	string s;
	if (pmat->get_dimension() == DIM_MATRIX)
		s = "[";
	for (int i = 0; i < pmat->get_nb_lines(); i++) {
		if (i == 0)
			s.append("[ ");
		else
			s.append(" [ ");
		for (int j = 0; j < pmat->get_nb_columns(); j++) {
			s.append(pmat->get_value(i, j).to_string(tostr.get_type()));
			if (j != pmat->get_nb_columns() - 1)
				s.append(" ");
		}
		s.append(" ]");
		if (i < pmat->get_nb_lines() - 1) {
			tostr.add_string(s, i >= 1);
			s = "";
		}
	}
	if (pmat->get_dimension() == DIM_MATRIX)
		s.append("]");
	tostr.add_string(s, pmat->get_nb_lines() >= 2);
}

#define IMPLEMENT_MAT_SIZE(MATSI) \
st_err_t MATSI::op_size(StackItem*& ret) { \
	Coordinates coord; \
	st_err_t c = get_bounds(coord); \
	if (c != ST_ERR_OK) \
		throw(CalcFatal(__FILE__, __LINE__, "MATSI::op_size(): get_bounds() returned an error!!?")); \
	ret = forge_list_size(coord); \
	return ST_ERR_OK; \
}
IMPLEMENT_MAT_SIZE(StackItemMatrixReal)
IMPLEMENT_MAT_SIZE(StackItemMatrixCplx)

#define IMPLEMENT_MAT_ARRY_TO(MATSI, SI) \
st_err_t MATSI::op_arry_to(TransStack& ts) { \
	for (int i = 0; i < pmat->get_nb_lines(); i++) \
		for (int j = 0; j < pmat->get_nb_columns(); j++) \
			ts.transstack_push(new SI(pmat->get_value(i, j))); \
	StackItem *si; \
	if (op_size(si) != ST_ERR_OK) \
		throw(CalcFatal(__FILE__, __LINE__, "StackItemMatrix*::op_arry_to(): what's going on!!!??")); \
	ts.transstack_push(si); \
	return ST_ERR_OK; \
}
IMPLEMENT_MAT_ARRY_TO(StackItemMatrixReal, StackItemReal)
IMPLEMENT_MAT_ARRY_TO(StackItemMatrixCplx, StackItemCplx)

#define IMPLEMENT_MAT_OP_TRN(MATSI, SCALAR) \
st_err_t MATSI::op_trn(StackItem*& ret) { \
	Matrix<SCALAR> *ptrn; \
	st_err_t c = pmat->create_transpose(ptrn); \
	if (c == ST_ERR_OK) \
		ret = new MATSI(ptrn); \
	return c; \
}
IMPLEMENT_MAT_OP_TRN(StackItemMatrixReal, Real)
IMPLEMENT_MAT_OP_TRN(StackItemMatrixCplx, Cplx)

#define IMPLEMENT_MAT_RDM(MATSI) \
void MATSI::rdm(const dim_t& new_dimension, const int& new_nb_lines, const int& new_nb_columns) { \
	pmat->redim(new_dimension, new_nb_lines, new_nb_columns); \
}
IMPLEMENT_MAT_RDM(StackItemMatrixReal)
IMPLEMENT_MAT_RDM(StackItemMatrixCplx)

#define IMPLEMENT_MAT_OP_WITH_MAT(MATSI, SCALAR, OP) \
st_err_t MATSI::op_##OP(MATSI *arg1, StackItem*& ret) { \
	Matrix<SCALAR> *mres; \
	st_err_t c = arg1->pmat->create_##OP(pmat, mres); \
	if (c == ST_ERR_OK) \
		ret = new MATSI(mres); \
	return c; \
}
IMPLEMENT_MAT_OP_WITH_MAT(StackItemMatrixReal, Real, mul)
IMPLEMENT_MAT_OP_WITH_MAT(StackItemMatrixReal, Real, div)
IMPLEMENT_MAT_OP_WITH_MAT(StackItemMatrixCplx, Cplx, mul)
IMPLEMENT_MAT_OP_WITH_MAT(StackItemMatrixCplx, Cplx, div)
IMPLEMENT_MAT_OP_WITH_MAT(StackItemMatrixReal, Real, cross)
IMPLEMENT_MAT_OP_WITH_MAT(StackItemMatrixCplx, Cplx, cross)

#define IMPLEMENT_MAT_OP_CPLX_OP_REAL(OP) \
st_err_t StackItemMatrixReal::op_##OP(StackItemMatrixCplx *arg1, StackItem*& ret) { \
	Matrix<Cplx> *converted_to_matrix_cplx = matrix_real_to_cplx(pmat); \
	Matrix<Cplx> *mres; \
	st_err_t c = arg1->get_matrix()->create_##OP(converted_to_matrix_cplx, mres); \
	if (c == ST_ERR_OK) \
		ret = new StackItemMatrixCplx(mres); \
	delete converted_to_matrix_cplx; \
	return c; \
}
IMPLEMENT_MAT_OP_CPLX_OP_REAL(mul)
IMPLEMENT_MAT_OP_CPLX_OP_REAL(div)
IMPLEMENT_MAT_OP_CPLX_OP_REAL(cross)

#define IMPLEMENT_MAT_OP_REAL_OP_CPLX(OP) \
st_err_t StackItemMatrixCplx::op_##OP(StackItemMatrixReal *arg1, StackItem*& ret) { \
	Matrix<Cplx> *converted_to_matrix_cplx = matrix_real_to_cplx(arg1->get_matrix()); \
	Matrix<Cplx> *mres; \
	st_err_t c = converted_to_matrix_cplx->create_##OP(pmat, mres); \
	if (c == ST_ERR_OK) \
		ret = new StackItemMatrixCplx(mres); \
	delete converted_to_matrix_cplx; \
	return c; \
}
IMPLEMENT_MAT_OP_REAL_OP_CPLX(mul)
IMPLEMENT_MAT_OP_REAL_OP_CPLX(div)
IMPLEMENT_MAT_OP_REAL_OP_CPLX(cross)

#define IMPLEMENT_MAT_OP_NEG(SC) \
st_err_t StackItemMatrix##SC::op_neg(StackItem*& ret) { \
	Matrix<SC> *mres; \
	st_err_t c = pmat->create_neg(mres); \
	if (c == ST_ERR_OK) \
		ret = new StackItemMatrix##SC(mres); \
	return c; \
}
IMPLEMENT_MAT_OP_NEG(Real)
IMPLEMENT_MAT_OP_NEG(Cplx)

#define IMPLEMENT_MAT_DOT(MATSI, SCALAR, SI) \
st_err_t MATSI::op_dot(MATSI *arg1, StackItem*& ret) { \
	SCALAR s; \
	st_err_t c = arg1->pmat->dot(pmat, s); \
	if (c == ST_ERR_OK) \
		ret = new SI(s); \
	return c; \
}
IMPLEMENT_MAT_DOT(StackItemMatrixReal, Real, StackItemReal)
IMPLEMENT_MAT_DOT(StackItemMatrixCplx, Cplx, StackItemCplx)

st_err_t StackItemMatrixReal::op_dot(StackItemMatrixCplx *arg1, StackItem*& ret) {
	Matrix<Cplx> *converted_to_matrix_cplx = matrix_real_to_cplx(pmat);
	Cplx s;
	st_err_t c = arg1->get_matrix()->dot(converted_to_matrix_cplx, s);
	if (c == ST_ERR_OK)
		ret = new StackItemCplx(s);
	delete converted_to_matrix_cplx;
	return c;
}

st_err_t StackItemMatrixCplx::op_dot(StackItemMatrixReal *arg1, StackItem*& ret) {
	Matrix<Cplx> *converted_to_matrix_cplx = matrix_real_to_cplx(arg1->get_matrix());
	Cplx s;
	st_err_t c = converted_to_matrix_cplx->dot(pmat, s);
	if (c == ST_ERR_OK)
		ret = new StackItemCplx(s);
	delete converted_to_matrix_cplx;
	return c;
}

#define IMPLEMENT_MAT_ABS(MATSI) \
st_err_t MATSI::op_abs(StackItem*& ret) { \
	Real x; \
	st_err_t c = pmat->abs(x); \
	if (c == ST_ERR_OK) \
		ret = new StackItemReal(x); \
	return c; \
}
IMPLEMENT_MAT_ABS(StackItemMatrixReal)
IMPLEMENT_MAT_ABS(StackItemMatrixCplx)


//
// StackItemMatrixReal
//

StackItemMatrixReal::StackItemMatrixReal(const mat_read_t *pm, const dim_t& d, const int& l, const int& c)
	: pmat(new Matrix<Real>(pm, d, l, c)) {
	pmat->trim();
}

StackItemMatrixReal::StackItemMatrixReal(const StackItemMatrixReal& simr)
	: StackItem(), pmat(new Matrix<Real>(*simr.pmat)) {
	pmat->trim();
}

StackItemMatrixReal::StackItemMatrixReal(Matrix<Real> *p) : pmat(p) {
	pmat->trim();
}

StackItem *StackItemMatrixReal::dup() const { return new StackItemMatrixReal(*this); }
sitype_t StackItemMatrixReal::get_type() const { return SITYPE_MATRIX_REAL; }
bool StackItemMatrixReal::same(StackItem* si) const {
	if (si->get_type() == SITYPE_MATRIX_REAL)
		return pmat->cmp(*dynamic_cast<StackItemMatrixReal*>(si)->pmat) == 0;
	return false;
}
Matrix<Real> *StackItemMatrixReal::get_matrix() const { return pmat; }
void StackItemMatrixReal::to_string(ToString& tostr, const bool&) const { matrix_to_string(pmat, tostr); }
st_err_t StackItemMatrixReal::get_bounds(Coordinates& coord) const { return pmat->get_bounds(coord); }

st_err_t StackItemMatrixReal::op_mul(StackItemReal *arg1, StackItem*& ret) {
	return si_matrix_md<Real, StackItemMatrixReal>(Real_mul, pmat, arg1->get_Real(), ret);
}
st_err_t StackItemMatrixReal::op_mul(StackItemCplx *arg1, StackItem*& ret) {
	return si_matrix_md2(Cplx_mul, pmat, arg1->get_Cplx(), ret);
}
st_err_t StackItemMatrixReal::op_add(StackItemMatrixReal *arg1, StackItem*& ret) {
	return si_matrix_as<Real, StackItemMatrixReal>(Real_add, arg1->pmat, pmat, ret);
}
st_err_t StackItemMatrixReal::op_sub(StackItemMatrixReal *arg1, StackItem*& ret) {
	return si_matrix_as<Real, StackItemMatrixReal>(Real_sub, arg1->pmat, pmat, ret);
}
st_err_t StackItemMatrixReal::op_add(StackItemMatrixCplx *arg1, StackItem*& ret) {
	return si_matrix_as3(Cplx_add, arg1->get_matrix(), pmat, ret);
}
st_err_t StackItemMatrixReal::op_sub(StackItemMatrixCplx *arg1, StackItem*& ret) {
	return si_matrix_as3(Cplx_sub, arg1->get_matrix(), pmat, ret);
}

st_err_t StackItemMatrixReal::op_r_to_c(StackItemMatrixReal *arg1, StackItem*& ret) {
	Matrix<Cplx> *mres;
	st_err_t c = mat_r_to_c(arg1->pmat, pmat, mres);
	if (c == ST_ERR_OK)
		ret = new StackItemMatrixCplx(mres);
	return c;
}


//
// StackItemMatrixCplx
//

StackItemMatrixCplx::StackItemMatrixCplx(const mat_read_t *pm, const dim_t& d, const int& l, const int& c)
	: pmat(new Matrix<Cplx>(pm, d, l, c)) {
	pmat->trim();
}

StackItemMatrixCplx::StackItemMatrixCplx(const StackItemMatrixCplx& simr)
	: StackItem(), pmat(new Matrix<Cplx>(*simr.pmat)) {
	pmat->trim();
}

StackItemMatrixCplx::StackItemMatrixCplx(Matrix<Cplx> *p) : pmat(p) {
	pmat->trim();
}

StackItem *StackItemMatrixCplx::dup() const { return new StackItemMatrixCplx(*this); }
sitype_t StackItemMatrixCplx::get_type() const { return SITYPE_MATRIX_COMPLEX; }
bool StackItemMatrixCplx::same(StackItem* si) const {
	if (si->get_type() == SITYPE_MATRIX_COMPLEX)
		return pmat->cmp(*dynamic_cast<StackItemMatrixCplx*>(si)->pmat) == 0;
	return false;
}
Matrix<Cplx> *StackItemMatrixCplx::get_matrix() const { return pmat; }
void StackItemMatrixCplx::to_string(ToString& tostr, const bool&) const { matrix_to_string(pmat, tostr); }
st_err_t StackItemMatrixCplx::get_bounds(Coordinates& coord) const { return pmat->get_bounds(coord); }

st_err_t StackItemMatrixCplx::op_mul(StackItemCplx *arg1, StackItem*& ret) {
	return si_matrix_md<Cplx, StackItemMatrixCplx>(Cplx_mul, pmat, arg1->get_Cplx(), ret);
}
st_err_t StackItemMatrixCplx::op_mul(StackItemReal *arg1, StackItem*& ret) {
	return si_matrix_md<Cplx, StackItemMatrixCplx>(Cplx_mul, pmat, Cplx(arg1->get_Real()), ret);
}
st_err_t StackItemMatrixCplx::op_add(StackItemMatrixCplx *arg1, StackItem*& ret) {
	return si_matrix_as<Cplx, StackItemMatrixCplx>(Cplx_add, arg1->pmat, pmat, ret);
}
st_err_t StackItemMatrixCplx::op_sub(StackItemMatrixCplx *arg1, StackItem*& ret) {
	return si_matrix_as<Cplx, StackItemMatrixCplx>(Cplx_sub, arg1->pmat, pmat, ret);
}
st_err_t StackItemMatrixCplx::op_add(StackItemMatrixReal *arg1, StackItem*& ret) {
	return si_matrix_as2(Cplx_add, arg1->get_matrix(), pmat, ret);
}
st_err_t StackItemMatrixCplx::op_sub(StackItemMatrixReal *arg1, StackItem*& ret) {
	return si_matrix_as2(Cplx_sub, arg1->get_matrix(), pmat, ret);
}

st_err_t StackItemMatrixCplx::op_c_to_r(TransStack& ts) {
	Matrix<Real> *m_re;
	Matrix<Real> *m_im;
	mat_c_to_r(pmat, m_re, m_im);
	ts.transstack_push(new StackItemMatrixReal(m_re));
	ts.transstack_push(new StackItemMatrixReal(m_im));
	return ST_ERR_OK;
}

#define IMPLEMENT_MAT_CPLX_RE_OR_IM(OP) \
st_err_t StackItemMatrixCplx::op_##OP(StackItem*& ret) { \
	Matrix<Real> *m; \
	mat_c_to_##OP(pmat, m); \
	ret = new StackItemMatrixReal(m); \
	return ST_ERR_OK; \
}
IMPLEMENT_MAT_CPLX_RE_OR_IM(re)
IMPLEMENT_MAT_CPLX_RE_OR_IM(im)

st_err_t StackItemMatrixCplx::op_conj(StackItem*& ret) {
	Matrix<Cplx> *m_cplx;
	mat_c_conj(pmat, m_cplx);
	ret = new StackItemMatrixCplx(m_cplx);
	return ST_ERR_OK;
}


//
// StackItemString
//

st_err_t parser_str_to(TransStack& ts, string s, string& cmd_err);
st_err_t StackItemString::op_str_to(TransStack& ts, string& cmd_err) {
	return parser_str_to(ts, s, cmd_err);
}

StackItem *StackItemString::dup() const { return new StackItemString(s); }

sitype_t StackItemString::get_type() const { return SITYPE_STRING; }
bool StackItemString::same(StackItem* si) const {
	if (si->get_type() == SITYPE_STRING)
		return (s == dynamic_cast<StackItemString*>(si)->s);
	return false;
}

void StackItemString::to_string(ToString& tostr, const bool&) const { tostr.add_string("\"" + s + "\""); }

st_err_t StackItemString::op_add(StackItemString *arg1, StackItem*& ret) { ret = new StackItemString(arg1->s + s); return ST_ERR_OK; }

st_err_t StackItemString::op_lower(StackItemString *arg1, StackItem*& ret) {
	real res = (arg1->s < s ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

st_err_t StackItemString::op_lower_or_equal(StackItemString *arg1, StackItem*& ret) {
	real res = (arg1->s <= s ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

st_err_t StackItemString::op_greater(StackItemString *arg1, StackItem*& ret) {
	real res = (arg1->s > s ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

st_err_t StackItemString::op_greater_or_equal(StackItemString *arg1, StackItem*& ret) {
	real res = (arg1->s >= s ? 1 : 0);
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}

st_err_t read_rc_file(TransStack*, const tostring_t&, std::istream&, const char *sz, const bool&, std::string&, std::string&, const int& = -1);
st_err_t StackItemString::op_read(TransStack& ts) {
	string error_l1, error_l2;

	ifstream ifs(s.c_str(), ifstream::in);
	st_err_t c = read_rc_file(&ts, TOSTRING_PORTABLE, ifs, s.c_str(), false, error_l1, error_l2, 1);
	ifs.close();

	return c;
}

st_err_t StackItemString::op_write(StackItem& arg1) {
	fstream ofs(s.c_str(), fstream::out | fstream::trunc);
	st_err_t c = ST_ERR_OK;
	if (ofs.good())
		write_si(TOSTRING_PORTABLE, &arg1, ofs);
	else
		c = ST_ERR_FILE_WRITE_ERROR;
	ofs.close();
	return c;
}

st_err_t StackItemString::op_size(StackItem*& ret) {
	ret = new StackItemReal(Real(static_cast<int>(E->get_string_length(s.c_str()))));
	return ST_ERR_OK;
}

st_err_t StackItemString::op_cmdsub(const int& n1, const int& n2, StackItem*& ret) {
	string s_sub;
	if (n1 > n2)
		s_sub = "";
	else
		s_sub = E->substr(s, static_cast<size_t>(n1 - 1), static_cast<size_t>(n2 - n1 + 1));
	ret = new StackItemString(s_sub);
	return ST_ERR_OK;
}


//
// StackItemExpression
//

StackItemExpression::StackItemExpression(const string& ee, const bool& qq) : quoted(qq), e(ee) { }

StackItem *StackItemExpression::dup() const { return new StackItemExpression(e, quoted); }

sitype_t StackItemExpression::get_type() const { return is_varname() ? SITYPE_NAME : SITYPE_EXPRESSION; }
bool StackItemExpression::same(StackItem* si) const {
	if (si->get_type() == get_type()) {
		StackItemExpression *sie = dynamic_cast<StackItemExpression*>(si);
		return (quoted == sie->quoted && e == sie->e);
	}
	return false;
}

/*st_err_t StackItemExpression::op_equal_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_equal(this, ret); }
st_err_t StackItemExpression::op_notequal_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_notequal(this, ret); }*/
st_err_t StackItemExpression::op_equal(StackItemExpression* arg2, StackItem*& ret) {
	ret = new StackItemExpression(arg2->e + "==" + e, quoted);
	return ST_ERR_OK;
}
st_err_t StackItemExpression::op_notequal(StackItemExpression* arg2, StackItem*& ret) {
	ret = new StackItemExpression(arg2->e + "<>" + e, quoted);
	return ST_ERR_OK;
}

void StackItemExpression::to_string(ToString& tostr, const bool& without_quotes) const {
	if (quoted && (!without_quotes || tostr.get_type() != TOSTRING_DISPLAY))
		tostr.add_string("'" + e + "'");
	else
		tostr.add_string(e);
}

bool StackItemExpression::is_varname() const {
	bool prems = true;
	for (string::const_iterator it = e.begin(); it < e.end(); it++) {
		if ((*it) == '-') {
			it++;
			if (it >= e.end())
				return false;
			else if ((*it) != '<' && (*it) != '>')
				return false;
		} else if ((!isalnum(*it) || (isdigit(*it) && prems)) && (*it) != '_')
			return false;
		prems = false;
	}
	return true;
}

bool StackItemExpression::is_unquoted_varname() const {
	return (is_varname() && !quoted);
}

const string& StackItemExpression::get_varname() const {
	return e;
}

st_err_t StackItemExpression::op_sto(TransStack& ts, SIO& arg1) {
	if (!is_varname() || !quoted)
		return ST_ERR_BAD_ARGUMENT_TYPE;
	StackItem *si = arg1.si;
	get_ready_si(arg1, si);
	return ts.sto(e, si);
}

st_err_t StackItemExpression::op_rcl(TransStack& ts) {
	if (!is_varname() || !quoted)
		return ST_ERR_BAD_ARGUMENT_TYPE;
	StackItem *si;
	Var *entry;
	st_err_t c = ts.rcl(e, si, entry);
	if (c != ST_ERR_OK)
		return c;
	ts.transstack_push(si);
	return ST_ERR_OK;
}

st_err_t StackItemExpression::op_purge(TransStack& ts) {
	if (!is_varname() || !quoted)
		return ST_ERR_BAD_ARGUMENT_TYPE;
	return ts.purge(e);
}

st_err_t StackItemExpression::op_crdir(TransStack& ts) {
	if (!is_varname() || !quoted)
		return ST_ERR_BAD_ARGUMENT_TYPE;
	return ts.crdir(e);
}

st_err_t StackItemExpression::eval(const eval_t& et, TransStack& ts, manage_si_t& msi, string& cmd_err) {
	msi = MANAGE_SI_DONOTHING;

	if (et == EVAL_SOFT && quoted) {
		msi = MANAGE_SI_PUSH;
		return ST_ERR_OK;
	}

	SIO s;
	st_err_t c = ts.rcl_for_eval(e, s.si);
	if (c == ST_ERR_UNDEFINED_NAME && !quoted) {
		ts.transstack_push(new StackItemExpression(e, true));
		c = ST_ERR_OK;
	} else if (c == ST_ERR_UNDEFINED_NAME) {
		msi = MANAGE_SI_PUSH;
		c = ST_ERR_OK;
	} else if (c == ST_ERR_OK && s.si != 0) {
		  // Found, and the variable did contain an item
		s.ownership = TSO_OWNED_BY_TS;
		c = ts.inner_push_eval(EVAL_HARD, s, true, cmd_err);
	}
	return c;
}


//
// StackItemMeta
//

StackItemMeta::~StackItemMeta() {
	for (vector<StackItem*>::iterator it = list.begin(); it < list.end(); it++)
		delete *it;
}

StackItemMeta::StackItemMeta(const std::vector<StackItem*>& sim) : list(sim) { copy_items(); }

void StackItemMeta::copy_items() {
	for (size_t i = 0; i < list.size(); i++)
		list[i] = list[i]->dup();
}

void StackItemMeta::insert_item(StackItem *si) { list.insert(list.begin(), si); }
void StackItemMeta::append_item(StackItem *si) { list.push_back(si); }

size_t StackItemMeta::get_nb_items() const { return list.size(); }

sitype_t StackItemMeta::get_type() const { return SITYPE_INTERNAL_META; }
bool StackItemMeta::same(StackItem* si, const sitype_t& expected_type) const {
	if (si->get_type() != expected_type)
		return false;
	vector<StackItem*>* remote_list = &(static_cast<StackItemMeta*>(si)->list);
	vector<StackItem*>::iterator remote_it = remote_list->begin();
	for (vector<StackItem*>::const_iterator my_it = list.begin(); my_it != list.end(); my_it++) {
		if (remote_it == remote_list->end())
			return false;
		if (!(*my_it)->same(*remote_it))
			return false;
		remote_it++;
	}
	return remote_it == remote_list->end();
}

StackItem *StackItemMeta::get_dup_nth_item(const int& n) const {
	if (n < 1 || static_cast<unsigned int>(n) > list.size())
		throw(CalcFatal(__FILE__, __LINE__, "StackItemMeta::get_dup_nth_item(): n out of bounds!"));
	return list[n - 1]->dup();
}

void StackItemMeta::replace_nth_item(int n, SIO& s) {
	StackItem *si;
	get_ready_si(s, si);
	--n;
	delete list[n];
	list[n] = si;
}


//
// StackItemList
//

StackItemList::StackItemList(const std::vector<StackItem*>& l) : StackItemMeta(l) { }

StackItem *StackItemList::dup() const { return new StackItemList(list); }

sitype_t StackItemList::get_type() const { return SITYPE_LIST; }
bool StackItemList::same(StackItem* si) const {
	return StackItemMeta::same(si, SITYPE_LIST);
}

void StackItemList::to_string(ToString& tostr, const bool&) const {
	tostr.add_string("{");
	for (size_t i = 0; i < list.size(); i++)
		list[i]->to_string(tostr, true);
	tostr.add_string("}");
}

st_err_t StackItemList::op_list_to(TransStack& ts, const tso_t& o) {
	SIO s;
	s.ownership = o;
	StackItem *si;
	for (size_t i = 0; i < list.size(); i++) {
		s.si = list[i];
		get_si_dup_if_necessary(s, si);
		ts.transstack_push(si);
	}
	ts.transstack_push(new StackItemReal(Real(list.size())));
	if (o == TSO_OUTSIDE_TS)
		list.clear();
	return ST_ERR_OK;
}

st_err_t StackItemList::inner_get_coordinates(TransStack& ts, Coordinates& coord) {
	st_err_t c = ST_ERR_OK;

	size_t nb = get_nb_items();
	if (nb >= 1 && nb <= 2) {
		int value;
		coord.j = -1;
		for (int ii = 0; static_cast<size_t>(ii) < nb; ii++) {
			c = list[ii]->to_integer(value);
			if (c != ST_ERR_OK || value < 1) {
				c = ST_ERR_BAD_ARGUMENT_VALUE;
				break;
			}
			if (ii == 0)
				coord.i = value;
			else if (ii == 1)
				coord.j = value;
			else
				throw(CalcFatal(__FILE__, __LINE__, "StackItemList::inner_get_coordinates(): inconsistent values encountered!!!??"));
		}
		coord.d = (nb == 1 ? DIM_VECTOR : DIM_MATRIX);
	} else
		c = ST_ERR_BAD_ARGUMENT_VALUE;

	if (c != ST_ERR_OK) {
		TransStack *tmpts = new TransStack(true, false, ts.get_exec_stack(), 0);
		bool inside_undo_sequence = false;
		string cmd_err;
		c = ST_ERR_OK;
		SIO s;

#ifdef DEBUG
		for (vector<StackItem*>::iterator it = list.begin(); it != list.end() && c == ST_ERR_OK; it++) {
			debug_write("Item to evaluate separately: ");
			string x = simple_string(*it);
			debug_write(x.c_str());
		}
#endif

		for (vector<StackItem*>::iterator it = list.begin(); it != list.end() && c == ST_ERR_OK; it++) {
			s.ownership = TSO_OWNED_BY_TS;
			s.si = (*it)->dup();
			c = tmpts->do_push_eval(s, inside_undo_sequence, cmd_err, EVAL_HARD);
		}
		if (c != ST_ERR_OK) {
			c = ST_ERR_BAD_ARGUMENT_VALUE;
		} else {
			int n = static_cast<int>(tmpts->stack_get_count());

#ifdef DEBUG
			const StackItem *csi;
			for (int i = 1; i <= n; i++) {
				csi = tmpts->transstack_get_const_si(i);
				debug_write_i("Result: item number %i", i);
				debug_write(simple_string(const_cast<StackItem*>(csi)).c_str());
			}
#endif

			if (n >= 1 && n <= 2) {
				SIO *items = new SIO[n];
				for (int i = 0; i < n; i++)
					items[i] = tmpts->transstack_pop();
				int value;
				for (int i = 0; i < n; i++) {
					c = items[i].si->to_integer(value);
					if (c == ST_ERR_OK && value >= 1) {
						if (n - 1 - i == 0)
							coord.i = value;
						else
							coord.j = value;
					} else {
						c = ST_ERR_BAD_ARGUMENT_VALUE;
						break;
					}
				}
				for (int i = 0; i < n; i++)
					items[i].cleanup();
				delete []items;
				if (c == ST_ERR_OK)
					coord.d = (n == 1 ? DIM_VECTOR : DIM_MATRIX);
				else
					c = ST_ERR_BAD_ARGUMENT_VALUE;
			} else {
				c = ST_ERR_BAD_ARGUMENT_VALUE;
			}
		}
		delete tmpts;

#ifdef DEBUG
		debug_write("coord.d:");
		if (coord.d == DIM_VECTOR)
			debug_write("vector");
		else
			debug_write("matrix");
		debug_write_i("coord.i = %i", coord.i);
		debug_write_i("coord.j = %i", coord.j);
#endif
	}

	return c;
}

st_err_t StackItemList::get_bounds(Coordinates& coord) const {
	coord.d = DIM_VECTOR;
	coord.i = get_nb_items();
	coord.j = -1;
	return ST_ERR_OK;
}

st_err_t StackItemList::op_size(StackItem*& ret) {
	ret = new StackItemReal(Real(static_cast<int>(get_nb_items())));
	return ST_ERR_OK;
}

st_err_t StackItemList::get_coordinates(TransStack& ts, StackItem *si, Coordinates& coord) {
	Coordinates bounds;
	st_err_t c = inner_get_coordinates(ts, coord);
	if (c != ST_ERR_OK)
		return c;
	if ((c = si->get_bounds(bounds)) != ST_ERR_OK)
		return c;
	if ((c = check_coordinates(bounds, coord)) != ST_ERR_OK)
		return c;
	return ST_ERR_OK;
}

st_err_t StackItem::op_put(TransStack& ts, StackItemMatrixReal* simr, SIO& s) { return s.si->op_put_matrix(ts, this, simr); }
st_err_t StackItem::op_put(TransStack& ts, StackItemMatrixCplx* simc, SIO& s) { return s.si->op_put_matrix(ts, this, simc); }

static void increment_coordinates(const Coordinates& bounds, Coordinates& coord) {
	if (bounds.d != coord.d)
		return;
	if (coord.d == DIM_VECTOR) {
		coord.i++;
		if (coord.i > bounds.i) {
			coord.i = 1;
		}
	} else {
		  // dim_t == DIM_MATRIX
		coord.j++;
		if (coord.j > bounds.j) {
			coord.j = 1;
			coord.i++;
			if (coord.i > bounds.i) {
				coord.i = 1;
			}
		}
	}
}

st_err_t StackItemList::increment_coord(TransStack& ts, const Coordinates& bounds) {
	Coordinates coord;
	st_err_t c = inner_get_coordinates(ts, coord);
	if (c != ST_ERR_OK)
		return c;
	if (coord.d != bounds.d)
		return ST_ERR_BAD_ARGUMENT_VALUE;
	if ((coord.d == DIM_VECTOR && get_nb_items() != 1) || (coord.d == DIM_MATRIX && get_nb_items() != 2))
		throw(CalcFatal(__FILE__, __LINE__, "StackItemList::list_increment_coord(): internal error"));
	increment_coordinates(bounds, coord);
	if (list[0]->get_type() == SITYPE_REAL)
		dynamic_cast<StackItemReal*>(list[0])->set_Real(Real(coord.i));
	else {
		delete list[0];
		list[0] = new StackItemReal(Real(coord.i));
	}
	if (coord.d == DIM_MATRIX) {
		if (list[1]->get_type() == SITYPE_REAL)
			dynamic_cast<StackItemReal*>(list[1])->set_Real(Real(coord.j));
		else {
			delete list[1];
			list[1] = new StackItemReal(Real(coord.j));
		}
	}
	return ST_ERR_OK;
}

st_err_t StackItemList::op_to_arry(TransStack& ts) {
	Coordinates coord;
	st_err_t c = inner_get_coordinates(ts, coord);
	if (c != ST_ERR_OK)
		return c;
	if (coord.i < 1)
		return ST_ERR_BAD_ARGUMENT_VALUE;
	if (coord.d == DIM_MATRIX && coord.j < 1)
		return ST_ERR_BAD_ARGUMENT_VALUE;
	int nb_lines = (coord.d == DIM_VECTOR ? 1 : coord.i);
	int nb_columns = (coord.d == DIM_VECTOR ? coord.i : coord.j);
	int n = (nb_lines * nb_columns);
	if (static_cast<size_t>(n) > ts.stack_get_count())
		return ST_ERR_TOO_FEW_ARGUMENTS;
	SIO *items = new SIO[n];
	for (int i = n - 1; i >= 0; i--)
		items[i] = ts.transstack_pop();
	sitype_t matrix_type = SITYPE_MATRIX_REAL;
	  // c IS equal to ST_ERR_OK at this stage, but it is safer to assign it the correct
	  // value for the code that follows. Who knows...
	c = ST_ERR_OK;
	  // Below we check whether or not items are of the correct type (real or complex),
	  // by the way we work out whether we are creating a real or complex array.
	for (int i = 0; i < n; i++) {
		if (items[i].si->get_type() == SITYPE_COMPLEX)
			matrix_type = SITYPE_MATRIX_COMPLEX;
		else if (items[i].si->get_type() != SITYPE_REAL) {
			c = ST_ERR_BAD_ARGUMENT_VALUE;
			break;
		}
	}
	if (c != ST_ERR_OK) {
		for (int i = 0; i < n; i++)
			ts.transstack_push(items[i].si);
		delete []items;
		return c;
	}

// At this point, we've got the list of scalar values in the array "items".
// Each is either real or complex, if there's at least one that's complex, then
// matrix_type is equal to SITYPE_MATRIX_COMPLEX, otherwise it is equal to
// SITYPE_MATRIX_REAL.

	Matrix<Real> *pmatr = NULL;
	Matrix<Cplx> *pmatc = NULL;
	if (matrix_type == SITYPE_MATRIX_REAL)
		pmatr = new Matrix<Real>(coord.d, nb_lines, nb_columns, Real(0));
	else if (matrix_type == SITYPE_MATRIX_COMPLEX)
		pmatc = new Matrix<Cplx>(coord.d, nb_lines, nb_columns, Cplx(0, 0));
	else
		throw(CalcFatal(__FILE__, __LINE__, "StackItemList::op_to_arry(): I'm creating an array that contains neither REALs, nor COMPLEXs. How hard you are with me!"));
	int idx = 0;
	StackItem *si;
	sitype_t t;
	Real scalar_real;
	Cplx scalar_cplx;
	for (int line = 0; line < nb_lines; line++) {
		for (int column = 0; column < nb_columns; column++) {
			si = items[idx].si;
			t = si->get_type();
			if (matrix_type == SITYPE_MATRIX_REAL) {
				if (t != SITYPE_REAL)
					throw(CalcFatal(__FILE__, __LINE__, "StackItemList::op_to_arry(): inconsistent internal variables!"));
				scalar_real = dynamic_cast<StackItemReal*>(si)->get_Real();
				pmatr->set_value(line, column, scalar_real);
			} else if (matrix_type == SITYPE_MATRIX_COMPLEX) {
				if (t == SITYPE_REAL)
					scalar_cplx = Cplx(dynamic_cast<StackItemReal*>(si)->get_Real());
				else if (t == SITYPE_COMPLEX)
					scalar_cplx = dynamic_cast<StackItemCplx*>(si)->get_Cplx();
				else
					throw(CalcFatal(__FILE__, __LINE__, "StackItemList::op_to_arry(): inconsistent internal variables! #2"));
				pmatc->set_value(line, column, scalar_cplx);
			} else
				throw(CalcFatal(__FILE__, __LINE__, "StackItemList::op_to_arry(): inconsistent internal variables! #3"));
			idx++;
		}
	}
	if (idx != n)
		throw(CalcFatal(__FILE__, __LINE__, "StackItemList::op_to_arry(): guess what? I walked through items with a lot of discipline but I ended up upside down! There are missing or extra items, what a mess!"));

	StackItem *si2 = NULL;
	if (matrix_type == SITYPE_MATRIX_REAL)
		si2 = new StackItemMatrixReal(pmatr);
	else if (matrix_type == SITYPE_MATRIX_COMPLEX)
		si2 = new StackItemMatrixCplx(pmatc);
	ts.transstack_push(si2);

	for (int i = 0; i < n; i++)
		items[i].cleanup();
	delete []items;
	return ST_ERR_OK;
}

st_err_t StackItemList::list_add(const bool& insert, StackItem* si, StackItem*& ret) {
	StackItemList *sil = dynamic_cast<StackItemList*>(dup());
	if (insert)
		sil->insert_item(si->dup());
	else
		sil->append_item(si->dup());
	ret = sil;
	return ST_ERR_OK;
}

st_err_t StackItemBinary::op_add(StackItemList* sil, StackItem*& ret) { return sil->list_add(false, this, ret); }
st_err_t StackItemReal::op_add(StackItemList* sil, StackItem*& ret) { return sil->list_add(false, this, ret); }
st_err_t StackItemCplx::op_add(StackItemList* sil, StackItem*& ret) { return sil->list_add(false, this, ret); }
st_err_t StackItemMatrixReal::op_add(StackItemList* sil, StackItem*& ret) { return sil->list_add(false, this, ret); }
st_err_t StackItemMatrixCplx::op_add(StackItemList* sil, StackItem*& ret) { return sil->list_add(false, this, ret); }
st_err_t StackItemString::op_add(StackItemList* sil, StackItem*& ret) { return sil->list_add(false, this, ret); }
st_err_t StackItemExpression::op_add(StackItemList* sil, StackItem*& ret) { return sil->list_add(false, this, ret); }
st_err_t StackItemList::op_add(StackItemList* sil, StackItem*& ret) {
	StackItemList *copy_sil = dynamic_cast<StackItemList*>(sil->dup());
	for (vector<StackItem*>::iterator it = list.begin(); it != list.end(); it++)
		copy_sil->append_item((*it)->dup());
	ret = copy_sil;
	return ST_ERR_OK;
}
st_err_t StackItemProgram::op_add(StackItemList* sil, StackItem*& ret) { return sil->list_add(false, this, ret); }
st_err_t StackItemList::op_add(StackItemBinary* si, StackItem*& ret) { return list_add(true, si, ret); }
st_err_t StackItemList::op_add(StackItemReal* si, StackItem*& ret) { return list_add(true, si, ret); }
st_err_t StackItemList::op_add(StackItemCplx* si, StackItem*& ret) { return list_add(true, si, ret); }
st_err_t StackItemList::op_add(StackItemMatrixReal* si, StackItem*& ret) { return list_add(true, si, ret); }
st_err_t StackItemList::op_add(StackItemMatrixCplx* si, StackItem*& ret) { return list_add(true, si, ret); }
st_err_t StackItemList::op_add(StackItemString* si, StackItem*& ret) { return list_add(true, si, ret); }
st_err_t StackItemList::op_add(StackItemExpression* si, StackItem*& ret) { return list_add(true, si, ret); }
st_err_t StackItemList::op_add(StackItemProgram* si, StackItem*& ret) { return list_add(true, si, ret); }

#define IMPLEMENT_LIST_OP_RDM(MATSI, SCALAR) \
st_err_t StackItemList::op_rdm(TransStack& ts, MATSI* sim, StackItem*& ret) { \
	Coordinates coord; \
	st_err_t c = inner_get_coordinates(ts, coord); \
	if (c != ST_ERR_OK) \
		return c; \
	int i, j; \
	COORD_TO_MATRIX_IJ(coord, i, j); \
 \
	if (cfg_rdm_behavior == RDM_TABLE) { \
		MATSI *sim_copy = new MATSI(*sim); \
		sim_copy->rdm(coord.d, i + 1, j + 1); \
		ret = sim_copy; \
	} else if (cfg_rdm_behavior == RDM_HP) { \
		Matrix<SCALAR> *pmat = new Matrix<SCALAR>(coord.d, i + 1, j + 1, SCALAR(Real(0))); \
		pmat->copy_linear(sim->get_matrix()); \
		ret = new MATSI(pmat); \
	} else \
		throw(CalcFatal(__FILE__, __LINE__, "StackItemList::op_rdm(): don't know how to redim array")); \
 \
	return ST_ERR_OK; \
}
IMPLEMENT_LIST_OP_RDM(StackItemMatrixReal, Real)
IMPLEMENT_LIST_OP_RDM(StackItemMatrixCplx, Cplx)

st_err_t StackItemList::op_cmdsub(const int& n1, const int& n2, StackItem*& ret) {
	StackItemList *sil = new StackItemList();
	for (int i = n1; i <= n2 && static_cast<size_t>(i) <= get_nb_items(); i++)
		sil->append_item(list[i - 1]->dup());
	ret = sil;
	return ST_ERR_OK;
}


//
// StackItemProgram
//

StackItemProgram::StackItemProgram(const bool& b, VarDirectory *const &v) : brackets(b), lvars(v) { }

StackItemProgram::StackItemProgram(const StackItemProgram& sip) : StackItemMeta(sip.list), brackets(sip.brackets), lvars(sip.lvars) {
	if (lvars != 0)
		lvars = dynamic_cast<VarDirectory*>(lvars->dup(false));
}

StackItemProgram::~StackItemProgram() {
	if (lvars != NULL)
		delete lvars;
}

StackItem *StackItemProgram::dup() const { return new StackItemProgram(*this); }

sitype_t StackItemProgram::get_type() const { return SITYPE_PROGRAM; }
bool StackItemProgram::same(StackItem* si) const {
	if (!StackItemMeta::same(si, SITYPE_PROGRAM))
		return false;
	StackItemProgram* sip = dynamic_cast<StackItemProgram*>(si);
	if ((lvars != NULL && sip->lvars == NULL) || (lvars == NULL && sip->lvars != NULL))
		return false;
	if (lvars != NULL)
		return lvars->same(sip->lvars);
	return true;
}

void StackItemProgram::to_string(ToString& tostr, const bool&) const {
	lvars_to_string(lvars, tostr);
	if (brackets)
		tostr.add_string("<<");
	for (size_t i = 0; i < list.size(); i++)
		list[i]->to_string(tostr);
	if (brackets)
		tostr.add_string(">>");
}

st_err_t StackItemProgram::eval(const eval_t& et, TransStack&, manage_si_t& msi, string&) {
	msi = ((brackets && et == EVAL_SOFT && !lvars) ? MANAGE_SI_PUSH : MANAGE_SI_EXEC);
	return ST_ERR_OK;
}

st_err_t StackItemProgram::exec1(TransStack& ts, int& ip, string& cmd_err) {
	if (ip < 0 || ip > (int)list.size() - 1) {
		ip = IP_END;
		return ST_ERR_OK;
	}
	SIO s = SIO(TSO_FROZEN, list[ip++]);
	st_err_t c = ts.inner_push_eval(EVAL_SOFT, s, true, cmd_err);
	return c;
}

void StackItemProgram::next_instruction(const int& ip, std::string& s) const {
	if (ip < 0 || ip > (int)list.size() - 1) {
		s = (ip < 0 ? "<<" : ">>");
		return;
	}
	if (ip == 0) {
		StackItem::next_instruction(0, s);
	} else {
		list[ip]->next_instruction(0, s);
	}
}

VarDirectory *StackItemProgram::get_lvars(bool& feed_with_stack) const {
	feed_with_stack = true;
	return lvars;
}

void StackItemProgram::clear_lvars() {
	if (lvars != 0)
		lvars->clear_si();
}


//
// StackItemBranch
//

sitype_t StackItemBranch::get_type() const { return SITYPE_INTERNAL_BRANCH; }
bool StackItemBranch::same(StackItemBranch* si) const {
	if (sis_size != si->sis_size)
		return false;
	for (int i = 0; i < sis_size; i++) {
		if ((sis[i] != NULL && si->sis[i] == NULL) || (sis[i] == NULL && si->sis[i] != NULL))
			return false;
		if (sis[i] != NULL)
			if (!sis[i]->same(si->sis[i]))
				return false;
	}
	return true;
}

StackItemBranch::StackItemBranch(const int& s) : sis_size(s) {
	sis = new StackItemProgram*[sis_size];
	for (int i = 0; i < sis_size; i++)
		sis[i] = NULL;
}

StackItemBranch::StackItemBranch(const StackItemBranch& sib) : StackItem(), sis_size(sib.sis_size) {
	sis = new StackItemProgram*[sis_size];
	for (int i = 0; i < sis_size; i++) {
		sis[i] = sib.sis[i];
		if (sis[i] != NULL)
			sis[i] = dynamic_cast<StackItemProgram*>(sis[i]->dup());
	}
}

StackItemBranch::~StackItemBranch() {
	for (int i = 0; i < sis_size; i++)
		if (sis[i] != 0)
			delete sis[i];
	delete []sis;
}

void StackItemBranch::add_item(const int& s, StackItem *si) {
	if (s < 0 || s >= sis_size)
		throw(CalcFatal(__FILE__, __LINE__, "MyClass::add_item(): call with a bad sequence number"));
	if (sis[s] == NULL)
		sis[s] = new StackItemProgram(false, NULL);
	if (si != NULL)
		sis[s]->append_item(si);
}

st_err_t StackItemBranch::eval(const eval_t&, TransStack&, manage_si_t& msi, string&) {
	msi = MANAGE_SI_EXEC;
	return ST_ERR_OK;
}


//
// StackItemBranchIf
//

StackItemBranchIf::StackItemBranchIf() : StackItemBranch(3) { }
StackItemBranchIf::StackItemBranchIf(const StackItemBranchIf& sib) : StackItemBranch(sib) { }
StackItemBranchIf::~StackItemBranchIf() { }

StackItem *StackItemBranchIf::dup() const { return new StackItemBranchIf(*this); }

sitype_t StackItemBranchIf::get_type() const { return SITYPE_INTERNAL_BRANCH_IF; }
bool StackItemBranchIf::same(StackItem* si) const {
	if (si->get_type() == SITYPE_INTERNAL_BRANCH_IF)
		return StackItemBranch::same(dynamic_cast<StackItemBranchIf*>(si));
	return false;
}

void StackItemBranchIf::to_string(ToString& tostr, const bool&) const {
	tostr.add_string(branch_str[IT_IF]);
	if (sis[MY_IF] != 0)
		sis[MY_IF]->to_string(tostr);
	tostr.add_string(branch_str[IT_THEN]);
	if (sis[MY_THEN] != 0)
		sis[MY_THEN]->to_string(tostr);
	if (sis[MY_ELSE] != 0) {
		tostr.add_string(branch_str[IT_ELSE]);
		sis[MY_ELSE]->to_string(tostr);
	}
	tostr.add_string(branch_str[IT_END]);
}

st_err_t StackItemBranchIf::exec1(TransStack& ts, int& ip, string& cmd_err) {
	if (ip < 0 || ip > 2) {
		ip = IP_END;
		return ST_ERR_OK;
	}

	st_err_t c = ST_ERR_OK;

	if (ip == 1) {
		  // About to execute "THEN" part
		int n;
		c = ts.read_integer(n);
		if (c == ST_ERR_OK) {
			if (!n)
				ip = 2;
		} else
			cmd_err = branch_str[IT_IF];
	}

	if (c == ST_ERR_OK) {
		if (sis[ip]) {
			SIO s(TSO_FROZEN, sis[ip++]);
			c = ts.inner_push_eval(EVAL_SOFT, s, true, cmd_err);
		} else
			ip++;
		if (ip >= 2)
			ip = IP_END;
	}

	return c;
}


//
// StackItemBranchWhile
//

StackItemBranchWhile::StackItemBranchWhile() : StackItemBranch(2) { }
StackItemBranchWhile::StackItemBranchWhile(const StackItemBranchWhile& sib) : StackItemBranch(sib) { }
StackItemBranchWhile::~StackItemBranchWhile() { }

StackItem *StackItemBranchWhile::dup() const { return new StackItemBranchWhile(*this); }

sitype_t StackItemBranchWhile::get_type() const { return SITYPE_INTERNAL_BRANCH_WHILE; }
bool StackItemBranchWhile::same(StackItem* si) const {
	if (si->get_type() == SITYPE_INTERNAL_BRANCH_WHILE)
		return StackItemBranch::same(dynamic_cast<StackItemBranchWhile*>(si));
	return false;
}

void StackItemBranchWhile::to_string(ToString& tostr, const bool&) const {
	tostr.add_string(branch_str[IT_WHILE]);
	if (sis[MY_WHILE] != NULL)
		sis[MY_WHILE]->to_string(tostr);
	tostr.add_string(branch_str[IT_REPEAT]);
	if (sis[MY_REPEAT] != NULL)
		sis[MY_REPEAT]->to_string(tostr);
	tostr.add_string(branch_str[IT_END]);
}

st_err_t StackItemBranchWhile::exec1(TransStack& ts, int& ip, string& cmd_err) {
	if (ip < 0 || ip > 1) {
		ip = IP_END;
		return ST_ERR_OK;
	}

	st_err_t c = ST_ERR_OK;

	if (ip == 1) {
		int n;
		c = ts.read_integer(n);
		if (c == ST_ERR_OK) {
			if (!n)
				ip = IP_END;
		} else
			cmd_err = branch_str[IT_WHILE];
	}

	if (c == ST_ERR_OK) {
		if (ip >= 0 && ip <= 1) {
			if (sis[ip]) {
				SIO s(TSO_FROZEN, sis[ip++]);
				c = ts.inner_push_eval(EVAL_SOFT, s, true, cmd_err);
			} else
				ip++;
		}
		if (ip >= 2)
			ip = 0;
	}

	return c;
}


//
// StackItemBranchDo
//

StackItemBranchDo::StackItemBranchDo() : StackItemBranch(2) { }
StackItemBranchDo::StackItemBranchDo(const StackItemBranchDo& sib) : StackItemBranch(sib) { }
StackItemBranchDo::~StackItemBranchDo() { }

StackItem *StackItemBranchDo::dup() const { return new StackItemBranchDo(*this); }

sitype_t StackItemBranchDo::get_type() const { return SITYPE_INTERNAL_BRANCH_DO; }
bool StackItemBranchDo::same(StackItem* si) const {
	if (si->get_type() == SITYPE_INTERNAL_BRANCH_DO)
		return StackItemBranch::same(dynamic_cast<StackItemBranchDo*>(si));
	return false;
}

void StackItemBranchDo::to_string(ToString& tostr, const bool&) const {
	tostr.add_string(branch_str[IT_DO]);
	if (sis[MY_DO] != NULL)
		sis[MY_DO]->to_string(tostr);
	tostr.add_string(branch_str[IT_UNTIL]);
	if (sis[MY_UNTIL] != NULL)
		sis[MY_UNTIL]->to_string(tostr);
	tostr.add_string(branch_str[IT_END]);
}

st_err_t StackItemBranchDo::exec1(TransStack& ts, int& ip, string& cmd_err) {
	if (ip < 0 || ip > 2) {
		ip = IP_END;
		return ST_ERR_OK;
	}

	st_err_t c = ST_ERR_OK;

	if (ip == 2) {
		int n;
		c = ts.read_integer(n);
		if (c == ST_ERR_OK) {
			if (n)
				ip = IP_END;
			else
				ip = 0;
		} else
			cmd_err = branch_str[IT_UNTIL];
	}

	if (c == ST_ERR_OK) {
		if (ip >= 0 && ip <= 1) {
			if (sis[ip]) {
				SIO s(TSO_FROZEN, sis[ip++]);
				c = ts.inner_push_eval(EVAL_SOFT, s, true, cmd_err);
			} else
				ip++;
		}
	}

	return c;
}


//
// StackItemBranchLoop
//

StackItemBranchLoop::StackItemBranchLoop(const int& o) : StackItemBranch(1), loop_open(o), loop_close(LOOP_CLOSE_UNDEFINED),
	value_begin(0), value_end(0), lvars(new VarDirectory(0, 0)), varname("") { }
StackItemBranchLoop::StackItemBranchLoop(const StackItemBranchLoop& sib) : StackItemBranch(sib),
	loop_open(sib.loop_open), loop_close(sib.loop_close), value_begin(sib.value_begin), value_end(sib.value_end),
	lvars(sib.lvars), varname(sib.varname) {
	if (loop_close == LOOP_CLOSE_UNDEFINED)
		throw(CalcFatal(__FILE__, __LINE__,
			"StackItemBranchLoop(const StackItemBranchLoop& sil): loop close style undefined"));
	if (lvars != NULL)
		lvars = dynamic_cast<VarDirectory*>(lvars->dup(false));
}
StackItemBranchLoop::~StackItemBranchLoop() {
	if (lvars != NULL)
		delete lvars;
}

void StackItemBranchLoop::set_loop_close(const int& lc) { loop_close = lc; }

sitype_t StackItemBranchLoop::get_type() const { return SITYPE_INTERNAL_BRANCH_LOOP; }
bool StackItemBranchLoop::same(StackItem* si) const {
	if (si->get_type() == SITYPE_INTERNAL_BRANCH_LOOP) {
		StackItemBranchLoop *sil = dynamic_cast<StackItemBranchLoop*>(si);
		if (StackItemBranch::same(sil))
			return loop_open == sil->loop_open && loop_close == sil->loop_close && varname == sil->varname;
	}
	return false;
}

StackItem *StackItemBranchLoop::dup() const {
	if (loop_close == LOOP_CLOSE_UNDEFINED)
		throw(CalcFatal(__FILE__, __LINE__, "StackItemBranchLoop::dup(): loop close style undefined"));
	return new StackItemBranchLoop(*this);
}

void StackItemBranchLoop::to_string(ToString& tostr, const bool&) const {
	tostr.add_string(branch_str[loop_open == LOOP_OPEN_FOR ? IT_FOR : IT_START]);
	if (loop_open == LOOP_OPEN_FOR)
		tostr.add_string(varname);
	if (sis[0] != NULL)
		sis[0]->to_string(tostr);
	tostr.add_string(branch_str[loop_close == LOOP_CLOSE_NEXT ? IT_NEXT : IT_STEP]);
}

st_err_t StackItemBranchLoop::eval(const eval_t& et, TransStack& ts, manage_si_t& msi, string& cmd_err) {
	if (loop_close == LOOP_CLOSE_UNDEFINED)
		throw(CalcFatal(__FILE__, __LINE__, "StackItemBranchLoop::eval(): loop close style undefined"));
	return StackItemBranch::eval(et, ts, msi, cmd_err);
}

st_err_t StackItemBranchLoop::exec1(TransStack& ts, int& ip, string& cmd_err) {
	if (loop_close == LOOP_CLOSE_UNDEFINED)
		throw(CalcFatal(__FILE__, __LINE__, "StackItemBranchLoop::exec1(): loop close style undefined"));
	st_err_t c = ST_ERR_OK;

	if (ip == 0) {
		  // Init loop (read range from stack)
		c = ts.read_integer(value_end);
		if (c == ST_ERR_OK)
			c = ts.read_integer(value_begin);
		if (c == ST_ERR_OK) {
			ip++;
			set_loop_varvalue(value_begin);
		} else {
			cmd_err = branch_str[loop_open == LOOP_OPEN_FOR ? IT_FOR : IT_START];
		}
	} else if (ip == 1) {
		ip++;
		if (sis[0] != NULL) {
			SIO s(TSO_FROZEN, sis[0]);
			c = ts.inner_push_eval(EVAL_SOFT, s, true, cmd_err);
		}
	} else if (ip == 2) {
		int step = 1;
		if (loop_close == LOOP_CLOSE_STEP)
			c = ts.read_integer(step);
		else if (value_begin > value_end)
			step = -1;
		if (c != ST_ERR_OK)
			cmd_err = branch_str[loop_close == LOOP_CLOSE_NEXT ? IT_NEXT : IT_STEP];
		else {
			int newval = get_loop_varvalue() + step;
			if ((step >= 0 && newval > value_end) || (step < 0 && newval < value_end))
				ip = IP_END;
			else
				ip = 1;
			set_loop_varvalue(newval);
		}
	} else
		ip = IP_END;
	return c;
}

VarDirectory *StackItemBranchLoop::get_lvars(bool& feed_with_stack) const {
	feed_with_stack = false;
	return loop_open == LOOP_OPEN_FOR ? lvars : NULL;
}

void StackItemBranchLoop::clear_lvars() {
	if (lvars != 0)
		lvars->clear_si();
}

VarFile *StackItemBranchLoop::get_loop_varfile() const {
	if (lvars == NULL)
		throw(CalcFatal(__FILE__, __LINE__, "StackItemBranchLoop::get_loop_varfile(): no VarFile object found!!!??"));
	Var *e;
	if ((e = lvars->get_var(varname)) != NULL)
		if (e->get_type() == VAR_FILE)
			return dynamic_cast<VarFile*>(e);
	throw(CalcFatal(__FILE__, __LINE__, "StackItemBranchLoop::get_loop_varfile(): found a local directory!!!??"));
}

void StackItemBranchLoop::set_loop_varname(const std::string& s) {
	if (!varname.empty())
		throw(CalcFatal(__FILE__, __LINE__,
			"StackItemBranchLoop::set_loop_varname(): can't change loop variable name once it has been set"));
	varname = s;
	lvars->sto(varname, NULL);
}

int StackItemBranchLoop::get_loop_varvalue() const {
	VarFile *e = get_loop_varfile();
	const StackItem *si = e->get_const_si();
	int n;
	st_err_t c = si->to_integer(n);
	if (c != ST_ERR_OK)
		throw(CalcFatal(__FILE__, __LINE__, "StackItemBranchLoop::get_loop_varvalue(): StackItem is not real ?!!"));
	return n;
}

void StackItemBranchLoop::set_loop_varvalue(const int& n) {
	VarFile *e = get_loop_varfile();
	e->attach_si(new StackItemReal(Real(n)));
}

