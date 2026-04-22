/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPCalTOFCorrection.h>

using namespace std;

namespace Belle2 {

  void TOPCalTOFCorrection::set(const TProfile* pf)
  {
    m_zmin = pf->GetXaxis()->GetXmin();
    m_dz = pf->GetXaxis()->GetBinWidth(1);
    for (int i = 1; i <= pf->GetNbinsX(); i++) {
      if (pf->GetBinEntries(i) > 100) m_corrections.push_back(pf->GetBinContent(i));
      else m_corrections.push_back(0.);
    }
  }

  double TOPCalTOFCorrection::get(double z) const
  {
    if (m_corrections.empty()) return 0;
    if (z < m_zmin) return 0;

    unsigned i = (z - m_zmin) / m_dz;
    if (i < m_corrections.size()) return m_corrections[i];
    return 0;
  }

} // end Belle2 namespace

