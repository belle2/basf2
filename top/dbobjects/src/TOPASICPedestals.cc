/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPASICPedestals.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

bool TOPASICPedestals::setPedestals(const TProfile* profile)
{
  if (!profile) return false;
  if (profile->GetNbinsX() != c_WindowSize) {
    B2ERROR("TOPASICPedestals::setPedestals:  TProfile with wrong number of bins");
    return false;
  }
  m_offset = profile->GetMinimum();
  for (int i = 0; i < c_WindowSize; i++) {
    unsigned dif = int(profile->GetBinContent(i + 1) - m_offset + 0.5);
    if (dif > 255) return false;
    unsigned err = int(profile->GetBinError(i + 1) * sqrt(12.0) + 0.5);
    if (err > 255) return false;
    m_pedestals[i] = dif + (err << 8);
  }

  return true;

}


ClassImp(TOPASICPedestals);



