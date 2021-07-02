/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <hlt/softwaretrigger/core/utilities.h>
#include <TRandom.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    bool makePreScale(const unsigned int& preScaleFactor, uint32_t* counter)
    {
      // A prescale factor of one is always true...
      if (preScaleFactor == 1) {
        return true;
        // ... and a prescale factor of 0 is always false...
      } else if (preScaleFactor == 0) {
        return false;
      } else {
        // All other cases are a bit more interesting.
        if (not counter) {
          // We do this by drawing a random number between 0 and preScaleFactor - 1 and comparing it to 0.
          // The probability to get back a true result is then given by 1/preScaleFactor.
          const unsigned int randomNumber = gRandom->Integer(preScaleFactor);
          return randomNumber == 0;
        } else {
          // Similar as above, but using the remainder between the counter and preScaleFactor.
          // If the remainder is not 0 we increment the counter, otherwise we reset the counter.
          const auto remainder = *counter % preScaleFactor;
          *counter = (remainder == 0) ? 1 : *counter + 1;
          return remainder == 0;
        }
      }
    }
  }
}
