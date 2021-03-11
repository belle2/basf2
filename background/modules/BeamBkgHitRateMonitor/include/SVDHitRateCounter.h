/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Hikaru Tanigawa, Ludovico Massaccesi       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/calibration/SVDHotStripsCalibrations.h>
#include <svd/calibration/SVDFADCMaskedStrips.h>
#include <TTree.h>
#include <map>
#include <string>


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
        float l3LadderSensorAverageRates[7][2] = {0}; /**< Layer 3 sensors [#ladder][#sensor] */
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
          for (auto& row : l3LadderSensorAverageRates) {
            for (auto& rate : row) {
              rate /= numEvents;
            }
          }
        }
      };

      /**
       * Constructor
       * @param svdShaperDigitsName name of the input SVDShaperDigits collection
       * @param thrCharge cut on cluster energy in electrons
       * @param ignoreHotStripsPayload Count also hot strips as active
       * @param ignoreMaskedStripsPayload Count also FADC-masked strips as active
       */
      explicit SVDHitRateCounter(const std::string& svdShaperDigitsName, double thrCharge,
                                 bool ignoreHotStripsPayload = false,
                                 bool ignoreMaskedStripsPayload = false):
        m_svdShaperDigitsName(svdShaperDigitsName), m_thrCharge(thrCharge),
        m_ignoreHotStripsPayload(ignoreHotStripsPayload),
        m_ignoreMaskedStripsPayload(ignoreMaskedStripsPayload)
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
       * Normalize TreeStruct
       * @param rates TreeStruct to be normalized
       */
      virtual void normalize_rates(TreeStruct& rates);

      /**
       * Normalize a TreeStruct that stores charge/energy, not hits.
       * @param rates TreeStruct to be normalized.
       *
       * Assumes the TreeStruct contains the total accumulated cluster
       * charge (in e-), and uses Const::ehEnergy and the mass of the
       * sensors to convert to the average dose rate per event in
       * mrad/s.
       *
       * Assumes the integration time is 155 ns (6 samples).
       */
      virtual void normalize_energy_rates(TreeStruct& rates);

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

      /**
       * Returns the (active) mass of the given sensor in Kg.
       * @param layer tha layer number (starting from 0, not 3)
       * @param ladder the ladder number (starting from 0, not 1)
       * @param sensor the sensor number (starting from 0, not 1)
       */
      double massOfSensor(int layer, int ladder, int sensor);

      /**
       * Returns wether a strips is active (neither hot nor masked),
       * taking into account the ignoreHotStrips and ignoreMaskedStrips
       * settings.
       * @param sensorID The VxdID of the sensor
       * @param isU The side of the sensor
       * @param strip The strip index
       */
      bool isStripActive(const VxdID& sensorID, const bool& isU, const unsigned short& strip);

    private:

      // class parameters: to be set via constructor or setters
      int m_nLayers = 4; /**< number of layers */
      int m_nLadders[4] = {7, 10, 12, 16}; /**< number of ladders on each layer */
      int m_nSensors[4] = {2, 3, 4, 5}; /**< number of sensors on a ladder on each layer */

      // tree structure
      TreeStruct m_rates; /**< tree variables for fired strips */
      TreeStruct m_rates_highE; /**< tree variables for high-energy clusters */
      TreeStruct m_rates_lowE; /**< tree variables for low-energy clusters */
      TreeStruct m_rates_energyU; /**< Tree variables for deposited energy (U-side) */

      // buffer
      std::map<unsigned, TreeStruct> m_buffer; /**< average strip occupancies in time stamps */
      std::map<unsigned, TreeStruct> m_buffer_highE; /**< average cluster occupancies (high energy) in time stamps */
      std::map<unsigned, TreeStruct> m_buffer_lowE; /**< average cluster occupancies (low energy) in time stamps */
      std::map<unsigned, TreeStruct> m_buffer_energyU; /**< Average deposited energy (U-side) per event in time stamps */

      // collections
      StoreArray<SVDShaperDigit> m_digits;  /**< collection of digits */
      StoreArray<SVDCluster> m_clusters;  /**< collection of clusters */

      // store object
      StoreObjPtr<SoftwareTriggerResult> m_resultStoreObjectPointer; /**< trigger decision */

      // DB payloads
      SVDHotStripsCalibrations m_HotStripsCalib; /**< payload for hot strips */
      SVDFADCMaskedStrips m_FADCMaskedStrips; /**< payload for strips masked on FADC level */

      // other
      std::string m_svdShaperDigitsName; /**< name of the input SVDShaperDigits collection */
      int m_activeStrips = 0; /**< number of active strips */
      int m_layerActiveStrips[4] = {0}; /**< number of active strips in each layer */
      int m_layerLadderActiveStrips[4][16] = {0}; /**< number of active strips in each layer, ladder */
      int m_layerSensorActiveStrips[4][5] = {0}; /**< number of active strips in each layer, sensor position */
      int m_l3LadderSensorActiveStrips[7][2] = {0}; /**< number of active strips in each sensor in Layer 3 */
      double m_thrCharge = 0; /**< cut on cluster energy in electrons */
      bool m_ignoreHotStripsPayload; /**< count hot strips as active */
      bool m_ignoreMaskedStripsPayload; /**< SVD: count FAD-masked strips as active */

      double m_massKg = 0; /**< Active mass of the whole SVD in Kg. */
      double m_layerMassKg[4] = {0}; /**< Active mass of each layer in Kg. */
      double m_layerLadderMassKg[4][16] = {0}; /**< Active mass of each ladder of each layer, in Kg. */
      double m_layerSensorMassKg[4][5] = {0}; /**< Active mass of each ladder/sensor position, in Kg. */
      // No need for m_l3LadderSensorMassKg, massOfSensor() can be used.
    };

  } // Background namespace
} // Belle2 namespace
