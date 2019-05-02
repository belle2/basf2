/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Andrea Fodor                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/database/DBObjPtr.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <calibration/CalibrationCollectorModule.h>
#include <framework/geometry/B2Vector3.h>
#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <functional>
#include <algorithm>

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
        float averageDspBkgRate[16] = {0}; /**<background rate calculated from ECL waveforms [hits/second] */
        int numEventsSegments[16] = {0}; /**< number of events per segment */
        bool validDspRate = false; /**< status for rates calculated from waveforms, true if waveforms for all crystals are recorded */

        /**
         * normalize accumulated hits to single event
         */
        void normalizeDigits()
        {
          if (numEvents == 0) return;
          averageRate /= numEvents;
        }

        /**
         * normalize accumulated rates based on ECL waveforms
         */
        void normalizeDsps()
        {
          if (numEventsSegments == 0) return;
          std::transform(averageDspBkgRate, averageDspBkgRate + 16, numEventsSegments, averageDspBkgRate, std::divides<float>());

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

      //functions
      /**
       * Performs ECL segmentation;
       * Done once per run;
       * Populates a map which connects each ECL crystal with a segment number (0-15);
       * Segments 0-3 are in the forward endcap, 4-7 are in the barrel with z<0,
       * 8-11 are in the barrel with z>0, 12-15 are in the backward endcap;
       * Segment 0 contains crystals with 45deg < phi < 135deg,
       * Segment 1 contains crystals with 135deg < phi < 225deg,
       * Segment 2 contains crystals with 225deg < phi < 315deg,
       * Segment 3 contains crystals with phi < 45deg or phi > 315deg,
       * With the same angular patter continuing for barrel and BWD encap segments
       */
      void segmentECL();

      /**
       * Find the correcsponding ECL segment based on the cellID
       *@param cellid ECL crystal CellID
       */
      int findECLSegment(int cellid)
      {
        return m_segmentMap.find(cellid)->second;
      }

      /**
       * Find the electronics noise correction for each cellID
       * Reads a file with a histogram containing electronics noise level of each crystal
       */
      void findElectronicsNoise();

      // tree structure
      TreeStruct m_rates; /**< tree variables */

      // buffer
      std::map<unsigned, TreeStruct> m_buffer; /**< average rates in time stamps */

      // collections
      StoreArray<ECLDigit> m_digits;  /**< collection of digits */
      StoreArray<ECLDsp> m_dsps; /**< collection of ECL waveforms */

      std::vector<float> m_ADCtoEnergy; /**< vector used to store ECL calibration constants for each crystal */

      // other
      Belle2::ECL::ECLGeometryPar* m_geometry{nullptr}; /**< pointer to ECLGeometryPar */
      std::map<int, int> m_segmentMap; /**< map with keys containing ECL CellID and values containing segment number */
      std::map<int, float> m_noiseMap; /**< map with keys containing ECL CellID and values containing electronics noise [GeV] */
    };
  }
}

