/*
  fsm@robots.ox.ac.uk
*/

// The purpose of this is to check there are no
// clashes between vcl_sqrt() and vcl_abs().
#include <vcl_complex.h>
#include <vcl_cmath.h>
#include <vcl_cstdlib.h>

#include <vcl_iostream.h>
#define macro(cond, type) \
do { \
  if (cond) \
    vcl_cout << "vcl_abs(" #type ") PASSED" << vcl_endl; \
  else \
    vcl_cerr << "vcl_abs(" #type ") *** FAILED *** " << vcl_endl; \
} while (false)

int main()
{
  int    xi = 314159265;
  long   xl = 314159265L;
  float  xf = 13.14159265358979323846;
  double xd = 23.14159265358979323846;
  // + long double
  
  macro(vcl_abs(- xi) == xi, int);
  macro(vcl_abs(- xl) == xl, long);
  macro(vcl_abs(- xf) == xf, float);
  macro(vcl_abs(- xd) == xd, double);
  
  return 0;
}
