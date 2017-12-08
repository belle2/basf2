#include <ecl/digitization/ECLDspFitterPure.h>
#include <TVectorD.h>
#include <TMatrixD.h>
#include <TDecompLU.h>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <cassert>
using namespace Belle2;
using namespace Belle2::ECL;
using namespace std;

double Belle2::ECL::func(int i, int ifine,
                         EclConfigurationPure::signalsamplepure_t& signal)
{
  if (i == 0) return 0;
  if (ifine < 0) {
    i--;
    if (i == 0) return 0;
    ifine = EclConfigurationPure::m_ndtPure + ifine;
  }
  return signal.m_ft[(i - 1) * EclConfigurationPure::m_ndtPure + ifine ];
}

double Belle2::ECL::func1(int i, int ifine,
                          EclConfigurationPure::signalsamplepure_t& signal)
{
  if (i == 0) return 0;
  if (ifine < 0) {
    i--;
    if (i == 0) return 0;
    ifine = EclConfigurationPure::m_ndtPure + ifine;
  }
  return signal.m_ft1[(i - 1) * EclConfigurationPure::m_ndtPure + ifine ];
}

void Belle2::ECL::initParams(EclConfigurationPure::fitparamspure_t& params,
                             EclConfigurationPure::signalsamplepure_t& signal)
{

  for (int i = 0; i < 16; i++) {
    for (int k = -EclConfigurationPure::m_ndtPure; k < EclConfigurationPure::m_ndtPure; k++) {
      // cout << i << ":" << k << " " << func( i, k, signal) << " " << func1(i, k, signal) << endl;
      if (k < 0) params.f[i][ EclConfigurationPure::m_ndtPure - 1 - k] = func(i, k, signal);
      else     params.f[i][k] = func(i, k, signal);
    }
  }

  double kdt = 0.001 / (EclConfigurationPure::m_tickPure / EclConfigurationPure::m_ndtPure) ; // df / dtau = df / dt * dt / dtau

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
      for (int j = 0; j < 16; j++) {
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

  int off = EclConfigurationPure::m_ndtPure - 1;
  for (int k = 1; k < EclConfigurationPure::m_ndtPure; k++) {
    params.c110[k + off] = 0;
    params.c200[k + off] = 0;
    params.c020[k + off] = 0;
    params.c101[k + off] = 0;
    params.c011[k + off] = 0;
    for (int i = 0; i < 16; i++) {
      for (int j = 0; j < 16; j++) {
        params.c110[k + off] += kdt * func1(i, -k, signal) * params.invC[i][j] * func(j, -k, signal);
        params.c200[k + off] += func(i, -k, signal) * params.invC[i][j] * func(j, -k, signal);
        params.c020[k + off] += kdt * func1(i, -k, signal) * params.invC[i][j] * func1(j, -k, signal) * kdt;
        params.c101[k + off] += func(i, -k, signal) * params.invC[i][j];
        params.c011[k + off] += kdt * func1(i, -k, signal) * params.invC[i][j];
      }
    }
    params.c110[k + off] *= 2;
    params.c101[k + off] *= 2;
    params.c011[k + off] *= 2;
  }


  for (int h = 0; h < 16; h++) {

    for (int k = 0; k < EclConfigurationPure::m_ndtPure; k++) {
      params.c100[h][k] = 0;
      params.c010[h][k] = 0;
      for (int i = 0; i < 16; i++) {
        params.c100[h][k] -= func(i, k, signal) * params.invC[i][h];
        params.c010[h][k] -= kdt * func1(i, k, signal) * params.invC[i][h];
      }
      params.c100[h][k] *= 2;
      params.c010[h][k] *= 2;
    }
    for (int k = 1; k < EclConfigurationPure::m_ndtPure; k++) {
      params.c100[h][k + off] = 0;
      params.c010[h][k + off] = 0;
      for (int i = 0; i < 16; i++) {
        params.c100[h][k + off] -= func(i, -k, signal) * params.invC[i][h];
        params.c010[h][k + off] -= kdt * func1(i, -k, signal) * params.invC[i][h];
      }
      params.c100[h][k + off] *= 2;
      params.c010[h][k + off] *= 2;
    }

  }
}

void Belle2::ECL::DSPFitterPure(const EclConfigurationPure::fitparamspure_t& f  , const int* FitA, const int ttrig, int& amp,
                                double& time, double& chi2, int& iter)
{
  int baseline = accumulate(FitA, FitA + 16, 0) / 16;
  const int* imax =  max_element(FitA, FitA + EclConfigurationPure::m_nsmp);

  amp = *imax - baseline;
  //time = imax - FitA;


  int dt = ttrig;
  double y[16];
  y[0] = baseline;
  for (int i = 16; i < EclConfigurationPure::m_nsmp; i++) y[i - 15] = FitA[i];

  //cout << "ttrig = " << dt << endl;
  //  for (int i=0; i<16; i++ ) cout << "y_" << i << " = " << y[i] << endl;

  double k100 { 0 }, k010{ 0 }, k001{ 0 };
  int offset = EclConfigurationPure::m_ndtPure;
  if (dt < 0) dt = offset - 1 - dt;
  for (int i = 0; i < 16; i++) {
    k100 += f.c100[i][dt] * y[i];
    k010 += f.c010[i][dt] * y[i];
    k001 += f.c001[i]     * y[i];
  }

  double a[] = {   2 * f.c200[dt], f.c110[dt], f.c101[dt] ,
                   f.c110[dt],   2 * f.c020[dt], f.c011[dt],
                   f.c101[dt],   f.c011[dt],   2 * f.c002
               };

  TMatrixD M1(3, 3);
  M1.SetMatrixArray(a);

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
  /*
  if ( !ok ) {
    cout << "Fit failed" << endl;
    cout << "dt = " << dt << endl;
    M1.Print();
    cout << "solution -->" << endl;
    C.Print();
  }
  */
  amp = C[0];

  /*
  cout << "t0 = " << ttrig << endl;
  cout << "P  = " << C[2] << endl;
  cout << "A  = " << C[0] << endl;
  cout << "B = A * t  = " << C[1] << endl;
  cout << "dt = B / A = " << C[1] / C[0] << endl;
  */

  double A = C[0];
  double B = C[1];
  double P = C[2];

  /*
  for (int i=1; i<16; i++)
    cout << "A= " << C[0] << " f = " << f.f[i][dt] << " A*f = " << C[0]*f.f[i][dt] << " y = " << y[i] << endl;
  */

  chi2 = f.c200[dt] * A * A +  f.c020[dt] * B * B +  f.c002 * P * P +
         f.c110[dt] * A * B + f.c101[dt] * A * P + f.c011[dt] * B * P +
         k100 * A  + k010 * B + k001 * P ;
  for (int i = 0; i < 16; i++)
    for (int j = 0; j < 16; j++)
      chi2 += y[i] * f.invC[i][j] * y[j];

  // cout << "chi2 = " << chi2 << endl;
  double deltaT = C[1] / C[0] ;
  int intTime = ttrig + round(deltaT) ;
  //  time = -ttrig - C[1] / C[0] ;
  time = -(ttrig + deltaT);
  // cout << "time : "  << time << endl;
  bool intime = abs(intTime) < EclConfigurationPure::m_ndtPure - 1;
  if (iter < 3 && intime) {
    iter++;
    DSPFitterPure(f, FitA, intTime, amp, time, chi2, iter);
  }

  if (intime && abs(deltaT) > 0.5 && iter++ < 10)
    DSPFitterPure(f, FitA, intTime, amp, time, chi2, iter);
}
