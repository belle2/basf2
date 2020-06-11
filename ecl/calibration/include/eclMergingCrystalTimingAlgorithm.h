/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Take the previously created DBObjects for ECL crystal timing from      *
 * bhabha events for crystals inside the CDC acceptance and radiative     *
 * bhabha and/or cosmic muons for crystal near and outside the CDC        *
 * acceptance and merge them.  Does not not explicitly check the          *
 * regions where the crystals are, it just prioritises certain            *
 * payloads over others.                                                  *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *   Ewan Hill   <ehill@mail.ubc.ca>                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/database/DBObjPtr.h>
#include <calibration/CalibrationAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLReferenceCrystalPerCrateCalib.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using previously created payloads */
    class eclMergingCrystalTimingAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclMergingCrystalTimingAlgorithm();

      /**..Destructor */
      virtual ~eclMergingCrystalTimingAlgorithm() {}

    protected:

      /**..Run algorithm */
      virtual EResult calibrate() override;

    private:

      /** The output payload name */
      const std::string m_payloadName = "ECLCrystalTimeOffset";

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
      const int m_numCrystals = 8736;

      /** Number of Crates expected */
      const int m_numCrates = 52;
    };
  }
} // namespace Belle2


