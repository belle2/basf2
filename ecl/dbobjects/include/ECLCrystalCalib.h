/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>

/* Basf2 headers. */
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /** General DB object to store one calibration number per ECL crystal. */
  class ECLCrystalCalib: public TObject {

  public:

    /** Constructor. */
    ECLCrystalCalib() : m_CalibConst(), m_CalibConstUnc() { };

    /** Get vector of calibration constants. One entry per crystal. */
    const std::vector<float>& getCalibVector() const {return m_CalibConst;};

    /** Get vector of uncertainties on calibration constants. */
    const std::vector<float>& getCalibUncVector() const {return m_CalibConstUnc;};

    /** Set vector of constants with uncertainties. One entry per crystal */
    void setCalibVector(const std::vector<float>& CalibConst, const std::vector<float>& CalibConstUnc)
    {
      if ((CalibConst.size() != ECLElementNumbers::c_NCrystals) ||
          (CalibConstUnc.size() != ECLElementNumbers::c_NCrystals)) {
        B2FATAL("ECLCrystalCalib: wrong size vector uploaded, " <<
                CalibConst.size() << " " << CalibConstUnc.size() <<
                " instead of " << ECLElementNumbers::c_NCrystals);
      }
      m_CalibConst = CalibConst;
      m_CalibConstUnc = CalibConstUnc;
    };

  private:
    std::vector<float> m_CalibConst; /**< Calibration constants */
    std::vector<float> m_CalibConstUnc; /**< Uncertainty on constants */

    ClassDef(ECLCrystalCalib, 1); /**< ClassDef */
  };
}
