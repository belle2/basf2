/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <framework/database/DBObjPtr.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <calibration/CalibrationCollectorModule.h>
#include <TTree.h>
#include <map>


namespace Belle2 {
  namespace Background {

    /**
     * Class for monitoring beam background hit rates of ECL
     */
    class ECLHitRateCounter: public HitRateBase {

    public:

      /**
       * tree structure
       */
      struct TreeStruct {

        float averageRate = 0; /**< total detector average hit rate */
        int numEvents = 0; /**< number of events accumulated */
        bool valid = false;  /**< status: true = rates valid */
        float averageDspBkgRate = 0; /**<background rate calculated from ECL waveforms */

        /**
         * normalize accumulated hits to single event
         */
        void normalize()
        {
          if (numEvents == 0) return;
          averageRate /= numEvents;
          averageDspBkgRate /= numEvents;
        }

      };

      /**
       * Constructor
       */
      ECLHitRateCounter()
      {}

      /**
       * Class initializer: set branch addresses and other staf
       * @param tree a valid TTree pointer
       */
      virtual void initialize(TTree* tree) override;

      /**
       * Clear time-stamp buffer to prepare for 'accumulate'
       */
      virtual void clear() override;

      /**
       * Accumulate hits
       * @param timeStamp time stamp
       */
      virtual void accumulate(unsigned timeStamp) override;

      /**
       * Normalize accumulated hits (e.g. transform to rates)
       * @param timeStamp time stamp
       */
      virtual void normalize(unsigned timeStamp) override;

    private:

      // class parameters: to be set via constructor or setters

      // tree structure
      TreeStruct m_rates; /**< tree variables */

      // buffer
      std::map<unsigned, TreeStruct> m_buffer; /**< average rates in time stamps */

      // collections
      StoreArray<ECLDigit> m_digits;  /**< collection of digits */
      StoreArray<ECLDsp> m_dsps;

      // DB payloads
      // Enegy and electronics calibration for ECL
      //DBObjPtr<ECLCrystalCalib> m_ElectronicsCalib("ECLCrystalElectronics"), m_ECLECalib("ECLCrystalEnergy");

      std::vector<float> electronicsCalib;
      std::vector<float> energyCalib;

      // other

    };

  } // Background namespace
} // Belle2 namespace

