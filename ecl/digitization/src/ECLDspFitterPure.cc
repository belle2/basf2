#include <ecl/digitization/ECLDspFitterPure.h>
#include <TVectorD.h>
#include <TMatrixD.h>
#include <TDecompLU.h>
#include <numeric>
#include <algorithm>
#include <iostream>

using namespace Belle2;
using namespace Belle2::ECL;
using namespace std;

double Belle2::ECL::func(int i, int ifine,
                         EclConfigurationPure::signalsamplepure_t& signal)
{
  if (i == 0) return 0;
  return signal.m_ft[(i - 1) * EclConfigurationPure::m_ndtPure + ifine ];
}

double Belle2::ECL::func1(int i, int ifine,
                          EclConfigurationPure::signalsamplepure_t& signal)
{
  if (i == 0) return 0;
  return signal.m_ft1[(i - 1) * EclConfigurationPure::m_ndtPure + ifine ];
}

void Belle2::ECL::initParams(EclConfigurationPure::fitparamspure_t& params,
                             EclConfigurationPure::signalsamplepure_t& signal)
{

  double kdt = 0.001 / (EclConfigurationPure::m_tickPure / EclConfigurationPure::m_ndtPure) ; // df / dtau = df / dt * dt / dtau
  double sigma2 = 1.0;

  for (int i = 0; i < 16; i++)
    for (int j = 0; j < 16; j++)
      if (i != j) params.invC[i][j] = 0;
      else params.invC[i][j] = 1 / sigma2;

  params.c002 = 0;
  for (int h = 0; h < 16; h++) {
    params.c001[h] = 0;
    for (int j = 0; j < 16; j++) {
      params.c001[h] -= params.invC[h][j];
      params.c002 += params.invC[h][j];
    }
    params.c001[h] *= 2;
  }



  for (int k = 0; k < EclConfigurationPure::m_ndtPure; k++) {
    params.c110[k] = 0;
    params.c200[k] = 0;
    params.c020[k] = 0;
    params.c101[k] = 0;
    params.c011[k] = 0;
    for (int i = 0; i < 16; i++) {
      //int indexfi = i*EclConfigurationPure::m_ndtPure + k;
      for (int j = 0; j < 16; j++) {
        /*
        int indexfj = j*EclConfigurationPure::m_ndtPure + k;

        params.c110[k] -= kdt * signal.m_ft1[indexfi] * params.invC[i][j] * signal.m_ft[indexfj] ;
        params.c200[k] += signal.m_ft[indexfi] * params.invC[i][j] * signal.m_ft[indexfj];
        params.c020[k] += kdt * signal.m_ft1[indexfi] * params.invC[i][j] * signal.m_ft1[indexfj] * kdt;
        params.c101[k] += signal.m_ft[indexfi] * params.invC[i][j];
        params.c011[k] += kdt * signal.m_ft1[indexfi] * params.invC[i][j];
        */
        params.c110[k] += kdt * func1(i, k, signal) * params.invC[i][j] * func(j, k, signal);
        params.c200[k] += func(i, k, signal) * params.invC[i][j] * func(j, k, signal);
        params.c020[k] += kdt * func1(i, k, signal) * params.invC[i][j] * func1(j, k, signal) * kdt;
        params.c101[k] += func(i, k, signal) * params.invC[i][j];
        params.c011[k] += kdt * func1(i, k, signal) * params.invC[i][j];
      }
    }
    params.c110[k] *= 2;
    params.c101[k] *= 2;
    params.c011[k] *= 2;
  }

  for (int h = 0; h < 16; h++) {
    for (int k = 0; k < EclConfigurationPure::m_ndtPure; k++) {
      params.c100[h][k] = 0;
      params.c010[h][k] = 0;
      for (int i = 0; i < 16; i++) {
        /* int indexfi = i*EclConfigurationPure::m_ndtPure + k;
        params.c100[h][k] -= signal.m_ft[indexfi] * params.invC[i][h];
        params.c010[h][k] += kdt * signal.m_ft1[indexfi] * params.invC[i][h];
        */
        params.c100[h][k] -= func(i, k, signal) * params.invC[i][h];
        params.c010[h][k] -= kdt * func1(i, k, signal) * params.invC[i][h];
      }
      params.c100[h][k] *= 2;
      params.c010[h][k] *= 2;

    }
  }
}

void Belle2::ECL::DSPFitterPure(const EclConfigurationPure::fitparamspure_t& f  , const int* FitA, const int ttrig, int& amp,
                                double& time, double& chi2, int& iter)
{
  int baseline = accumulate(FitA, FitA + 16, 0) / 16;
  const int* imax =  max_element(FitA, FitA + EclConfigurationPure::m_nsmp);

  amp = *imax - baseline;
  time = imax - FitA;


  int dt = ttrig;
  double y[16];
  y[0] = baseline;
  for (int i = 16; i < EclConfigurationPure::m_nsmp; i++) y[i - 15] = FitA[i];

  //cout << "ttrig = " << dt << endl;
  //  for (int i=0; i<16; i++ ) cout << "y_" << i << " = " << y[i] << endl;

  double k100 { 0 }, k010{ 0 }, k001{ 0 };
  for (int i = 0; i < 16; i++) {
    k100 += f.c100[i][dt] * y[i];
    k010 += f.c010[i][dt] * y[i];
    k001 += f.c001[i]     * y[i];
  }

  double a[] = {   2 * f.c200[dt], f.c110[dt], f.c101[dt] ,
                   f.c110[dt],   2 * f.c020[dt], f.c011[dt],
                   f.c101[dt],   f.c011[dt],   2 * f.c002
               };

  //for (int i=0; i<9; i++) cout << a[i] << endl;

  TMatrixD M1(3, 3);
  M1.SetMatrixArray(a);

  //  M1.Print();

  TVectorD C(3);
  C[0] = - k100;
  C[1] = - k010;
  C[2] = - k001;
  /*
  cout << "(" << -k100 << "==" << C[0] << ","
       << "(" << -k010 << "==" << C[1] << ","
       << "(" << -k001 << "==" << C[2] << ","
       << endl;

  */
  //  C.Print();
  TDecompLU solver(M1);
  bool ok;
  C = solver.Solve(C, ok);
  //if ( !ok ) cout << "Fit failed" << endl;
  //cout << "solution -->" << endl;
  // C.Print();
  amp = C[0];

  /*
  cout << "A  = " << C[0] << endl;
  cout << "t0 = " << ttrig << endl;
  cout << "B = A * t  = " << C[1] << endl;
  cout << "P  = " << C[2] << endl;
  cout << "t = B / A = " << C[1]/C[0] << endl;
  */


  double A = C[0];
  double B = C[1];
  double P = C[2];

  chi2 = f.c200[dt] * A * A +  f.c020[dt] * B * B +  f.c002 * P * P +
         f.c110[dt] * A * B + f.c101[dt] * A * P + f.c011[dt] * B * P +
         k100 * A  + k010 * B + k001 * P ;
  for (int i = 0; i < 16; i++)
    for (int j = 0; j < 16; j++)
      chi2 += y[i] * f.invC[i][j] * y[j];

  time = ttrig - C[1] / C[0] ;
  iter++;
}
