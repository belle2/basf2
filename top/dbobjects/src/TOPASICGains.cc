/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPASICGains.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  bool TOPASICGains::setGains(const std::vector<float>& gains, float error)
  {

    if (gains.size() != c_WindowSize) {
      B2ERROR("TOPASICGains::setGains:  vector with wrong number of elements");
      return false;
    }

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

} // end Belle2 namespace

