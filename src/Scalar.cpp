// Scalar.cpp
// Stack implementation and StackItems

// RPN calculator

// Sébastien Millet
// August-September 2009

#include "Common.h"
#include "Scalar.h"
#include <sstream>
#include <cerrno>
#include <cmath>

using namespace std;

extern const real MINR = 1e-199;
extern const real MAXR = 9.99999999999e199;


// General functions

void prepare_arith() { errno = 0; }

real real_trim(const real& r) {
	real r2;
	string s = real_to_string(r, get_decimal_separator(false));
	if (s == "-0")
		s = "0";

	//debug_write("s = ");
	//debug_write(s.c_str());

	istringstream iss(s);
	iss >> r2;
	return r2;
}

const string real_to_string(const real& r, const char& ds) {
	ostringstream o;
	o.precision(12);
	o << r;
	string s = o.str();
	size_t p;
	if ((p = s.find_first_of("eE")) != string::npos) {
		if (s.substr(p + 1, 1) == "+")
			s.erase(p + 1, 1);
	}
	if (ds != '.') {
		size_t p = s.find_first_of('.');
		if (p != string::npos)
			s[p] = ds;
	}
	return s;
}

st_err_t real_check_bounds(const bool& can_be_zero, const int& sign, real& r, const bool& force_strict) {
	enum {VALID, OVERFLOW_NEG, OVERFLOW_POS, UNDERFLOW_NEG, UNDERFLOW_POS} status = VALID;
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
			if (flags[FL_OVERFLOW_OK].value || force_strict)
				return ST_ERR_OVERFLOW;
			else
				r = -MAXR;
			break;
		case OVERFLOW_POS:
			if (flags[FL_OVERFLOW_OK].value || force_strict)
				return ST_ERR_OVERFLOW;
			else
				r = MAXR;
			break;
		case UNDERFLOW_NEG:
			if (flags[FL_UNDERFLOW_OK].value || force_strict)
				return ST_ERR_UNDERFLOW_NEG;
			else
				r = 0;
			break;
		case UNDERFLOW_POS:
			if (flags[FL_UNDERFLOW_OK].value || force_strict)
				return ST_ERR_UNDERFLOW_POS;
			else
				r = 0;
			break;
		default:
			return ST_ERR_INTERNAL;
	}
	return ST_ERR_OK;
}

int real_sign(const real& r) {
	return (r > 0 ? 1 : (r < 0 ? -1 : 0));
}

real real_ip(const real& r) {
	if (r < 0)
		return real(ceil(r));
	else
		return real(floor(r));
}

real get_max_real_from_bin_size(const int& m) {
	return exp((real)m * log(2));
}


// Arithmetic functions

void numeric_add(const real& r1, const real& r2, st_err_t& c, real& res) {
	int s1 = real_sign(r1);
	int s2 = real_sign(r2);
	int s;
	if (c != ST_ERR_OK)
		return;
	if (s1 == s2)
		s = s1;
	else if (s1 < 0)
		s = (r1 <= -r2 ? -1 : s2);
	else if (s1 > 0)
		s = (r1 >= -r2 ? 1 : s2);
	else
		s = s2;
	res = r1 + r2;
	c = real_check_bounds(true, s, res);
}

void numeric_sub(const real& r1, const real& r2, st_err_t& c, real& res) {
	int s1 = real_sign(r1);
	int s2 = -real_sign(r2);
	int s;
	if (c != ST_ERR_OK)
		return;
	if (s1 == s2)
		s = s1;
	else if (s1 < 0)
		s = (r1 <= r2 ? -1 : s2);
	else if (s1 > 0)
		s = (r1 >= r2 ? 1 : s2);
	else
		s = s2;
	res = r1 - r2;
	c = real_check_bounds(true, s, res);
}

void numeric_mul(const real& r1, const real& r2, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	res = r1 * r2;
	c = real_check_bounds(r1 == 0 || r2 == 0, real_sign(r1) * real_sign(r2), res);
}

void numeric_div(const real& r1, const real& r2, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	if (r2 == 0) {
		if (r1 == 0)
			c = ST_ERR_UNDEFINED_RESULT;
		else if (flags[FL_INFINITE].value)
			c = ST_ERR_INFINITE_RESULT;
		else {
			res = (real_sign(r1) < 0 ? -MAXR : MAXR);
			c = ST_ERR_OK;
		}
	} else {
		res = r1 / r2;
		c = real_check_bounds(r1 == 0, real_sign(r1) * real_sign(r2), res);
	}
}

void numeric_ln(const real& r, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	if (r < 0)
		c = ST_ERR_BAD_ARGUMENT_VALUE;
	else if (r == 0) {
		c = ST_ERR_OK;
		res = -MAXR;
		if (flags[FL_INFINITE].value)
			c = ST_ERR_INFINITE_RESULT;
	} else {
		res = log(r);
		c = real_check_bounds(true, res < 0 ? -1 : 1, res);
	}
}

void numeric_exp(const real& r, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	res = exp(r);
	if (errno && r < 0) {
		res = 0;
		errno = 0;
	}
	c = real_check_bounds(false, 1, res);
}

void numeric_pow(const real& r1, const real& r2, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	real res2;
	  // a^b = exp(b * ln(a))
	numeric_ln(r1, c, res);
	numeric_mul(r2, res, c, res2);
	numeric_exp(res2, c, res);

}

void numeric_cos(const real& r, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	res = cos(r);
}

void numeric_acos(const real& r, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	res = acos(r);
	if (errno == ERANGE)
		c = ST_ERR_BAD_ARGUMENT_VALUE;
}

void numeric_sin(const real& r, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	res = sin(r);
}

void numeric_asin(const real& r, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	res = asin(r);
	if (errno == ERANGE)
		c = ST_ERR_BAD_ARGUMENT_VALUE;
}

void numeric_tan(const real& r, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	res = tan(r);
	c = real_check_bounds(true, res >= 0 ? 1 : -1, res);
}

void numeric_atan(const real& r, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	res = atan(r);
	if (errno == ERANGE)
		c = ST_ERR_BAD_ARGUMENT_VALUE;
}

real numeric_abs(const real& r) {
	return abs(r);
}

void numeric_sqrt(const real& r, st_err_t& c, real& res) {
	if (c != ST_ERR_OK)
		return;
	if (r < 0)
		c = ST_ERR_BAD_ARGUMENT_VALUE;
	else {
		res = sqrt(r);
		c = real_check_bounds(r == 0, 1, res);
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
		return '0' + i;
	else if (i <= 15)
		return 'A' + i - 10;
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
// REAL
//

#ifdef DEBUG_CLASS_COUNT
long Real::class_count = 0;
#endif

const string Real::to_string(const tostring_t& t) const {
	return real_to_string(r, get_decimal_separator(t != TOSTRING_PORTABLE));
}

Real& Real::operator=(const Cplx& rv) {
	r = rv.get_re();
	return *this;
}

void Real::trim() { r = real_trim(r); }

int Real::cmp(const Real& rv) const { return r < rv.r ? -1 : (r > rv.r ? 1 : 0); }

st_err_t Real_arith(void (*f)(const real&, const real&, st_err_t&, real&), const Real& lv, const Real& rv, Real& res) {
	st_err_t c = ST_ERR_OK;
	(*f)(lv.r, rv.r, c, res.r);
	return c;
}

st_err_t Real_add(const Real& lv, const Real& rv, Real& res) { return Real_arith(numeric_add, lv, rv, res); }
st_err_t Real_sub(const Real& lv, const Real& rv, Real& res) { return Real_arith(numeric_sub, lv, rv, res); }
st_err_t Real_mul(const Real& lv, const Real& rv, Real& res) { return Real_arith(numeric_mul, lv, rv, res); }
st_err_t Real_div(const Real& lv, const Real& rv, Real& res) { return Real_arith(numeric_div, lv, rv, res); }
st_err_t Real_pow(const Real& lv, const Real& rv, Real& res) { return Real_arith(numeric_pow, lv, rv, res); }

st_err_t Real::add(const Real& rv, Real& res) { return Real_add(*this, rv, res); }
st_err_t Real::sub(const Real& rv, Real& res) { return Real_sub(*this, rv, res); }
st_err_t Real::mul(const Real& rv, Real& res) { return Real_mul(*this, rv, res); }
st_err_t Real::div(const Real& rv, Real& res) { return Real_div(*this, rv, res); }
st_err_t Real::pow(const Real& rv, Real& res) { return Real_pow(*this, rv, res); }


//
// CPLX
//

#ifdef DEBUG_CLASS_COUNT
long Cplx::class_count = 0;
#endif

const string Cplx::to_string(const tostring_t& t) const {
	const char ds = get_decimal_separator(t != TOSTRING_PORTABLE);
	return "(" + real_to_string(re, ds) + get_complex_separator(t != TOSTRING_PORTABLE) + real_to_string(im, ds) + ")";
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

st_err_t Cplx_add(const Cplx& lv, const Cplx& rv, Cplx& res) {
	st_err_t c = ST_ERR_OK;
	  // Calculate real part
	numeric_add(lv.re, rv.re, c, res.re);
	numeric_add(lv.im, rv.im, c, res.im);
	return c;
}

st_err_t Cplx_sub(const Cplx& lv, const Cplx& rv, Cplx& res) {
	st_err_t c = ST_ERR_OK;
	  // Calculate real part
	numeric_sub(lv.re, rv.re, c, res.re);
	numeric_sub(lv.im, rv.im, c, res.im);
	return c;
}

st_err_t Cplx_mul(const Cplx& lv, const Cplx& rv, Cplx& res) {
	real x;
	real y = 0;
	st_err_t c = ST_ERR_OK;
	  // Calculate real part
	numeric_mul(lv.re, rv.re, c, x);
	numeric_mul(lv.im, rv.im, c, y);
	numeric_sub(x, y, c, res.re);
	  // Calculate imaginary part
	numeric_mul(lv.im, rv.re, c, x);
	numeric_mul(lv.re, rv.im, c, y);
	numeric_add(x, y, c, res.im);
	return c;
}

st_err_t Cplx_div(const Cplx& lv, const Cplx& rv, Cplx& res) {
	real x = 0, y = 0, d = 0, t = 0, u = 0, v = 0;
	st_err_t c = ST_ERR_OK;

	if (rv.re == 0 && rv.im == 0 && (lv.re != 0 || lv.im != 0)) {
		if (flags[FL_INFINITE].value)
			return ST_ERR_INFINITE_RESULT;
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
		return ST_ERR_OK;
	}

	  // Complex division: x = (ac + bd)/(a^2 + b^2), y = (ad - bc)/(a^2 + b^2)
	numeric_mul(rv.re, rv.re, c, x);
	numeric_mul(rv.im, rv.im, c, y);
	numeric_add(x, y, c, d);
	  // Calculate real part
	numeric_mul(rv.re, lv.re, c, t);
	numeric_mul(rv.im, lv.im, c, u);
	numeric_add(t, u, c, v);
	numeric_div(v, d, c, res.re);
	  // Calculate imaginary part
	numeric_mul(rv.re, lv.im, c, t);
	numeric_mul(rv.im, lv.re, c, u);
	numeric_sub(t, u, c, v);
	numeric_div(v, d, c, res.im);
	return c;
}

//st_err_t Real_mul_by_Cplx(const Real& lv, const Cplx& rv, Cplx& res) { return Cplx_mul(Cplx(lv), rv, res); }
//st_err_t Real_div_by_Cplx(const Real& lv, const Cplx& rv, Cplx& res) { return Cplx_div(Cplx(lv), rv, res); }

st_err_t Cplx::add(const Cplx& rv, Cplx& res) { return Cplx_add(*this, rv, res); }
st_err_t Cplx::sub(const Cplx& rv, Cplx& res) { return Cplx_sub(*this, rv, res); }
st_err_t Cplx::mul(const Cplx& rv, Cplx& res) { return Cplx_mul(*this, rv, res); }
st_err_t Cplx::div(const Cplx& rv, Cplx& res) { return Cplx_div(*this, rv, res); }


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
			if (j > pm->at(i)->size() - 1)
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
		if (mat[i]->size() != nb_columns)
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
template<class Scalar> st_err_t Matrix<Scalar>::md(st_err_t (*f)(const Scalar&, const Scalar&, Scalar&), const Scalar& sc) {
	st_err_t c = ST_ERR_OK;
	Scalar res;
	for (int i = 0; i < nb_lines; i++) {
		for (int j = 0; j < nb_columns; j++) {
			if ((c = (*f)((*mat[i])[j], sc, res)) != ST_ERR_OK)
				break;
			(*mat[i])[j] = res;
		}
		if (c != ST_ERR_OK)
			break;
	}
	return c;
}

  // Addition or subtraction with another vector/matrix of same dimension
template<class Scalar> st_err_t Matrix<Scalar>::as(st_err_t (*f)(const Scalar&, const Scalar&, Scalar&),
														Matrix<Scalar>& op) {
	if (nb_lines != op.nb_lines || nb_columns != op.nb_columns || dimension != op.dimension)
		return ST_ERR_INVALID_DIMENSION;
	st_err_t c = ST_ERR_OK;
	Scalar res;
	const std::vector< std::vector<Scalar>* >& m2 = op.mat;
	for (int i = 0; i < nb_lines; i++) {
		for (int j = 0; j < nb_columns; j++) {
			if ((c = (*f)((*mat[i])[j], (*m2[i])[j], res)) != ST_ERR_OK)
				break;
			(*mat[i])[j] = res;
		}
		if (c != ST_ERR_OK)
			break;
	}
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

template<class Scalar> bool Matrix<Scalar>::index_to_ij(const int& index, int& i, int& j) {
	i = index / nb_columns;
	j = index % nb_columns;
	//debug_write_i("index = %i", index);
	//debug_write_i("line = %i", i);
	//debug_write_i("column = %i", j);
	return (i >= 0 && i < nb_lines && j >= 0 && j < nb_columns);
}

template<class Scalar> void Matrix<Scalar>::copy_linear(Matrix<Scalar>* orig) {
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

template class Matrix<Real>;
template class Matrix<Cplx>;

