//*****************************************************************************
//-----------------------------------------------------------------------------
// Fast simulation and reconstruction package for TOP counter (F77 core)
// M. Staric, Jan-2010, March-2010, Sept-2011
//-----------------------------------------------------------------------------
//
// TOPtrack.cc
// C++ interface to F77 functions: track
//-----------------------------------------------------------------------------
//*****************************************************************************

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>
#include "top/modules/TOPReconstruction/TOPtrack.h"
#include "top/modules/TOPReconstruction/TOPf77fun.h"
#include <TRandom3.h>

#define C0 29.9798458 // speed of light in ns/cm

inline double GaussRnd(double sigma) {return gRandom->Gaus(0., sigma);}

TOPtrack::TOPtrack()
{
  m_X = 0;
  m_Y = 0;
  m_Z = 0;
  m_Px = 0;
  m_Py = 0;
  m_Pz = 0;
  m_Tlen = 0;
  m_Q = 0;
  m_LUND = 0;
  m_REF = 0;
  m_atTop = false;
  m_QbarID = -1;
}

TOPtrack::TOPtrack(double x, double y, double z,
                   double Px, double Py, double Pz,
                   double Tlen, int Q, int Lund, int label)
{
  m_X = x;
  m_Y = y;
  m_Z = z;
  m_Px = Px;
  m_Py = Py;
  m_Pz = Pz;
  m_Tlen = Tlen;
  m_Q = Q;
  m_LUND = Lund;
  m_REF = label;
  m_atTop = false;
  m_QbarID = -1;
}

double TOPtrack::p() {return sqrt(m_Px * m_Px + m_Py * m_Py + m_Pz * m_Pz);}

double TOPtrack::theta() {return acos(m_Pz / p());}

double TOPtrack::phi() {return atan2(m_Py, m_Px);}

int TOPtrack::Hyp()
{
  int lundc[5] = {11, 13, 211, 321, 2212};
  for (int i = 0; i < 5; i++) {
    if (abs(m_LUND) == lundc[i]) return i + 1;
  }
  return 0;
}

int TOPtrack::toTop()
{
  if (m_atTop) return m_QbarID;

  float r[3], p[3], q, t; int m;
  r[0] = m_X; r[1] = m_Y; r[2] = m_Z; p[0] = m_Px; p[1] = m_Py; p[2] = m_Pz;
  q = m_Q;
  track2top_(r, p, &q, &t, &m);
  m_X = r[0]; m_Y = r[1]; m_Z = r[2];
  m_Px = p[0]; m_Py = p[1]; m_Pz = p[2];
  m_Tlen += t * C0;
  m_atTop = true;
  m_QbarID = m;
  return m;
}

void TOPtrack::smear(double sig_x, double sig_z,
                     double sig_theta, double sig_phi)
{
  double p = TOPtrack::p();
  if (p == 0) return;
  double theta = acos(m_Pz / p) + GaussRnd(sig_theta);
  double phi = atan2(m_Py, m_Px) + GaussRnd(sig_phi);
  m_Px = p * cos(phi) * sin(theta);
  m_Py = p * sin(phi) * sin(theta);
  m_Pz = p * cos(theta);

  m_Z += GaussRnd(sig_z);
  double dx = GaussRnd(sig_x);
  if (m_X* m_X + m_Y* m_Y != 0) phi = atan2(m_Y, m_X);
  m_X += dx * sin(phi);
  m_Y -= dx * cos(phi);
}

void TOPtrack::Dump()
{
  double pi = 4 * atan(1);
  using namespace std;
  cout << "TOPtrack::Dump(): Ref=" << m_REF << " Lund=" << m_LUND;
  cout << " charge=" << m_Q << endl;
  cout << "  p=" << setprecision(3) << p() << " GeV/c";
  cout << "  theta=" << setprecision(3) << theta() / pi * 180;
  cout << "  phi=" << setprecision(3) << phi() / pi * 180 << endl;
  cout << "  x=" << m_X << " cm";
  cout << "  y=" << m_Y << " cm";
  cout << "  z=" << m_Z << " cm\n";
  cout << "  Tlen=" << setprecision(4) << m_Tlen << " cm";
  cout << "  atTop=" << m_atTop;
  cout << "  barID=" << m_QbarID;
  cout << endl;
}

