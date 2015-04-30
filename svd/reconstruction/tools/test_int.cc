// =====================================================================================
//
//       Filename:  test_int.cc
//
//    Description:  Test CLHEP definite integral code
//
//        Version:  1.0
//        Created:  29.4.2015 16:40:30
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Peter Kvasnicka (PQ), Peter.Kvasnicka@mff.cuni.cz
//   Organization:  IPNP, Charles University in Prague
//
// =====================================================================================
#include <root/TF1.h>
#include <boost/math/constants/constants.hpp>
#include <boost/math/distributions/normal.hpp>
#include <iostream>

using namespace std;

double precise_integral(double a, double b)
{
  using namespace boost::math::double_constants;
  using namespace boost::math;
  static normal dist;
  return cdf(dist, b) - cdf(dist, a);
}

int main(int argc, const char** argv)
{
  using namespace boost::math::double_constants;
  double a = -2;
  double b = 2;
  TF1* fg = new TF1("fg", "gaus(0)", -5, 5);
  double params[3] = {one_div_root_two_pi, 0, 1};
  double integral = fg->Integral(a, b, params);
  double pintegral = precise_integral(a, b);
  cout << integral << " vs. " << pintegral << endl;
  cout << "Relative error: " << (integral - pintegral) / pintegral << endl;

}
