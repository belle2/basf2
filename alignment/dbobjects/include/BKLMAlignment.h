/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/DBObjCalibrationConstMapBase.h>

namespace Belle2 {
  /// BKLM alignment (and maybe some calibration) parameters
  class BKLMAlignment: public DBObjCalibrationConstMapBase {
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
    BKLMAlignment() : DBObjCalibrationConstMapBase() {}
    /// Destructor
    ~BKLMAlignment() {}

    // ------------- Interface to global Millepede calibration ----------------
    /// Get global unique id
    static unsigned short getGlobalUniqueID() {return 30;}
    /// Get global parameter
    double getGlobalParam(unsigned short element, unsigned short param) {return get(element, param);}
    /// Set global parameter
    void setGlobalParam(double value, unsigned short element, unsigned short param) {set(element, param, value);}
    /// TODO: list stored global parameters
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() {return {};}
    // ------------------------------------------------------------------------

  private:
    ClassDef(BKLMAlignment, 2); /**< BKLM allignment and calibration constants */
  };
} // end namespace Belle2

