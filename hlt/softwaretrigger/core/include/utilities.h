/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <stdint.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /// Helper function to do a prescaling using a random integer number or a counter and the prescaling factor from the object.
    bool makePreScale(const unsigned int& preScaleFactor, uint32_t* counter = nullptr);
  }
}
