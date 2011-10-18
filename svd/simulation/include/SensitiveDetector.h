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

#ifndef SVDSENSITIVEDETECTOR_H
#define SVDSENSITIVEDETECTOR_H

#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/vxd/SensitiveDetector.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PXD */
  namespace svd {
    /** The PXD Sensitive Detector class. */
    typedef VXD::SensitiveDetector<SVDSimHit, SVDTrueHit> SensitiveDetector;
  } //end of namespace PXD
} // end of namespace Belle2

#endif /* SVDSENSITIVEDETECTOR_H */
