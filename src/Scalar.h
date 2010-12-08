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

extern bool cfg_realdisp_manage_std;
extern bool cfg_realdisp_remove_trailing_dot;

typedef enum {SCALAR_UNDEF, SCALAR_REAL, SCALAR_COMPLEX} scalar_t;
typedef enum {DIM_VECTOR, DIM_MATRIX, DIM_ANY} dim_t;

  // General functions
real real_trim(const real&);
const std::string real_to_string(const real&);
st_err_t real_check_bounds(const bool&, const int&, const real&, real&, const bool& = false);
real real_abs(const real&);
int real_sign(const real&);
real real_ip(const real&);
real real_floor(const real&);
real real_ceil(const real&);
real real_mant(const real&);
real real_xpon(const real&);
real get_max_real_from_bin_size(const int&);
int digit_to_int(const char&);
char int_to_digit(const int&);
void remove_leading_zeros(std::string&);


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
	friend void Real_arith(void (*f)(const real&, const real&, st_err_t&, real&), const Real&, const Real&, st_err_t&, Real&);

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
	  // Used in linear system resolution, to calculate which coeff
	  // is closest to 1 in its mantisse.
	real get_mantisse() const { return real_abs(real_mant(r)); }
	void trim();
	void neg() { r = (r == 0 ? 0 : -r); }
	void conj() { }
	int cmp(const Real&) const;
	void add(const Real&, st_err_t&, Real&) const;
	void sub(const Real&, st_err_t&, Real&) const;
	void mul(const Real&, st_err_t&, Real&) const;
	void div(const Real&, st_err_t&, Real&) const;
	void pow(const Real&, st_err_t&, Cplx&) const;
	void floor(st_err_t&, Real&) const;
	void mod(const Real&, st_err_t&, Real&) const;
	void square_of_abs(st_err_t&, Real&) const;
	void abs(st_err_t&, Real&) const;
	void sqr(st_err_t&, Cplx&) const;
	void ln(st_err_t&, Cplx&) const;
	void acos(st_err_t&, Cplx&) const;
	void asin(st_err_t&, Cplx&) const;
	void atan(st_err_t&, Real&) const;
	void cos(st_err_t&, Real&) const;
	void sin(st_err_t&, Real&) const;
	void tan(st_err_t&, Real&) const;
	void exp(st_err_t&, Real&) const;
	void log(st_err_t&, Cplx&) const;
	void alog(st_err_t&, Real&) const;
	void lnp1(st_err_t&, Real&) const;
	void expm(st_err_t&, Real&) const;
	void cosh(st_err_t&, Real&) const;
	void sinh(st_err_t&, Real&) const;
	void tanh(st_err_t&, Real&) const;
	void acosh(st_err_t&, Cplx&) const;
	void asinh(st_err_t&, Real&) const;
	void atanh(st_err_t&, Cplx&) const;
	void to_hms(st_err_t&, Real&) const;
	void hms_to(st_err_t&, Real&) const;
	void hms_add(const Real&, st_err_t&, Real&) const;
	void hms_sub(const Real&, st_err_t&, Real&) const;
	void d_to_r(st_err_t&, Real&) const;
	void r_to_d(st_err_t&, Real&) const;
	void zero() { r = 0; }
	const std::string to_string(const tostring_t&) const;
};

void Real_arith(void (*f)(const real&, const real&, st_err_t&, real&), const Real&, const Real&, st_err_t&, Real&);
void Real_add(const Real&, const Real&, st_err_t&, Real&);
void Real_sub(const Real&, const Real&, st_err_t&, Real&);
void Real_mul(const Real&, const Real&, st_err_t&, Real&);
void Real_div(const Real&, const Real&, st_err_t&, Real&);
void Real_pow(const Real&, const Real&, st_err_t&, Real&);


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
	friend void Cplx_add(const Cplx&, const Cplx&, st_err_t& c, Cplx&);
	friend void Cplx_sub(const Cplx&, const Cplx&, st_err_t& c, Cplx&);
	friend void Cplx_mul(const Cplx&, const Cplx&, st_err_t& c, Cplx&);
	friend void Cplx_div(const Cplx&, const Cplx&, st_err_t& c, Cplx&);

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
	  // Used in linear system resolution, to calculate which coeff
	  // is closest to 1 in its mantisse.
	real get_mantisse() const {
		real r1 = real_abs(real_mant(re));
		real r2 = real_abs(real_mant(im));
		if (r1 < r2)
			return r2;
		else
			return r1;
	}
	void trim();
	void zero() { re = 0; im = 0; }
	void neg() { re = (re == 0 ? 0 : -re); im = (im == 0 ? 0 : -im); }
	void conj() { im = -im; }
	void square_of_abs(st_err_t&, Real&) const;
	void abs(st_err_t&, Real&) const;
	void arg(st_err_t&, Real&) const;
	void r_to_p(st_err_t&, Cplx&) const;
	void p_to_r(st_err_t&, Cplx&) const;
	void ln(st_err_t&, Cplx&) const;
	void exp(st_err_t&, Cplx&) const;
	void log(st_err_t&, Cplx&) const;
	void alog(st_err_t&, Cplx&) const;
	void pow(const Cplx&, st_err_t&, Cplx&) const;
	void sqr(st_err_t&, Cplx&) const;
	void acos(st_err_t&, Cplx&) const;
	void asin(st_err_t&, Cplx&) const;
	void atan(st_err_t&, Cplx&) const;
	void cos(st_err_t&, Cplx&) const;
	void sin(st_err_t&, Cplx&) const;
	void tan(st_err_t&, Cplx&) const;
	void cosh(st_err_t&, Cplx&) const;
	void sinh(st_err_t&, Cplx&) const;
	void tanh(st_err_t&, Cplx&) const;
	void acosh(st_err_t&, Cplx&) const;
	void asinh(st_err_t&, Cplx&) const;
	void atanh(st_err_t&, Cplx&) const;
	void sign(st_err_t&, Cplx&) const;
	int cmp(const Cplx&) const;
	void add(const Cplx&, st_err_t&, Cplx&) const;
	void sub(const Cplx&, st_err_t&, Cplx&) const;
	void mul(const Cplx&, st_err_t&, Cplx&) const;
	void div(const Cplx&, st_err_t&, Cplx&) const;
	const std::string to_string(const tostring_t&) const;
};

void Cplx_add(const Cplx&, const Cplx&, st_err_t&, Cplx&);
void Cplx_sub(const Cplx&, const Cplx&, st_err_t&, Cplx&);
void Cplx_mul(const Cplx&, const Cplx&, st_err_t&, Cplx&);
void Cplx_div(const Cplx&, const Cplx&, st_err_t&, Cplx&);


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
	virtual bool index_to_ij(const int&, int&, int&) const;
	virtual Scalar get_value(const int& i, const int& j) const { return (*mat[i])[j]; }
	virtual Scalar get_value(int n) const {
		int i, j;
		if (index_to_ij(--n, i, j))
			return (*mat[i])[j];
		else
			return Scalar(Real(0));
	}
	virtual void set_value(const int& i, const int& j, const Scalar& v) { (*mat[i])[j] = v; }
	virtual void set_value(int n, const Scalar& v) {
		int i, j;
		if (index_to_ij(--n, i, j))
			(*mat[i])[j] = v;
	}
	virtual void redim(const dim_t&, const int&, const int&);
	virtual void copy_linear(const Matrix<Scalar>*);
	virtual st_err_t create_transpose(Matrix<Scalar>*&);
	virtual st_err_t create_neg(Matrix<Scalar>*&);
	virtual st_err_t get_bounds(Coordinates& coord) {
		coord.d = dimension;
		coord.i = (dimension == DIM_VECTOR ? nb_columns : nb_lines);
		coord.j = (dimension == DIM_VECTOR ? 1 : nb_columns);
		return ST_ERR_OK;
	}

	virtual void trim();
	virtual int cmp(const Matrix<Scalar>&) const;

	  // Multiplication or division by a scalar
	virtual st_err_t md(void (*f)(const Scalar&, const Scalar&, st_err_t&, Scalar&), const Scalar&);
	  // Addition or subtraction with another vector/matrix of same dimension
	virtual st_err_t as(void (*f)(const Scalar&, const Scalar&, st_err_t&, Scalar&), Matrix<Scalar>&);
	virtual st_err_t create_mul(const Matrix<Scalar>*, Matrix<Scalar>*&) const;
	virtual st_err_t create_div(const Matrix<Scalar>*, Matrix<Scalar>*&) const;
	virtual st_err_t create_cross(const Matrix<Scalar>*, Matrix<Scalar>*&) const;
	virtual st_err_t dot(const Matrix<Scalar>*, Scalar&) const;
	virtual st_err_t abs(Real&) const;
};

st_err_t mat_r_to_c(Matrix<Real>*, Matrix<Real>*, Matrix<Cplx>*&);
void mat_c_to_r(Matrix<Cplx>*, Matrix<Real>*&, Matrix<Real>*&);
void mat_c_to_re(Matrix<Cplx>*, Matrix<Real>*&);
void mat_c_to_im(Matrix<Cplx>*, Matrix<Real>*&);
void mat_c_conj(Matrix<Cplx>*, Matrix<Cplx>*&);

#endif // SCALAR_H

