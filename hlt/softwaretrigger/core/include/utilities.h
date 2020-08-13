/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

namespace Belle2 {
  namespace SoftwareTrigger {
    /// Helper function to do a prescaling using a random integer number and the prescaling factor from the object.
    bool makePreScale(const unsigned int& preScaleFactor);
  }
}
