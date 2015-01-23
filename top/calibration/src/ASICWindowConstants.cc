/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/calibration/ASICWindowConstants.h>

using namespace std;
using namespace Belle2;

bool TOP::ASICWindowConstants::setPedestals(const TProfile* profile)
{
  if (!profile) return false;
  if (profile->GetNbinsX() != c_WindowSize) return false;

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


bool TOP::ASICWindowConstants::setGains(const std::vector<float> gains, float error)
{

  if (gains.size() != c_WindowSize) return false;

  // TODO: better procedure!
  for (int i = 0; i < c_WindowSize; i++) {
    float gain = gains[i] * m_unit;
    if (gain > 0 and (gain + 0.5) < 0x10000) {
      m_gains[i] = int(gain + 0.5);
    } else {
      return false;
    }
  }
  error *= m_unit;
  m_gainError = int(error + 0.5);
  return true;

}


ClassImp(TOP::ASICWindowConstants)



