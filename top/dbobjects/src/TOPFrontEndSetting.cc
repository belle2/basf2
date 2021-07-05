/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPFrontEndSetting.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  void TOPFrontEndSetting::setWindowShifts(std::vector<int> shifts)
  {
    if (shifts.size() != 6) {
      B2ERROR("TOPFrontEndSetting::setWindowShifts: vector size must be 6");
      return;
    }
    for (int i = 0; i < 6; i++) m_windowShifts[i] = shifts[i];
  }


} // end Belle2 namespace

