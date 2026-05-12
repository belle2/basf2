/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPCalPrecision.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  void TOPCalPrecision::set(const TProfile* pf)
  {
    if (pf->GetNbinsX() != 16) B2ERROR("TOPCalPrecision::set(TProfile*): profile histogram must have 16 bins, but found " <<
                                         pf->GetNbinsX());

    for (int i = 1; i <= pf->GetNbinsX(); i++) {
      m_precisions.push_back(pf->GetBinError(i));
    }
  }

  double TOPCalPrecision::get(int moduleID) const
  {
    unsigned i = moduleID - 1;
    if (i < m_precisions.size()) return m_precisions[i];
    return 0;
  }

} // end Belle2 namespace
