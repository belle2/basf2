/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
