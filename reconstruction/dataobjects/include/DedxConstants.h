/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  namespace Dedx {
    /** total number of tracking detectors considered */
    const int c_num_detectors = 3;

    /** list of trackig detectors, starting with innermost detector */
    enum Detector { c_PXD, c_SVD, c_CDC };
  }
}
