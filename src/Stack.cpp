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

using namespace std;

Flag flags[] = {
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
		{true, _N("Binary integers word size"), true},	// FL_BIN_SIZE		37-42
	{false, "", false}, {false, _N("Numeric base for binary integers"), false},		// FL_BIN_BASE		43-44
	{true, _N("Level 1 display"), true},	// FL_DISPLAY_L1	45
	{false, _N("Reserved"), false},	// FL_RESERVED1		46
	{false, _N("Reserved"), false},	// FL_RESERVED2		47
	{false, _N("Decimal separator"), false},	// FL_DECIMAL_SEP	48
	{false, "", false}, {false, _N("Real numbers format"), false},						// FL_REAL_FORMAT	49-50
	{false, _N("Tone"), false},	// FL_TONE			51
	{false, _N("Fast printing"), false},	// FL_FAST_PRINT	52
	{false, "", false}, {false, "", false}, {false, "", false}, {false, _N("Number of decimals"), false},			// FL_REAL_NB_DECS	53-56
	{false, _N("Underflow processed normally"), false},	// FL_UNDERFLOW_OK	57
	{false, _N("Overflow processed normally"), false},					// FL_OVERFLOW_OK	58
	{true, _N("Infinite Result processed normally"), true},			// FL_INFINITE		59
	{false, _N("Angle"), false},										// FL_ANGLE			60
	{false, _N("Underflow- processed as an exception"), false},			// FL_UNDERFLOW_NEG_EXCEPT	61
	{false, _N("Underdlow+ processed as an exception"), false},			// FL_UNDERFLOW_POS_EXCEPT	62
	{false, _N("Overflow processed as an exception"), false},			// FL_OVERFLOW_EXCEPT		63
	{false, _N("Infinite Result processed as an exception"), false}	// FL_INFINITE_EXCEPT		64
};

const int DEFAULT_PORTABLE_BIN_BASE = 16;

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

static const int DEFAULT_UNDO_LEVELS = 50;
static const int HARD_MAX_UNDO_LEVELS = -1;
static int undo_levels = DEFAULT_UNDO_LEVELS;


//
// Functions
//

const string simple_string(StackItem *si) {
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

int get_base_from_flags() {
	if (!flags[FL_BIN_BASE_2].value && !flags[FL_BIN_BASE_2 + 1].value)
		return 10;
	else if (!flags[FL_BIN_BASE_2].value && flags[FL_BIN_BASE_2 + 1].value)
		return 2;
	else if (flags[FL_BIN_BASE_2].value && !flags[FL_BIN_BASE_2 + 1].value)
		return 8;
	else if (flags[FL_BIN_BASE_2].value && flags[FL_BIN_BASE_2 + 1].value)
		return 16;
	  // Never happens ; written to avoid a warning
	return -1;
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

int get_bin_size_from_flags() {
	int r = 0;
	int m = 1;
	for (int i = FL_BIN_SIZE_6; i <= FL_BIN_SIZE_6 + 5; i++) {
		if (flags[i].value)
			r += m;
		m *= 2;
	}
	return r + 1;
}

const string get_error_string(const st_err_t& c) { return st_errors[c]; }

  // Number of columns in a file containing StackItems
#define RC_FILE_WIDTH			78

void write_si(const tostring_t& tostring, const StackItem* csi, ostream& oss) {
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
	char complex_separator = get_complex_separator(type != TOSTRING_PORTABLE);
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
	StackItem* *new_values = new StackItem*[new_alloc];
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

/*
int my_strlen_utf8_c(char *s) {
  int i = 0, j = 0;
  while (s[i]) {
    if ((s[i] & 0xc0) != 0x80) j++;
    i++;
  }
  return j;
}*/

const string Stack::get_display_line(const DisplayStackLayout& dsl, const int& line_number,
						IntervalShift& ishift, bool& recalc, bool& no_more_lines) {
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
							(flags[FL_DISPLAY_L1].value && dsl.get_max_stack() >= 1 ? dsl.get_max_stack()
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

	int item_number;

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

	/*if (w >= 1 && s.length() > static_cast<size_t>(w) && dsl.get_max_stack() >= 1) {
		s.erase(w - dsl.get_to_be_continued_length());
		s.append(dsl.get_to_be_continued());
	}*/
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

int NodeStack::nodestack_push(StackItem *si) {
	int v = st->stack_push(si);
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

	return v;
}

SIO NodeStack::nodestack_pop() {
	SIO s;
	s.si = st->stack_pop();
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
	return s;
}

#endif // STACK_USE_MAP


//
// Exec
//

Exec::Exec(const SIO& ss, VarDirectory *l) : s(ss), lvars(l), ip(0) {
	//cout << "Creation of a new exec level: " << s.ownership << " / " << s.si << endl;
}

Exec::~Exec() {
	//cout << "Deleting one Exec: " << this << "\n";
}

Exec::Exec(const Exec& e) : s(e.s), lvars(e.lvars), ip(e.ip) {
	//cout << "Creating one Exec: " << this << "\n";
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
		l->add_item(new StackItemExpression(*it, false));
	}
	delete directories;
	return l;
}

bool TransStack::TSVars::update_si_path() {
	if (si_path != 0 && !tree->get_pwd_has_changed())
		return false;
	if (si_path != 0)
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

TransStack::TransStack() : count(0), modified_flag(true) {
#ifdef DEBUG_CLASS_COUNT
		class_count++;
#endif
	tail = new NodeStack(0, 0, 0);
	head = tail;
}

TransStack::~TransStack() {
#ifdef DEBUG_CLASS_COUNT
		class_count--;
#endif
#ifdef DEBUG_TRANSSTACK
	cout << "TransStack::~TransStack() (start), count = " << count << endl;
#endif // DEBUG_TRANSSTACK

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

/*sitype_t TransStack::si_get_type(StackItem *const &si) {
	return si->get_type();
}*/

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
#endif // DEBUG_TRANSSTACK

}

void TransStack::control_undos_chain_size() {
	while (undo_levels >= 0 && count > undo_levels + 1)
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
#endif // DEBUG_TRANSSTACK

}

int TransStack::transstack_push(StackItem *si) {
	set_modified_flag(true);
	return head->nodestack_push(si);
}

SIO TransStack::transstack_pop() {
	set_modified_flag(true);
	return head->nodestack_pop();
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
						IntervalShift& ishift, bool& recalc, bool& no_more_lines) {
	return head->st->get_display_line(dsl, line_number, ishift, recalc, no_more_lines);
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
			exec_stack.push_back(Exec(s, v));
			if (feed_with_stack)
				c = read_lvars(v, cmd_err);
		} else if (msi != MANAGE_SI_EXEC && s.ownership == TSO_OWNED_BY_TS)
			delete s.si;
	}
	return c;
}

st_err_t TransStack::do_push_eval(SIO& s, const bool& inside_undo_sequence, string& cmd_err) {
	st_err_t c = inner_push_eval(EVAL_SOFT, s, inside_undo_sequence, cmd_err);
	while (c == ST_ERR_OK && !exec_stack.empty()) {
		c = exec1(cmd_err);
	}
	clear_exec_stack();

#ifdef DEBUG_TRANSSTACK
	cout << "TransStack::tail = [" << tail->get_object_serial() << "], ::head = [" << head->get_object_serial() <<
		"], ::count = " << count << ", ::modified_flag = " << modified_flag << endl;
#endif // DEBUG_TRANSSTACK

	return c;
}

size_t TransStack::stack_get_count() const { return head->st->get_count(); }

const StackItem *TransStack::transstack_get_const_si(const int& stack_item_number) const {
	return head->st->stack_get_const_si(stack_item_number);
}

st_err_t TransStack::exec1(string& cmd_err) {
	if (exec_stack.empty())
		return ST_ERR_OK;
	Exec& e = exec_stack.back();

	st_err_t c = e.s.si->exec1(*this, e.ip, cmd_err);

	if (c != ST_ERR_OK)
		return c;
	Exec& e2 = exec_stack.back();
	if (e2.ip == IP_END)
		pop_exec_stack();
	return ST_ERR_OK;
}

void TransStack::push_exec_stack(SIO s, VarDirectory *lvars) {
	exec_stack.push_back(Exec(s, lvars));
}

void TransStack::clear_exec_stack() {
	while (pop_exec_stack())
		;
}

size_t TransStack::pop_exec_stack() {
	if (exec_stack.empty())
		return 0;
	Exec& e = exec_stack.back();
	if (e.s.ownership == TSO_OWNED_BY_TS)
		delete e.s.si;
	else
		e.s.si->clear_lvars();
	exec_stack.pop_back();
	return exec_stack.size();
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
	for (vector<string>::const_iterator it = by_order->begin(); it != by_order->end(); it++) {
		s = transstack_pop();
		if (s.ownership == TSO_OWNED_BY_TS)
			s.si = s.si->dup();
		if (v->sto(*it, s.si) != ST_ERR_OK)
			throw(CalcFatal(__FILE__, __LINE__, "unable to execute lvars->sto()"));
	}
	return ST_ERR_OK;
}

VarFile *TransStack::get_local_var(const string& s) {
	Var *e;
	for (std::vector<Exec>::reverse_iterator it = exec_stack.rbegin(); it != exec_stack.rend(); it++) {
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

VarDirectory *StackItem::get_lvars(bool& feed_with_stack) const {
	feed_with_stack = false;
	return NULL;
}

void StackItem::clear_lvars() { }


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

  // Arithmetic

void prepare_arith();
static st_err_t bc_add(StackItem& op1, StackItem& op2, StackItem*& ret, string&) { prepare_arith(); return op1.op_add_generic(op2, ret); }
static st_err_t bc_sub(StackItem& op1, StackItem& op2, StackItem*& ret, string&) { prepare_arith(); return op1.op_sub_generic(op2, ret); }
static st_err_t bc_mul(StackItem& op1, StackItem& op2, StackItem*& ret, string&) { prepare_arith(); return op1.op_mul_generic(op2, ret); }
static st_err_t bc_div(StackItem& op1, StackItem& op2, StackItem*& ret, string&) { prepare_arith(); return op1.op_div_generic(op2, ret); }
static st_err_t bc_pow(StackItem& op1, StackItem& op2, StackItem*& ret, string&) { prepare_arith(); return op1.op_pow_generic(op2, ret); }

  // Real functions

static st_err_t bc_cos(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_cos(ret); }
static st_err_t bc_sin(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_sin(ret); }
static st_err_t bc_tan(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_tan(ret); }
static st_err_t bc_acos(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_acos(ret); }
static st_err_t bc_asin(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_asin(ret); }
static st_err_t bc_atan(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_atan(ret); }
static st_err_t bc_ln(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_ln(ret); }
static st_err_t bc_exp(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_exp(ret); }
static st_err_t bc_neg(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_neg(ret); }
static st_err_t bc_ip(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_ip(ret); }
static st_err_t bc_inv(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_inv(ret); }
static st_err_t bc_sq(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_sq(ret); }
static st_err_t bc_sqr(StackItem& op1, StackItem*& ret, string&) { prepare_arith(); return op1.op_sqr(ret); }
static st_err_t bc_r_to_c(StackItem& op1, StackItem& op2, StackItem*& ret, string&) { prepare_arith(); return op1.op_r_to_c_generic(op2, ret); }
static st_err_t bc_c_to_r(TransStack& ts, SIO *args, string&) { return args[0].si->op_c_to_r(ts); }

  // Comparison functions

static st_err_t bc_same(StackItem& op1, StackItem& op2, StackItem*& ret, string&) {
	int res = op1.same(&op2) ? 1 : 0;
	ret = new StackItemReal(Real(res));
	return ST_ERR_OK;
}
static st_err_t bc_equal(StackItem& op1, StackItem& op2, StackItem*& ret, string&) {
	return op1.op_equal_generic(op2, ret);
}
static st_err_t bc_notequal(StackItem& op1, StackItem& op2, StackItem*& ret, string&) {
	return op1.op_notequal_generic(op2, ret);
}
static st_err_t bc_lower(StackItem& op1, StackItem& op2, StackItem*& ret, string&) {
	return op1.op_lower_generic(op2, ret);
}
static st_err_t bc_lower_or_equal(StackItem& op1, StackItem& op2, StackItem*& ret, string&) {
	return op1.op_lower_or_equal_generic(op2, ret);
}
static st_err_t bc_greater(StackItem& op1, StackItem& op2, StackItem*& ret, string&) {
	return op1.op_greater_generic(op2, ret);
}
static st_err_t bc_greater_or_equal(StackItem& op1, StackItem& op2, StackItem*& ret, string&) {
	return op1.op_greater_or_equal_generic(op2, ret);
}

  // Stack manipulation commands

static st_err_t bc_dup(TransStack& ts, SIO *args, string&) {
	ts.transstack_push(args[0].si);
	args[0].ownership = TSO_OWNED_BY_TS;
	ts.transstack_push(args[0].si->dup());
	return ST_ERR_OK;
}

static st_err_t bc_swap(TransStack& ts, SIO *args, string&) {
	ts.transstack_push(args[1].si);
	args[1].ownership = TSO_OWNED_BY_TS;
	ts.transstack_push(args[0].si);
	args[0].ownership = TSO_OWNED_BY_TS;
	return ST_ERR_OK;
}

static st_err_t bc_drop(TransStack&, SIO*, string&) { return ST_ERR_OK; }

static st_err_t bc_clear(TransStack& ts, SIO*, string&) {
	while (ts.stack_get_count() >= 1)
		ts.transstack_pop().cleanup();
	return ST_ERR_OK;
}

static st_err_t bc_over(TransStack& ts, SIO *args, string&) {
	ts.transstack_push(args[0].si);
	args[0].ownership = TSO_OWNED_BY_TS;
	ts.transstack_push(args[1].si);
	args[1].ownership = TSO_OWNED_BY_TS;
	ts.transstack_push(args[0].si->dup());
	return ST_ERR_OK;
}

static st_err_t bc_dup2(TransStack& ts, SIO *args, string&) {
	ts.transstack_push(args[0].si);
	args[0].ownership = TSO_OWNED_BY_TS;
	ts.transstack_push(args[1].si);
	args[1].ownership = TSO_OWNED_BY_TS;
	for (int i = 0; i <= 1; i++)
		ts.transstack_push(args[i].si->dup());
	return ST_ERR_OK;
}

  // Identical of bc_drop(), yes, I know, but I find it cleaner to define a dedicated
  // function for drop2, instead of re-using bc_drop().
static st_err_t bc_drop2(TransStack&, SIO*, string&) { return ST_ERR_OK; }

static st_err_t bc_rot(TransStack& ts, SIO *args, string&) {
	ts.transstack_push(args[1].si);
	args[1].ownership = TSO_OWNED_BY_TS;
	ts.transstack_push(args[2].si);
	args[2].ownership = TSO_OWNED_BY_TS;
	ts.transstack_push(args[0].si);
	args[0].ownership = TSO_OWNED_BY_TS;
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
		flags[n].value = value;
	return c;
}

static st_err_t get_flag_value(const StackItem& op, StackItem*& ret, const bool& value) {
	int n;
	st_err_t c = get_flag_number(op, n);
	if (c == ST_ERR_OK) {
		ret = new StackItemReal(Real(flags[n].value == value ? 1 : 0));
	}
	return c;
}

static st_err_t get_flag_value_and_update(const StackItem& op, StackItem*& ret, const bool& value) {
	int n;
	st_err_t c = get_flag_number(op, n);
	if (c == ST_ERR_OK) {
		ret = new StackItemReal(Real(flags[n].value == value ? 1 : 0));
		flags[n].value = false;
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
	} else if (c == ST_ERR_STR_TO_BAD_ARGUMENT_TYPE && flags[FL_LAST].value)
		ts.transstack_push(s.si);
	else
		s.cleanup();

	return c;
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
	for (int i = 0; i < n; i++)
		if (items[i].ownership == TSO_OUTSIDE_TS) {
			sil->add_item(items[i].si);
			items[i].ownership = TSO_OWNED_BY_TS;
		} else
			sil->add_item(items[i].si->dup());
	ts.transstack_push(sil);
	if (c == ST_ERR_EXIT)
		delete []items;
	return ST_ERR_OK;
}

static st_err_t bc_list_to(TransStack& ts, SIO *args, string&) { return args[0].si->op_list_to(ts, args[0].ownership); }

static st_err_t bc_get(StackItem& op1, StackItem& op2, StackItem*& ret, string&) { return op1.op_get_generic(op2, ret); }

static st_err_t bc_geti(TransStack& ts, SIO *args, string& cmd_err) {
	StackItem* ret;
	st_err_t c = bc_get(*(args[0].si), *(args[1].si), ret, cmd_err);
	if (c == ST_ERR_OK) {
		ts.transstack_push(args[0].si);
		args[1].ownership = TSO_OWNED_BY_TS;
		ts.transstack_push(args[1].si);
		args[1].ownership = TSO_OWNED_BY_TS;
		ts.transstack_push(ret);
	}
	return c;
}

  // Variables

static st_err_t bc_sto(TransStack& ts, SIO *args, string&) { return args[1].si->op_sto(ts, args[0]); }

static st_err_t bc_rcl(TransStack& ts, SIO *args, string&) { return args[0].si->op_rcl(ts); }

static st_err_t bc_purge(TransStack& ts, SIO *args, string&) { return args[0].si->op_purge(ts); }

static st_err_t bc_vars(TransStack& ts, SIO*, string&) {
	vector<string> *by_order;
	ts.vars.get_var_list(by_order);
	StackItemList *l = new StackItemList();
	for (vector<string>::iterator it = by_order->begin(); it != by_order->end(); it++) {
		l->add_item(new StackItemExpression(*it, true));
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

static void set_bin_base_flags(const bool& flag1, const bool& flag2) {
	flags[FL_BIN_BASE_2].value = flag1;
	flags[FL_BIN_BASE_2 + 1].value = flag2;
}

static st_err_t bc_bin(TransStack&, SIO*, string&) {
	set_bin_base_flags(false, true);
	return ST_ERR_OK;
}

static st_err_t bc_oct(TransStack&, SIO*, string&) {
	set_bin_base_flags(true, false);
	return ST_ERR_OK;
}

static st_err_t bc_dec(TransStack&, SIO*, string&) {
	set_bin_base_flags(false, false);
	return ST_ERR_OK;
}

static st_err_t bc_hex(TransStack&, SIO*, string&) {
	set_bin_base_flags(true, true);
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
	n--;
	for (int i = FL_BIN_SIZE_6; i <= FL_BIN_SIZE_6 + 5; i++) {
		flags[i].value = (n % 2 != 0);
		n = n / 2;
	}
	return ST_ERR_OK;
}

static st_err_t bc_rcws(StackItem*& si, string&) {
	si = new StackItemReal(Real(get_bin_size_from_flags()));
	return ST_ERR_OK;
}

static st_err_t bc_b_to_r(StackItem& op1, StackItem*& ret, string&) { return op1.op_b_to_r(ret); }
static st_err_t bc_r_to_b(StackItem& op1, StackItem*& ret, string&) { return op1.op_r_to_b(ret); }

static st_err_t lowlevel_bc_rclf(StackItem*& si, const int& nb_bits) {
	bitset<G_HARD_MAX_NB_BITS> bits;

	for (int p = 0; p < nb_bits; p++)
		bits[p] = flags[p + 1].value;

	si = new StackItemBinary(bits);
	return ST_ERR_OK;
}

static st_err_t bc_rclf(StackItem*& si, string&) {
	return lowlevel_bc_rclf(si, get_bin_size_from_flags());
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

  // Misc

static st_err_t bc_read(TransStack& ts, SIO *args, string&) { return args[0].si->op_read(ts); }

static st_err_t bc_write(StackItem& op1, StackItem& op2, StackItem*& ret, string&) { return op2.op_write(op1); }

static st_err_t bc_undo(TransStack& ts, SIO*, string&) {
	if (!ts.get_modified_flag())
		ts.backward_head();
	ts.backward_head();
	return ST_ERR_OK;
}

static st_err_t bc_undo_levels(StackItem& op1, StackItem*&, string&) {
	int n;
	st_err_t c = op1.to_integer(n);
	if (c != ST_ERR_OK)
		return c;
	if (n < -1)
		n = -1;
	if (HARD_MAX_UNDO_LEVELS >= 0 && n > HARD_MAX_UNDO_LEVELS)
		n = HARD_MAX_UNDO_LEVELS;
	undo_levels = n;
	return ST_ERR_OK;
}

static st_err_t bc_undo_levels_get(StackItem*& si, string&) {
	si = new StackItemReal(Real(undo_levels));
	return ST_ERR_OK;
}

  // Must be here for the sake of avoiding declarations of bc_ functions above
#include "Commands.h"

st_err_t StackItemBuiltinCommand::eval(const eval_t&, TransStack& ts, manage_si_t& msi, string& cmd_err) {
	BuiltinCommandDescriptor& bc = builtinCommands[builtin_id];
	msi = MANAGE_SI_DONOTHING;

	cmd_err = bc.command;

	if (ts.stack_get_count() < bc.nb_args)
		return ST_ERR_TOO_FEW_ARGUMENTS;

	SIO *args;
	if (bc.nb_args >= 1)
		args = new SIO[bc.nb_args];
	else
		args = NULL;

	debug_write_i("nb_args = %i", bc.nb_args);

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

	if (resp != ST_ERR_OK && resp != ST_ERR_EXIT && flags[FL_LAST].value) {
		for (size_t i = 0; i < bc.nb_args; i++)
			ts.transstack_push(args[i].si);
	} else
		for (int i = bc.nb_args - 1; i >= 0; i--)
			args[i].cleanup();

	if (bc.type == BC_FUNCTION_WRAPPER || bc.type == BC_COMMAND_WRAPPER)
		if (resp != ST_ERR_OK && resp != ST_ERR_EXIT && flags[FL_LAST].value)
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
	int b = (tostr.get_type() == TOSTRING_PORTABLE ? DEFAULT_PORTABLE_BIN_BASE : get_base_from_flags());
	tostr.add_string("# "  + bin.to_string(b, get_bin_size_from_flags()) + base_int_to_letter(b));
}

st_err_t StackItemBinary::op_b_to_r(StackItem*& ret) {
	ret = new StackItemReal(Real(bin.to_real(get_bin_size_from_flags())));
	return ST_ERR_OK;
}

st_err_t StackItemBinary::op_stof(TransStack&) {
	for (int p = 0; p < get_bin_size_from_flags() && p + 1 <= FL_TAG_IT_END; p++)
		flags[p + 1].value = bin.get_bit(p);
	for (int p = get_bin_size_from_flags(); p + 1 <= FL_TAG_IT_END; p++)
		flags[p + 1].value = false;
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
	st_err_t c = Scalar##_mul(sc, sc, res); \
	if (c == ST_ERR_OK) \
		ret = new SI(Scalar(res)); \
	return c; \
}
IMPLEMENT_SCALAR_SQ(Real, StackItemReal)
IMPLEMENT_SCALAR_SQ(Cplx, StackItemCplx)

#define IMPLEMENT_INV(Scalar, SI, Unit) \
st_err_t SI::op_inv(StackItem*& ret) { \
	Scalar res; \
	st_err_t c = Scalar##_div(Unit, sc, res); \
	if (c == ST_ERR_OK) \
		ret = new SI(res); \
	return c; \
}
IMPLEMENT_INV(Real, StackItemReal, Real(1))
IMPLEMENT_INV(Cplx, StackItemCplx, Cplx(1, 0))

template<class Scalar, class SI> st_err_t si_arith
		(st_err_t (*f)(const Scalar&, const Scalar&, Scalar&), const Scalar& lv, const Scalar& rv, StackItem*& ret) {
	Scalar res;
	st_err_t c = (*f)(lv, rv, res);
	if (c == ST_ERR_OK)
		ret = new SI(res);
	return c;
}

template<class Scalar, class SI_Mat> st_err_t si_matrix_md(st_err_t (*f)(const Scalar&, const Scalar&, Scalar&), Matrix<Scalar> *lv, const Scalar& rv, StackItem*& ret) {
	Matrix<Scalar> *mres = new Matrix<Scalar>(*lv);
	st_err_t c = mres->md(f, rv);
	if (c == ST_ERR_OK)
		ret = new SI_Mat(mres);
	else
		delete mres;
	return c;
}

Matrix<Cplx> *matrix_real_to_cplx(Matrix<Real> *m) {
	Matrix<Cplx> *mres = new Matrix<Cplx>(m->get_dimension(), m->get_nb_lines(), m->get_nb_columns());
	for (size_t i = 0; i < m->get_nb_lines(); i++)
		for (size_t j = 0; j < m->get_nb_columns(); j++)
			mres->set_value(i, j, Cplx(m->get_value(i, j)));
	return mres;
}

st_err_t si_matrix_md2(st_err_t (*f)(const Cplx&, const Cplx&, Cplx&), Matrix<Real> *lv, const Cplx& rv, StackItem*& ret) {
	Matrix<Cplx> *mres = matrix_real_to_cplx(lv);
	st_err_t c = mres->md(f, rv);
	if (c == ST_ERR_OK)
		ret = new StackItemMatrixCplx(mres);
	else
		delete mres;
	return c;
}

template<class Scalar, class SI_Mat> st_err_t si_matrix_as(st_err_t (*f)(const Scalar&, const Scalar&, Scalar&), Matrix<Scalar> *lv, Matrix<Scalar> *rv, StackItem*& ret) {
	Matrix<Scalar> *mres = new Matrix<Scalar>(*lv);
	st_err_t c = mres->as(f, *rv);
	if (c == ST_ERR_OK)
		ret = new SI_Mat(mres);
	else
		delete mres;
	return c;
}

st_err_t si_matrix_as2(st_err_t (*f)(const Cplx&, const Cplx&, Cplx&), Matrix<Real> *lv, Matrix<Cplx> *rv, StackItem*& ret) {
	Matrix<Cplx> *mres = matrix_real_to_cplx(lv);
	st_err_t c = mres->as(f, *rv);
	if (c == ST_ERR_OK)
		ret = new StackItemMatrixCplx(mres);
	else
		delete mres;
	return c;
}

st_err_t si_matrix_as3(st_err_t (*f)(const Cplx&, const Cplx&, Cplx&), Matrix<Cplx> *lv, Matrix<Real> *rv, StackItem*& ret) {
	Matrix<Cplx> *new_rv = matrix_real_to_cplx(rv);
	st_err_t c = si_matrix_as<Cplx, StackItemMatrixCplx>(f, lv, new_rv, ret);
	delete new_rv;
	return c;
}


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

st_err_t StackItemReal::op_ip(StackItem*& ret) {
	ret = new StackItemReal(Real(real_ip(sc.get_value())));
	return ST_ERR_OK;
}

#define IMPLEMENT_REAL_FUNCTION(op, numeric_op)	\
st_err_t StackItemReal::op(StackItem*& ret) { \
	st_err_t c = ST_ERR_OK; \
	real res; \
	numeric_op(sc.get_value(), c, res); \
	if (c == ST_ERR_OK) \
		ret = new StackItemReal(Real(res)); \
	return c; \
}

IMPLEMENT_REAL_FUNCTION(op_cos, numeric_cos)
IMPLEMENT_REAL_FUNCTION(op_sin, numeric_sin)
IMPLEMENT_REAL_FUNCTION(op_tan, numeric_tan)
IMPLEMENT_REAL_FUNCTION(op_acos, numeric_acos)
IMPLEMENT_REAL_FUNCTION(op_asin, numeric_asin)
IMPLEMENT_REAL_FUNCTION(op_atan, numeric_atan)
IMPLEMENT_REAL_FUNCTION(op_ln, numeric_ln)
IMPLEMENT_REAL_FUNCTION(op_exp, numeric_exp)

st_err_t StackItemReal::op_sqr(StackItem*& ret) {
	real tmp_r = numeric_abs(sc.get_value());
	st_err_t c = ST_ERR_OK;
	real res;
	numeric_sqrt(tmp_r, c, res);
	if (c == ST_ERR_OK) {
		if (sc.get_value() < 0)
			ret = new StackItemCplx(Cplx(0, res));
		else
			ret = new StackItemReal(Real(res));
	}
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

st_err_t StackItemReal::op_r_to_b(StackItem*& ret) {
	real r = real_ip(sc.get_value() + .5);
	bitset<G_HARD_MAX_NB_BITS> bits;
	int max = get_bin_size_from_flags();

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

st_err_t StackItemCplx::op_c_to_r(TransStack& ts) {
	ts.transstack_push(new StackItemReal(Real(sc.get_re())));
	ts.transstack_push(new StackItemReal(Real(sc.get_im())));
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
	for (size_t i = 0; i < pmat->get_nb_lines(); i++) {
		if (i == 0)
			s.append("[ ");
		else
			s.append(" [ ");
		for (size_t j = 0; j < pmat->get_nb_columns(); j++) {
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


//
// StackItemMatrixReal
//

StackItemMatrixReal::StackItemMatrixReal(const mat_read_t *pm, const dim_t& d, const size_t& l, const size_t& c)
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


//
// StackItemMatrixCplx
//

StackItemMatrixCplx::StackItemMatrixCplx(const mat_read_t *pm, const dim_t& d, const size_t& l, const size_t& c)
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

st_err_t read_rc_file(TransStack*, const tostring_t&, const std::string&, const std::string&, const bool&, std::string&, std::string&, const int& = -1);
st_err_t StackItemString::op_read(TransStack& ts) {
	string error_l1, error_l2;
	st_err_t c = read_rc_file(&ts, TOSTRING_PORTABLE, s, s, false, error_l1, error_l2, 1);
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
	if (arg1.ownership == TSO_OWNED_BY_TS)
		si = si->dup();
	else
		arg1.ownership = TSO_OWNED_BY_TS;
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

StackItemMeta::StackItemMeta(const std::vector<StackItem*>& sim) : list(sim) {
	copy_items();
}

void StackItemMeta::copy_items() {
	for (size_t i = 0; i < list.size(); i++)
		list[i] = list[i]->dup();
}

void StackItemMeta::add_item(StackItem *si) {
	list.push_back(si);
}

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
	for (size_t i = 0; i < list.size(); i++)
		if (o == TSO_OUTSIDE_TS)
			ts.transstack_push(list[i]);
		else
			ts.transstack_push(list[i]->dup());
	ts.transstack_push(new StackItemReal(Real(list.size())));
	if (o == TSO_OUTSIDE_TS)
		list.clear();
	return ST_ERR_OK;
}

st_err_t StackItemList::get_coordinates(dim_t& d, int& x, int& y) {
	size_t nb = get_nb_items();
	if (nb < 1 || nb > 2)
		return ST_ERR_BAD_ARGUMENT_VALUE;
	st_err_t c;
	int value;
	for (int i = 0; i < nb; i++) {
		c = list[i]->to_integer(value);
		if (c != ST_ERR_OK)
			break;
		if (i == 0)
			x = value;
		else if (i == 1)
			y = value;
		else
			throw(CalcFatal(__FILE__, __LINE__, "StackItemList::get_coordinates(): inconsistent values encountered!!!??"));
	}
	d = (nb == 1 ? DIM_VECTOR : DIM_MATRIX);
	return ST_ERR_OK;
}

st_err_t StackItemList::op_get(StackItemList* sil, StackItem*& ret) {
	dim_t d;
	int x; int y;
	st_err_t c = get_coordinates(d, x, y);
	if (c == ST_ERR_OK && d == DIM_MATRIX)
		return ST_ERR_BAD_ARGUMENT_VALUE;
	if (x < 1 || x > sil->get_nb_items())
		return ST_ERR_BAD_ARGUMENT_VALUE;
	ret = sil->list[x - 1]->dup();
	return ST_ERR_OK;
}

st_err_t StackItemList::op_geti(StackItemList* sil, TransStack& ts) {
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
		sis[s]->add_item(si);
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

