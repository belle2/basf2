/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Peter Kvasnicka             *
 *               Martin Ritter, Tadeas Bilka                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TEL_SENSITIVEDETECTOR_H_
#define TEL_SENSITIVEDETECTOR_H_

#include <testbeam/vxd/dataobjects/TelSimHit.h>
#include <testbeam/vxd/dataobjects/TelTrueHit.h>
#include <vxd/simulation/SensitiveDetector.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstruction of the TB */
  namespace TEL {
    /** The TB Sensitive Detector class. */
    typedef VXD::SensitiveDetector<TelSimHit, TelTrueHit> SensitiveDetector;
  } //end of namespace TB
} // end of namespace Belle2

#endif /* TEL_SENSITIVEDETECTOR_H_ */
