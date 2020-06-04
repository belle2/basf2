/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Take the previously created DBObjects for ECL crystal energy from      *
 * gamma pair events, mumu events, and bhabha events. Use these to create *
 * the final values for ECLCrystalEnergy DBObjects                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett, Christopher Hearty                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/database/DBObjPtr.h>
#include <calibration/CalibrationAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using previously created payloads */
    class eclMergingCrystalEAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclMergingCrystalEAlgorithm();

      /**..Destructor */
      virtual ~eclMergingCrystalEAlgorithm() {}

    protected:

      /**..Run algorithm */
      virtual EResult calibrate() override;

    private:

      /** The output payload name */
      const std::string m_payloadName = "ECLCrystalEnergy";

      /** ECLCrystalEnergyGammaGamma payloads that we want to read from the DB */
      DBObjPtr<ECLCrystalCalib> m_existingGammaGamma;

      /** ECLCrystalEnergyMuMu payloads that we want to read from the DB */
      DBObjPtr<ECLCrystalCalib> m_existingMuMu;

      /** ECLCrystalEnergyee5x5 payloads that we want to read from the DB */
      DBObjPtr<ECLCrystalCalib> m_existing5x5;

      /** ECLCrystalEnergy payloads that we want to read from the DB */
      DBObjPtr<ECLCrystalCalib> m_existing;

      /** Number of Crystals expected */
      const int m_numCrystals = 8736;
    };
  }
} // namespace Belle2


