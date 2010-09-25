// Stack.h
// Stack implementation and StackItems

// RPN calculator

// Sébastien Millet
// August-September 2009

#ifndef STACK_H
#define STACK_H

#include "Common.h"
#include "Scalar.h"
#include <string>
#include <fstream>

class DisplayStackLayout;
struct IntervalShift;

  //
  // If STACK_USE_MAP is defined, use a map object to keep StackItem objets, otherwise use a simple flat array
  // (simpler but less efficient)
  // Measures done by repeating 2^n DUPN commands, followed by CLEAR.
  // During these measures, the stack and nodestack objects were using flat arrays, not vectors, with an
  // alloc step equal to 3 in each.
  // 	524288 items
  //	    with map: 368.29 seconds user time (~ 6 minutes)
  //	    without: 3347.78 seconds user time (~ 55 minutes)
  //	262144 items
  //	    with map:  92.55 seconds user time
  //	    without:  845.88 seconds user time (~ 14 minutes)
  // 	32768 items (
  // 		with map:   2.37 seconds user time
  // 		without:   11.82 seconds user time
  //	8192 items
  //		with map:   0.44 seconds user time
  //		without:    1.47 seconds user time
  //	1024 items, repeated 200 times (to get something measurable)
  //		with map:   2.85 seconds user time
  //		without:    3.75 seconds user time
  //
  // One test done with use of MAP and VECTORS: 33.79 second user time for up to 2097152 items in the stack
  //

  // I brief: you should leave STACK_USE_MAP defined
#define STACK_USE_MAP

#ifdef STACK_USE_MAP
#include <map>
#endif

  // Leave USE_VECTOR defined
#define USE_VECTOR
#ifdef USE_VECTOR
#include <vector>
#endif

extern int g_min_int;
extern int g_max_int;
extern bool opt_batch;
extern std::string branch_str[];

extern int g_display_width;
extern int g_display_stack_min_lines;
extern int g_display_stack_max_lines;

int cfg_get_undo_levels();

typedef enum {
	BC_FUNCTION_WRAPPER,	// Returns a StackItem*, example: +, COS
	BC_COMMAND_WRAPPER,		// Same as above, but doesn't return a value, example: SF, CF
	BC_RAW					// Cannot be seen as a function, example: SWAP, DROP
} st_builtin_command_t;

typedef enum {TSO_OWNED_BY_TS, TSO_OUTSIDE_TS, TSO_FROZEN} tso_t;

typedef enum {SITYPE_REAL, SITYPE_COMPLEX, SITYPE_STRING, SITYPE_MATRIX_REAL, SITYPE_MATRIX_COMPLEX, SITYPE_LIST,
	SITYPE_NAME, SITYPE_LOCAL_NAME, SITYPE_PROGRAM, SITYPE_EXPRESSION, SITYPE_BINARY_INTEGER, SITYPE_INTERNAL_META,
	SITYPE_INTERNAL_BUILTIN_COMMAND, SITYPE_INTERNAL_BRANCH, SITYPE_INTERNAL_BRANCH_IF,
	SITYPE_INTERNAL_BRANCH_WHILE, SITYPE_INTERNAL_BRANCH_DO, SITYPE_INTERNAL_BRANCH_LOOP} sitype_t;

typedef enum {EVAL_SOFT, EVAL_HARD} eval_t;

typedef enum {MANAGE_SI_DONOTHING, MANAGE_SI_PUSH, MANAGE_SI_EXEC} manage_si_t;

class Stack;
class TransStack;
class StackItem;
class StackItemBinary;
class StackItemReal;
class StackItemCplx;
class StackItemMatrixReal;
class StackItemMatrixCplx;
class StackItemMeta;
class StackItemList;
class StackItemProgram;
class StackItemString;
class StackItemExpression;
struct SIO;

class Var;
class VarFile;
class VarDirectory;
class Tree;

const std::string simple_string(StackItem*);
int get_base_from_flags();
const std::string base_int_to_letter(const int&);

const std::string get_error_string(const st_err_t&);

const std::string get_rclf_portable_string();

void write_si(const tostring_t&, const StackItem*, std::ostream&);


//
// ToString
//

class ToString {

#ifdef DEBUG_CLASS_COUNT
	static long class_count;
#endif

	std::vector<std::string> v;
	bool locked;
	int actual_index;
	tostring_t type;
	size_t max_height;
	size_t max_width;
	const char *get_line();

public:
	ToString(const tostring_t&, const size_t&, const size_t&);
	virtual ~ToString();

#ifdef DEBUG_CLASS_COUNT
	static int get_class_count() { return class_count; }
#endif

	virtual void reset(const tostring_t&, const size_t&, const size_t&);
	virtual bool add_string(const std::string&, const bool& = false);
	virtual size_t get_max_height() const { return max_height; }
	virtual tostring_t get_type() const { return type; }
	virtual const std::vector<std::string>* get_value() const { return &v; }
	virtual void lock();
	virtual const char *get_first_line();
	virtual const char *get_next_line();
	virtual void unlock();
};


//
// StackItem
//

class StackItem {

#ifdef DEBUG_CLASS_COUNT
	static long class_count;
#endif
#ifdef DEBUG_TRANSSTACK
	static int class_serial;
	int object_serial;
#endif

	static std::string empty_string;

public:
	StackItem();
	virtual ~StackItem();

#ifdef DEBUG_CLASS_COUNT
	static int get_class_count() { return class_count; }
#endif
#ifdef DEBUG_TRANSSTACK
	int get_object_serial() const { return object_serial; }
#endif

	static StackItem* forge_list_size(const Coordinates&);

	virtual st_err_t eval(const eval_t&, TransStack&, manage_si_t&, std::string&);
	virtual VarDirectory* get_lvars(bool&) const;
	virtual void clear_lvars();
	virtual StackItem* dup() const = 0;
	virtual sitype_t get_type() const = 0;
	virtual bool same(StackItem*) const = 0;
	virtual st_err_t op_equal_generic(StackItem&, StackItem*&);
	virtual st_err_t op_notequal_generic(StackItem&, StackItem*&);
	virtual st_err_t op_equal(StackItemExpression*, StackItem*&);
	virtual st_err_t op_notequal(StackItemExpression*, StackItem*&);

	virtual bool is_varname() const;
	virtual bool is_unquoted_varname() const;

	virtual st_err_t get_bounds(Coordinates&) const { return ST_ERR_BAD_ARGUMENT_TYPE; }

	virtual const std::string& get_varname() const;

	virtual void to_string(ToString&, const bool& = false) const = 0;
	virtual st_err_t to_integer(int&) const { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_ip(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }

	virtual st_err_t op_add_generic(StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_sub_generic(StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_mul_generic(StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_div_generic(StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_pow_generic(StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_cos(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_sin(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_tan(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_acos(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_asin(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_atan(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_ln(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_exp(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_neg(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_sq(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_inv(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_sqr(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_r_to_c_generic(StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_c_to_r(TransStack&) { return ST_ERR_BAD_ARGUMENT_TYPE; }

	  // REAL
	virtual st_err_t op_add(StackItemReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_sub(StackItemReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_mul(StackItemReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_div(StackItemReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_pow(StackItemReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_r_to_c(StackItemReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	  // COMPLEX
	virtual st_err_t op_add(StackItemCplx*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_sub(StackItemCplx*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_mul(StackItemCplx*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_div(StackItemCplx*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	  // MATRIX
	virtual st_err_t op_mul(StackItemMatrixReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_mul(StackItemMatrixCplx*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_div(StackItemMatrixReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_div(StackItemMatrixCplx*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_add(StackItemMatrixReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_add(StackItemMatrixCplx*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_sub(StackItemMatrixReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_sub(StackItemMatrixCplx*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	  // Comparisons
	virtual st_err_t op_lower_generic(StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_lower_or_equal_generic(StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_greater_generic(StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_greater_or_equal_generic(StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_lower(StackItemReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_lower_or_equal(StackItemReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_greater(StackItemReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_greater_or_equal(StackItemReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_lower(StackItemBinary*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_lower_or_equal(StackItemBinary*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_greater(StackItemBinary*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_greater_or_equal(StackItemBinary*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_lower(StackItemString*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_lower_or_equal(StackItemString*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_greater(StackItemString*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_greater_or_equal(StackItemString*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }

	  // LIST AND ARRAYS
	virtual st_err_t op_list_to(TransStack&, const tso_t&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_get_generic(TransStack&, StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_get(TransStack&, StackItemList*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_get(TransStack&, StackItemMatrixReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_get(TransStack&, StackItemMatrixCplx*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_put_generic(TransStack&, StackItem&, SIO&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_put(TransStack&, StackItemList*, SIO&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_put(TransStack&, StackItemMatrixReal*, SIO&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_put(TransStack&, StackItemMatrixCplx*, SIO&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_put_matrix(TransStack&, StackItemList*, StackItemMatrixReal*) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_put_matrix(TransStack&, StackItemList*, StackItemMatrixCplx*) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_size(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_arry_to(TransStack&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_to_arry(TransStack&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_con_generic(TransStack&, StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_con(TransStack&, StackItemList*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_con(TransStack&, StackItemMatrixReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_con(TransStack&, StackItemMatrixCplx*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_rdm_generic(TransStack&, StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_rdm(TransStack&, StackItemMatrixReal*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_rdm(TransStack&, StackItemMatrixCplx*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_idn(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_trn(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	  // VARIABLES
	virtual st_err_t op_sto(TransStack&, SIO&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_rcl(TransStack&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_purge(TransStack&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_crdir(TransStack&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	  // STRING
	virtual st_err_t op_str_to(TransStack&, std::string&) { return ST_ERR_STR_TO_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_add(StackItemString*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_cmdsub(const int&, const int&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	  // BINARIES
	virtual st_err_t op_b_to_r(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_r_to_b(StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_stof(TransStack&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_add(StackItemBinary*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_sub(StackItemBinary*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_mul(StackItemBinary*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_div(StackItemBinary*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	  // _READ, _WRITE
	virtual st_err_t op_read(TransStack&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_write(StackItem&) { return ST_ERR_BAD_ARGUMENT_TYPE; }

	virtual st_err_t op_disp(StackItem&) { return ST_ERR_BAD_ARGUMENT_TYPE; }

	  // Program execution
	virtual st_err_t exec1(TransStack&, int&, std::string&);

	virtual st_err_t op_add(StackItemList*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_add(StackItemExpression*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
	virtual st_err_t op_add(StackItemProgram*, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }
};


//
// StackItemBuiltinCommand
//

struct BuiltinCommandDescriptor {
	size_t nb_args;
	st_builtin_command_t type;
	const char *command;
	st_err_t (*functionS)(TransStack&, SIO*, std::string&);
	st_err_t (*function0)(StackItem*&, std::string&);
	st_err_t (*function1)(StackItem&, StackItem*&, std::string&);
	st_err_t (*function2)(StackItem&, StackItem&, StackItem*&, std::string&);
	st_err_t (*function3)(StackItem&, StackItem&, StackItem&, StackItem*&, std::string&);
	const char *short_description;
};

class StackItemBuiltinCommand : public StackItem {
	int builtin_id;
public:
	StackItemBuiltinCommand(const int& id) : builtin_id(id) { }
	virtual ~StackItemBuiltinCommand() { }
	virtual int get_id() const { return builtin_id; }
	virtual st_err_t eval(const eval_t&, TransStack&, manage_si_t&, std::string&);
	virtual StackItem* dup() const;
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;

	virtual void to_string(ToString&, const bool& = false) const;
};


//
// StackItemBinary
//

st_err_t bin_arith(st_err_t (*f)(const Binary&, const Binary&, Binary&), const Binary&, const Binary&, StackItem*&);

class StackItemBinary : public StackItem {
	Binary bin;
public:
	StackItemBinary(const Binary&);
	virtual ~StackItemBinary();
	virtual StackItem* dup() const;
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;
	virtual void to_string(ToString&, const bool& = false) const;

	virtual st_err_t op_b_to_r(StackItem*&);
	virtual st_err_t op_stof(TransStack&);

	virtual st_err_t op_add_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_add(this, ret); }
	virtual st_err_t op_sub_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_sub(this, ret); }

	virtual st_err_t op_add(StackItemBinary* arg1, StackItem*& ret) { return bin_arith(Binary_add, arg1->bin, bin, ret); }
	virtual st_err_t op_sub(StackItemBinary* arg1, StackItem*& ret) { return bin_arith(Binary_sub, arg1->bin, bin, ret); }

	virtual st_err_t op_lower_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_lower(this, ret); }
	virtual st_err_t op_lower_or_equal_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_lower_or_equal(this, ret); }
	virtual st_err_t op_greater_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_greater(this, ret); }
	virtual st_err_t op_greater_or_equal_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_greater_or_equal(this, ret); }
	virtual st_err_t op_lower(StackItemBinary*, StackItem*&);
	virtual st_err_t op_lower_or_equal(StackItemBinary*, StackItem*&);
	virtual st_err_t op_greater(StackItemBinary*, StackItem*&);
	virtual st_err_t op_greater_or_equal(StackItemBinary*, StackItem*&);

	virtual st_err_t op_add(StackItemList*, StackItem*&);
};


//
// StackItemReal and StackItemCplx
//

template<class Scalar, class SI> st_err_t si_arith(st_err_t (*f)(const Scalar&, const Scalar&, Scalar&), const Scalar&, const Scalar&, StackItem*&);
template<class Scalar, class SI_Mat> st_err_t si_matrix_md(st_err_t (*f)(const Scalar&, const Scalar&, Scalar&), Matrix<Scalar>*, const Scalar&, StackItem*&);

//
// StackItemReal
//

class StackItemReal : public StackItem {
	Real sc;
public:
	StackItemReal(const Real& cc) : sc(real_trim(cc.get_value())) { }
	virtual ~StackItemReal() { }
	virtual StackItem* dup() const;
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;

	virtual Real get_Real() const;
	virtual void set_Real(const Real&);

	virtual void to_string(ToString&, const bool& = false) const;
	virtual st_err_t to_integer(int&) const;
	virtual st_err_t op_ip(StackItem*&);

	virtual st_err_t op_add_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_add(this, ret); }
	virtual st_err_t op_sub_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_sub(this, ret); }
	virtual st_err_t op_mul_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_mul(this, ret); }
	virtual st_err_t op_div_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_div(this, ret); }

	virtual st_err_t op_pow_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_pow(this, ret); }
	virtual st_err_t op_r_to_c_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_r_to_c(this, ret); }

	virtual st_err_t op_add(StackItemReal* arg1, StackItem*& ret) { return si_arith<Real, StackItemReal>(Real_add, arg1->sc, sc, ret); }
	virtual st_err_t op_sub(StackItemReal* arg1, StackItem*& ret) { return si_arith<Real, StackItemReal>(Real_sub, arg1->sc, sc, ret); }
	virtual st_err_t op_mul(StackItemReal* arg1, StackItem*& ret) { return si_arith<Real, StackItemReal>(Real_mul, arg1->sc, sc, ret); }
	virtual st_err_t op_div(StackItemReal* arg1, StackItem*& ret) { return si_arith<Real, StackItemReal>(Real_div, arg1->sc, sc, ret); }
	virtual st_err_t op_add(StackItemCplx*, StackItem*&);
	virtual st_err_t op_sub(StackItemCplx*, StackItem*&);
	virtual st_err_t op_mul(StackItemCplx*, StackItem*&);
	virtual st_err_t op_div(StackItemCplx*, StackItem*&);
	virtual st_err_t op_pow(StackItemReal* arg1, StackItem*& ret) { return si_arith<Real, StackItemReal>(Real_pow, arg1->sc, sc, ret); }

	virtual st_err_t op_r_to_c(StackItemReal*, StackItem*&);

	virtual st_err_t op_mul(StackItemMatrixReal*, StackItem*&);
	virtual st_err_t op_div(StackItemMatrixReal*, StackItem*&);
	virtual st_err_t op_mul(StackItemMatrixCplx*, StackItem*&);
	virtual st_err_t op_div(StackItemMatrixCplx*, StackItem*&);

	virtual st_err_t op_cos(StackItem*&);
	virtual st_err_t op_sin(StackItem*&);
	virtual st_err_t op_tan(StackItem*&);
	virtual st_err_t op_acos(StackItem*&);
	virtual st_err_t op_asin(StackItem*&);
	virtual st_err_t op_atan(StackItem*&);
	virtual st_err_t op_ln(StackItem*&);
	virtual st_err_t op_exp(StackItem*&);
	virtual st_err_t op_neg(StackItem*&);
	virtual st_err_t op_sq(StackItem*&);
	virtual st_err_t op_inv(StackItem*&);
	virtual st_err_t op_sqr(StackItem*&);

	virtual st_err_t op_lower_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_lower(this, ret); }
	virtual st_err_t op_lower_or_equal_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_lower_or_equal(this, ret); }
	virtual st_err_t op_greater_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_greater(this, ret); }
	virtual st_err_t op_greater_or_equal_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_greater_or_equal(this, ret); }
	virtual st_err_t op_lower(StackItemReal*, StackItem*&);
	virtual st_err_t op_lower_or_equal(StackItemReal*, StackItem*&);
	virtual st_err_t op_greater(StackItemReal*, StackItem*&);
	virtual st_err_t op_greater_or_equal(StackItemReal*, StackItem*&);

	virtual st_err_t op_r_to_b(StackItem*&);

	virtual st_err_t op_put_matrix(TransStack&, StackItemList*, StackItemMatrixReal*);
	virtual st_err_t op_put_matrix(TransStack&, StackItemList*, StackItemMatrixCplx*);

	virtual st_err_t op_add(StackItemList*, StackItem*&);

	virtual st_err_t op_con(TransStack&, StackItemList*, StackItem*&);
	virtual st_err_t op_con(TransStack&, StackItemMatrixReal*, StackItem*&);
	virtual st_err_t op_con(TransStack&, StackItemMatrixCplx*, StackItem*&);
	virtual st_err_t op_idn(StackItem*&);

	virtual st_err_t op_disp(StackItem&);
};


//
// StackItemCplx
//

st_err_t si_cplx_arith(st_err_t (*f)(const Cplx&, const Cplx&, Cplx&), const Cplx&, const Cplx&, StackItem*&);

class StackItemCplx : public StackItem {
	Cplx sc;
public:
	StackItemCplx(const Cplx& cc) : sc(Cplx(real_trim(cc.get_re()), real_trim(cc.get_im()))) { }
	virtual ~StackItemCplx() { }
	virtual StackItem* dup() const;
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;

	virtual Cplx get_Cplx() { return sc; }

	virtual void to_string(ToString&, const bool& = false) const;

	virtual st_err_t op_c_to_r(TransStack&);

	virtual st_err_t op_add_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_add(this, ret); }
	virtual st_err_t op_sub_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_sub(this, ret); }
	virtual st_err_t op_mul_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_mul(this, ret); }
	virtual st_err_t op_div_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_div(this, ret); }

	virtual st_err_t op_add(StackItemCplx* arg1, StackItem*& ret) { return si_arith<Cplx, StackItemCplx>(Cplx_add, arg1->sc, sc, ret); }
	virtual st_err_t op_sub(StackItemCplx* arg1, StackItem*& ret) { return si_arith<Cplx, StackItemCplx>(Cplx_sub, arg1->sc, sc, ret); }
	virtual st_err_t op_mul(StackItemCplx* arg1, StackItem*& ret) { return si_arith<Cplx, StackItemCplx>(Cplx_mul, arg1->sc, sc, ret); }
	virtual st_err_t op_div(StackItemCplx* arg1, StackItem*& ret) { return si_arith<Cplx, StackItemCplx>(Cplx_div, arg1->sc, sc, ret); }
	virtual st_err_t op_add(StackItemReal* arg1, StackItem*& ret) {
		return si_arith<Cplx, StackItemCplx>(Cplx_add, Cplx(arg1->get_Real()), sc, ret);
	}
	virtual st_err_t op_sub(StackItemReal* arg1, StackItem*& ret) {
		return si_arith<Cplx, StackItemCplx>(Cplx_sub, Cplx(arg1->get_Real()), sc, ret);
	}
	virtual st_err_t op_mul(StackItemReal* arg1, StackItem*& ret) {
		return si_arith<Cplx, StackItemCplx>(Cplx_mul, Cplx(arg1->get_Real()), sc, ret);
	}
	virtual st_err_t op_div(StackItemReal* arg1, StackItem*& ret) {
		return si_arith<Cplx, StackItemCplx>(Cplx_div, Cplx(arg1->get_Real()), sc, ret);
	}

	virtual st_err_t op_mul(StackItemMatrixCplx*, StackItem*&);
	virtual st_err_t op_div(StackItemMatrixCplx*, StackItem*&);
	virtual st_err_t op_mul(StackItemMatrixReal*, StackItem*&);
	virtual st_err_t op_div(StackItemMatrixReal*, StackItem*&);

	virtual st_err_t op_neg(StackItem*&);
	virtual st_err_t op_sq(StackItem*&);
	virtual st_err_t op_inv(StackItem*&);

	virtual st_err_t op_put_matrix(TransStack&, StackItemList*, StackItemMatrixCplx*);

	virtual st_err_t op_add(StackItemList*, StackItem*&);

	virtual st_err_t op_con(TransStack&, StackItemList*, StackItem*&);
	virtual st_err_t op_con(TransStack&, StackItemMatrixCplx*, StackItem*&);
};


#define COORD_TO_MATRIX_IJ(coord, i, j) { \
	if (coord.d == DIM_VECTOR) { \
		i = 0; \
		j = coord.i - 1; \
	} else { \
		i = coord.i - 1; \
		j = coord.j - 1; \
	} \
}

//
// StackItemMatrixReal
//

class StackItemMatrixReal : public StackItem {
	Matrix<Real>* pmat;
public:
	StackItemMatrixReal(const mat_read_t*, const dim_t&, const int&, const int&);
	StackItemMatrixReal(const StackItemMatrixReal&);
	StackItemMatrixReal(Matrix<Real>*);
	virtual ~StackItemMatrixReal() { delete pmat; };
	virtual StackItem* dup() const;
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;
	virtual Matrix<Real>* get_matrix() const;
	virtual st_err_t get_bounds(Coordinates&) const;
	Real get_value(const Coordinates& coord) const {
		int i, j;
		COORD_TO_MATRIX_IJ(coord, i, j);
		return pmat->get_value(i, j);
	}
	void set_value(const Coordinates& coord, const Real& r) {
		int i, j;
		COORD_TO_MATRIX_IJ(coord, i, j);
		pmat->set_value(i, j, r);
	}

	virtual void to_string(ToString&, const bool& = false) const;

	virtual st_err_t op_add_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_add(this, ret); }
	virtual st_err_t op_sub_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_sub(this, ret); }
	virtual st_err_t op_mul_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_mul(this, ret); }
	virtual st_err_t op_div_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_div(this, ret); }
	virtual st_err_t op_mul(StackItemReal*, StackItem*&);
	virtual st_err_t op_mul(StackItemCplx*, StackItem*&);
	virtual st_err_t op_add(StackItemMatrixReal*, StackItem*&);
	virtual st_err_t op_sub(StackItemMatrixReal*, StackItem*&);
	virtual st_err_t op_add(StackItemMatrixCplx*, StackItem*&);
	virtual st_err_t op_sub(StackItemMatrixCplx*, StackItem*&);

	virtual st_err_t op_get_generic(TransStack& ts, StackItem& arg2, StackItem*& ret) { return arg2.op_get(ts, this, ret); }
	virtual st_err_t op_put_generic(TransStack& ts, StackItem& arg2, SIO& s) { return arg2.op_put(ts, this, s); }
	virtual st_err_t op_size(StackItem*&);
	virtual st_err_t op_arry_to(TransStack&);
	virtual st_err_t op_rdm_generic(TransStack& ts, StackItem& arg2, StackItem*& ret) { return arg2.op_rdm(ts, this, ret); }
	virtual st_err_t op_con_generic(TransStack& ts, StackItem& arg2, StackItem*& ret) { return arg2.op_con(ts, this, ret); }

	virtual st_err_t op_add(StackItemList*, StackItem*&);
	virtual st_err_t op_trn(StackItem*&);
	virtual void rdm(const dim_t&, const int&, const int&);
};


//
// StackItemMatrixCplx
//


class StackItemMatrixCplx : public StackItem {
	Matrix<Cplx>* pmat;
public:
	StackItemMatrixCplx(const mat_read_t*, const dim_t&, const int&, const int&);
	StackItemMatrixCplx(const StackItemMatrixCplx&);
	StackItemMatrixCplx(Matrix<Cplx>*);
	virtual ~StackItemMatrixCplx() { delete pmat; };
	virtual StackItem* dup() const;
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;
	virtual Matrix<Cplx>* get_matrix() const;
	virtual st_err_t get_bounds(Coordinates&) const;
	Cplx get_value(const Coordinates& coord) const {
		int i, j;
		COORD_TO_MATRIX_IJ(coord, i, j);
		return pmat->get_value(i, j);
	}
	void set_value(const Coordinates& coord, const Cplx& c) {
		int i, j;
		COORD_TO_MATRIX_IJ(coord, i, j);
		pmat->set_value(i, j, c);
	}

	virtual void to_string(ToString&, const bool& = false) const;

	virtual st_err_t op_add_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_add(this, ret); }
	virtual st_err_t op_sub_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_sub(this, ret); }
	virtual st_err_t op_mul_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_mul(this, ret); }
	virtual st_err_t op_div_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_div(this, ret); }
	virtual st_err_t op_mul(StackItemReal*, StackItem*&);
	virtual st_err_t op_mul(StackItemCplx*, StackItem*&);
	virtual st_err_t op_add(StackItemMatrixCplx*, StackItem*&);
	virtual st_err_t op_sub(StackItemMatrixCplx*, StackItem*&);
	virtual st_err_t op_add(StackItemMatrixReal*, StackItem*&);
	virtual st_err_t op_sub(StackItemMatrixReal*, StackItem*&);

	virtual st_err_t op_get_generic(TransStack& ts, StackItem& arg2, StackItem*& ret) { return arg2.op_get(ts, this, ret); }
	virtual st_err_t op_put_generic(TransStack& ts, StackItem& arg2, SIO& s) { return arg2.op_put(ts, this, s); }
	virtual st_err_t op_size(StackItem*&);
	virtual st_err_t op_arry_to(TransStack&);
	virtual st_err_t op_rdm_generic(TransStack& ts, StackItem& arg2, StackItem*& ret) { return arg2.op_rdm(ts, this, ret); }
	virtual st_err_t op_con_generic(TransStack& ts, StackItem& arg2, StackItem*& ret) { return arg2.op_con(ts, this, ret); }

	virtual st_err_t op_add(StackItemList*, StackItem*&);
	virtual st_err_t op_trn(StackItem*&);
	virtual void rdm(const dim_t&, const int&, const int&);
};


//
// StackItemString
//

class StackItemString : public StackItem {
	std::string s;
public:
	StackItemString(const std::string& ss) : s(ss) {
		if (s.length() >= 2 && s.at(0) == '\"' && s.at(s.length() - 1) == '\"') {
			s.erase(0, 1);
			s.erase(s.length() - 1);
		}
	}
	virtual ~StackItemString() { }
	virtual StackItem* dup() const;
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;

	virtual void to_string(ToString& tostr, const bool& = false) const;

	virtual st_err_t op_add_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_add(this, ret); }
	virtual st_err_t op_add(StackItemString*, StackItem*&);
	virtual st_err_t op_str_to(TransStack&, std::string&);

	virtual st_err_t op_lower_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_lower(this, ret); }
	virtual st_err_t op_lower_or_equal_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_lower_or_equal(this, ret); }
	virtual st_err_t op_greater_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_greater(this, ret); }
	virtual st_err_t op_greater_or_equal_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_greater_or_equal(this, ret); }
	virtual st_err_t op_lower(StackItemString*, StackItem*&);
	virtual st_err_t op_lower_or_equal(StackItemString*, StackItem*&);
	virtual st_err_t op_greater(StackItemString*, StackItem*&);
	virtual st_err_t op_greater_or_equal(StackItemString*, StackItem*&);

	virtual st_err_t op_read(TransStack&);
	virtual st_err_t op_write(StackItem&);

	virtual st_err_t op_add(StackItemList*, StackItem*&);
	virtual st_err_t op_size(StackItem*&);
	virtual st_err_t op_cmdsub(const int&, const int&, StackItem*&);
};


//
// StackItemExpression
//

class StackItemExpression : public StackItem {
	bool quoted;
	std::string e;
	StackItemExpression(const StackItemExpression&);
public:
	StackItemExpression(const std::string&, const bool&);
	virtual ~StackItemExpression() { }
	virtual StackItem* dup() const;
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;
	virtual st_err_t op_equal_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_equal(this, ret); }
	virtual st_err_t op_notequal_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_notequal(this, ret); }
	virtual st_err_t op_equal(StackItemExpression*, StackItem*&);
	virtual st_err_t op_notequal(StackItemExpression*, StackItem*&);

	virtual bool is_varname() const;
	virtual bool is_unquoted_varname() const;
	virtual const std::string& get_varname() const;
	virtual void to_string(ToString&, const bool& = false) const;

	virtual st_err_t op_sto(TransStack&, SIO&);
	virtual st_err_t op_rcl(TransStack&);
	virtual st_err_t op_purge(TransStack&);
	virtual st_err_t op_crdir(TransStack&);

	virtual st_err_t eval(const eval_t&, TransStack&, manage_si_t&, std::string&);

	virtual st_err_t op_add_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_add(this, ret); }

	virtual st_err_t op_add(StackItemList*, StackItem*&);
};


//
// StackItemMeta
//

class StackItemMeta : public StackItem {
protected:
	std::vector<StackItem*> list;
	StackItemMeta(const StackItemMeta&);
public:
	StackItemMeta() { }
	StackItemMeta(const std::vector<StackItem*>&);
	virtual ~StackItemMeta();
	void copy_items();
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*, const sitype_t&) const;

	virtual void insert_item(StackItem*);
	virtual void append_item(StackItem*);
	virtual size_t get_nb_items() const;
};


//
// StackItemList
//

class StackItemList : public StackItemMeta {
	StackItemList(const StackItemList&);
public:
	StackItemList() { }
	StackItemList(const std::vector<StackItem*>&);
	virtual ~StackItemList() { }
	virtual StackItem* dup() const;
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;

	virtual st_err_t get_bounds(Coordinates&) const;
	st_err_t increment_list(TransStack&, const Coordinates&);

	virtual st_err_t get_coordinates(TransStack&, Coordinates&);
	virtual void to_string(ToString&, const bool& = false) const;
	virtual st_err_t op_list_to(TransStack&, const tso_t&);
	virtual st_err_t prepare_list_access(TransStack&, StackItem*, Coordinates&);
	virtual st_err_t op_get_generic(TransStack& ts, StackItem& arg2, StackItem*& ret) { return arg2.op_get(ts, this, ret); }
	virtual st_err_t op_get(TransStack&, StackItemList*, StackItem*&);
	virtual st_err_t op_get(TransStack&, StackItemMatrixReal*, StackItem*&);
	virtual st_err_t op_get(TransStack&, StackItemMatrixCplx*, StackItem*&);
	virtual st_err_t op_put_generic(TransStack& ts, StackItem& arg2, SIO& s) { return arg2.op_put(ts, this, s); }
	virtual st_err_t op_put(TransStack&, StackItemList*, SIO&);
	virtual st_err_t op_put(TransStack&, StackItemMatrixReal*, SIO&);
	virtual st_err_t op_put(TransStack&, StackItemMatrixCplx*, SIO&);
	virtual st_err_t op_size(StackItem*&);
	virtual st_err_t op_to_arry(TransStack&);
	virtual st_err_t op_con_generic(TransStack& ts, StackItem& arg2, StackItem*& ret) { return arg2.op_con(ts, this, ret); }
	virtual st_err_t op_rdm(TransStack&, StackItemMatrixReal*, StackItem*&);
	virtual st_err_t op_rdm(TransStack&, StackItemMatrixCplx*, StackItem*&);

	virtual st_err_t list_add(const bool&, StackItem*, StackItem*&);
	virtual st_err_t op_add_generic(StackItem& arg2, StackItem*& ret) { return arg2.op_add(this, ret); }
	virtual st_err_t op_add(StackItemBinary*, StackItem*&);
	virtual st_err_t op_add(StackItemReal*, StackItem*&);
	virtual st_err_t op_add(StackItemCplx*, StackItem*&);
	virtual st_err_t op_add(StackItemMatrixReal*, StackItem*&);
	virtual st_err_t op_add(StackItemMatrixCplx*, StackItem*&);
	virtual st_err_t op_add(StackItemString*, StackItem*&);
	virtual st_err_t op_add(StackItemExpression*, StackItem*&);
	virtual st_err_t op_add(StackItemList*, StackItem*&);
	virtual st_err_t op_add(StackItemProgram*, StackItem*&);

	virtual st_err_t op_cmdsub(const int&, const int&, StackItem*&);
};


//
// StackItemProgram
//

class StackItemProgram : public StackItemMeta {
	bool brackets;
	VarDirectory* lvars;
public:
	StackItemProgram(const bool&, VarDirectory* const &);
	StackItemProgram(const StackItemProgram&);
	virtual ~StackItemProgram();
	virtual StackItem* dup() const;
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;
	virtual void to_string(ToString&, const bool& = false) const;
	virtual VarDirectory* get_lvars(bool&) const;
	virtual void clear_lvars();
	virtual st_err_t eval(const eval_t&, TransStack&, manage_si_t&, std::string&);
	virtual st_err_t exec1(TransStack&, int&, std::string&);

	virtual st_err_t op_add_generic(StackItem&, StackItem*&) { return ST_ERR_BAD_ARGUMENT_TYPE; }

	virtual st_err_t op_add(StackItemList*, StackItem*&);
};


//
// StackItemBranch
//

class StackItemBranch : public StackItem {
protected:
	int sis_size;
	StackItemProgram* *sis;
public:
	StackItemBranch(const int&);
	StackItemBranch(const StackItemBranch&);
	virtual ~StackItemBranch();

	  // The line below is useless, I put it just to remind StackItemBranch is abstract
	virtual StackItem* dup() const = 0;

	virtual void add_item(const int&, StackItem*);
	virtual st_err_t eval(const eval_t&, TransStack&, manage_si_t& msi, std::string&);
	virtual sitype_t get_type() const;
	virtual bool same(StackItemBranch*) const;
};


//
// StackItemBranchIf
//

class StackItemBranchIf : public StackItemBranch {
	enum {MY_IF = 0, MY_THEN = 1, MY_ELSE = 2};
public:
	StackItemBranchIf();
	StackItemBranchIf(const StackItemBranchIf&);
	virtual ~StackItemBranchIf();
	virtual StackItem* dup() const;
	virtual void to_string(ToString&, const bool& = false) const;
	virtual st_err_t exec1(TransStack&, int&, std::string&);
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;
};


//
// StackItemBranchWhile
//

class StackItemBranchWhile : public StackItemBranch {
	enum {MY_WHILE = 0, MY_REPEAT = 1};
public:
	StackItemBranchWhile();
	StackItemBranchWhile(const StackItemBranchWhile&);
	virtual ~StackItemBranchWhile();
	virtual StackItem* dup() const;
	virtual void to_string(ToString&, const bool& = false) const;
	virtual st_err_t exec1(TransStack&, int&, std::string&);
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;
};


//
// StackItemBranchDo
//

class StackItemBranchDo : public StackItemBranch {
	enum {MY_DO = 0, MY_UNTIL = 1, MY_DO_TEST = 2};
public:
	StackItemBranchDo();
	StackItemBranchDo(const StackItemBranchDo&);
	virtual ~StackItemBranchDo();
	virtual StackItem* dup() const;
	virtual void to_string(ToString&, const bool& = false) const;
	virtual st_err_t exec1(TransStack&, int&, std::string&);
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;
};


//
// StackItemBranchLoop
//

enum {LOOP_OPEN_FOR, LOOP_OPEN_START};
enum {LOOP_CLOSE_UNDEFINED, LOOP_CLOSE_NEXT, LOOP_CLOSE_STEP};

class StackItemBranchLoop : public StackItemBranch {
	int loop_open;
	int loop_close;
	int value_begin;
	int value_end;
	VarDirectory* lvars;
	std::string varname;

	virtual VarFile *get_loop_varfile() const;
	virtual int get_loop_varvalue() const;
	virtual void set_loop_varvalue(const int&);
public:
	StackItemBranchLoop(const int&);
	StackItemBranchLoop(const StackItemBranchLoop&);
	virtual ~StackItemBranchLoop();
	virtual void set_loop_close(const int&);
	virtual StackItem* dup() const;
	virtual void to_string(ToString&, const bool& = false) const;
	virtual VarDirectory* get_lvars(bool&) const;
	virtual void clear_lvars();
	virtual void set_loop_varname(const std::string&);
	virtual st_err_t eval(const eval_t&, TransStack&, manage_si_t&, std::string&);
	virtual st_err_t exec1(TransStack&, int&, std::string&);
	virtual sitype_t get_type() const;
	virtual bool same(StackItem*) const;
};


//
// Stack
//

const int stack_alloc_step = 5000;

class Stack {

#ifdef DEBUG_CLASS_COUNT
	static long class_count;
#endif

#ifdef DEBUG_TRANSSTACK
	static int class_serial;
	int object_serial;
#endif

#ifndef USE_VECTOR
	size_t count;
	size_t allocated;
	StackItem** values;
	void re_alloc(const size_t&);
#else // USE_VECTOR is defined
	std::vector<StackItem*> values;
#endif

	bool has_ownership;
public:
	Stack(const size_t& pre_alloc = 0);
	Stack(const Stack&);
	virtual ~Stack();

#ifdef DEBUG_CLASS_COUNT
	static int get_class_count() { return class_count; }
#endif

	virtual size_t stack_push(StackItem*);
	virtual StackItem* stack_pop();
#ifndef USE_VECTOR
	virtual size_t get_count() const { return count; }
#else
	virtual size_t get_count() const { return values.size(); }
#endif
	virtual const StackItem *stack_get_const_si(const int&) const;
	virtual const std::string get_display_line(const DisplayStackLayout&, const int&, IntervalShift&, bool&, bool&);
	virtual void take_ownership() { has_ownership = true; }
};


//
// NodeStack
//

struct SIO {
	tso_t ownership;
	StackItem* si;
	SIO() : ownership(TSO_OWNED_BY_TS), si(NULL) { }
	SIO(const tso_t& o, StackItem* const &s) : ownership(o), si(s) { }
	SIO(const SIO& sio) {
		ownership = sio.ownership;
		si = sio.si;
	}
	~SIO() { }
	void cleanup() {
		if (ownership == TSO_OUTSIDE_TS && si != NULL) {
			delete si;
		}
	}
};

const int nodestack_alloc_step = 5000;

class NodeStack {

#ifdef DEBUG_CLASS_COUNT
	static long class_count;
#endif

#ifdef DEBUG_TRANSSTACK
	static int class_serial;
	int object_serial;
#endif

	Stack* st;

	typedef enum {SI_UNDEF = 0, SI_ADDED, SI_DELETED} si_keeper_t;

#ifdef DEBUG_TRANSSTACK
	const std::string debug_keeper_type_name(si_keeper_t t) {
		switch (t) {
			case SI_UNDEF:
				return "undefined";
			case SI_ADDED:
				return "added";
			case SI_DELETED:
				return "deleted";
			default:
				return "!!!unknown type value!!!";
		}
	}
#endif

#ifndef STACK_USE_MAP
	struct ListSI {
		si_keeper_t type;
		StackItem* si;
	}* si_keeper;
	int count;
	int allocated;
	void re_alloc(const int&);
#else // STACK_USE_MAP is defined
	std::map<StackItem*, si_keeper_t> si_map_keeper;
#endif
	NodeStack* previous;
	NodeStack* next;
public:
	friend class TransStack;
	NodeStack(NodeStack*, NodeStack*, Stack*);
	virtual ~NodeStack();

#ifdef DEBUG_CLASS_COUNT
	static int get_class_count() { return class_count; }
#endif
#ifdef DEBUG_TRANSSTACK
	int get_object_serial() const { return object_serial; }
#endif

#ifndef STACK_USE_MAP
	virtual int add(const ListSI&);
#endif
	virtual int nodestack_push(StackItem*);
	virtual SIO nodestack_pop();
};


//
// Exec
//

const int IP_END = -1;

class Exec {
private:

#ifdef DEBUG_CLASS_COUNT
	static long class_count;
#endif

public:
	SIO s;
	VarDirectory* lvars;
	int ip;

	Exec(const SIO&, VarDirectory*);
	Exec(const Exec&);
	virtual ~Exec();

#ifdef DEBUG_CLASS_COUNT
	static int get_class_count() { return class_count; }
#endif

};


//
// TransStack
//

class TransStack {

#ifdef DEBUG_CLASS_COUNT
	static long class_count;
#endif

	int count;
	bool modified_flag;
	NodeStack* tail;
	NodeStack* head;
	std::vector<Exec> *exec_stack;
	bool temporary_copy;
	size_t ground_level;

	virtual st_err_t exec1(std::string&);
	virtual VarFile* get_local_var(const std::string&);

	TransStack(const TransStack&);
public:
	class TSVars {
		Tree* tree;
		StackItemList* si_path;
		std::string s;
		TSVars(const TSVars&);
	public:
		friend class TransStack;
		TSVars();
		virtual ~TSVars();
		void get_path(std::vector<std::string>*&) const;
		virtual StackItemList* get_si_path() const;
		virtual bool update_si_path();
		virtual const std::string& get_si_path_string() const;
		virtual void get_var_list(std::vector<std::string>*&) const;
		virtual void home();
		virtual void up();
		VarDirectory *save_pwd() const;
		void recall_pwd(VarDirectory*);
	};
	TSVars vars;

	//sitype_t si_get_type(StackItem* const &);

	TransStack(const bool& = false, std::vector<Exec>* = NULL);
	virtual ~TransStack();

#ifdef DEBUG_CLASS_COUNT
	static int get_class_count() { return class_count; }
#endif

	std::vector<Exec> *get_exec_stack() const { return exec_stack; }

	virtual void forward_tail();
	virtual void control_undos_chain_size();
	virtual void forward_head();
	virtual void backward_head();

	virtual bool get_modified_flag() const;
	virtual void set_modified_flag(const bool& m);

	virtual int transstack_push(StackItem*);
	virtual int transstack_push_SIO(SIO&);
	virtual SIO transstack_pop();
	virtual st_err_t read_integer(int&);

	virtual const std::string transstack_get_display_line(const DisplayStackLayout&, const int&, IntervalShift&, bool&, bool&);
	virtual st_err_t inner_push_eval(const eval_t&, SIO&, const bool&, std::string&);
	virtual st_err_t do_push_eval(SIO&, const bool&, std::string&, const eval_t& = EVAL_SOFT);
	virtual size_t stack_get_count() const;
	virtual const StackItem *transstack_get_const_si(const int&) const;

	virtual void push_exec_stack(SIO, VarDirectory*);
	virtual void clear_exec_stack();
	virtual size_t pop_exec_stack();
	virtual st_err_t read_lvars(VarDirectory*, std::string&);

	virtual st_err_t crdir(const std::string&);
	virtual st_err_t sto(const std::string&, StackItem* const, const bool& = true);
	virtual st_err_t purge(const std::string&);
	virtual st_err_t rcl(const std::string&, StackItem*&, Var*&, const bool& = true);
	virtual st_err_t rcl_const(const std::string&, const StackItem*&, Var*&);
	virtual st_err_t rcl_for_eval(const std::string&, StackItem*&, const bool& = true);
};

#endif // STACK_H

