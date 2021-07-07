/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/utilities/Chi2MinimumFinder1D.h>

#include <framework/logging/Logger.h>

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

    Chi2MinimumFinder1D::Chi2MinimumFinder1D(const std::shared_ptr<TH1D> h)
    {
      m_xmin = h->GetXaxis()->GetXmin();
      m_xmax = h->GetXaxis()->GetXmax();
      m_dx = h->GetBinWidth(1);
      for (int i = 0; i < h->GetNbinsX(); i++) {
        m_x.push_back(h->GetBinCenter(i + 1));
        m_chi2.push_back(h->GetBinContent(i + 1));
      }
      m_entries = h->GetEntries() / h->GetNbinsX();
    }

    void Chi2MinimumFinder1D::clear()
    {
      for (auto& chi2 : m_chi2) chi2 = 0;
      m_entries = 0;
      m_searched = false;
    }


    void Chi2MinimumFinder1D::add(unsigned i, double chi2)
    {
      if (i < m_chi2.size()) {
        m_chi2[i] += chi2;
        m_searched = false;
        if (i == 0) m_entries++;
      } else {
        B2WARNING("Chi2MinimumFinder1D::add: index out of range");
      }
    }


    Chi2MinimumFinder1D& Chi2MinimumFinder1D::add(const Chi2MinimumFinder1D& other)
    {
      if (other.getXmin() !=  m_xmin or other.getXmax() !=  m_xmax or
          other.getBinCenters().size() != m_x.size()) {
        B2ERROR("Chi2MinimumFinder1D::add: finders with different ranges or binning "
                "can't be added");
        return *this;
      }
      const auto& chi2 = other.getChi2Values();
      for (unsigned i = 0; i < m_chi2.size(); i++) {
        m_chi2[i] += chi2[i];
      }
      m_entries += other.getEntries();
      m_searched = false;
      return *this;
    }


    void Chi2MinimumFinder1D::findMinimum()
    {
      if (m_chi2.size() < 3) {
        m_minimum.position = (m_xmin + m_xmax) / 2;
        m_minimum.error = (m_xmax - m_xmin) / 2;
        return;
      }

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
