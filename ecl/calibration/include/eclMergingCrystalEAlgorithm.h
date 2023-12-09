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
#include <ecl/dbobjects/ECLCrystalCalib.h>

/* Basf2 headers. */
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

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
      const int m_numCrystals = ECLElementNumbers::c_NCrystals;
    };
  }
} // namespace Belle2


