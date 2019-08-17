/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/DBObjCalibrationConstMapBase.h>

namespace Belle2 {
  /// BKLM alignment (and maybe some calibration) parameters
  class BKLMMisAlignment: public DBObjCalibrationConstMapBase {
  public:
    // Alignment parameters unique identifiers:
    /// Sector/SuperLayer/Module alignment in local u
    static const baseType dU = 1;
    /// Sector/SuperLayer/Module alignment in local v
    static const baseType dV = 2;
    /// Sector/SuperLayer/Module alignment in local w
    static const baseType dW = 3;
    /// Sector/SuperLayer/Module alignment in local alpha
    static const baseType dAlpha = 4;
    /// Sector/SuperLayer/Module alignment in local beta
    static const baseType dBeta = 5;
    /// Sector/SuperLayer/Module alignment in local gamma
    static const baseType dGamma = 6;

    /// Constructor
    BKLMMisAlignment() : DBObjCalibrationConstMapBase() {}
    /// Destructor
    ~BKLMMisAlignment() {}
  private:
    ClassDef(BKLMMisAlignment, 1); /**< BKLM mis-alignment constants */
  };
} // end namespace Belle2
