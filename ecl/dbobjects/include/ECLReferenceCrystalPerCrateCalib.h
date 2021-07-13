/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/**************************************************************************
 * Title:  ECLReferenceCrystalPerCrateCalib                               *
 *                                                                        *
 * General database object to keep track of the reference crystals,       *
 * one for each crate.  This was Originally designed for                  *
 * determining which crystal to define as having a crystal time           *
 * calibration consant fixed to a specific value to ensure there are      *
 * no extra degrees of freedom with the combiation of crystal and crate   *
 * time calibration constants.                                            *
 *                                                                        *
 * 1 <= cell id <= 8736                                                   *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /** General DB object to store one reference crystal per per ECL crate
      for calibration purposes. */
  class ECLReferenceCrystalPerCrateCalib: public TObject {

  public:

    /** Constructor. */
    ECLReferenceCrystalPerCrateCalib() : m_refCrystals() { };

    /** Get vector of calibration constants. One entry per crate. */
    const std::vector<short>& getReferenceCrystals() const {return m_refCrystals;};

    /** Set vector of constants with uncertainties. One entry per crate. */
    void setCalibVector(const std::vector<short>& refCrystals)
    {
      if (refCrystals.size() != 52) {B2FATAL("ECLReferenceCrystalPerCrateCalib: wrong size vector uploaded, " << refCrystals.size() << " instead of 52 (one crystal per crate)");}
      for (int i = 0; i < 52; i++) {
        if ((refCrystals[i] < 1) || (refCrystals[i] > 8736)) {
          B2FATAL("ECLReferenceCrystalPerCrateCalib: cell id outside valid range of 1..8736.  Crate " << i << " has reference cid = " <<
                  refCrystals[i]);
        }
      }
      m_refCrystals = refCrystals;
    };

  private:
    std::vector<short> m_refCrystals; /**< Calibration constants */

    ClassDef(ECLReferenceCrystalPerCrateCalib, 3); /**< ClassDef */
  };
}
