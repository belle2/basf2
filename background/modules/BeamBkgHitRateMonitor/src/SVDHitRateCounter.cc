/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/SVDHitRateCounter.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Background {

    void SVDHitRateCounter::initialize(TTree* tree)
    {
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional();

      B2DEBUG(10, "SVDHitRateCounter: initialize()");
      // set branch address
      tree->Branch("svd", &m_rates,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:numEvents/I:valid/O");

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

    }

    void SVDHitRateCounter::clear()
    {
      m_buffer.clear();
    }

    void SVDHitRateCounter::accumulate(unsigned timeStamp)
    {
      B2DEBUG(10, "SVDHitRateCounter: accumulate()");
      // check if data are available
      if (not m_digits.isValid()) return;

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
      }

      // set flag to true to indicate the rates are valid
      rates.valid = true;

    }

    void SVDHitRateCounter::normalize(unsigned timeStamp)
    {
      B2DEBUG(10, "SVDHitRateCounter: normalize()");
      // copy buffer element
      m_rates = m_buffer[timeStamp];

      if (not m_rates.valid) return;

      // normalize -> nHits on each segment in single event
      m_rates.normalize();

      // optionally: convert to MHz, correct for the masked-out channels etc.
      // convert to occupancy [%]
      m_rates.averageRate /= m_activeStrips / 100;
      for (int layer = 0; layer < m_nLayers; layer++) {
        m_rates.layerAverageRates[layer] /= m_layerActiveStrips[layer] / 100;
        for (int ladder = 0; ladder < m_nLadders[layer]; ladder++) {
          m_rates.layerLadderAverageRates[layer][ladder] /= m_layerLadderActiveStrips[layer][ladder] / 100;
        }
        for (int sensor = 0; sensor < m_nSensors[layer]; sensor++) {
          m_rates.layerSensorAverageRates[layer][sensor] /= m_layerSensorActiveStrips[layer][sensor] / 100;
        }
      }

    }


  } // background namespace
} // Belle2 namespace

