/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

