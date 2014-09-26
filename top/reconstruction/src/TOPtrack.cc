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

    /**
     * Gaussian random number
     * @param sigma: sigma
     */
    inline double GaussRnd(double sigma) {return gRandom->Gaus(0., sigma);}

    TOPtrack::TOPtrack()
    {
      m_x = 0;
      m_y = 0;
      m_z = 0;
      m_px = 0;
      m_py = 0;
      m_pz = 0;
      m_trackLength = 0;
      m_charge = 0;
      m_pdg = 0;
      for (int i = 0; i < 10; i++) m_label[i] = 0;
      m_atTop = false;
      m_barID = 0;
    }

    TOPtrack::TOPtrack(double x, double y, double z,
                       double Px, double Py, double Pz,
                       double Tlen, int Q, int Lund)
    {
      m_x = x;
      m_y = y;
      m_z = z;
      m_px = Px;
      m_py = Py;
      m_pz = Pz;
      m_trackLength = Tlen;
      m_charge = Q;
      m_pdg = Lund;
      for (int i = 0; i < 10; i++) m_label[i] = 0;
      m_atTop = false;
      m_barID = 0;
    }

    double TOPtrack::getTOF(int Lund)
    {
      int lundc[5] = {11, 13, 211, 321, 2212};
      double masses[5] = {.511E-3, .10566, .13957, .49368, .93827};

      double mass = 0;
      for (int i = 0; i < 5; i++) {
        if (abs(Lund) == lundc[i]) {mass = masses[i]; break;}
      }
      double pmom = getP();
      double beta = pmom / sqrt(pmom * pmom + mass * mass);
      return m_trackLength / beta / Const::speedOfLight;
    }

    void TOPtrack::setTrackLength(double tof, double mass)
    {
      double pmom = getP();
      double beta = pmom / sqrt(pmom * pmom + mass * mass);
      m_trackLength = tof * Const::speedOfLight * beta;
    }

    int TOPtrack::getHypID()
    {
      int lundc[5] = {11, 13, 211, 321, 2212};
      for (int i = 0; i < 5; i++) {
        if (abs(m_pdg) == lundc[i]) return i + 1;
      }
      return 0;
    }

    int TOPtrack::toTop()
    {
      if (m_atTop) return m_barID;

      float r[3] = {(float) m_x, (float) m_y, (float) m_z};
      float p[3] = {(float) m_px, (float) m_py, (float) m_pz};
      float q = (float) m_charge;
      float t; int m;
      track2top_(r, p, &q, &t, &m);
      m_x = r[0]; m_y = r[1]; m_z = r[2];
      m_px = p[0]; m_py = p[1]; m_pz = p[2];
      m_trackLength += t * Const::speedOfLight;
      m_atTop = true;
      m_barID = m + 1;
      return m_barID;
    }

    void TOPtrack::smear(double sig_x, double sig_z,
                         double sig_theta, double sig_phi)
    {
      double p = getP();
      if (p == 0) return;
      double theta = acos(m_pz / p) + GaussRnd(sig_theta);
      double phi = atan2(m_py, m_px) + GaussRnd(sig_phi);
      m_px = p * cos(phi) * sin(theta);
      m_py = p * sin(phi) * sin(theta);
      m_pz = p * cos(theta);

      m_z += GaussRnd(sig_z);
      double dx = GaussRnd(sig_x);
      if (m_x * m_x + m_y * m_y != 0) phi = atan2(m_y, m_x);
      m_x += dx * sin(phi);
      m_y -= dx * cos(phi);
    }

    void TOPtrack::dump()
    {
      double pi = 4 * atan(1);
      using namespace std;
      cout << "TOPtrack::dump(): labels";
      for (int i = 0; i < 10; i++) cout << " " << m_label[i];
      cout << " PDG=" << m_pdg;
      cout << " charge=" << m_charge << endl;
      cout << "  p=" << setprecision(3) << getP() << " GeV/c";
      cout << "  theta=" << setprecision(3) << getTheta() / pi * 180;
      cout << "  phi=" << setprecision(3) << getPhi() / pi * 180 << endl;
      cout << "  x=" << m_x << " cm";
      cout << "  y=" << m_y << " cm";
      cout << "  z=" << m_z << " cm\n";
      cout << "  trackLength=" << setprecision(4) << m_trackLength << " cm";
      cout << "  atTop=" << m_atTop;
      cout << "  barID=" << m_barID;
      cout << endl;
    }

  } // end top namespace
} // end Belle2 namespace

