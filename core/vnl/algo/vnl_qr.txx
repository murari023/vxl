// This is vxl/vnl/algo/vnl_qr.txx
#ifndef vnl_qr_txx_
#define vnl_qr_txx_
//:
// \file
// \author Andrew W. Fitzgibbon, Oxford RRG
// \date   08 Dec 96

#include "vnl_qr.h"
#include <vcl_iostream.h>
#include <vcl_complex.h>
#include <vnl/vnl_math.h>
#include <vnl/vnl_complex.h>  // vnl_math_squared_magnitude()
#include <vnl/vnl_matlab_print.h>
#include <vnl/vnl_complex_traits.h>
#include "vnl_netlib.h"

#ifdef HAS_FSM_PACK
template <typename T> int fsm_qrdc_cxx(vnl_netlib_qrdc_proto(T));
template <typename T> int fsm_qrsl_cxx(vnl_netlib_qrsl_proto(T));
# define vnl_linpack_qrdc fsm_qrdc_cxx
# define vnl_linpack_qrsl fsm_qrsl_cxx

#else
// use C++ overloading to call the right linpack routine from the template code :
#define macro(p, T) \
inline void vnl_linpack_qrdc(vnl_netlib_qrdc_proto(T)) \
{ p##qrdc_(vnl_netlib_qrdc_params); } \
inline void vnl_linpack_qrsl(vnl_netlib_qrsl_proto(T)) \
{ p##qrsl_(vnl_netlib_qrsl_params); }
macro(s, float);
macro(d, double);
macro(c, vcl_complex<float>);
macro(z, vcl_complex<double>);
#undef macro
#endif

template <class T>
vnl_qr<T>::vnl_qr(vnl_matrix<T> const& M):
  qrdc_out_(M.columns(), M.rows()),
  qraux_(M.columns()),
  jpvt_(M.rows()),
  Q_(0),
  R_(0)
{
  // Fill transposed O/P matrix
  int c = M.columns();
  int r = M.rows();
  for (int i = 0; i < r; ++i)
    for (int j = 0; j < c; ++j)
      qrdc_out_(j,i) = M(i,j);

  int do_pivot = 0; // Enable[!=0]/disable[==0] pivoting.
  jpvt_.fill(0); // Allow all columns to be pivoted if pivoting is enabled.

  vnl_vector<T> work(M.rows());
  vnl_linpack_qrdc(qrdc_out_.data_block(), // On output, UT is R, below diag is mangled Q
                   &r, &r, &c,
                   qraux_.data_block(), // Further information required to demangle Q
                   jpvt_.data_block(),
                   work.data_block(),
                   &do_pivot);
}

template <class T>
vnl_qr<T>::~vnl_qr()
{
  delete Q_;
  delete R_;
}

//: Return the determinant of M.  This is computed from M = Q R as follows:
// |M| = |Q| |R|
// |R| is the product of the diagonal elements.
// |Q| is (-1)^n as it is a product of Householder reflections.
// So det = -prod(-r_ii).
template <class T>
T vnl_qr<T>::determinant() const
{
  int m = vnl_math_min((int)qrdc_out_.columns(), (int)qrdc_out_.rows());
  T det = qrdc_out_(0,0);

  for (int i = 1; i < m; ++i)
    det *= -qrdc_out_(i,i);

  return det;
}

//: Unpack and return unitary part Q.
template <class T>
vnl_matrix<T> const& vnl_qr<T>::Q() const
{
  int m = qrdc_out_.columns(); // column-major storage
  int n = qrdc_out_.rows();

  bool verbose = false;

  if (!Q_) {
    ((vnl_matrix<T>*&)Q_) = new vnl_matrix<T>(m,m);
    // extract Q.
    if (verbose) {
      vcl_cerr << __FILE__ ": vnl_qr<T>::Q()\n";
      vcl_cerr << " m,n = " << m << ", " << n << vcl_endl;
      vcl_cerr << " qr0 = [" << qrdc_out_ << "];\n";
      vcl_cerr << " aux = [" << qraux_ << "];\n";
    }

    Q_->set_identity();
    vnl_matrix<T>& Q = *Q_;

    vnl_vector<T> v(m, T(0));
    vnl_vector<T> w(m, T(0));

    // Golub and vanLoan, p199.  backward accumulation of householder matrices
    // Householder vector k is [zeros(1,k-1) qraux_[k] qrdc_out_[k,:]]
    typedef typename vnl_numeric_traits<T>::abs_t abs_t;
    for (int k = n-1; k >= 0; --k) {
      if (k >= m) continue;
      // Make housevec v, and accumulate norm at the same time.
      v[k] = qraux_[k];
      abs_t sq = vnl_math_squared_magnitude(v[k]);
      for (int j = k+1; j < m; ++j) {
        v[j] = qrdc_out_(k,j);
        sq += vnl_math_squared_magnitude(v[j]);
      }
      if (verbose) vnl_matlab_print(vcl_cerr, v, "v");

#define c vnl_complex_traits<T>::conjugate
      // Premultiply emerging Q by house(v), noting that v[0..k-1] == 0.
      // Q_new = (1 - (2/v'*v) v v')Q
      // or Q -= (2/v'*v) v (v'Q)
      if (sq > abs_t(0)) {
        abs_t scale = abs_t(2)/sq;
        // w = (2/v'*v) v' Q
        for (int i = k; i < m; ++i) {
          w[i] = T(0);
          for (int j = k; j < m; ++j)
            w[i] += scale * c(v[j]) * Q(j, i);
        }
        if (verbose) vnl_matlab_print(vcl_cerr, w, "w");

        // Q -= v w
        for (int i = k; i < m; ++i)
          for (int j = k; j < m; ++j)
            Q(i,j) -= (v[i]) * (w[j]);
      }
#undef c
    }
  }
  return *Q_;
}

//: Unpack and return R.
template <class T>
vnl_matrix<T> const& vnl_qr<T>::R() const
{
  if (!R_) {
    int m = qrdc_out_.columns(); // column-major storage
    int n = qrdc_out_.rows();
    ((vnl_matrix<T>*&)R_) = new vnl_matrix<T>(m,n);
    vnl_matrix<T> & R = *R_;

    for (int i = 0; i < m; ++i)
      for (int j = 0; j < n; ++j)
        if (i > j)
          R(i, j) = T(0);
        else
          R(i, j) = qrdc_out_(j,i);
  }

  return *R_;
}

template <class T>
vnl_matrix<T> vnl_qr<T>::recompose() const
{
  return Q() * R();
}

// JOB: ABCDE decimal
// A     B     C     D              E
// ---   ---   ---   ---            ---
// Qb    Q'b   x     norm(A*x - b)  A*x


//: Solve equation M x = b for x using the computed decomposition.
template <class T>
vnl_vector<T> vnl_qr<T>::solve(const vnl_vector<T>& b) const
{
  int n = qrdc_out_.columns();
  int p = qrdc_out_.rows();
  const T* b_data = b.data_block();
  vnl_vector<T> QtB(n);
  vnl_vector<T> x(p);

  // see comment above
  int JOB = 100;

  int info = 0;
  vnl_linpack_qrsl(qrdc_out_.data_block(),
                   &n, &n, &p,
                   qraux_.data_block(),
                   b_data, (T*)0, QtB.data_block(),
                   x.data_block(),
                   (T*)0/*residual*/,
                   (T*)0/*Ax*/,
                   &JOB,
                   &info);

  if (info > 0)
    vcl_cerr << __FILE__ ": vnl_qr<T>::solve() : matrix is rank-deficient by " << info << vcl_endl;

  return x;
}

//: Return residual vector d of M x = b -> d = Q'b
template <class T>
vnl_vector<T> vnl_qr<T>::QtB(const vnl_vector<T>& b) const
{
  int n = qrdc_out_.columns();
  int p = qrdc_out_.rows();
  const T* b_data = b.data_block();
  vnl_vector<T> QtB(n);

  // see comment above
  int JOB = 1000;

  int info = 0;
  vnl_linpack_qrsl(qrdc_out_.data_block(),
                   &n, &n, &p,
                   qraux_.data_block(),
                   b_data,
                   (T*)0,               // A: Qb
                   QtB.data_block(),    // B: Q'b
                   (T*)0,               // C: x
                   (T*)0,               // D: residual
                   (T*)0,               // E: Ax
                   &JOB,
                   &info);

  if (info > 0)
    vcl_cerr << __FILE__ ": vnl_qr<T>::QtB() -- matrix is rank-def by " << info << vcl_endl;

  return QtB;
}

//--------------------------------------------------------------------------------

#define VNL_QR_INSTANTIATE(T) \
 template class vnl_qr<T >; \
 VCL_INSTANTIATE_INLINE(T vnl_qr_determinant(vnl_matrix<T > const&))

#endif
