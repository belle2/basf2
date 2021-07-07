/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef VTX_SENSITIVEDETECTOR_H_
#define VTX_SENSITIVEDETECTOR_H_

#include <vtx/dataobjects/VTXSimHit.h>
#include <vtx/dataobjects/VTXTrueHit.h>
#include <vxd/simulation/SensitiveDetector.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the VTX */
  namespace VTX {
    /** The VTX Sensitive Detector class. */
    typedef VXD::SensitiveDetector<VTXSimHit, VTXTrueHit> SensitiveDetector;
  } //end of namespace VTX
} // end of namespace Belle2

#endif /* VTX_SENSITIVEDETECTOR_H_ */
