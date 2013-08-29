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
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPf77fun.h>
#include <TRandom3.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {
  namespace TOP {

    /*! Gaussian random number
     * @param sigma: sigma
     */
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
      for (int i = 0; i < 10; i++) m_Label[i] = 0;
      m_atTop = false;
      m_QbarID = -1;
    }

    TOPtrack::TOPtrack(double x, double y, double z,
                       double Px, double Py, double Pz,
                       double Tlen, int Q, int Lund)
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
      for (int i = 0; i < 10; i++) m_Label[i] = 0;
      m_atTop = false;
      m_QbarID = -1;
    }

    double TOPtrack::Tof(int Lund)
    {
      int lundc[5] = {11, 13, 211, 321, 2212};
      double masses[5] = {.511E-3, .10566, .13957, .49368, .93827};

      double mass = 0;
      for (int i = 0; i < 5; i++) {
        if (abs(Lund) == lundc[i]) {mass = masses[i]; break;}
      }
      double pmom = p();
      double beta = pmom / sqrt(pmom * pmom + mass * mass);
      return m_Tlen / beta / Const::speedOfLight;
    }

    void TOPtrack::setTrackLength(double tof, double mass)
    {
      double pmom = p();
      double beta = pmom / sqrt(pmom * pmom + mass * mass);
      m_Tlen = tof * Const::speedOfLight * beta;
    }

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

      float r[3] = {(float) m_X, (float) m_Y, (float) m_Z};
      float p[3] = {(float) m_Px, (float) m_Py, (float) m_Pz};
      float q = (float) m_Q;
      float t; int m;
      track2top_(r, p, &q, &t, &m);
      m_X = r[0]; m_Y = r[1]; m_Z = r[2];
      m_Px = p[0]; m_Py = p[1]; m_Pz = p[2];
      m_Tlen += t * Const::speedOfLight;
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
      if (m_X * m_X + m_Y * m_Y != 0) phi = atan2(m_Y, m_X);
      m_X += dx * sin(phi);
      m_Y -= dx * cos(phi);
    }

    void TOPtrack::Dump()
    {
      double pi = 4 * atan(1);
      using namespace std;
      cout << "TOPtrack::Dump(): labels";
      for (int i = 0; i < 10; i++) cout << " " << m_Label[i];
      cout << " Lund=" << m_LUND;
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

  } // end top namespace
} // end Belle2 namespace

