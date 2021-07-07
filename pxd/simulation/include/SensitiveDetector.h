/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <vxd/simulation/SensitiveDetector.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PXD */
  namespace PXD {
    /** The PXD Sensitive Detector class. */
    typedef VXD::SensitiveDetector<PXDSimHit, PXDTrueHit> SensitiveDetector;
  } //end of namespace PXD
} // end of namespace Belle2
