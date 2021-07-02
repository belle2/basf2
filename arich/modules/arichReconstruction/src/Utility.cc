/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "arich/modules/arichReconstruction/Utility.h"
#include "TVector3.h"
#include "TRotation.h"
#include <cmath>


namespace Belle2 {
  namespace arich {
    //****************************************************************************
    // originated from the Numerical recipes error function
    double Erf(double x)
    {
      const double Z1 = 1;
      const double HF = Z1 / 2;
      const double C1 = 0.56418958;

      const double P10 = +3.6767877;
      const double Q10 = +3.2584593;
      const double P11 = -9.7970465E-2;

      static double P2[5] = { 7.3738883, 6.8650185,  3.0317993, 0.56316962, 4.3187787e-5 };
      static double Q2[5] = { 7.3739609, 15.184908, 12.79553,   5.3542168,  1. };

      const double P30 = -1.2436854E-1;
      const double Q30 = +4.4091706E-1;
      const double P31 = -9.6821036E-2;

      double V = fabs(x);
      double H = 0;
      double Y;

      if (V < HF) {
        Y = V * V;
        H = x * (P10 + P11 * Y) / (Q10 + Y);
      } else {
        if (V < 4) {
          double AP = P2[4];
          double AQ = Q2[4];
          for (int c1 = 3; c1 >= 0; c1--) {
            AP = P2[c1] + V * AP;
            AQ = Q2[c1] + V * AQ;
          }
          H = 1 - exp(-V * V) * AP / AQ;
        } else {
          Y = 1. / V * V;
          H = 1 - exp(-V * V) * (C1 + Y * (P30 + P31 * Y) / (Q30 + Y)) / V;
        }
        if (x < 0)
          H = - H;
      }
      return H;
    }


    //*******************************************************************************

    TVector3 Refraction(const TVector3 s,  const double n)
    {
      double x = s.x() / n;
      double y = s.y() / n;

      if (x * x + y * y > 1) return TVector3();
      if (s.z() < 0) return  TVector3(x, y, -sqrt(1 - x * x - y * y));
      else   return  TVector3(x, y, sqrt(1 - x * x - y * y));
    }
    //---------------------------------------------------------------


    int Refraction(TVector3 s, TVector3 norm, double n, TVector3
                   &newdir)
    {
      double sp = s * norm;
      double sign = (sp > 0) * 2 - 1;
      TVector3 vsth = (s - sp * norm) * (1 / n);
      double cth = sqrt(1 - vsth.Mag2());
      newdir = vsth + cth * norm * sign;
      return 1;
    }

    //---------------------------------------------------------------
    TVector3 setThetaPhi(double theta, double fi)
    {
      TVector3 r;
      r[0] = cos(fi) * sin(theta);
      r[1] = sin(fi) * sin(theta);
      r[2] = cos(theta);
      return r;

    }
    //---------------------------------------------------------------

    TRotation TransformToFixed(TVector3 r)
    {
      // Description: Calculates the base vectors of the track system
      // Output:
      TVector3  fX, fY, fZ;// base vectors

      double ss = 1 - r.y() * r.y();
      if (ss > 0) {
        ss = sqrt(ss);
        fZ = r;
        fX = TVector3(fZ.z() / ss, 0, -fZ.x() / ss);
        fY = fZ.Cross(fX);
      } else {
        fZ = TVector3(0, 1, 0);
        fX = TVector3(0, 0, 1);
        fY = TVector3(1, 0, 0);
      }
      TRotation transform;
      transform.RotateAxes(fX, fY, fZ);
      transform.Invert();
      return transform;
    }

    TRotation TransformFromFixed(TVector3 r)
    {
      // Description: Calculates the base vectors of the track system
      // Output:
      TVector3  fX, fY, fZ;// base vectors

      double ss = 1 - r.y() * r.y();
      if (ss > 0) {
        ss = sqrt(ss);
        fZ = r;
        fX = TVector3(fZ.z() / ss, 0, -fZ.x() / ss);
        fY = fZ.Cross(fX);
      } else {
        fZ = TVector3(0, 1, 0);
        fX = TVector3(0, 0, 1);
        fY = TVector3(1, 0, 0);
      }
      TRotation transform;
      transform.RotateAxes(fX, fY, fZ);
      return transform;
    }


    double  ExpectedCherenkovAngle(double p, double mass, double refind)
    {
      // Description:
      // calculates Cerenkov angle of a particle of mass MASS and momentum P
      //
      // Input Parameters:
      // p  momentum of the particle
      // mass of the particle
      //
      // return expected cherenkov angle

      if (mass <= 0 || p < 0) return 0;
      double a = p / mass;
      double b = a / sqrt(1 + a * a) * refind;
      if (b > 1) return acos(1 / b);
      return 0;
    }

    double SquareInt(double aaa, double fi, double mean, double sigma)
    {

      double aa = aaa / sqrt(2); // 2
      fi += M_PI / 4;
      fi = fi / (2 * M_PI);
      fi = 8 * (fi - floor(fi));

      if (fi < 0) fi += 16;
      if (fi > 8) fi = 16 - fi;
      if (fi > 4) fi = 8 - fi;
      if (fi > 2) fi = 4 - fi;
      if (fi > 1) fi = 2 - fi;
      fi = fi * M_PI / 4;

      double sf = aa * sin(fi);
      double cf = aa * cos(fi);

      double a[4], b[4];
      //double n0 = 2*cf;
      double  n0 = 2 * aa * aa / (cf + sf); // A.Petelin 21.12.2006

      const double sq2pi      = 1 / sqrt(2.*M_PI);
      double sqrt2sigma = 1 / (M_SQRT2 * sigma);


      a[0] = (mean + sf) * sqrt2sigma;
      a[1] = (mean - sf) * sqrt2sigma;
      a[2] = (mean + cf) * sqrt2sigma;
      a[3] = (mean - cf) * sqrt2sigma;

      for (int i = 0; i < 4; i++) b[i] = exp(-a[i] * a[i]);
      for (int i = 0; i < 4; i++) a[i] = Erf(a[i]);

      double gint = 0;
      const double dsf = 0.001;
      if (fabs(sf) > dsf) gint += 0.5 * n0 * (a[0] - a[1]);

      double dy = (cf - sf);
      double dx = (cf + sf);

      if (fabs(dx) > dsf && fabs(dy) > dsf) {

        double k = dy / dx + dx / dy;
        // double n = k * cf - sf;
        double n = k * cf;   // A.Petelin 21.12.2006
        //                        +sf    -sf    +cf    -cf
        gint +=
          sq2pi      * k * sigma * (-b[0]  - b[1]  + b[2]  + b[3]) +
          0.5        * k * mean  * (-a[0]  - a[1]  + a[2]  + a[3]) +
          0.5        * n       * (-a[0]  + a[1]  + a[2]  - a[3]);

      }

      return gint / (aa);
    }
  }
}
