/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Matthew Barrett                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPTBDETECTORTYPES_H
#define TOPTBDETECTORTYPES_H


namespace Belle2 {
  namespace TOPTB {

    /**
     * detector type enumerators
     * relation to detector creator functions must be one-to-one
     */
    enum EDetectorType {c_undefined, c_scintillator, c_sciFi, c_timing, c_veto};

  }
}

#endif
