/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/utilities/Chi2MinimumFinder1D.h>
#include <math.h>

namespace Belle2 {
  namespace TOP {

    Chi2MinimumFinder1D::Chi2MinimumFinder1D(int nbins, double xmin, double xmax):
      m_xmin(xmin), m_xmax(xmax)
    {
      if (nbins <= 0) {
        B2ERROR("Chi2MinimumFinder1D: nbins must be positive integer");
        return;
      }
      if (m_xmax <= m_xmin) {
        B2ERROR("Chi2MinimumFinder1D: search range max < min");
        return;
      }
      m_dx = (m_xmax - m_xmin) / nbins;
      double x = m_xmin + m_dx / 2;
      while (x < m_xmax) {
        m_x.push_back(x);
        x += m_dx;
      }
      m_chi2.resize(m_x.size(), 0);
    }


    void Chi2MinimumFinder1D::add(unsigned i, double chi2)
    {
      if (i < m_chi2.size()) {
        m_chi2[i] += chi2;
        m_searched = false;
      } else B2WARNING("Chi2MinimumFinder1D::add: index out of range");
    }


    void Chi2MinimumFinder1D::findMinimum()
    {
      unsigned i0 = 1;
      for (unsigned i = i0; i < m_chi2.size() - 1; i++) {
        if (m_chi2[i] < m_chi2[i0]) i0 = i;
      }
      auto x = getMinimum(m_chi2[i0 - 1], m_chi2[i0], m_chi2[i0 + 1]);
      m_minimum = Minimum(m_x[i0] + x.position * m_dx, x.error * m_dx, x.chi2, x.valid);
    }


    Chi2MinimumFinder1D::Minimum Chi2MinimumFinder1D::getMinimum(double yLeft,
        double yCenter,
        double yRight) const
    {
      // parabola: y = ax^2 + bx + c
      double DL = yLeft - yCenter;
      double DR = yRight - yCenter;
      double A = (DR + DL) / 2;
      if (A <= 0) return Minimum(0, 0, yCenter, false);
      double B = (DR - DL) / 2;
      double x = - B / 2 / A;
      double chi2_min = A * x * x + B * x + yCenter;
      return Minimum(x, sqrt(1 / A), chi2_min, true);
    }


    TH1F Chi2MinimumFinder1D::getHistogram(std::string name, std::string title) const
    {
      TH1F h(name.c_str(), title.c_str(), m_x.size(), m_xmin, m_xmax);
      for (unsigned i = 0; i < m_chi2.size(); i++) {
        h.SetBinContent(i + 1, m_chi2[i] - m_minimum.chi2);
      }
      return h;
    }

  } // namespace TOP
} // namespace Belle2
