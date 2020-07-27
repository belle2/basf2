/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

