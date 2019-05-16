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
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/calibration/SVDHotStripsCalibrations.h>
#include <svd/calibration/SVDFADCMaskedStrips.h>
#include <TTree.h>
#include <map>


namespace Belle2 {
  namespace Background {

    /**
     * Class for monitoring beam background hit rates of SVD
     */
    class SVDHitRateCounter: public HitRateBase {

    public:

      /**
       * tree structure
       */
      struct TreeStruct {
        float layerAverageRates[4] = {0}; /**< layer average occupancy */
        float layerLadderAverageRates[4][16] = {0}; /**< [#layer][#ladder] */
        float layerSensorAverageRates[4][5] = {0}; /**< [#layer][#sensor] */
        float averageRate = 0; /**< total SVD average occupancy */
        int numEvents = 0; /**< number of events accumulated */
        bool valid = false;  /**< status: true = rates valid */

        /**
         * normalize accumulated hits to single event
         */
        void normalize()
        {
          if (numEvents == 0) return;
          averageRate /= numEvents;
          for (auto& rate : layerAverageRates) rate /= numEvents;
          for (auto& row : layerLadderAverageRates) {
            for (auto& rate : row) {
              rate /= numEvents;
            }
          }
          for (auto& row : layerSensorAverageRates) {
            for (auto& rate : row) {
              rate /= numEvents;
            }
          }
        }
      };

      /**
       * Constructor
       */
      SVDHitRateCounter()
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

      /**
       * Return number of strips on a sensor.
       * @param layer layer number of the sensor (starting from 0)
       * @param isU true if the sensor is U side, false if V.
       */
      int nStripsOnLayerSide(int layer, bool isU)
      {
        if (!isU && layer > 0) return 512; // V side on Layer 4,5,6
        else return 768;
      }

    private:

      // class parameters: to be set via constructor or setters
      int m_nLayers = 4; /**< number of layers */
      int m_nLadders[4] = {7, 10, 12, 16}; /**< number of ladders on each layer */
      int m_nSensors[4] = {2, 3, 4, 5}; /**< number of sensors on a ladder on each layer */

      // tree structure
      TreeStruct m_rates; /**< tree variables */

      // buffer
      std::map<unsigned, TreeStruct> m_buffer; /**< average rates in time stamps */

      // collections
      StoreArray<SVDShaperDigit> m_digits;  /**< collection of digits */

      // DB payloads
      SVDHotStripsCalibrations m_HotStripsCalib; /**< payload for hot strips */
      SVDFADCMaskedStrips m_FADCMaskedStrips; /**< payload for strips masked on FADC level */

      // other
      int m_activeStrips; /**< number of active strips */
      int m_layerActiveStrips[4] = {0}; /**< number of active strips in each layer */
      int m_layerLadderActiveStrips[4][16] = {0}; /**< number of active strips in each layer, ladder */
      int m_layerSensorActiveStrips[4][5] = {0}; /**< number of active strips in each layer, sensor position */

    };

  } // Background namespace
} // Belle2 namespace
