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
#include <map>
#include <iostream>
#include <fstream>
#include <string>
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
        float averageDspBkgRate[16]; /**<background rate calculated from ECL waveforms */
        int numEventsSegments[16]; /**< number of events per segment */
        bool validDspRate = false; /**< status for rates calculated from waveforms, true if waveforms for all crystals are recorded */

        /**
         * normalize accumulated hits to single event
         */
        void normalizeDigits()
        {
          if (numEvents == 0) return;
          averageRate /= numEvents;
        }
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

      // tree structure
      TreeStruct m_rates; /**< tree variables */

      // buffer
      std::map<unsigned, TreeStruct> m_buffer; /**< average rates in time stamps */

      // collections
      StoreArray<ECLDigit> m_digits;  /**< collection of digits */
      StoreArray<ECLDsp> m_dsps;

      std::vector<float> electronicsCalib;
      std::vector<float> energyCalib;

      // other
      std::ifstream ifss;
      std::string segments;
      Belle2::ECL::ECLGeometryPar* geom;

      int findSegment(int cellid)
      {
        return segment_map.find(cellid)->second;
      }

      std::map<int, int> segment_map;

      void segmentECL()
      {
        geom = Belle2::ECL::ECLGeometryPar::Instance();
        for (int cid = 1; cid < 8737; cid++) {
          geom->Mapping(cid);
          const B2Vector3D position = geom->GetCrystalPos(cid - 1);
          const double phi = position.Phi();
          const double z = position.Z();

          if (cid < 1297) {
            if (phi > 0.7853 && phi < 2.356) {
              segment_map.insert(std::pair<int, int>(cid, 0));
            } else if (phi >= 2.356 || phi <= -2.356) {
              segment_map.insert(std::pair<int, int>(cid, 1));
            } else if (phi > -2.356 && phi < -0.7853) {
              segment_map.insert(std::pair<int, int>(cid, 2));
            } else {
              segment_map.insert(std::pair<int, int>(cid, 3));
            }
          } else if (cid > 1296 && cid < 7777) {
            if (z > 0) {
              if (phi > 0.7853 && phi < 2.356) {
                segment_map.insert(std::pair<int, int>(cid, 4));
              } else if (phi >= 2.356 && phi <= -2.356) {
                segment_map.insert(std::pair<int, int>(cid, 5));
              } else if (phi > -2.356 && phi < -0.7853) {
                segment_map.insert(std::pair<int, int>(cid, 6));
              } else {
                segment_map.insert(std::pair<int, int>(cid, 7));
              }
            } else {
              if (phi > 0.7853 && phi < 2.356) {
                segment_map.insert(std::pair<int, int>(cid, 8));
              } else if (phi >= 2.356 && phi <= -2.356) {
                segment_map.insert(std::pair<int, int>(cid, 9));
              } else if (phi > -2.356 && phi < -0.7853) {
                segment_map.insert(std::pair<int, int>(cid, 10));
              } else {
                segment_map.insert(std::pair<int, int>(cid, 11));
              }
            }
          } else {
            if (phi > 0.7853 && phi < 2.356) {
              segment_map.insert(std::pair<int, int>(cid, 12));
            } else if (phi >= 2.356 && phi <= -2.356) {
              segment_map.insert(std::pair<int, int>(cid, 13));
            } else if (phi > -2.356 && phi < -0.7853) {
              segment_map.insert(std::pair<int, int>(cid, 14));
            } else {
              segment_map.insert(std::pair<int, int>(cid, 15));
            }
          }
        }
      }
    };
  }
}

