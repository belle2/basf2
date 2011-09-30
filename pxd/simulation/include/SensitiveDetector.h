/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Peter Kvasnicka             *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXD_SENSITIVEDETECTOR_H_
#define PXD_SENSITIVEDETECTOR_H_

#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/vxd/SensitiveDetector.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PXD */
  namespace PXD {
    /** The PXD Sensitive Detector class. */
    typedef VXD::SensitiveDetector<PXDSimHit, PXDTrueHit> SensitiveDetector;
  } //end of namespace PXD
} // end of namespace Belle2

#endif /* PXD_SENSITIVEDETECTOR_H_ */
