/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/**************************************************************************
 * Description:                                                           *
 * Take the previously created DBObjects for ECL crystal timing from      *
 * bhabha events for crystals inside the CDC acceptance and radiative     *
 * bhabha and/or cosmic muons for crystal near and outside the CDC        *
 * acceptance and merge them.  Does not not explicitly check the          *
 * regions where the crystals are, it just prioritises certain            *
 * payloads over others.                                                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLReferenceCrystalPerCrateCalib.h>

/* Basf2 headers. */
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using previously created payloads */
    class eclMergingCrystalTimingAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclMergingCrystalTimingAlgorithm();

      /**..Destructor */
      ~eclMergingCrystalTimingAlgorithm() {}


      /*** Parameters ***/

      /**< Read the previous crystal payload values for comparison */
      bool readPrevCrysPayload;

    protected:

      /**..Run algorithm */
      EResult calibrate() override;

    private:

      // Payloads to be merged
      /** ECLCrystalTimeOffsetBhabha payload that we want to read from the DB */
      DBObjPtr<ECLCrystalCalib> m_ECLCrystalTimeOffsetBhabha;

      /** ECLCrystalTimeOffsetBhabhaGamma payload that we want to read from the DB */
      DBObjPtr<ECLCrystalCalib> m_ECLCrystalTimeOffsetBhabhaGamma;

      /** ECLCrystalTimeOffsetCosmic payload that we want to read from the DB */
      DBObjPtr<ECLCrystalCalib> m_ECLCrystalTimeOffsetCosmic;

      // Additional payloads
      /** ECLReferenceCrystalPerCrateCalib payload that we want to read from the DB */
      DBObjPtr<ECLReferenceCrystalPerCrateCalib> m_ECLReferenceCrystalPerCrateCalib;

      /** ECLCrateTimeOffset payload that we want to read from the DB */
      DBObjPtr<ECLCrystalCalib> m_ECLCrateTimeOffset;

      /** Number of Crystals expected */
      static constexpr int m_numCrystals = ECLElementNumbers::c_NCrystals;

      /** Number of Crates expected */
      static constexpr int m_numCrates = 52;
    };
  }
} // namespace Belle2


