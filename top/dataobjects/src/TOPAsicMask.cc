/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dataobjects/TOPAsicMask.h>

namespace Belle2 {

  bool TOPAsicMask::isActive(int moduleID, unsigned channel) const
  {
    if (m_masks.empty()) return true;

    unsigned asic = channel / 8;
    unsigned bs = (moduleID - 1) * 4 + asic / 16;
    if (bs >= m_masks.size()) return true;

    unsigned word = m_masks[bs];
    unsigned bit = asic % 16;
    return ((word >> bit) & 0x1) == 0;
  }

} // end namespace Belle2

