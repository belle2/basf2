/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/EnergyMask.h>
#include <framework/logging/Logger.h>
#include <algorithm>
#include <cmath>


namespace Belle2 {
  namespace TOP {

    unsigned EnergyMask::s_Nmin = 4;
    unsigned EnergyMask::s_Nmax = 256;

    EnergyMask::EnergyMask(double dyde, double dydL, double dydx, double dy, double dL, double dx, double dE):
      m_dE(dE), m_Wy(dy / abs(dyde))
    {
      if (dy <= 0) {
        B2ERROR("TOP::EnergyMask: dy must be > 0");
        return;
      }

      std::vector<double> x;
      x.push_back(m_Wy);
      x.push_back(abs(dL * dydL / dyde));
      x.push_back(abs(dx * dydx / dyde));
      std::sort(x.begin(), x.end());

      m_A = x[2];
      m_B = x[1];
      m_C = x[0];

      unsigned N = (m_A + m_B + m_C) / 2.0 / dE + 1;
      if (N < s_Nmin or N > s_Nmax) return;

      if (m_C / m_A > 0.001) {
        for (unsigned i = 0; i <= N; i++) {
          double E = i * dE;
          double p = threeSquareConvolution(E);
          m_mask.push_back(p);
        }
      } else {
        for (unsigned i = 0; i <= N; i++) {
          double E = i * dE;
          double p = twoSquareConvolution(E);
          m_mask.push_back(p);
        }
      }
    }


    double EnergyMask::getMask(double E) const
    {
      if (m_C / m_A > 0.001) {
        return threeSquareConvolution(E);
      } else {
        return twoSquareConvolution(E);
      }
    }


    double EnergyMask::getMask(int i) const
    {
      if (m_mask.empty()) return getMask(i * m_dE);
      return mask(i);
    }


    double EnergyMask::getMask(int i, double fract) const
    {
      if (m_mask.empty()) return getMask((i + fract) * m_dE);

      if (fract < 0) {
        i--;
        fract += 1;
      }
      return mask(i) * (1 - fract) + mask(i + 1) * fract;
    }


    double EnergyMask::twoSquareConvolution(double E) const
    {
      double x1 = (m_A - m_B) / 2;
      double x2 = (m_A + m_B) / 2;
      double x = abs(E);

      if (x < x1) {
        return 1 / m_A * m_Wy;
      } else if (x < x2) {
        return (x2 - x) / (x2 - x1) / m_A * m_Wy;
      } else {
        return 0;
      }
    }


    double EnergyMask::threeSquareConvolution(double E) const
    {
      double halfWid = (m_A + m_B + m_C) / 2;
      if (abs(E) >= halfWid) return 0;

      double t1 = halfWid - E;
      double t2 = halfWid + E;
      double p = t1 * t1 + t2 * t2;

      double t3 = t1 - m_A;
      double t4 = t2 - m_A;
      p -= copysign(t3 * t3, t3) + copysign(t4 * t4, t4);

      t3 = t1 - m_B;
      t4 = t2 - m_B;
      p -= copysign(t3 * t3, t3) + copysign(t4 * t4, t4);

      t3 = t1 - m_C;
      t4 = t2 - m_C;
      p -= copysign(t3 * t3, t3) + copysign(t4 * t4, t4);

      return p / (4 * m_A * m_B * m_C) * m_Wy;
    }

  } //TOP
} //Belle2
