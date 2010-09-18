// Scalar.h
// Arithmetic and objects to deal with real and complex numbers

// RPN calculator

// Sébastien Millet
// August-November 2009

#ifndef SCALAR_H
#define SCALAR_H

#include "Common.h"
#include <string>
#include <bitset>
#include <vector>

typedef enum {SCALAR_UNDEF, SCALAR_REAL, SCALAR_COMPLEX} scalar_t;
typedef enum {DIM_VECTOR, DIM_MATRIX} dim_t;

  // General functions
real real_trim(const real&);
const std::string real_to_string(const real&);
st_err_t real_check_bounds(const bool&, const int&, real&, const bool& = false);
int real_sign(const real&);
real real_ip(const real&);
real get_max_real_from_bin_size(const int&);
int digit_to_int(const char&);
char int_to_digit(const int&);
void remove_leading_zeros(std::string&);

  // Arithmetic
void numeric_add(const real&, const real&, st_err_t&, real&);
void numeric_sub(const real&, const real&, st_err_t&, real&);
void numeric_mul(const real&, const real&, st_err_t&, real&);
void numeric_div(const real&, const real&, st_err_t&, real&);
void numeric_ln(const real&, st_err_t&, real&);
void numeric_exp(const real&, st_err_t&, real&);
void numeric_pow(const real&, st_err_t&, real&);
void numeric_cos(const real&, st_err_t&, real&);
void numeric_sin(const real&, st_err_t&, real&);
void numeric_tan(const real&, st_err_t&, real&);
void numeric_acos(const real&, st_err_t&, real&);
void numeric_asin(const real&, st_err_t&, real&);
void numeric_atan(const real&, st_err_t&, real&);
void numeric_sqrt(const real&, st_err_t&, real&);

real numeric_abs(const real&);


//
// Coordinates
//

struct Coordinates {
	dim_t d;
	int i;
	int j;
};


//
// Binary
//

extern int g_max_nb_bits;

class Binary {

#ifdef DEBUG_CLASS_COUNT
	static long class_count;
#endif

	std::bitset<G_HARD_MAX_NB_BITS> bits;
public:
	friend st_err_t Binary_add(const Binary&, const Binary&, Binary&);
	friend st_err_t Binary_sub(const Binary&, const Binary&, Binary&);
	Binary();
	Binary(const Binary&);
	Binary(const std::bitset<G_HARD_MAX_NB_BITS>);
	~Binary();

#ifdef DEBUG_CLASS_COUNT
	static int get_class_count() { return class_count; }
#endif

	void set(std::bitset<G_HARD_MAX_NB_BITS>);
	const std::string to_string(const int&, const int&) const;

	real to_real(const int&) const;
	bool get_bit(const int&) const;
	int cmp(const Binary&) const;

#ifdef DEBUG
	void debug_cout_litteral(const std::string& prefix) const {
		std::cout << prefix;
		for (int i = 0; i < g_max_nb_bits; i++)
			std::cout << (bits.test(i) ? "1" : "0");
		std::cout << std::endl;
	}
#endif

};

st_err_t Binary_add(const Binary&, const Binary&, Binary&);
st_err_t Binary_sub(const Binary&, const Binary&, Binary&);


//
// SCAL
//

class Scal {
};

//
// REAL
//

class Cplx;
class Real : public Scal {

#ifdef DEBUG_CLASS_COUNT
	static long class_count;
#endif

	real r;
public:
	friend st_err_t Real_arith(void (*f)(const real&, const real&, st_err_t&, real&), const Real&, const Real&, Real&);

	Real() : r(0) {
		//std::cout << "Creation of a REAL\n";
#ifdef DEBUG_CLASS_COUNT
		class_count++;
#endif
	}

	explicit Real(const real& rr) : r(rr) {
		//std::cout << "Creation of a REAL\n";
#ifdef DEBUG_CLASS_COUNT
		class_count++;
#endif
	}

	  // Copy-constructor
	Real(const Real& re) : r(re.r) {
		//std::cout << "Copy-Creation of a REAL\n";
#ifdef DEBUG_CLASS_COUNT
		class_count++;
#endif
 	}

	~Real() {
		//std::cout << "Destruction of a REAL\n";
#ifdef DEBUG_CLASS_COUNT
		class_count--;
#endif
	}

#ifdef DEBUG_CLASS_COUNT
	static int get_class_count() { return class_count; }
#endif

	Real& operator=(const Real& rv) { r = rv.r; return *this; }
	Real& operator=(const Cplx&);
	real get_value() const { return r; }
	void trim();
	void neg() { r = (r == 0 ? 0 : -r); }
	void conj() { }
	int cmp(const Real&) const;
	st_err_t add(const Real&, Real&);
	st_err_t sub(const Real&, Real&);
	st_err_t mul(const Real&, Real&);
	st_err_t div(const Real&, Real&);
	st_err_t pow(const Real&, Real&);
	void zero() { r = 0; }
	const std::string to_string(const tostring_t&) const;
};

st_err_t Real_arith(void (*f)(const real&, const real&, st_err_t&, real&), const Real&, const Real&, Real&);
st_err_t Real_add(const Real&, const Real&, Real&);
st_err_t Real_sub(const Real&, const Real&, Real&);
st_err_t Real_mul(const Real&, const Real&, Real&);
st_err_t Real_div(const Real&, const Real&, Real&);
st_err_t Real_pow(const Real&, const Real&, Real&);


//
// CPLX
//

class Cplx : public Scal {

#ifdef DEBUG_CLASS_COUNT
	static long class_count;
#endif

	real re;
	real im;
public:
	friend st_err_t Cplx_add(const Cplx&, const Cplx&, Cplx&);
	friend st_err_t Cplx_sub(const Cplx&, const Cplx&, Cplx&);
	friend st_err_t Cplx_mul(const Cplx&, const Cplx&, Cplx&);
	friend st_err_t Cplx_div(const Cplx&, const Cplx&, Cplx&);
	//friend st_err_t Cplx_mul_by_Real(const Cplx&, const Real&, Cplx&);
	//friend st_err_t Cplx_div_by_Real(const Cplx&, const Real&, Cplx&);

	Cplx() : re(0), im(0) {
		//std::cout << "Creation of a CPLX\n";
#ifdef DEBUG_CLASS_COUNT
		class_count++;
#endif
	}

	explicit Cplx(const real& rree, const real& iimm) : re(rree), im(iimm) {
		//std::cout << "Creation of a CPLX\n";
#ifdef DEBUG_CLASS_COUNT
		class_count++;
#endif
	}

	explicit Cplx(const Real& r) : re(r.get_value()), im(0) {
		//std::cout << "Creation of a CPLX\n";
#ifdef DEBUG_CLASS_COUNT
		class_count++;
#endif
	}

	  // Copy-constructor
	Cplx(const Cplx& cc) : re(cc.re), im(cc.im) {
		//std::cout << "Creation of a CPLX\n";
#ifdef DEBUG_CLASS_COUNT
		class_count++;
#endif
	}

	~Cplx() {
		//std::cout << "Destruction of a CPLX\n";
#ifdef DEBUG_CLASS_COUNT
		class_count--;
#endif
	}

#ifdef DEBUG_CLASS_COUNT
	static int get_class_count() { return class_count; }
#endif

	Cplx& operator=(const Cplx&);
	Cplx& operator=(const Real&);
	real get_re() const { return re; }
	real get_im() const { return im; }
	void trim();
	void zero() { re = 0; im = 0; }
	void neg() { re = (re == 0 ? 0 : -re); im = (im == 0 ? 0 : -im); }
	void conj() { im = -im; }
	int cmp(const Cplx&) const;
	st_err_t add(const Cplx&, Cplx&);
	st_err_t sub(const Cplx&, Cplx&);
	st_err_t mul(const Cplx&, Cplx&);
	st_err_t div(const Cplx&, Cplx&);
	const std::string to_string(const tostring_t&) const;
};

st_err_t Cplx_add(const Cplx&, const Cplx&, Cplx&);
st_err_t Cplx_sub(const Cplx&, const Cplx&, Cplx&);
st_err_t Cplx_mul(const Cplx&, const Cplx&, Cplx&);
st_err_t Cplx_div(const Cplx&, const Cplx&, Cplx&);


//
// MATRIX
//

struct ReadMatrixCell {
	scalar_t type;
	Scal* s;

#ifdef DEBUG_CLASS_COUNT
private:
	static long class_count;
public:
	ReadMatrixCell() : s(0) { class_count++; }
	 // Copy-constructor
	ReadMatrixCell(const ReadMatrixCell& rmc) : type(rmc.type), s(rmc.s) { class_count++; }
	~ReadMatrixCell() { class_count--; }
	ReadMatrixCell& operator=(const ReadMatrixCell& rmc) {
		type = rmc.type;
		s = rmc.s;
		return *this;
	}
	static int get_class_count() { return class_count; }
#endif

};
typedef std::vector< std::vector<ReadMatrixCell>* > mat_read_t;

#ifdef DEBUG_CLASS_COUNT
class CCMatrix {
	static long class_count;
public:
	CCMatrix() { class_count++; }
	~CCMatrix() { class_count--; }
	static int get_class_count() { return class_count; }
};
#endif

template<class Scalar> class Matrix {

#ifdef DEBUG_CLASS_COUNT
	CCMatrix* ccm;
#endif

	dim_t dimension;
	std::vector< std::vector<Scalar>* > mat;
	int nb_lines;
	int nb_columns;
	Matrix& operator=(const Matrix&);
public:

	  // Empty matrix constructor
	Matrix<Scalar>(const dim_t&, const int&, const int&, const Scalar&);
	  // Copy-constructor
	Matrix(const Matrix<Scalar>&);
	  // Constructor from an input, stored in pm
	Matrix(const mat_read_t*&, const dim_t&, const int&, const int&);

	virtual ~Matrix();

	virtual dim_t get_dimension() const { return dimension; }
	virtual int get_nb_lines() const { return nb_lines; }
	virtual int get_nb_columns() const { return nb_columns; }
	virtual Scalar get_value(const int& i, const int& j) const { return (*mat[i])[j]; }
	virtual void set_value(const int& i, const int& j, const Scalar& v) { (*mat[i])[j] = v; }
	virtual void redim(const dim_t&, const int&, const int&);
	virtual bool index_to_ij(const int&, int&, int&);
	virtual void copy_linear(Matrix<Scalar>*);
	virtual st_err_t create_transpose(Matrix<Scalar>*&);
	virtual st_err_t get_bounds(Coordinates& coord) {
		coord.d = dimension;
		coord.i = (dimension == DIM_VECTOR ? nb_columns : nb_lines);
		coord.j = (dimension == DIM_VECTOR ? 1 : nb_columns);
		return ST_ERR_OK;
	}

	virtual void trim();
	virtual int cmp(const Matrix<Scalar>&) const;

	  // Multiplication or division by a scalar
	virtual st_err_t md(st_err_t (*f)(const Scalar&, const Scalar&, Scalar&), const Scalar&);
	  // Addition or subtraction with another vector/matrix of same dimension
	virtual st_err_t as(st_err_t (*f)(const Scalar&, const Scalar&, Scalar&), Matrix<Scalar>&);

};

#endif // SCALAR_H

