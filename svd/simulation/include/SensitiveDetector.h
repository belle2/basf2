/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVDSENSITIVEDETECTOR_H
#define SVDSENSITIVEDETECTOR_H

#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/simulation/SensitiveDetector.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the SVD */
  namespace SVD {
    /** The SVD Sensitive Detector class. */
    typedef VXD::SensitiveDetector<SVDSimHit, SVDTrueHit> SensitiveDetector;
  } //end of namespace svd
} // end of namespace Belle2

#endif /* SVDSENSITIVEDETECTOR_H */
