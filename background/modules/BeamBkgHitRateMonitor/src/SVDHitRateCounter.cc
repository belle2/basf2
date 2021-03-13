/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Hikaru Tanigawa, Ludovico Massaccesi       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/SVDHitRateCounter.h>

// framework aux
#include <framework/logging/Logger.h>
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>

using namespace std;

#define LOGRATIO(x,y) (x) << " / " << (y) << " = " << ((x) * 100 / (y)) << "%"

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
      tree->Branch("svdU", &m_ratesU,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:l3LadderSensorAverageRates[7][2]/F:numEvents/I:valid/O");
      tree->Branch("svdV", &m_ratesV,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:l3LadderSensorAverageRates[7][2]/F:numEvents/I:valid/O");
      tree->Branch("svd_highE", &m_rates_highE,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:l3LadderSensorAverageRates[7][2]/F:numEvents/I:valid/O");
      tree->Branch("svd_lowE", &m_rates_lowE,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:l3LadderSensorAverageRates[7][2]/F:numEvents/I:valid/O");
      tree->Branch("svd_clustersU", &m_clustersU,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:l3LadderSensorAverageRates[7][2]/F:numEvents/I:valid/O");
      tree->Branch("svd_clustersV", &m_clustersV,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:l3LadderSensorAverageRates[7][2]/F:numEvents/I:valid/O");
      tree->Branch("svd_energyU", &m_rates_energyU,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:l3LadderSensorAverageRates[7][2]/F:numEvents/I:valid/O");
      tree->Branch("svd_energyV", &m_rates_energyV,
                   "layerAverageRates[4]/F:layerLadderAverageRates[4][16]/F:layerSensorAverageRates[4][5]:averageRate/F:l3LadderSensorAverageRates[7][2]/F:numEvents/I:valid/O");

      // count active strips
      for (int layer = 0; layer < m_nLayers; layer++) {
        for (int ladder = 0; ladder < m_nLadders[layer]; ladder++) {
          for (int sensor = 0; sensor < m_nSensors[layer]; sensor++) {
            VxdID sensorID(layer + 3, ladder + 1, sensor + 1);
            for (bool isU : {true, false}) {
              int nStrips = nStripsOnLayerSide(layer, isU);
              for (int strip = 0; strip < nStrips; strip++) {
                if (isStripActive(sensorID, isU, strip)) {
                  m_activeStrips ++;
                  m_layerActiveStrips[layer] ++;
                  m_layerLadderActiveStrips[layer][ladder] ++;
                  m_layerSensorActiveStrips[layer][sensor] ++;
                  if (isU) {
                    m_activeStripsU++;
                    m_layerActiveStripsU[layer]++;
                    m_layerLadderActiveStripsU[layer][ladder]++;
                    m_layerSensorActiveStripsU[layer][sensor]++;
                  } else {
                    m_activeStripsV++;
                    m_layerActiveStripsV[layer]++;
                    m_layerLadderActiveStripsV[layer][ladder]++;
                    m_layerSensorActiveStripsV[layer][sensor]++;
                  }
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
              if (isStripActive(sensorID, isU, strip)) {
                m_l3LadderSensorActiveStrips[ladder][sensor] ++;
                if (isU)
                  m_l3LadderSensorActiveStripsU[ladder][sensor]++;
                else
                  m_l3LadderSensorActiveStripsV[ladder][sensor]++;
              }
            }
          }
        }
      }
      B2INFO("SVD active strips = " << LOGRATIO(m_activeStrips, 223744));
      for (layer = 0; layer < m_nLayers; layer++)
        B2INFO("  Active strips L" << layer + 3 << ".X.X = "
               << LOGRATIO(m_layerActiveStrips[layer], m_nLadders[layer] * m_nSensors[layer] * (nStripsOnLayerSide(layer, false) + 768)));
      for (layer = 0; layer < m_nLayers; layer++)
        for (int ladder = 0; ladder < m_nLadders[layer]; ladder++)
          B2INFO("  Active strips L" << layer + 3 << "." << ladder + 1 << ".X = "
                 << LOGRATIO(m_layerLadderActiveStrips[layer][ladder], m_nSensors[layer] * (nStripsOnLayerSide(layer, false) + 768)));
      for (layer = 0; layer < m_nLayers; layer++)
        for (int sensor = 0; sensor < m_nSensors[layer]; sensor++)
          B2INFO("  Active strips L" << layer + 3 << ".X." << sensor + 1 << " = "
                 << LOGRATIO(m_layerSensorActiveStrips[layer][sensor], m_nLadders[layer] * (nStripsOnLayerSide(layer, false) + 768)));
      layer = 0;
      for (int ladder = 0; ladder < m_nLadders[layer]; ladder++)
        for (int sensor = 0; sensor < m_nSensors[layer]; sensor++)
          B2INFO("  Active strips L3." << ladder + 1 << "." << sensor + 1 << " = "
                 << LOGRATIO(m_l3LadderSensorActiveStrips[ladder][sensor], 2 * 768));

      // Compute active mass
      for (layer = 0; layer < m_nLayers; layer++) {
        for (int ladder = 0; ladder < m_nLadders[layer]; ladder++) {
          for (int sensor = 0; sensor < m_nSensors[layer]; sensor++) {
            double mass = massOfSensor(layer, ladder, sensor);
            m_massKg += mass;
            m_layerMassKg[layer] += mass;
            m_layerLadderMassKg[layer][ladder] += mass;
            m_layerSensorMassKg[layer][sensor] += mass;
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
        auto& ratesU = m_bufferU[timeStamp];
        auto& ratesV = m_bufferV[timeStamp];

        // increment event counter
        rates.numEvents++;
        ratesU.numEvents++;
        ratesV.numEvents++;

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

          if (digit.isUStrip()) {
            ratesU.layerAverageRates[layer]++;
            ratesU.layerLadderAverageRates[layer][ladder]++;
            ratesU.layerSensorAverageRates[layer][sensor]++;
            ratesU.averageRate++;
            if (layer == 0)
              ratesU.l3LadderSensorAverageRates[ladder][sensor]++;
          } else {
            ratesV.layerAverageRates[layer]++;
            ratesV.layerLadderAverageRates[layer][ladder]++;
            ratesV.layerSensorAverageRates[layer][sensor]++;
            ratesV.averageRate++;
            if (layer == 0)
              ratesV.l3LadderSensorAverageRates[ladder][sensor]++;
          }
        }

        // set flag to true to indicate the rates are valid
        rates.valid = true;
        ratesU.valid = true;
        ratesV.valid = true;
      }

      // check if data are available
      if (m_clusters.isValid()) {

        // get buffer element
        auto& rates_highE = m_buffer_highE[timeStamp];
        auto& rates_lowE = m_buffer_lowE[timeStamp];
        auto& clustersU = m_buffer_clustersU[timeStamp];
        auto& clustersV = m_buffer_clustersV[timeStamp];
        auto& rates_energyU = m_buffer_energyU[timeStamp];
        auto& rates_energyV = m_buffer_energyV[timeStamp];

        // increment event counter
        rates_highE.numEvents++;
        rates_lowE.numEvents++;
        clustersU.numEvents++;
        clustersV.numEvents++;
        rates_energyU.numEvents++;
        rates_energyV.numEvents++;

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
          if (cluster.isUCluster()) {
            rates_energyU.layerAverageRates[layer] += cluster.getCharge();
            rates_energyU.layerLadderAverageRates[layer][ladder] += cluster.getCharge();
            rates_energyU.layerSensorAverageRates[layer][sensor] += cluster.getCharge();
            rates_energyU.averageRate += cluster.getCharge();
            if (layer == 0)
              rates_energyU.l3LadderSensorAverageRates[ladder][sensor] += cluster.getCharge();
            clustersU.layerAverageRates[layer]++;
            clustersU.layerLadderAverageRates[layer][ladder]++;
            clustersU.layerSensorAverageRates[layer][sensor]++;
            clustersU.averageRate++;
            if (layer == 0)
              clustersU.l3LadderSensorAverageRates[ladder][sensor]++;
          } else {
            rates_energyV.layerAverageRates[layer] += cluster.getCharge();
            rates_energyV.layerLadderAverageRates[layer][ladder] += cluster.getCharge();
            rates_energyV.layerSensorAverageRates[layer][sensor] += cluster.getCharge();
            rates_energyV.averageRate += cluster.getCharge();
            if (layer == 0)
              rates_energyV.l3LadderSensorAverageRates[ladder][sensor] += cluster.getCharge();
            clustersV.layerAverageRates[layer]++;
            clustersV.layerLadderAverageRates[layer][ladder]++;
            clustersV.layerSensorAverageRates[layer][sensor]++;
            clustersV.averageRate++;
            if (layer == 0)
              clustersV.l3LadderSensorAverageRates[ladder][sensor]++;
          }
        }

        // set flag to true to indicate the rates are valid
        rates_highE.valid = true;
        rates_lowE.valid = true;
        clustersU.valid = true;
        clustersV.valid = true;
        rates_energyU.valid = true;
        rates_energyV.valid = true;
      }

    }

    void SVDHitRateCounter::normalize(unsigned timeStamp)
    {
      B2DEBUG(10, "SVDHitRateCounter: normalize()");
      // copy buffer element
      m_rates = m_buffer[timeStamp];
      m_ratesU = m_bufferU[timeStamp];
      m_ratesV = m_bufferV[timeStamp];
      m_rates_highE = m_buffer_highE[timeStamp];
      m_rates_lowE = m_buffer_lowE[timeStamp];
      m_clustersU = m_buffer_clustersU[timeStamp];
      m_clustersV = m_buffer_clustersV[timeStamp];
      m_rates_energyU = m_buffer_energyU[timeStamp];
      m_rates_energyV = m_buffer_energyV[timeStamp];

      SVDHitRateCounter::normalize_rates(m_rates);
      SVDHitRateCounter::normalize_rates(m_ratesU, true);
      SVDHitRateCounter::normalize_rates(m_ratesV, false, true);
      SVDHitRateCounter::normalize_rates(m_rates_highE);
      SVDHitRateCounter::normalize_rates(m_rates_lowE);
      SVDHitRateCounter::normalize_rates(m_clustersU, true);
      SVDHitRateCounter::normalize_rates(m_clustersV, false, true);
      SVDHitRateCounter::normalize_energy_rates(m_rates_energyU);
      SVDHitRateCounter::normalize_energy_rates(m_rates_energyV);
    }

    void SVDHitRateCounter::normalize_rates(TreeStruct& rates, bool isU, bool isV)
    {
      if (not rates.valid) return;

      // normalize -> nHits on each segment in single event
      rates.normalize();

      // Take the correct active strips counter
      const auto& activeStrips = isU ? m_activeStripsU : (isV ? m_activeStripsV : m_activeStrips);
      const auto& layerActiveStrips = isU ? m_layerActiveStripsU : (isV ? m_layerActiveStripsV : m_layerActiveStrips);
      const auto& layerLadderActiveStrips = isU ? m_layerLadderActiveStripsU
                                            : (isV ? m_layerLadderActiveStripsV : m_layerLadderActiveStrips);
      const auto& layerSensorActiveStrips = isU ? m_layerSensorActiveStripsU
                                            : (isV ? m_layerSensorActiveStripsV : m_layerSensorActiveStrips);
      const auto& l3LadderSensorActiveStrips = isU ? m_l3LadderSensorActiveStripsU
                                               : (isV ? m_l3LadderSensorActiveStripsV : m_l3LadderSensorActiveStrips);

      // convert to occupancy [%]
      rates.averageRate /= activeStrips / 100.0;
      for (int layer = 0; layer < m_nLayers; layer++) {
        rates.layerAverageRates[layer] /= layerActiveStrips[layer] / 100.0;
        for (int ladder = 0; ladder < m_nLadders[layer]; ladder++) {
          rates.layerLadderAverageRates[layer][ladder] /= layerLadderActiveStrips[layer][ladder] / 100.0;
        }
        for (int sensor = 0; sensor < m_nSensors[layer]; sensor++) {
          rates.layerSensorAverageRates[layer][sensor] /= layerSensorActiveStrips[layer][sensor] / 100.0;
        }
      }
      int layer = 0;
      for (int ladder = 0; ladder < m_nLadders[layer]; ladder++) {
        for (int sensor = 0; sensor < m_nSensors[layer]; sensor++) {
          rates.l3LadderSensorAverageRates[ladder][sensor] /= l3LadderSensorActiveStrips[ladder][sensor] / 100.0;
        }
      }
    }

    void SVDHitRateCounter::normalize_energy_rates(TreeStruct& rates)
    {
      static const double ehEnergyJoules = Const::ehEnergy / Unit::J;
      static const double integrationTime = 155 * Unit::ns;
      static const double integrationTimeSeconds = integrationTime / Unit::s;
      // Convert charge to mrad/s by multiplying by this constant and dividing by the mass
      static const double conv = ehEnergyJoules / integrationTimeSeconds * 100e3;

      if (!rates.valid) return;

      rates.normalize(); // Divide by nEvents
      // Convert to dose rate [mrad/s]
      rates.averageRate *= conv / m_massKg;
      for (int layer = 0; layer < m_nLayers; layer++) {
        rates.layerAverageRates[layer] *= conv / m_layerMassKg[layer];
        for (int ladder = 0; ladder < m_nLadders[layer]; ladder++)
          rates.layerLadderAverageRates[layer][ladder] *= conv / m_layerLadderMassKg[layer][ladder];
        for (int sensor = 0; sensor < m_nSensors[layer]; sensor++)
          rates.layerSensorAverageRates[layer][sensor] *= conv / m_layerSensorMassKg[layer][sensor];
      }
      int layer = 0;
      for (int ladder = 0; ladder < m_nLadders[layer]; ladder++)
        for (int sensor = 0; sensor < m_nSensors[layer]; sensor++)
          rates.l3LadderSensorAverageRates[ladder][sensor] *= conv / massOfSensor(layer, ladder, sensor);
    }

    // layer is unused, but this may be changed in the future. Better keep it.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    double SVDHitRateCounter::massOfSensor(int layer, int ladder, int sensor)
    {
      static const double rho_Si = 2.329 * Unit::g_cm3;
      static const double activeAreaHPKSmall = 122.9 * 38.55 * Unit::mm2;
      static const double activeAreaHPKLarge = 122.9 * 57.72 * Unit::mm2;
      static const double activeAreaMicron = 122.76 * (57.59 + 38.42) / 2 * Unit::mm2;
      static const double thicknessHPK = 320 * Unit::um;
      static const double thicknessMicron = 280 * Unit::um;
      static const double massHPKSmallKg = activeAreaHPKSmall * thicknessHPK * rho_Si / 1e3;
      static const double massHPKLargeKg = activeAreaHPKLarge * thicknessHPK * rho_Si / 1e3;
      static const double massMicronKg = activeAreaMicron * thicknessMicron * rho_Si / 1e3;
      return layer == 0 ? massHPKSmallKg : (sensor == 0 ? massMicronKg : massHPKLargeKg);
    }
#pragma GCC diagnostic pop

    bool SVDHitRateCounter::isStripActive(const VxdID& sensorID, const bool& isU,
                                          const unsigned short& strip)
    {
      return ((m_ignoreHotStripsPayload || !m_HotStripsCalib.isHot(sensorID, isU, strip))
              && (m_ignoreMaskedStripsPayload || !m_FADCMaskedStrips.isMasked(sensorID, isU, strip)));
    }

  } // background namespace
} // Belle2 namespace

