/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/**
 * @file
 * Two enumerations (separate file to avoid header circular dependency).
 */

#ifndef GEOEKLMTYPES_H
#define GEOEKLMTYPES_H

namespace Belle2 {

  namespace EKLM {

    /** Type of sensitive detector corresponding to this volume. */
    enum SensitiveType {
      /** Strip. */
      c_SensitiveStrip = 0,
      /** SiPM. */
      c_SensitiveSiPM = 1,
      /** Readout boards. */
      c_SensitiveBoard = 2,
    };

  }

}

#endif

