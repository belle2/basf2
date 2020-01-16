/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Hikaru Tanigawa                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/SVDHitRateCounter.h>

// framework aux
#include <framework/logging/Logger.h>
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>

using namespace std;

namespace Belle2 {
  namespace Background {

    void SVDHitRateCounter::initialize(TTree* tree)
    {
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional(m_svdShaperDigitsName);
      m_clusters.isOptional();

      B2DEBUG(10, "SVDHitRateCounter: initialize()");
      // set branch address
      tree->Branch("svd", &m_rates,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:l3LadderSensorAverageRates[7][2]/F:numEvents/I:valid/O");
      tree->Branch("svd_highE", &m_rates_highE,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:l3LadderSensorAverageRates[7][2]/F:numEvents/I:valid/O");
      tree->Branch("svd_lowE", &m_rates_lowE,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:l3LadderSensorAverageRates[7][2]/F:numEvents/I:valid/O");


      // count active strips
      for (int layer = 0; layer < m_nLayers; layer++) {
        for (int ladder = 0; ladder < m_nLadders[layer]; ladder++) {
          for (int sensor = 0; sensor < m_nSensors[layer]; sensor++) {
            VxdID sensorID(layer + 3, ladder + 1, sensor + 1);
            for (bool isU : {true, false}) {
              int nStrips = nStripsOnLayerSide(layer, isU);
              for (int strip = 0; strip < nStrips; strip++) {
                if (!m_HotStripsCalib.isHot(sensorID, isU, strip) && !m_FADCMaskedStrips.isMasked(sensorID, isU, strip)) {
                  m_activeStrips ++;
                  m_layerActiveStrips[layer] ++;
                  m_layerLadderActiveStrips[layer][ladder] ++;
                  m_layerSensorActiveStrips[layer][sensor] ++;
                }
              }
            }
          }
        }
      }
      int layer = 0;
      for (int ladder = 0; ladder < m_nLadders[layer]; ladder++) {
        for (int sensor = 0; sensor < m_nSensors[layer]; sensor++) {
          VxdID sensorID(layer + 3, ladder + 1, sensor + 1);
          for (bool isU : {true, false}) {
            int nStrips = nStripsOnLayerSide(layer, isU);
            for (int strip = 0; strip < nStrips; strip++) {
              if (!m_HotStripsCalib.isHot(sensorID, isU, strip) && !m_FADCMaskedStrips.isMasked(sensorID, isU, strip)) {
                m_l3LadderSensorActiveStrips[ladder][sensor] ++;
              }
            }
          }
        }
      }

    }

    void SVDHitRateCounter::clear()
    {
      m_buffer.clear();
    }

    void SVDHitRateCounter::accumulate(unsigned timeStamp)
    {
      B2DEBUG(10, "SVDHitRateCounter: accumulate()");

      // check if the event has passed HLT filter
      if (m_resultStoreObjectPointer.isValid()) {
        const bool eventAccepted = SoftwareTrigger::FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_resultStoreObjectPointer);
        if (!eventAccepted) return;
      }

      // check if data are available
      if (m_digits.isValid()) {

        // get buffer element
        auto& rates = m_buffer[timeStamp];

        // increment event counter
        rates.numEvents++;

        // accumulate hits
        for (const auto& digit : m_digits) {
          // select digits to count (usualy only good ones)
          VxdID sensorID = digit.getSensorID();
          int layer = sensorID.getLayerNumber() - 3;
          int ladder = sensorID.getLadderNumber() - 1;
          int sensor = sensorID.getSensorNumber() - 1;
          rates.layerAverageRates[layer] ++;
          rates.layerLadderAverageRates[layer][ladder] ++;
          rates.layerSensorAverageRates[layer][sensor] ++;
          rates.averageRate ++;
          if (layer == 0)
            rates.l3LadderSensorAverageRates[ladder][sensor] ++;
        }

        // set flag to true to indicate the rates are valid
        rates.valid = true;
      }

      // check if data are available
      if (m_clusters.isValid()) {

        // get buffer element
        auto& rates_highE = m_buffer_highE[timeStamp];
        auto& rates_lowE = m_buffer_lowE[timeStamp];

        // increment event counter
        rates_highE.numEvents++;
        rates_lowE.numEvents++;

        // accumulate clusters
        for (const auto& cluster : m_clusters) {
          VxdID sensorID = cluster.getSensorID();
          int layer = sensorID.getLayerNumber() - 3;
          int ladder = sensorID.getLadderNumber() - 1;
          int sensor = sensorID.getSensorNumber() - 1;
          if (cluster.getCharge() > m_thrCharge) {
            rates_highE.layerAverageRates[layer] ++;
            rates_highE.layerLadderAverageRates[layer][ladder] ++;
            rates_highE.layerSensorAverageRates[layer][sensor] ++;
            rates_highE.averageRate ++;
            if (layer == 0)
              rates_highE.l3LadderSensorAverageRates[ladder][sensor] ++;
          } else {
            rates_lowE.layerAverageRates[layer] ++;
            rates_lowE.layerLadderAverageRates[layer][ladder] ++;
            rates_lowE.layerSensorAverageRates[layer][sensor] ++;
            rates_lowE.averageRate ++;
            if (layer == 0)
              rates_lowE.l3LadderSensorAverageRates[ladder][sensor] ++;
          }
        }

        // set flag to true to indicate the rates are valid
        rates_highE.valid = true;
        rates_lowE.valid = true;
      }

    }

    void SVDHitRateCounter::normalize(unsigned timeStamp)
    {
      B2DEBUG(10, "SVDHitRateCounter: normalize()");
      // copy buffer element
      m_rates = m_buffer[timeStamp];
      m_rates_highE = m_buffer_highE[timeStamp];
      m_rates_lowE = m_buffer_lowE[timeStamp];

      SVDHitRateCounter::normalize_rates(m_rates);
      SVDHitRateCounter::normalize_rates(m_rates_highE);
      SVDHitRateCounter::normalize_rates(m_rates_lowE);
    }

    void SVDHitRateCounter::normalize_rates(TreeStruct& rates)
    {
      if (not rates.valid) return;

      // normalize -> nHits on each segment in single event
      rates.normalize();

      // convert to occupancy [%]
      rates.averageRate /= m_activeStrips / 100;
      for (int layer = 0; layer < m_nLayers; layer++) {
        rates.layerAverageRates[layer] /= m_layerActiveStrips[layer] / 100;
        for (int ladder = 0; ladder < m_nLadders[layer]; ladder++) {
          rates.layerLadderAverageRates[layer][ladder] /= m_layerLadderActiveStrips[layer][ladder] / 100;
        }
        for (int sensor = 0; sensor < m_nSensors[layer]; sensor++) {
          rates.layerSensorAverageRates[layer][sensor] /= m_layerSensorActiveStrips[layer][sensor] / 100;
        }
      }
      int layer = 0;
      for (int ladder = 0; ladder < m_nLadders[layer]; ladder++) {
        for (int sensor = 0; sensor < m_nSensors[layer]; sensor++) {
          rates.l3LadderSensorAverageRates[ladder][sensor] /= m_l3LadderSensorActiveStrips[ladder][sensor] / 100;
        }
      }
    }

  } // background namespace
} // Belle2 namespace

