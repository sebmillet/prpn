// Scalar.cpp
// Stack implementation and StackItems

// RPN calculator

// Sébastien Millet
// August-September 2009

// Note about complex functions extensions (cosine of a complex, arc-tangent-hyperbolic of complex, etc.):
// The formulas soucres were found in the following URLs:
//   http://www.suitcaseofdreams.net/Trigonometric_Functions.htm
//   http://www.suitcaseofdreams.net/Inverse_Functions.htm
//   http://www.suitcaseofdreams.net/Hyperbolic_Functions.htm
//   http://www.suitcaseofdreams.net/Inverse__Hyperbolic_Functions.htm

#include "Common.h"
#include "Scalar.h"
#include <sstream>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

  // Number of digits of reals
const int REAL_PRECISION = 12;
  // Lowest positive real
extern const real MINR = 1e-199;
  // Biggest positive real
extern const real MAXR = 9.99999999999e199;
static const real PI = 3.14159265359;
static const real PI_DIV_2 = 1.57079632679;
static const real LN_10 = 2.30258509299;
static const Cplx C_LN_10 = Cplx(2.30258509299, 0);

  // If the constant below is false, then, the "STD" display mode of
  // reals just uses C++ built-in language = the following code:
  // 	ostringstream o;
  // 	o.precision(REAL_PRECISION);
  // 	o << r;
  // 	string s = o.str();
  // If true, then, the "STD" mode to display reals is driven by
  // prpn itself, regardless of what C++ code above returns.
  // This code is written in the function user_real_to_string().
  // An example of difference between managed and language-builtin is:
  // Managed: 1.E-13     .0000001
  // Builtin: 1e-13      1e-07
  //
  // * NOTE IF YOU PUT FALSE BELOW, THE TESTS SERIES WILL FAIL IN SEVERAL TESTS *
  //
  // This variable can be modified using the
  //   _HACK-REAL-MGMT
  // command.
bool cfg_realdisp_manage_std = true;

  // Should we display certain reals like "1.E-13" (false => HP-28S flavor)
  // or like "1E-13" (true)?
  // I am used to HP-28S flavor although removing trailing dot is cleaner...
  //
  // * NOTE IF YOU PUT TRUE BELOW, THE TESTS SERIES WILL FAIL IN SEVERAL TESTS *
  //
  // This variable can be modified using the
  //   _HACK-REAL-DOT
  // command.
bool cfg_realdisp_remove_trailing_dot = false;


// General functions

#define ASSERT_STATUS(c) \
	if (c != ST_ERR_OK) \
		return;

void prepare_arith() { errno = 0; }

  // Remove trailing digits of reals
  // If we were using decimal coded binary reals, such a function would
  // not be needed. BUT, we are using internal double or long double and
  // thus need to chop trailing digits off
real real_trim(const real& r) {
	real r2;
	string s = real_to_string(r);
	if (s == "-0")
		s = "0";

	//debug_write("s = ");
	//debug_write(s.c_str());

	istringstream iss(s);
	iss >> r2;
	return r2;
}

  // Simple real to string converter
const string real_to_string(const real& r) {
	ostringstream o;
	o.precision(REAL_PRECISION);
	o << r;
	string s = o.str();
	return s;
}

  // Used only by user_real_to_string()
void ascii_integer_string_round(string& s, const int& nb_digits) {
	if (s.length() > static_cast<unsigned int>(nb_digits)) {
		char c = s.at(nb_digits);
		s.erase(nb_digits);
		if (c >= '5') {
			int r;
			int carry = 1;
			for (int i = nb_digits - 1; i >= 0 && carry == 1; i--) {
				r = (s[i] - '0') + 1;
				s[i] = '0' + (r % 10);
				carry = (r >= 10);
			}
			if (carry == 1)
				s.insert(0, "1");
		}
	}
	  // Ha! Infinite loop? This should not happen
	if (s.length() > static_cast<unsigned int>(nb_digits))
		ascii_integer_string_round(s, nb_digits);
}

  // Remove useless zeros left after the decimal separator ('.')
  // This function presumes there IS a decimal separator somwhere, don't
  // apply it on litteral integers.
  // Used only by user_real_to_string().
void ascii_remove_trailing_zeros(string& s, const bool& enforce_remove_trailing_dots = false) {
	int i;
	for (i = s.length() - 1; i >= 0 && s.at(i) == '0'; i--)
		;
	s.erase(i + 1);
	if (cfg_realdisp_remove_trailing_dot || enforce_remove_trailing_dots)
		if (s.at(s.length() - 1) == PORTABLE_DECIMAL_SEPARATOR)
			s.erase(s.length() - 1);
}

  // Convert a real into a string, taking into account
  // user settings like STD, FIX, SCI, ENG, and the
  // number of decimals if applicable.
const string user_real_to_string(const real& r, const tostring_t& t, const bool& enforce_sci = false) {
	string s = real_to_string(r);
	size_t p;

	/*debug_write("Start string:");
	debug_write(s.c_str());*/

	realdisp_t setting_rd;
	int setting_nbdecs;
	F->get_realdisp(t != TOSTRING_PORTABLE, setting_rd, setting_nbdecs);
	if (enforce_sci) {
		setting_rd = REALDISP_SCI;
		setting_nbdecs = REAL_PRECISION - 1;
	}

	/*debug_write_i("rd = %i", setting_rd);
	debug_write_i("nbdecs = %i", setting_nbdecs);*/

	if (setting_nbdecs < 0)
		setting_nbdecs = REAL_PRECISION - 1;

	string target2;

	if (setting_rd != REALDISP_STD || cfg_realdisp_manage_std) {
		  // Without the sign, and before "E"
		string part_mantisse;
		  // After "E"
		int part_exp;
		  // -1 or +1
		int part_sign = 1;
		  // Before .
		string part_mantisse_int;
		  // After .
		string part_mantisse_dec;

// Mantisse

		if ((p = s.find_first_of("eE")) != string::npos) {
			part_mantisse = s.substr(0, p);
			part_exp = string_to_integer(s.substr(p + 1));
		} else {
			part_mantisse = s;
			part_exp = 0;
		}

// Sign

		if (part_mantisse.length() >= 1) {
			if (part_mantisse.at(0) == '-') {
				part_sign = -1;
				part_mantisse.erase(0, 1);
			} else if (part_mantisse.at(0) == '+') {
				part_sign = 1;
				part_mantisse.erase(0, 1);
			} else {
				part_sign = 1;
			}
		}

// Integer and decimal parts of the mantisse

		if ((p = part_mantisse.find_first_of(PORTABLE_DECIMAL_SEPARATOR)) != string::npos) {
			part_mantisse_int = part_mantisse.substr(0, p);
			part_mantisse_dec = part_mantisse.substr(p + 1);
		} else {
			part_mantisse_int = part_mantisse;
			part_mantisse_dec = "";
		}
		if ((p = part_mantisse_int.find_first_not_of('0')) != string::npos)
			part_mantisse_int.erase(0, p);
		else
			part_mantisse_int = "";
		if ((p = part_mantisse_dec.find_first_not_of('0')) == string::npos)
			part_mantisse_dec = "";

		/*debug_write("Mantisse:");
		debug_write(part_mantisse.c_str());
		debug_write_i("Sign: %i", part_sign);
		debug_write("Mantisse INT:");
		debug_write(part_mantisse_int.c_str());
		debug_write("Mantisse DEC:");
		debug_write(part_mantisse_dec.c_str());
		debug_write_i("Exp: %i", part_exp);*/

		string target1 = part_mantisse_int + part_mantisse_dec;
		part_exp += part_mantisse_int.length() - 1;
		if ((p = target1.find_first_not_of('0')) != string::npos && p != 0) {
			part_exp -= p;
			target1.erase(0, p);
		}
		ascii_integer_string_round(target1, REAL_PRECISION);

		/*debug_write("Target1:");
		debug_write(target1.c_str());*/

		string disp_eex_noround;
		if (target1.length() == 0 || target1 == "0") {
			disp_eex_noround = "0";
			part_exp = 0;
			part_sign = 1;
		} else {
			disp_eex_noround = target1;
		}
		if (disp_eex_noround.length() < static_cast<unsigned int>(REAL_PRECISION))
			disp_eex_noround.append(string(REAL_PRECISION - disp_eex_noround.length(), '0'));
		ascii_integer_string_round(disp_eex_noround, REAL_PRECISION);
		target2 = disp_eex_noround;

		bool work_done = false;

		/*debug_write("** target2:");
		debug_write(target2.c_str());
		debug_write_i("** part_exp: %i", part_exp);*/

		switch (setting_rd) {
			case REALDISP_STD:
				if (part_exp < 0) {
					if ((p = target2.find_last_not_of('0')) != string::npos) {
						if (static_cast<int>(p) + 1 - part_exp <= REAL_PRECISION + 1) {
							target2.insert(0, string(-part_exp - 1, '0'));
							target2.erase(REAL_PRECISION);
							target2.insert(0, PORTABLE_STRING_DECIMAL_SEPARATOR);
							ascii_remove_trailing_zeros(target2);
							work_done = true;
						}
					} else {
						throw(CalcFatal(__FILE__, __LINE__, "user_real_to_string(): inconsistent data!"));
					}
				} else {
					if (part_exp <= REAL_PRECISION - 1) {
						if (part_exp < REAL_PRECISION - 1) {
							target2.insert(part_exp + 1, PORTABLE_STRING_DECIMAL_SEPARATOR);
							ascii_remove_trailing_zeros(target2, true);
						}
						work_done = true;
					}
				}
				break;
			case REALDISP_FIX:
				if (part_exp < 0) {
					if (-part_exp <= setting_nbdecs + 1) {
						string t = string(-part_exp, '0') + target2;
						ascii_integer_string_round(t, setting_nbdecs + 1);
						if ((p = t.find_last_not_of('0')) != string::npos) {
							target2 = t;
							target2.insert(1, PORTABLE_STRING_DECIMAL_SEPARATOR);
							work_done = true;
						}
					}
				} else {
					if (part_exp <= REAL_PRECISION - 1) {
						if (part_exp + 1 + setting_nbdecs < REAL_PRECISION)
							ascii_integer_string_round(target2, part_exp + setting_nbdecs + 1);
						target2.insert(part_exp + 1, PORTABLE_STRING_DECIMAL_SEPARATOR);
						work_done = true;
					}
				}
				break;
			case REALDISP_ENG:
				int point_pos;
				int new_part_exp;
				if (part_exp >= 0) {
					int t = part_exp / 3;
					new_part_exp = 3 * t;
				} else {
					int t = (-part_exp - 1) / 3;
					new_part_exp = -3 * (t + 1);
				}
				point_pos = part_exp - new_part_exp + 1;
				ascii_integer_string_round(target2, setting_nbdecs + 1);
				if (point_pos > setting_nbdecs + 1)
					target2.append(string(point_pos - setting_nbdecs - 1, '0'));
				target2.insert(point_pos, PORTABLE_STRING_DECIMAL_SEPARATOR);
				target2.append("E" + integer_to_string(new_part_exp));
				work_done = true;
				break;
			case REALDISP_SCI:
				break;
			default:
				throw(CalcFatal(__FILE__, __LINE__, "user_real_to_string(): unknown real display type!!"));
		}

		if (!work_done) {
			  // Work not done yet ==>> failover on SCI display
			ascii_integer_string_round(target2, setting_nbdecs + 1);
			target2.insert(1, PORTABLE_STRING_DECIMAL_SEPARATOR);
			if (setting_rd == REALDISP_STD)
				ascii_remove_trailing_zeros(target2);
			target2.append("E" + integer_to_string(part_exp));
		}

		if (part_sign < 0)
			target2.insert(0, "-");

		/*debug_write("target2:");
		debug_write(target2.c_str());*/

	} else {
		target2 = s;
	}

	if ((p = target2.find_first_of("eE")) != string::npos) {
		if (target2.substr(p + 1, 1) == "+")
			target2.erase(p + 1, 1);
	}
	char ds = F->get_decimal_separator(t != TOSTRING_PORTABLE);
	if (ds != PORTABLE_DECIMAL_SEPARATOR) {
		p = target2.find_first_of(PORTABLE_DECIMAL_SEPARATOR);
		if (p != string::npos)
			target2[p] = ds;
	}

	return target2;
}

st_err_t real_check_bounds(const bool& can_be_zero, const int& sign, const real& r, real& res, const bool& force_strict) {
	enum {VALID, OVERFLOW_NEG, OVERFLOW_POS, UNDERFLOW_NEG, UNDERFLOW_POS} status = VALID;
	res = r;
	if (errno == ERANGE)
		status = (sign >= 0 ? OVERFLOW_POS : OVERFLOW_NEG);
	else if (!can_be_zero && r == 0)
		status = (sign >= 0 ? UNDERFLOW_POS : UNDERFLOW_NEG);
	else if (r < -MAXR)
		status = OVERFLOW_NEG;
	else if (r > MAXR)
		status = OVERFLOW_POS;
	else if (r > -MINR && sign < 0 && !can_be_zero)
		status = UNDERFLOW_NEG;
	else if (r < MINR && sign >= 0 && !can_be_zero)
		status = UNDERFLOW_POS;
	switch (status) {
		case VALID:
			break;
		case OVERFLOW_NEG:
			if (F->get(FL_OVERFLOW_OK) || force_strict)
				return ST_ERR_OVERFLOW;
			else
				res = -MAXR;
			break;
		case OVERFLOW_POS:
			if (F->get(FL_OVERFLOW_OK) || force_strict)
				return ST_ERR_OVERFLOW;
			else
				res = MAXR;
			break;
		case UNDERFLOW_NEG:
			if (F->get(FL_UNDERFLOW_OK) || force_strict)
				return ST_ERR_UNDERFLOW_NEG;
			else
				res = 0;
			break;
		case UNDERFLOW_POS:
			if (F->get(FL_UNDERFLOW_OK) || force_strict)
				return ST_ERR_UNDERFLOW_POS;
			else
				res = 0;
			break;
		default:
			return ST_ERR_INTERNAL;
	}
	return ST_ERR_OK;
}

real real_abs(const real& r) { return abs(r); }
int real_sign(const real& r) { return (r > 0 ? 1 : (r < 0 ? -1 : 0)); }

real real_ip(const real& r) {
	if (r < 0)
		return real(ceil(r));
	else
		return real(floor(r));
}

real real_floor(const real& r) { return real(floor(r)); }
real real_ceil(const real& r) { return real(ceil(r)); }
real real_mant(const real& r) {
	string s = user_real_to_string(r, TOSTRING_PORTABLE, true);
	size_t p;
	if ((p = s.find_first_of('E')) == string::npos)
		throw(CalcFatal(__FILE__, __LINE__, "real_mant(): inconsistent data"));
	s.erase(p);
	real r2;
	istringstream iss(s);
	iss >> r2;
	return r2;
}
real real_xpon(const real& r) {
	string s = user_real_to_string(r, TOSTRING_PORTABLE, true);
	size_t p;
	if ((p = s.find_first_of('E')) == string::npos)
		throw(CalcFatal(__FILE__, __LINE__, "real_mant(): inconsistent data"));
	s.erase(0, p + 1);
	real r2;
	istringstream iss(s);
	iss >> r2;
	return r2;
}

real get_max_real_from_bin_size(const int& m) {
	return exp((real)m * log(2.0));
}

real my_rand() {
	int n = rand();
	return (static_cast<real>(n) / RAND_MAX);
}

void my_srand(real seed) {
	if (seed < 0)
		seed = 0;
	if (seed > 1)
		seed = 1;
	int sr;
	if (seed != 0)
		sr = static_cast<int>(seed * RAND_MAX);
	else
		sr = static_cast<int>(time(NULL));
	srand(sr);
}


// Arithmetic functions

// * ***************************************************************** *
// * --NUMERIC--                                                       *
// *                           WARNING                                 *
// *                                                                   *
// *            IMPORTANT NOTE ABOUT NUMERIC_* FUNCTIONS               *
// *                                                                   *
// * The functions MUST be built in such a way that res can point to   *
// * the same object as the first const real& (r or r1 in general.)    *
// *                                                                   *
// * This is why res1 intermediate is always used, as a precaution. In *
// * many cases it is useless but better stick to a unique, safe,      *
// * standard.                                                         *
// *                                                                   *
// * ***************************************************************** *

static void numeric_add(const real&, const real&, st_err_t&, real&);
static void numeric_sub(const real&, const real&, st_err_t&, real&);
static void numeric_mul(const real&, const real&, st_err_t&, real&);
static void numeric_div(const real&, const real&, st_err_t&, real&);
static void numeric_ln(const real&, st_err_t&, real&);
static void numeric_log(const real&, st_err_t&, real&);
static void numeric_exp(const real&, st_err_t&, real&);
static void numeric_alog(const real&, st_err_t&, real&);
static void numeric_pow(const real&, const real&, st_err_t&, real&);
static void numeric_cos(const real&, st_err_t&, real&);
static void numeric_sin(const real&, st_err_t&, real&);
static void numeric_tan(const real&, st_err_t&, real&);
static void numeric_acos(const real&, st_err_t&, real&);
static void numeric_asin(const real&, st_err_t&, real&);
static void numeric_atan(const real&, st_err_t&, real&);
static void numeric_sqr(const real&, st_err_t&, real&);

static void numeric_add(const real& r1, const real& r2, st_err_t& c, real& res) {
	real res1;
	int s1 = real_sign(r1);
	int s2 = real_sign(r2);
	int s;
	ASSERT_STATUS(c)
	if (s1 == s2)
		s = s1;
	else if (s1 < 0)
		s = (r1 <= -r2 ? -1 : s2);
	else if (s1 > 0)
		s = (r1 >= -r2 ? 1 : s2);
	else
		s = s2;
	res1 = r1 + r2;
	c = real_check_bounds(true, s, res1, res);
}

static void numeric_sub(const real& r1, const real& r2, st_err_t& c, real& res) {
	real res1;
	int s1 = real_sign(r1);
	int s2 = -real_sign(r2);
	int s;
	ASSERT_STATUS(c)
	if (s1 == s2)
		s = s1;
	else if (s1 < 0)
		s = (r1 <= r2 ? -1 : s2);
	else if (s1 > 0)
		s = (r1 >= r2 ? 1 : s2);
	else
		s = s2;
	res1 = r1 - r2;
	c = real_check_bounds(true, s, res1, res);
}

static void numeric_mul(const real& r1, const real& r2, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	res1 = r1 * r2;
	c = real_check_bounds(r1 == 0 || r2 == 0, real_sign(r1) * real_sign(r2), res1, res);
}

static void numeric_div(const real& r1, const real& r2, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	if (r2 == 0) {
		if (r1 == 0)
			c = ST_ERR_UNDEFINED_RESULT;
		else if (F->get(FL_INFINITE))
			c = ST_ERR_INFINITE_RESULT;
		else {
			res1 = (real_sign(r1) < 0 ? -MAXR : MAXR);
			c = ST_ERR_OK;
			res = res1;
		}
	} else {
		res1 = r1 / r2;
		c = real_check_bounds(r1 == 0, real_sign(r1) * real_sign(r2), res1, res);
	}
}

static void numeric_ln(const real& r, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	if (r < 0)
		c = ST_ERR_BAD_ARGUMENT_VALUE;
	else if (r == 0) {
		c = ST_ERR_OK;
		res1 = -MAXR;
		if (F->get(FL_INFINITE))
			c = ST_ERR_INFINITE_RESULT;
	} else {
		res1 = log(r);
		c = real_check_bounds(true, res1 < 0 ? -1 : 1, res1, res);
	}
}

static void numeric_log(const real& r, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	numeric_ln(r, c, res1);
	numeric_div(res1, LN_10, c, res);
}

static void numeric_exp(const real& r, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	res1 = exp(r);
	if (errno && r < 0) {
		res1 = 0;
		errno = 0;
	}
	c = real_check_bounds(false, 1, res1, res);
}

static void numeric_alog(const real& r, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	numeric_mul(r, LN_10, c, res1);
	numeric_exp(res1, c, res);
}

static void numeric_pow(const real& r1, const real& r2, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	real res2;
	  // a^b = exp(b * ln(a))
	numeric_ln(r1, c, res1);
	numeric_mul(r2, res1, c, res2);
	numeric_exp(res2, c, res);
}

static void numeric_cos(const real& r, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	res1 = cos(r);
	res = res1;
}

static void numeric_acos(const real& r, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	res1 = acos(r);
	if (errno == ERANGE)
		c = ST_ERR_BAD_ARGUMENT_VALUE;
	else
		res = res1;
}

static void numeric_sin(const real& r, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	res1 = sin(r);
	res = res1;
}

static void numeric_asin(const real& r, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	res1 = asin(r);
	if (errno == ERANGE)
		c = ST_ERR_BAD_ARGUMENT_VALUE;
	else
		res = res1;
}

static void numeric_tan(const real& r, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	res1 = tan(r);
	c = real_check_bounds(true, res >= 0 ? 1 : -1, res1, res);
}

static void numeric_atan(const real& r, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	res1 = atan(r);
	if (errno == ERANGE)
		c = ST_ERR_BAD_ARGUMENT_VALUE;
	else
		res = res1;
}

static void numeric_sqr(const real& r, st_err_t& c, real& res) {
	real res1;
	ASSERT_STATUS(c)
	if (r < 0)
		c = ST_ERR_BAD_ARGUMENT_VALUE;
	else {
		res1 = sqrt(r);
		c = real_check_bounds(r == 0, 1, res1, res);
	}
}


//
// Binary
//

#ifdef DEBUG_CLASS_COUNT
long Binary::class_count = 0;
#endif


Binary::Binary() {

	//cout << "B1: Binary::Binary() - " << this << endl;

#ifdef DEBUG_CLASS_COUNT
	class_count++;
#endif

}

Binary::Binary(const Binary& b) : bits(b.bits) {

	//cout << "B1: Binary::Binary(const Binary&) - " << this << endl;

#ifdef DEBUG_CLASS_COUNT
	class_count++;
#endif

}

Binary::Binary(const bitset<G_HARD_MAX_NB_BITS> b) {

	//cout << "B1: Binary::Binary(const bitset<G_HARD_MAX_NB_BITS>) - " << this << endl;

#ifdef DEBUG_CLASS_COUNT
	class_count++;
#endif

	bits = b;
}

Binary::~Binary() {

	//cout << "B1: Binary::~Binary() - " << this << endl;

#ifdef DEBUG_CLASS_COUNT
		class_count--;
#endif

}

void Binary::set(std::bitset<G_HARD_MAX_NB_BITS> b) {
	bits = b;
}

int digit_to_int(const char& c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else
		return -1;
}

char int_to_digit(const int& i) {
	if (i < 0)
		return '!';
	else if (i <= 9)
		return '0' + static_cast<char>(i);
	else if (i <= 15)
		return 'A' + static_cast<char>(i) - 10;
	else
		return '!';
}

void remove_leading_zeros(string& n) {
	size_t found = n.find_first_not_of('0');
	if (found >= 1)
		n.erase(0, found);
	if (n.empty())
		n = "0";
}

void add_bin_string(const string& op1, string& op2, const int& base) {

	//cout << "W0: op1 = " << op1 << ", op2 = " << op2 << endl;
	//cout << "W0: op1.size() = " << op1.size() << ", op2.size() = " << op2.size() << endl;

	if (op2.size() <= op1.size())
		op2.insert(0, string(op1.size() - op2.size() + 1, '0'));

	//cout << "W1: adding \"" << op1 << "\" to \"" << op2 << "\"\n";

	int p2 = (int)op2.size() - 1;
	int val1, val2, val, carry = 0;
	for (int p1 = (int)op1.size() - 1; p2 >= 0; p1--, p2--) {
		if (p1 >= 0)
			val1 = digit_to_int(op1.at(p1));
		else
			val1 = 0;
		if (p2 >= 0)
			val2 = digit_to_int(op2.at(p2));
		else
			val2 = 0;
		val = val1 + val2 + carry;
		if (val >= base) {
			val -= base;
			carry = 1;
		} else
			carry = 0;
		op2[p2] = int_to_digit(val);
	}
	remove_leading_zeros(op2);
}

const string Binary::to_string(const int& base, const int& nb_bits) const {
	string m = "1";
	string r = "0";

	//debug_cout_litteral("Z1: converting ");

	for (int i = 0; i < nb_bits; i++) {
		//cout << "Z1: m = \"" << m << "\"\n";
		//cout << "Z1: r = \"" << r << "\"\n";

		if (bits.test(i))
			add_bin_string(m, r, base);
		add_bin_string(m, m, base);
	}

	//cout << "Z1: result = \"" << r << "\"\n";

	return r;
}

real Binary::to_real(const int& nb_bits) const {
	real m = 1;
	real r = 0;
	for (int p = 0; p < nb_bits; p++) {
		if (bits.test(p))
			r += m;
		m *= 2;
	}
	return r;
}

bool Binary::get_bit(const int& bit_index) const {
	if (bit_index < 0 || bit_index >= g_max_nb_bits)
		throw(CalcFatal(__FILE__, __LINE__, "Binary::get_bit(): bad parameter bit_index value"));
	return bits.test(bit_index);
}

st_err_t Binary_add(const Binary& lv, const Binary& rv, Binary& res) {
	int carry = 0;
	int v1, v2, v;
	for (int p = 0; p < g_max_nb_bits; p++) {
		v1 = (lv.bits.test(p) ? 1 : 0);
		v2 = (rv.bits.test(p) ? 1 : 0);
		v = v1 + v2 + carry;
		if (v >= 2) {
			v -= 2;
			carry = 1;
		} else
			carry = 0;
		res.bits[p] = (v != 0);
	}
	return ST_ERR_OK;
}

st_err_t Binary_sub(const Binary& lv, const Binary& rv, Binary& res) {
	int carry = 0;
	int v1, v2, v;
	for (int p = 0; p < g_max_nb_bits; p++) {
		v1 = (lv.bits.test(p) ? 1 : 0);
		v2 = (rv.bits.test(p) ? 1 : 0);
		v = v1 - v2 - carry;
		if (v < 0) {
			v += 2;
			carry = 1;
		} else
			carry = 0;
		res.bits[p] = (v != 0);
	}
	return ST_ERR_OK;
}

int Binary::cmp(const Binary& b) const {
	int v1, v2;
	for (int p = 0; p < g_max_nb_bits; p++) {
		v1 = (bits.test(p) ? 1 : 0);
		v2 = (b.bits.test(p) ? 1 : 0);
		if (v1 < v2)
			return -1;
		else if (v1 > v2)
			return 1;
	}
	return 0;
}


//
// BOTH REAL AND CPLX
//

#define HYPERBOLIC_SCALAR_FUNCTIONS(SC) \
inline void hyperbolic_common(const SC& x, st_err_t& c, SC& exp_x, SC& exp_moins_x) { \
	ASSERT_STATUS(c) \
	SC moins_x = x; \
	moins_x.neg(); \
	x.exp(c, exp_x); \
	moins_x.exp(c, exp_moins_x); \
} \
void SC::cosh(st_err_t& c, SC& res) const { \
	ASSERT_STATUS(c) \
	SC exp_x, exp_moins_x, addit; \
	hyperbolic_common(*this, c, exp_x, exp_moins_x); \
	exp_x.add(exp_moins_x, c, addit); \
	addit.div(SC(Real(2)), c, res); \
} \
void SC::sinh(st_err_t& c, SC& res) const { \
	ASSERT_STATUS(c) \
	SC exp_x, exp_moins_x, addit; \
	hyperbolic_common(*this, c, exp_x, exp_moins_x); \
	exp_x.sub(exp_moins_x, c, addit); \
	addit.div(SC(Real(2)), c, res); \
} \
void SC::tanh(st_err_t& c, SC& res) const { \
	ASSERT_STATUS(c) \
	SC exp_x, exp_moins_x, addit, soust; \
	hyperbolic_common(*this, c, exp_x, exp_moins_x); \
	exp_x.add(exp_moins_x, c, addit); \
	exp_x.sub(exp_moins_x, c, soust); \
	soust.div(addit, c, res); \
}
HYPERBOLIC_SCALAR_FUNCTIONS(Real)
HYPERBOLIC_SCALAR_FUNCTIONS(Cplx)


//
// REAL
//

#ifdef DEBUG_CLASS_COUNT
long Real::class_count = 0;
#endif

const string Real::to_string(const tostring_t& t) const {
	return user_real_to_string(r, t);
}

Real& Real::operator=(const Cplx& rv) {
	r = rv.get_re();
	return *this;
}

void Real::trim() { r = real_trim(r); }

int Real::cmp(const Real& rv) const { return r < rv.r ? -1 : (r > rv.r ? 1 : 0); }

void Real_arith(void (*f)(const real&, const real&, st_err_t&, real&), const Real& lv, const Real& rv, st_err_t& c, Real& res) {
	ASSERT_STATUS(c)
	(*f)(lv.r, rv.r, c, res.r);
}

void Real_add(const Real& lv, const Real& rv, st_err_t& c, Real& res) { Real_arith(numeric_add, lv, rv, c, res); }
void Real_sub(const Real& lv, const Real& rv, st_err_t& c, Real& res) { Real_arith(numeric_sub, lv, rv, c, res); }
void Real_mul(const Real& lv, const Real& rv, st_err_t& c, Real& res) { Real_arith(numeric_mul, lv, rv, c, res); }
void Real_div(const Real& lv, const Real& rv, st_err_t& c, Real& res) { Real_arith(numeric_div, lv, rv, c, res); }
void Real_pow(const Real& lv, const Real& rv, st_err_t& c, Real& res) { Real_arith(numeric_pow, lv, rv, c, res); }

void Real::add(const Real& rv, st_err_t& c, Real& res) const { Real_add(*this, rv, c, res); }
void Real::sub(const Real& rv, st_err_t& c, Real& res) const { Real_sub(*this, rv, c, res); }
void Real::mul(const Real& rv, st_err_t& c, Real& res) const { Real_mul(*this, rv, c, res); }
void Real::div(const Real& rv, st_err_t& c, Real& res) const { Real_div(*this, rv, c, res); }

void Real::floor(st_err_t& c, Real& res) const {
	ASSERT_STATUS(c)
	res.r = real_floor(r);
}

void Real::mod(const Real& rv, st_err_t& c, Real& res) const {
	ASSERT_STATUS(c)
	Real real2, real3, real4;
	div(rv, c, real2);
	real2.floor(c, real3);
	real3.mul(rv, c, real4);
	sub(real4, c, res);
}

void Real::pow(const Real& rv, st_err_t& c, Cplx& cplx) const {
	ASSERT_STATUS(c)
	if (r >= 0) {
		Real real;
		Real_pow(*this, rv, c, real);
		cplx = Cplx(real.r, 0);
	} else {
		Cplx cplx2 = Cplx(r, 0);
		cplx2.pow(Cplx(rv.r, 0), c, cplx);
	}
}

void Real::sqr(st_err_t& c, Cplx& cplx) const {
	ASSERT_STATUS(c)
	real tmp_r = real_abs(r);
	real res;
	numeric_sqr(tmp_r, c, res);
	if (c == ST_ERR_OK) {
		if (r < 0)
			cplx = Cplx(0, res);
		else
			cplx = Cplx(res, 0);
	}
}

void Real::square_of_abs(st_err_t& c, Real& res) const {
	ASSERT_STATUS(c)
	Real x = *this;
	mul(x, c, res);
}

void Real::abs(st_err_t& c, Real& res) const {
	ASSERT_STATUS(c)
	res = Real(real_abs(r));
}

void Real::ln(st_err_t& c, Cplx& cplx) const {
	ASSERT_STATUS(c)
	if (r < 0) {
		Cplx(r, 0).ln(c, cplx);
		return;
	}
	real res;
	numeric_ln(r, c, res);
	cplx = Cplx(res, 0);
}

void Real::log(st_err_t& c, Cplx& cplx) const {
	ASSERT_STATUS(c)
	if (r < 0) {
		Cplx(r, 0).log(c, cplx);
		return;
	}
	real res;
	numeric_log(r, c, res);
	cplx = Cplx(res, 0);
}

void Real::acos(st_err_t& c, Cplx& cplx) const {
	ASSERT_STATUS(c)
	if (r < -1 || r > 1) {
		Cplx(r, 0).acos(c, cplx);
		return;
	}
	real res;
	numeric_acos(r, c, res);
	cplx = Cplx(res, 0);
}

void Real::asin(st_err_t& c, Cplx& cplx) const {
	ASSERT_STATUS(c)
	if (r < -1 || r > 1) {
		Cplx(r, 0).asin(c, cplx);
		return;
	}
	real res;
	numeric_asin(r, c, res);
	cplx = Cplx(res, 0);
}

#define IMPLEMENT_REAL_NUMERIC_FUNCTION(OP) \
void Real::OP(st_err_t& c, Real& res) const { \
	ASSERT_STATUS(c) \
	numeric_##OP(r, c, res.r); \
}
IMPLEMENT_REAL_NUMERIC_FUNCTION(atan)
IMPLEMENT_REAL_NUMERIC_FUNCTION(cos)
IMPLEMENT_REAL_NUMERIC_FUNCTION(sin)
IMPLEMENT_REAL_NUMERIC_FUNCTION(tan)
IMPLEMENT_REAL_NUMERIC_FUNCTION(exp)
IMPLEMENT_REAL_NUMERIC_FUNCTION(alog)

  // FIXME
  // lnp1(x) = ln(1+x), it is supposed to bring greater accuracy to the result
  // when x is close to 0.
  // As you can see below, for the moment it is a simple hack to get this command to work.
void Real::lnp1(st_err_t& c, Real& res) const {
	ASSERT_STATUS(c)
	real r2, r3;
	numeric_add(r, 1.0, c, r2);
	numeric_ln(r2, c, r3);
	res = Real(r3);
}

  // FIXME
  // expm(x) = exp(x)-1, it is supposed to bring greater accuracy to the result
  // when x is close to 0, see also comment about lnp1(x).
  // As you can see below, for the moment it is a simple hack to get this command to work.
void Real::expm(st_err_t& c, Real& res) const {
	ASSERT_STATUS(c)
	real r2, r3;
	numeric_exp(r, c, r2);
	numeric_sub(r2, static_cast<real>(1.0), c, r3);
	res = Real(r3);
}

void Real::acosh(st_err_t& c, Cplx& cplx) const {
	  // ACOSH real function: arccosh(x) = ln(x+sqr(x^2-1)) for x >= 1
	ASSERT_STATUS(c)
	if (r < 1) {
		Cplx(r, 0).acosh(c, cplx);
		return;
	}
	Real real3, real4, real5;
	mul(*this, c, real3);
	real3.sub(Real(1), c, real4);
	Cplx cplx2;
	real4.sqr(c, cplx2);
	add(Real(cplx2.get_re()), c, real5);
	real5.ln(c, cplx);
}

void Real::asinh(st_err_t& c, Real& real) const {
	  // ASINH real function: arcsinh(x) = ln(x+sqr(x^2+1))
	ASSERT_STATUS(c)
	Real real3, real4, real5;
	mul(*this, c, real3);
	real3.add(Real(1), c, real4);
	Cplx cplx;
	real4.sqr(c, cplx);
	add(Real(cplx.get_re()), c, real5);
	Cplx cplx2;
	real5.ln(c, cplx2);
	real = Real(cplx2.get_re());
}

void Real::atanh(st_err_t& c, Cplx& cplx) const {
	  // ATANH real function: arctanh(x) = ln((1+x)/(1-x))/2 for |x| < 1
	ASSERT_STATUS(c)
	if (r < -1 || r > 1) {
		Cplx(r, 0).atanh(c, cplx);
		return;
	}
	Real un_plus_x, un_moins_x, rapport, real2, real3;
	add(Real(1), c, un_plus_x);
	Real(1).sub(*this, c, un_moins_x);
	un_plus_x.div(un_moins_x, c, rapport);
	rapport.ln(c, cplx);
	real2 = Real(cplx.get_re());
	real2.div(Real(2), c, real3);
	cplx = Cplx(real3.r, 0);
}

  //
  // Take a 'decimal' real (a fractional number of hours) and convert it
  // to an HMS value.
  // An HMS value is of the form h.MMSSss, where h is a number of hours,
  // MM a number of minutes (0 <= MM <= 59), SS a number of seconds
  // (0 <= SS <= 59), and ss a *decimal* part of seconds.
  // On my old good HP-28S calculator, the number of decimals for the "sub-seconds"
  // (the ss part) is not limited to hundredths of seconds (two decimals.)
  // I limit it to two decimals so as to ease hms_add and hms_sub functions.
  //
  // Note: works fine with negative values.
  //
  // Example:
  //   2 hours, 35 minutes, 29 seconds and 78 hundredth of seconds is written (in HMS):
  //     2.352978
  //   In decimal form (fractional number of hours), it is equal to:
  //     2.59160555556
  //   The to_hms member-function will take 2.59160555556 and 'return' (put
  //   in res) the value 2.352978.
  //   The hms_to member-function (below) will take 2.352978 and 'return' (put
  //   in res) the value 2.59160555556.
void Real::to_hms(st_err_t& c, Real& res) const {
	ASSERT_STATUS(c)
	real abs_r = real_abs(r);
	real ip_abs_r = real_ip(abs_r);
	real fp = abs_r - ip_abs_r;
	real minutes = real_ip(fp * 60.0);
	fp = 60.0 * fp - minutes;
	real secondes = real_ip(fp * 60.0);
	fp = 60.0 * fp - secondes;
	  // We don't go beyond hundredth of seconds in precision, see comment above
	fp = real_ip(fp * 100.0 + .5) / 100.0;
	fp /= 10000.0;
	real tmp = ip_abs_r + minutes / 100.0 + secondes / 10000.0 + fp;
	if (r < 0)
		tmp = -tmp;
	res = Real(tmp);
}

  // Convert from HMS to a 'decimal' real, see Real::to_hms comment above
void Real::hms_to(st_err_t& c, Real& res) const {
	ASSERT_STATUS(c)
	real abs_r = real_abs(r);
	real ip_abs_r = real_ip(abs_r);
	real fp = abs_r - ip_abs_r;
	real minutes = real_ip(fp * 100.0);
	fp = 100.0 * fp - minutes;
	real secondes = real_ip(fp * 100.0);
	fp = 100.0 * fp - secondes;
	  // We don't go beyond hundredth of seconds in precision, see Real::to_hms comment above
	fp = real_ip(fp * 100.0 + .5) / 100.0;
	real tmp = ip_abs_r + minutes / 60.0 + secondes / 3600.0 + fp / 3600.0;
	if (r < 0)
		tmp = -tmp;
	res = Real(tmp);
}

#define IMPLEMENT_HMS_ADD_OR_SUB(OP) \
void Real::hms_##OP(const Real& rv, st_err_t& c, Real& res) const { \
	ASSERT_STATUS(c) \
	Real dec_me, dec_rv, sum; \
	hms_to(c, dec_me); \
	rv.hms_to(c, dec_rv); \
	dec_me.OP(dec_rv, c, sum); \
	sum.to_hms(c, res); \
}
IMPLEMENT_HMS_ADD_OR_SUB(add)
IMPLEMENT_HMS_ADD_OR_SUB(sub)

void Real::d_to_r(st_err_t& c, Real& res) const {
	ASSERT_STATUS(c)
	real r2, r3;
	numeric_mul(r, PI, c, r2);
	numeric_div(r2, 180.0, c, r3);
	res = Real(r3);
}

void Real::r_to_d(st_err_t& c, Real& res) const {
	ASSERT_STATUS(c)
	real r2, r3;
	numeric_mul(r, 180.0, c, r2);
	numeric_div(r2, PI, c, r3);
	res = Real(r3);
}


//
// CPLX
//

#ifdef DEBUG_CLASS_COUNT
long Cplx::class_count = 0;
#endif

const string Cplx::to_string(const tostring_t& t) const {
	return "(" + user_real_to_string(re, t) + F->get_complex_separator(t != TOSTRING_PORTABLE) +
		user_real_to_string(im, t) + ")";
}

Cplx& Cplx::operator=(const Cplx& rv) {
	re = rv.re;
	im = rv.im;
	return *this;
}

Cplx& Cplx::operator=(const Real& rv) {
	re = rv.get_value();
	im = 0;
	return *this;
}

void Cplx::trim() {
	re = real_trim(re);
	im = real_trim(im);
}

int Cplx::cmp(const Cplx& rv) const { return re == rv.re && im == rv.im ? 0 : 1; }

  //
  // Please see remark earlier in this file about numeric_ functions: res
  // could be the same object as lv. To find this remark directly, search
  // the string
  //   --NUMERIC--
void Cplx_add(const Cplx& lv, const Cplx& rv, st_err_t& c, Cplx& res) {
	Cplx res1;
	ASSERT_STATUS(c)
	  // Calculate real part
	numeric_add(lv.re, rv.re, c, res1.re);
	numeric_add(lv.im, rv.im, c, res1.im);
	res = res1;
}

void Cplx_sub(const Cplx& lv, const Cplx& rv, st_err_t& c, Cplx& res) {
	Cplx res1;
	ASSERT_STATUS(c)
	  // Calculate real part
	numeric_sub(lv.re, rv.re, c, res1.re);
	numeric_sub(lv.im, rv.im, c, res1.im);
	res = res1;
}

void Cplx_mul(const Cplx& lv, const Cplx& rv, st_err_t& c, Cplx& res) {
	Cplx res1;
	real x;
	real y = 0;
	ASSERT_STATUS(c)
	  // Calculate real part
	numeric_mul(lv.re, rv.re, c, x);
	numeric_mul(lv.im, rv.im, c, y);
	numeric_sub(x, y, c, res1.re);
	  // Calculate imaginary part
	numeric_mul(lv.im, rv.re, c, x);
	numeric_mul(lv.re, rv.im, c, y);
	numeric_add(x, y, c, res1.im);
	res = res1;
}

void Cplx_div(const Cplx& lv, const Cplx& rv, st_err_t& c, Cplx& res) {
	Cplx res1;
	real x = 0, y = 0, d = 0, t = 0, u = 0, v = 0;
	ASSERT_STATUS(c)
	if (rv.re == 0 && rv.im == 0 && (lv.re != 0 || lv.im != 0)) {
		if (F->get(FL_INFINITE)) {
			c = ST_ERR_INFINITE_RESULT;
			return;
		}
		if (lv.re == 0)
			res.re = 0;
		else if (lv.re < 0)
			res.re = -MAXR;
		else
			res.re = MAXR;
		if (lv.im == 0)
			res.im = 0;
		else if (lv.im < 0)
			res.im = -MAXR;
		else
			res.im = MAXR;
		return;
	}

	  // Complex division: x = (ac + bd)/(a^2 + b^2), y = (ad - bc)/(a^2 + b^2)
	numeric_mul(rv.re, rv.re, c, x);
	numeric_mul(rv.im, rv.im, c, y);
	numeric_add(x, y, c, d);
	  // Calculate real part
	numeric_mul(rv.re, lv.re, c, t);
	numeric_mul(rv.im, lv.im, c, u);
	numeric_add(t, u, c, v);
	numeric_div(v, d, c, res1.re);
	  // Calculate imaginary part
	numeric_mul(rv.re, lv.im, c, t);
	numeric_mul(rv.im, lv.re, c, u);
	numeric_sub(t, u, c, v);
	numeric_div(v, d, c, res1.im);
	res = res1;
}

void Cplx::add(const Cplx& rv, st_err_t& c, Cplx& res) const { Cplx_add(*this, rv, c, res); }
void Cplx::sub(const Cplx& rv, st_err_t& c, Cplx& res) const { Cplx_sub(*this, rv, c, res); }
void Cplx::mul(const Cplx& rv, st_err_t& c, Cplx& res) const { Cplx_mul(*this, rv, c, res); }
void Cplx::div(const Cplx& rv, st_err_t& c, Cplx& res) const { Cplx_div(*this, rv, c, res); }

void Cplx::square_of_abs(st_err_t& c, Real& res) const {
	real x, y, t;
	ASSERT_STATUS(c)
	numeric_mul(re, re, c, x);
	numeric_mul(im, im, c, y);
	numeric_add(x, y, c, t);
	res = Real(t);
}

void Cplx::abs(st_err_t& c, Real& res) const {
	Real x;
	real r;
	ASSERT_STATUS(c)
	square_of_abs(c, x);
	numeric_sqr(x.get_value(), c, r);
	res = Real(r);
}

void Cplx::arg(st_err_t& c, Real& res) const {
	real t, r1, r;
	ASSERT_STATUS(c)
	if (re == 0) {
		if (im > 0)
			r = PI_DIV_2;
		else if (im < 0)
			r = -PI_DIV_2;
		else
			r = 0;
	} else {
		numeric_div(im, re, c, t);
		numeric_atan(t, c, r1);
		if (re < 0)
			if (im >= 0)
				numeric_add(r1, PI, c, r);
			else
				numeric_sub(r1, PI, c, r);
		else
			r = r1;
	}
	ASSERT_STATUS(c)
	res = Real(r);
}

void Cplx::r_to_p(st_err_t& c, Cplx& cplx) const {
	Real cplx_re, cplx_im;
	ASSERT_STATUS(c)
	abs(c, cplx_re);
	arg(c, cplx_im);
	if (c == ST_ERR_OK)
		cplx = Cplx(cplx_re.get_value(), cplx_im.get_value());
}

void Cplx::p_to_r(st_err_t& c, Cplx& cplx) const {
	real x, y, cplx_re, cplx_im;
	ASSERT_STATUS(c)
	numeric_cos(im, c, x);
	numeric_mul(x, re, c, cplx_re);
	numeric_sin(im, c, y);
	numeric_mul(y, re, c, cplx_im);
	if (c == ST_ERR_OK)
		cplx = Cplx(cplx_re, cplx_im);
}

void Cplx::ln(st_err_t& c, Cplx& cplx) const {
	Real x, new_im;
	real new_re;
	ASSERT_STATUS(c)
	abs(c, x);
	arg(c, new_im);
	numeric_ln(x.get_value(), c, new_re);
	if (c == ST_ERR_OK)
		cplx = Cplx(new_re, new_im.get_value());
}

void Cplx::log(st_err_t& c, Cplx& cplx) const {
	Cplx tmp;
	ASSERT_STATUS(c)
	ln(c, tmp);
	tmp.div(C_LN_10, c, cplx);
}

void Cplx::exp(st_err_t& c, Cplx& cplx) const {
	real new_re;
	ASSERT_STATUS(c)
	numeric_exp(re, c, new_re);
	if (c == ST_ERR_OK) {
		Cplx c2(new_re, im);
		c2.p_to_r(c, cplx);
	}
}

void Cplx::alog(st_err_t& c, Cplx& cplx) const {
	ASSERT_STATUS(c)
	Cplx tmp;
	mul(C_LN_10, c, tmp);
	tmp.exp(c, cplx);
}

void Cplx::pow(const Cplx& rv, st_err_t& c, Cplx& cplx) const {
	Cplx cplx2, cplx3;
	ASSERT_STATUS(c)
	ln(c, cplx2);
	rv.mul(cplx2, c, cplx3);
	cplx3.exp(c, cplx);
	if (rv.im == 0 && real_ip(rv.re) == rv.re && im == 0)
		cplx = Cplx(cplx.re, 0);
}

void Cplx::sqr(st_err_t& c, Cplx& cplx) const {
	ASSERT_STATUS(c)
	if (re == 0 && im == 0)
		cplx = Cplx(0,0);
	else {
		pow(Cplx(.5, 0), c, cplx);
		if (im == 0 && re < 0)
			cplx.re = 0;
	 }
}

void Cplx::acos(st_err_t& c, Cplx& cplx) const {
	  // ACOS complex function: arccos(z) = -i*ln(z+sqr(z^2-1))
	Cplx cplx2(*this);
	Cplx cplx3, cplx4, cplx5, cplx6;
	ASSERT_STATUS(c)
	cplx2.mul(*this, c, cplx3);
	cplx3.sub(Cplx(1, 0), c, cplx4);
	cplx4.sqr(c, cplx5);
	cplx5.add(*this, c, cplx5);
	cplx5.ln(c, cplx6);
	cplx6.mul(Cplx(0, -1), c, cplx);
}

void Cplx::asin(st_err_t& c, Cplx& cplx) const {
	  // ASIN complex function: arcsin(z) = -i*ln(i*z+sqr(1-z^2))
	Cplx cplx2(*this);
	Cplx cplx3, cplx4, cplx5, cplx6, cplx7, cplx8;
	ASSERT_STATUS(c)
	cplx2.mul(*this, c, cplx3);
	Cplx(1,0).sub(cplx3, c, cplx4);
	cplx4.sqr(c, cplx5);
	mul(Cplx(0,1), c, cplx6);
	cplx6.add(cplx5, c, cplx7);
	cplx7.ln(c, cplx8);
	cplx8.mul(Cplx(0, -1), c, cplx);
}

void Cplx::atan(st_err_t& c, Cplx& cplx) const {
	  // ATAN complex function: arctan(z) = i/2*(ln(1-iz)-ln(1+iz))
	Cplx iz, un_moins_iz, un_plus_iz, ln_un_moins_iz, ln_un_plus_iz, soust;
	ASSERT_STATUS(c)
	mul(Cplx(0, 1), c, iz);
	Cplx(1, 0).sub(iz, c, un_moins_iz);
	un_moins_iz.ln(c, ln_un_moins_iz);
	Cplx(1, 0).add(iz, c, un_plus_iz);
	un_plus_iz.ln(c, ln_un_plus_iz);
	ln_un_moins_iz.sub(ln_un_plus_iz, c, soust);
	Cplx(0, .5).mul(soust, c, cplx);
}

inline void cplx_trigo_common(const Cplx& z, st_err_t& c, Cplx& exp_iz, Cplx& exp_moins_iz) {
	Cplx iz, cplx2;
	ASSERT_STATUS(c)
	z.mul(Cplx(0,1), c, iz);
	cplx2 = iz;
	cplx2.neg();
	iz.exp(c, exp_iz);
	cplx2.exp(c, exp_moins_iz);
}

void Cplx::cos(st_err_t& c, Cplx& cplx) const {
	  // COS complex function: cos(z) = (exp(i*z)+exp(-i*z))/2
	Cplx exp_iz, exp_moins_iz, cplx2;
	ASSERT_STATUS(c)
	cplx_trigo_common(*this, c, exp_iz, exp_moins_iz);
	exp_iz.add(exp_moins_iz, c, cplx2);
	cplx2.div(Cplx(2, 0), c, cplx);
}

void Cplx::sin(st_err_t& c, Cplx& cplx) const {
	  // SIN complex function: sin(z) = (exp(i*z)-exp(-i*z))/(2*i)
	Cplx exp_iz, exp_moins_iz, cplx2;
	ASSERT_STATUS(c)
	if (re == 0 && im == 0) {
		cplx.zero();
		return;
	}
	cplx_trigo_common(*this, c, exp_iz, exp_moins_iz);
	exp_iz.sub(exp_moins_iz, c, cplx2);
	cplx2.div(Cplx(0, 2), c, cplx);
}

void Cplx::tan(st_err_t& c, Cplx& cplx) const {
	  // TAN complex function: tan(z) = (exp(i*z)-exp(-i*z)/(i*(exp(i*z)+exp(-i*z))
	Cplx exp_iz, exp_moins_iz, soust, addit, cplx2;
	ASSERT_STATUS(c)
	cplx_trigo_common(*this, c, exp_iz, exp_moins_iz);
	exp_iz.sub(exp_moins_iz, c, soust);
	exp_iz.add(exp_moins_iz, c, addit);
	soust.div(addit, c, cplx2);
	cplx2.div(Cplx(0, 1), c, cplx);
}

void Cplx::acosh(st_err_t& c, Cplx& cplx) const {
	  // ACOSH complex function: arccosh(z) = ln(z+sqr(z+1)*sqr(z-1))
	ASSERT_STATUS(c)
	Cplx z_plus_un, z_moins_un, rc_z_plus_un, rc_z_moins_un, produit, t;
	add(Cplx(1, 0), c, z_plus_un);
	z_plus_un.sqr(c, rc_z_plus_un);
	sub(Cplx(1, 0), c, z_moins_un);
	z_moins_un.sqr(c, rc_z_moins_un);
	rc_z_plus_un.mul(rc_z_moins_un, c, produit);
	add(produit, c, t);
	t.ln(c, cplx);
}

void Cplx::asinh(st_err_t& c, Cplx& cplx) const {
	  // ASINH complex function: arcsinh(z) = ln(z+sqr(z^2+1))
	ASSERT_STATUS(c)
	Cplx carre_de_z, carre_de_z_plus_1, rc_carre_de_z_plus_1, t;
	mul(*this, c, carre_de_z);
	carre_de_z.add(Cplx(1, 0), c, carre_de_z_plus_1);
	carre_de_z_plus_1.sqr(c, rc_carre_de_z_plus_1);
	add(rc_carre_de_z_plus_1, c, t);
	t.ln(c, cplx);
}

void Cplx::atanh(st_err_t& c, Cplx& cplx) const {
	  // ATANH complex function: arctanh(z) = (ln(1+z)-ln(1-z))/2
	ASSERT_STATUS(c)
	Cplx un_plus_z, un_moins_z, ln_un_plus_z, ln_un_moins_z, soust;
	add(Cplx(1, 0), c, un_plus_z);
	un_plus_z.ln(c, ln_un_plus_z);
	Cplx(1, 0).sub(*this, c, un_moins_z);
	un_moins_z.ln(c, ln_un_moins_z);
	ln_un_plus_z.sub(ln_un_moins_z, c, soust);
	soust.div(Cplx(2, 0), c, cplx);
}

void Cplx::sign(st_err_t& c, Cplx& cplx) const {
	Real a;
	real new_re, new_im;
	ASSERT_STATUS(c)
	if (re != 0 || im != 0) {
		abs(c, a);
		numeric_div(re, a.get_value(), c, new_re);
		numeric_div(im, a.get_value(), c, new_im);
		if (c == ST_ERR_OK) {
			cplx = Cplx(new_re, new_im);
		}
	}
}


//
// MATRIX
//

#ifdef DEBUG_CLASS_COUNT
long ReadMatrixCell::class_count = 0;
long CCMatrix::class_count = 0;
#endif	// DEBUG_CLASS_COUNT

  // Normal constructor
template<class Scalar> Matrix<Scalar>::Matrix(const dim_t& d, const int& l, const int& c, const Scalar& default_value) :
		dimension(d), nb_lines(l), nb_columns(c) {
#ifdef DEBUG_CLASS_COUNT
	ccm = new CCMatrix();
#endif
	for (int i = 0; i < nb_lines; i++)
		mat.push_back(new std::vector<Scalar>(nb_columns, default_value));
}

  // Copy-constructor
template<class Scalar> Matrix<Scalar>::Matrix(const Matrix<Scalar>& m) : dimension(m.dimension), nb_lines(m.nb_lines), nb_columns(m.nb_columns) {
#ifdef DEBUG_CLASS_COUNT
	ccm = new CCMatrix();
#endif
	for (int i = 0; i < nb_lines; i++)
		mat.push_back(new std::vector<Scalar>(*m.mat[i]));
}

  // Constructor from an input, stored in pm
template<class Scalar> Matrix<Scalar>::Matrix(const mat_read_t*& pm, const dim_t& d, const int& l, const int& c)
		: dimension(d), nb_lines(l), nb_columns(c) {
#ifdef DEBUG_CLASS_COUNT
	ccm = new CCMatrix();
#endif
	if (dimension == DIM_VECTOR && nb_lines != 1)
		throw(CalcFatal(__FILE__, __LINE__, "Matrix::Matrix(...), creation error, inconsistent dimensions"));
	Scalar sc;
	ReadMatrixCell* pe;
	Real* pr;
	Cplx* pc;
	for (int i = 0; i < nb_lines; i++)
		mat.push_back(new std::vector<Scalar>(nb_columns));
	for (int i = 0; i < nb_lines; i++) {
		for (int j = 0; j < nb_columns; j++) {
			if (static_cast<size_t>(j) > pm->at(i)->size() - 1)
				sc.zero();
			else {
				pe = &(pm->at(i)->at(j));
				if (pe->type == SCALAR_REAL) {
					pr = static_cast<Real*>(pe->s);
					sc = *pr;
					delete pr;
				} else if (pe->type == SCALAR_COMPLEX) {
					pc = static_cast<Cplx*>(pe->s);
					sc = *pc;
					delete pc;
				}
			}
			(*mat[i])[j] = sc;
		}
	}
	for (int i = 0; i < nb_lines; i++)
		if (mat[i]->size() != static_cast<size_t>(nb_columns))
			throw(CalcFatal(__FILE__, __LINE__, "inconsistent line sizes during matrix build"));
}

template<class Scalar> Matrix<Scalar>::~Matrix() {
#ifdef DEBUG_CLASS_COUNT
	delete ccm;
#endif
	for (int i = 0; i < nb_lines; i++)
		delete mat[i];
}

  // Multiplication or division by a scalar
template<class Scalar> st_err_t Matrix<Scalar>::md(void (*f)(const Scalar&, const Scalar&, st_err_t& c, Scalar&), const Scalar& sc) {
	st_err_t c = ST_ERR_OK;
	Scalar res;
	for (int i = 0; i < nb_lines; i++) {
		for (int j = 0; j < nb_columns; j++) {
			(*f)((*mat[i])[j], sc, c, res);
			if (c != ST_ERR_OK)
				break;
			(*mat[i])[j] = res;
		}
		if (c != ST_ERR_OK)
			break;
	}
	return c;
}

  // Addition or subtraction with another vector/matrix of same dimension
template<class Scalar> st_err_t Matrix<Scalar>::as(void (*f)(const Scalar&, const Scalar&, st_err_t&, Scalar&),
														Matrix<Scalar>& op) {
	if (nb_lines != op.nb_lines || nb_columns != op.nb_columns || dimension != op.dimension)
		return ST_ERR_INVALID_DIMENSION;
	st_err_t c = ST_ERR_OK;
	Scalar res;
	const std::vector< std::vector<Scalar>* >& m2 = op.mat;
	for (int i = 0; i < nb_lines; i++) {
		for (int j = 0; j < nb_columns; j++) {
			(*f)((*mat[i])[j], (*m2[i])[j], c, res);
			if (c != ST_ERR_OK)
				break;
			(*mat[i])[j] = res;
		}
		if (c != ST_ERR_OK)
			break;
	}
	return c;
}

  // Multiply a matrix with a vector or a matrix with a matrix
template<class Scalar> st_err_t Matrix<Scalar>::create_mul(const Matrix<Scalar> *multiplier, Matrix<Scalar>*& mres) const {
	st_err_t c = ST_ERR_OK;
	if (dimension == multiplier->dimension && dimension == DIM_MATRIX) {

// Case 1: matrix * matrix

		if (nb_columns == multiplier->nb_lines) {
			Scalar cell;
			Scalar total;
			mres = new Matrix<Scalar>(DIM_MATRIX, nb_lines, multiplier->nb_columns, Scalar(Real(0)));
			for (int i = 0; i < nb_lines; i++) {
				for (int j = 0; j < multiplier->nb_columns; j++) {
					total.zero();
					for (int cursor = 0; cursor < nb_columns; cursor++) {
						(*mat[i])[cursor].mul((*(multiplier->mat)[cursor])[j], c, cell);
						cell.add(total, c, total);
					}
					if (c != ST_ERR_OK)
						break;
					mres->set_value(i, j, total);
				}
				if (c != ST_ERR_OK)
					break;
			}
			if (c != ST_ERR_OK)
				delete mres;
		} else {
			c = ST_ERR_INVALID_DIMENSION;
		}
	} else if (dimension == DIM_MATRIX && multiplier->dimension == DIM_VECTOR) {

// Case 2: matrix * vector

		if (nb_columns == multiplier->nb_columns) {
			Scalar cell;
			Scalar total;
			mres = new Matrix<Scalar>(DIM_VECTOR, 1, nb_lines, Scalar(Real(0)));
			for (int i = 0; i < nb_lines; i++) {
				total.zero();
				for (int cursor = 0; cursor < nb_columns; cursor++) {
					(*mat[i])[cursor].mul((*(multiplier->mat)[0])[cursor], c, cell);
					cell.add(total, c, total);
				}
				if (c != ST_ERR_OK)
					break;
				mres->set_value(0, i, total);
			}
			if (c != ST_ERR_OK)
				delete mres;
		} else {
			c = ST_ERR_INVALID_DIMENSION;
		}
	} else
		c = ST_ERR_INVALID_DIMENSION;
	return c;
}

  // Output a matrix (or vector) using debug_write() function
#ifdef DEBUG
template<class Scalar> void debug_matrix_to_string(Matrix<Scalar> *pmat, int *reorder) {
	string s;
	if (pmat->get_dimension() == DIM_MATRIX)
		s = "[";
	for (int i = 0; i < pmat->get_nb_lines(); i++) {
		int i_reordered = i;
		if (pmat->get_dimension() == DIM_MATRIX)
			i_reordered = reorder[i];
		if (i_reordered == 0)
			s.append("[ ");
		else
			s.append(" [ ");
		for (int j = 0; j < pmat->get_nb_columns(); j++) {
			int j_reordered = j;
			if (pmat->get_dimension() == DIM_VECTOR)
				j_reordered = reorder[j];
			s.append(pmat->get_value(i_reordered, j_reordered).to_string(TOSTRING_DISPLAY));
			if (j_reordered != pmat->get_nb_columns() - 1)
				s.append(" ");
		}
		s.append(" ]");
		if (i_reordered < pmat->get_nb_lines() - 1) {
			debug_write(s.c_str());
			s = "";
		}
	}
	if (pmat->get_dimension() == DIM_MATRIX)
		s.append("]");
	debug_write(s.c_str());
}
#endif

  //
  // * ************************ *
  // * Linear system resolution *
  // * ************************ *
  //
  // (= a vector divided by a matrix)
  // Done in two steps.
  //
  // STEP 1: go downward and rightward in the matrix, starting from the top left corner,
  //         so as to zero the left column starting at the top left corner of the actual
  //         sub-matrix.
  //         At the end of step 1, the bottom left triangle of the matrix is zeroed.
  //
  // STEP 2: go upward to calculate the solutions one by one.
  //
  // Details about the algorithm
  //
  // If we are here at the i-nth iteration:
  //
  //   +------------------------------ ...       | v(1)
  //   | ...                           ...       | ...
  //   | ... +------------------------ ...       | v(i-1)
  //   | ... |m(i,i)   m(i,i+1)   m(i,i+2)   ... | v(i)
  //   | ... |m(i+1,i) m(i+1,i+1) m(i+1,i+2) ... | v(i+1)
  //   | ... |...                            ... | ...
  //
  // Then for j taking all values in [i+1,n] and k taking all values in [i, n] (n being the number of
  // lines = number of columns of the matrix), we do: m(j,k) := m(j,k)-m(i,k)*m(j,i)/m(i,i)
  // This is a safe transformation as it consists in adding the line i to the lines j, with the
  // coefficient m(j,i)/m(i,i). Obviously the same needs to be done in the values at the right:
  // v(j) := v(j)-v(i)*m(j,i)/m(i,i).
  // For the column i itself (when k == i), it results in the special case:
  //     m(j,i) := m(j,i)-m(i,i)*m(j,i)/m(i,i) := m(j,i)-m(j,i) := 0.
  // This way, the i column below the i line (the values m(j,i)) is all set to zero. Once done,
  // the same transformation can be done one line below and obviously one column rigthward as the
  // left value is null. At the end, all values in the bottom left rectangle (all m(j,i) with j>i)
  // are equal to zero.
  //
  // Once done, the solution s(n) can be calculated immediately (s(n) := v(n)/m(n,n)) as all other
  // values of the n-th line are equal to zero. Once calculated, v(n-1) can be calculated as well,
  // and so on and so forth during STEP 2.
  //
  // The array reorder[] is necessary to switch lines of the matrix if need be - it is necessary
  // if a value m(i,i) happens to be equal to zero.
  // There can be situations where no switch is possible - if all values of a column happen to
  // be null at some point. It means a value is not determined by the system. This algorithm
  // doesn't deal with such a situation and simply returns an error (infinite result.) Note that
  // would 59 flag be unset (59 FS? returns 0), a positive real divided by zero will return
  // MAXR, a negative real divided by zero will return -MAXR, and no error will be returned.
  //
template<class Scalar> st_err_t Matrix<Scalar>::create_div(const Matrix<Scalar> *divis, Matrix<Scalar>*& mres) const {
	debug_write("Matrix<Scalar>::create_div()");
	st_err_t c = ST_ERR_OK;

	if (dimension == DIM_VECTOR && divis->dimension == DIM_MATRIX && nb_columns == divis->nb_columns &&
			divis->nb_columns == divis->nb_lines) {
		int n = nb_columns;
		debug_write_i("Matrix<Scalar>::create_div(): n = %i", n);
		int *reorder;
		reorder = new int[n];
		for (int i = 0; i < n; i++)
			reorder[i] = i;

		Matrix<Scalar> *mm = new Matrix<Scalar>(divis->dimension, divis->nb_lines, divis->nb_columns, Scalar(Real(0)));
		mm->copy_linear(divis);
		Matrix<Scalar> *vv = new Matrix<Scalar>(dimension, nb_lines, nb_columns, Scalar(Real(0)));
		vv->copy_linear(this);

		int ii = -1;

// STEP 1: go down the matrix to zero the column below the top left corner, column by column

		for (int go_down = 0; go_down < n - 1; go_down++) {
			real t;
			real best = 10;
			for (int i = go_down; i < n; i++) {
				t = (*(mm->mat)[i])[go_down].get_mantisse();
				if (t < best && t > 0) {
					best = t;
					ii = i;
				}
			}
			if (ii < 0)
				ii = 0;
			int temporary_integer = reorder[ii];
			reorder[ii] = reorder[go_down];
			reorder[go_down] = temporary_integer;

#ifdef DEBUG
			debug_write("mm:");
			debug_matrix_to_string(mm, reorder);
			debug_write("vv:");
			debug_matrix_to_string(vv, reorder);
#endif // DEBUG

			Scalar alpha, beta, top;
			beta = (*(mm->mat)[reorder[go_down]])[go_down];

			debug_write("beta = ");
			debug_write(beta.to_string(TOSTRING_PORTABLE).c_str());

			for (int i = go_down + 1; i < n && c == ST_ERR_OK; i++) {
				alpha = (*(mm->mat)[reorder[i]])[go_down];

				debug_write_i("i = %i", i);
				debug_write("alpha = ");
				debug_write(alpha.to_string(TOSTRING_PORTABLE).c_str());

				(*(mm->mat)[reorder[i]])[go_down] = Scalar(Real(0));

				  // Modify each cell of the matrix
				for (int j = go_down + 1; j < n && c == ST_ERR_OK; j++) {
					top = (*(mm->mat)[reorder[go_down]])[j];

					debug_write("top(1):");
					debug_write(top.to_string(TOSTRING_PORTABLE).c_str());

					top.mul(alpha, c, top);
					if (c == ST_ERR_OK) {

						debug_write("top(2):");
						debug_write(top.to_string(TOSTRING_PORTABLE).c_str());

						top.div(beta, c, top);
						if (c == ST_ERR_OK) {

							debug_write("top(3):");
							debug_write(top.to_string(TOSTRING_PORTABLE).c_str());

							(*(mm->mat)[reorder[i]])[j].sub(top, c, (*(mm->mat)[reorder[i]])[j]);
						}
					}
				}

				if (c == ST_ERR_OK) {
					  // Modify the value of the vector the same way as cells done just before
					top = (*(vv->mat)[0])[reorder[go_down]];
					top.mul(alpha, c, top);
					top.div(beta, c, top);
					(*(vv->mat)[0])[reorder[i]].sub(top, c, (*(vv->mat)[0])[reorder[i]]);
				}

#ifdef DEBUG
				debug_write("mm:");
				debug_matrix_to_string(mm, reorder);
				debug_write("vv:");
				debug_matrix_to_string(vv, reorder);
#endif // DEBUG

			}
		}

// STEP 2: calculate solutions, going upward in the matrix

		Scalar t, m;
		for (int go_up = n - 1; go_up >= 0 && c == ST_ERR_OK; go_up--) {
			t = (*(vv->mat)[0])[reorder[go_up]];
			for (int i = go_up + 1; i < n; i++) {
				m = (*(mm->mat)[reorder[go_up]])[i];
				m.mul((*(vv->mat)[0])[reorder[i]], c, m);
				t.sub(m, c, t);
			}
			t.div((*(mm->mat)[reorder[go_up]])[go_up], c, (*(vv->mat)[0])[reorder[go_up]]);
		}

		if (c == ST_ERR_OK) {
			mres = new Matrix<Scalar>(DIM_VECTOR, 1, n, Scalar(Real(0)));
			  // Re-order elements
			for (int i = 0; i < n; i++)
				(*(mres->mat)[0])[i] = (*(vv->mat)[0])[reorder[i]];

		}
		delete vv;
		delete mm;
		delete []reorder;
	} else
		c = ST_ERR_INVALID_DIMENSION;
	return c;
}

template<class Scalar> st_err_t Matrix<Scalar>::create_cross(const Matrix<Scalar>* rv, Matrix<Scalar>*& mres) const {
	st_err_t c = ST_ERR_OK;
	if (dimension == DIM_VECTOR && rv->dimension == DIM_VECTOR && nb_columns == 3 &&
			rv->nb_columns == 3 && nb_lines == 1 && rv->nb_lines == 1) {
		mres = new Matrix<Scalar>(DIM_VECTOR, 1, 3, Scalar(Real(0)));
		Scalar s1, s2, s3;
		  // (AxB)(1) = A(2)*B(3) - A(3)*B(2)
		get_value(0, 1).mul(rv->get_value(0, 2), c, s1);
		get_value(0, 2).mul(rv->get_value(0, 1), c, s2);
		s1.sub(s2, c, s3);
		mres->set_value(0, 0, s3);
		  // (AxB)(2) = A(3)*B(1) - A(1)*B(3)
		get_value(0, 2).mul(rv->get_value(0, 0), c, s1);
		get_value(0, 0).mul(rv->get_value(0, 2), c, s2);
		s1.sub(s2, c, s3);
		mres->set_value(0, 1, s3);
		  // (AxB)(3) = A(1)*B(2) - A(2)*B(1)
		get_value(0, 0).mul(rv->get_value(0, 1), c, s1);
		get_value(0, 1).mul(rv->get_value(0, 0), c, s2);
		s1.sub(s2, c, s3);
		mres->set_value(0, 2, s3);
	} else
		c = ST_ERR_INVALID_DIMENSION;
	return c;
}

template<class Scalar> st_err_t Matrix<Scalar>::dot(const Matrix<Scalar>* rv, Scalar& dot) const {
	st_err_t c = ST_ERR_OK;
	if (dimension == DIM_VECTOR && rv->dimension == DIM_VECTOR &&
			nb_columns == rv->nb_columns && nb_lines == 1 && rv->nb_lines == 1) {
		dot = Scalar(Real(0));
		Scalar s, t;
		for (int i = 0; i < nb_columns; i++) {
			get_value(0, i).mul(rv->get_value(0, i), c, s);
			dot.add(s, c, t);
			dot = t;
		}
	} else
		c = ST_ERR_INVALID_DIMENSION;
	return c;
}

template<class Scalar> st_err_t Matrix<Scalar>::abs(Real& res) const {
	st_err_t c = ST_ERR_OK;
	res = Real(0);
	Real tmp, tmp2;
	for (int i = 0; i < nb_lines; i++) {
		for (int j = 0; j < nb_columns; j++) {
			get_value(i, j).square_of_abs(c, tmp);
			res.add(tmp, c, tmp2);
			res = tmp2;
		}
	}
	Cplx cplx;
	res.sqr(c, cplx);
	res = Real(cplx.get_re());
	return c;
}

template<class Scalar> void Matrix<Scalar>::trim() {
	for (int i = 0; i < nb_lines; i++)
		for (int j = 0; j < nb_columns; j++)
			(*mat[i])[j].trim();
}

template<class Scalar> void Matrix<Scalar>::redim(const dim_t& new_dimension, const int& new_nb_lines, const int& new_nb_columns) {

	//debug_write_i("new dimension = %i", (int)new_dimension);
	//debug_write_i("new lines = %i", new_nb_lines);
	//debug_write_i("new columns = %i", new_nb_columns);

	dimension = new_dimension;
	for (int i = new_nb_lines; i < nb_lines; i++) {
		//debug_write_i("Deleting %i", (int)i);
		delete mat[i];
	}
	if (new_nb_lines < nb_lines)
		mat.resize(new_nb_lines);
	for (int i = nb_lines; i < new_nb_lines; i++)
		mat.push_back(new vector<Scalar>(new_nb_columns));
	for (int i = 0; i < new_nb_lines; i++)
		(*mat[i]).resize(new_nb_columns);
	nb_lines = new_nb_lines;
	nb_columns = new_nb_columns;
	//debug_write_i("vector new lines = %i", (int)mat.size());
	//debug_write_i("vector new columns = %i", (int)((*mat[0]).size()));
}

template<class Scalar> bool Matrix<Scalar>::index_to_ij(const int& index, int& i, int& j) const {
	i = index / nb_columns;
	j = index % nb_columns;
	//debug_write_i("index = %i", index);
	//debug_write_i("line = %i", i);
	//debug_write_i("column = %i", j);
	return (i >= 0 && i < nb_lines && j >= 0 && j < nb_columns);
}

template<class Scalar> void Matrix<Scalar>::copy_linear(const Matrix<Scalar>* orig) {
	int idx = 0;
	int orig_i, orig_j;
	for (int i = 0; i < nb_lines; i++) {
		for (int j = 0; j < nb_columns; j++) {
			if (orig->index_to_ij(idx, orig_i, orig_j)) {
				set_value(i, j, orig->get_value(orig_i, orig_j));
			}
			idx++;
		}
	}
}

template<class Scalar> st_err_t Matrix<Scalar>::create_transpose(Matrix<Scalar>*& ret) {
	if (dimension != DIM_MATRIX)
		return ST_ERR_INVALID_DIMENSION;
	ret = new Matrix<Scalar>(DIM_MATRIX, nb_columns, nb_lines, Scalar(Real(0)));
	Scalar sc;
	for (int i = 0; i < nb_lines; i++) {
		for (int j = 0; j < nb_columns; j++) {
			sc = get_value(i, j);
			sc.conj();
			ret->set_value(j, i, sc);
		}
	}
	return ST_ERR_OK;
}

template<class Scalar> st_err_t Matrix<Scalar>::create_neg(Matrix<Scalar>*& ret) {
	ret = new Matrix<Scalar>(dimension, nb_lines, nb_columns, Scalar(Real(0)));
	Scalar sc;
	for (int i = 0; i < nb_lines; i++) {
		for (int j = 0; j < nb_columns; j++) {
			sc = get_value(i, j);
			sc.neg();
			ret->set_value(i, j, sc);
		}
	}
	return ST_ERR_OK;
}

template<class Scalar> int Matrix<Scalar>::cmp(const Matrix<Scalar>& op) const {
	if (nb_lines != op.nb_lines || nb_columns != op.nb_columns || dimension != op.dimension)
		return 1;
	const std::vector< std::vector<Scalar>* >& m2 = op.mat;
	for (int i = 0; i < nb_lines; i++) {
		for (int j = 0; j < nb_columns; j++) {
			if ((*mat[i])[j].cmp((*m2[i])[j]) != 0)
				return 1;
		}
	}
	return 0;
}

//st_err_t mat_r_to_c(Matrix<Real> *m_re, Matrix<Cplx> *m_im, Matrix<Cplx>*& mres) {
//	return ST_ERR_INTERNAL;
//}

template class Matrix<Real>;
template class Matrix<Cplx>;

st_err_t mat_r_to_c(Matrix<Real> *m_re, Matrix<Real> *m_im, Matrix<Cplx>*& m_cplx) {
	if (m_re->get_dimension() != m_im->get_dimension() ||
			m_re->get_nb_lines() != m_im->get_nb_lines() ||
			m_re->get_nb_columns() != m_im->get_nb_columns())
		return ST_ERR_INVALID_DIMENSION;
	m_cplx = new Matrix<Cplx>(m_re->get_dimension(), m_re->get_nb_lines(), m_re->get_nb_columns(), Cplx(0, 0));
	for (int i = 0; i < m_re->get_nb_lines(); i++)
		for (int j = 0; j < m_re->get_nb_columns(); j++)
			m_cplx->set_value(i, j, Cplx(m_re->get_value(i, j).get_value(), m_im->get_value(i, j).get_value()));
	return ST_ERR_OK;
}

void mat_c_to_r(Matrix<Cplx> *m_cplx, Matrix<Real>*& m_re, Matrix<Real>*& m_im) {
	m_re = new Matrix<Real>(m_cplx->get_dimension(), m_cplx->get_nb_lines(), m_cplx->get_nb_columns(), Real(0));
	m_im = new Matrix<Real>(m_cplx->get_dimension(), m_cplx->get_nb_lines(), m_cplx->get_nb_columns(), Real(0));
	for (int i = 0; i < m_cplx->get_nb_lines(); i++) {
		for (int j = 0; j < m_cplx->get_nb_columns(); j++) {
			m_re->set_value(i, j, Real(m_cplx->get_value(i, j).get_re()));
			m_im->set_value(i, j, Real(m_cplx->get_value(i, j).get_im()));
		}
	}
}

#define IMPLEMENT_MAT_C_TO_RE_OR_IM(OP) \
void mat_c_to_##OP(Matrix<Cplx>* m_cplx, Matrix<Real>*& m) { \
	m = new Matrix<Real>(m_cplx->get_dimension(), m_cplx->get_nb_lines(), m_cplx->get_nb_columns(), Real(0)); \
	for (int i = 0; i < m_cplx->get_nb_lines(); i++) \
		for (int j = 0; j < m_cplx->get_nb_columns(); j++) \
			m->set_value(i, j, Real(m_cplx->get_value(i, j).get_##OP())); \
}
IMPLEMENT_MAT_C_TO_RE_OR_IM(re)
IMPLEMENT_MAT_C_TO_RE_OR_IM(im)

void mat_c_conj(Matrix<Cplx>* m_cplx, Matrix<Cplx>*& m_conj) {
	m_conj = new Matrix<Cplx>(m_cplx->get_dimension(), m_cplx->get_nb_lines(), m_cplx->get_nb_columns(), Cplx(0, 0));
	Cplx cplx;
	for (int i = 0; i < m_cplx->get_nb_lines(); i++) {
		for (int j = 0; j < m_cplx->get_nb_columns(); j++) {
			cplx = m_cplx->get_value(i, j);
			cplx.conj();
			m_conj->set_value(i, j, cplx);
		}
	}
}

