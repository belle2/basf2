/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Analze 25-crystal energy sums from Bhabha events to obtain single      *
 * crystal calibrations. Can also be used to find the expected values     *
 * for the 25-crystal sums.                                               *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <ecl/calibration/eclee5x5Algorithm.h>
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using Bhabha events */
    class eclee5x5Algorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclee5x5Algorithm();

      /**..Destructor */
      virtual ~eclee5x5Algorithm() {}

      /** Setter for m_outputName */
      void setOutputName(const std::string& outputName) {m_outputName = outputName;}

      /** Getter for m_outputName */
      std::string getOutputName() {return m_outputName;}

      /** Setter for m_minEntries */
      void setMinEntries(int minEntries) {m_minEntries = minEntries;}

      /** Getter for m_minEntries */
      int getMinEntries() {return m_minEntries;}

      /** Setter for m_payloadName */
      void setPayloadName(const std::string& payloadname) {m_payloadName = payloadname;}

      /** Getter for m_payloadname */
      std::string getPayloadName() {return m_payloadName;}

      /** Setter for m_storeConst */
      void setStoreConst(bool storeConst) {m_storeConst = storeConst;}

      /** Getter for m_storeConst */
      bool getStoreConst() {return m_storeConst;}


    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate();

    private:

      /**..Parameters to control job to find energy calibration using Bhabhas */
      std::string m_outputName = "eclee5x5Algorithm.root"; /**< file name for histogram output */
      int m_minEntries = 150;  /**< all crystals to be calibrated must have this many entries */
      /** Name of the payload to be stored. options: ECLCrystalEnergy5x5, ECLExpee5x5E, ECLeedPhiData, ECLeedPhiMC, or None */
      std::string m_payloadName = "ECLCrystalEnergy5x5";
      bool m_storeConst = true; /**< write payload to localdb if true */
    };
  }
} // namespace Belle2


