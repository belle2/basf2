/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/PXDHitRateCounter.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>
#include <pxd/reconstruction/PXDPixelMasker.h>
#include <string>

// framework aux
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Background {

    void PXDHitRateCounter::initialize(TTree* tree)
    {
      //Pointer to GeoTools instance
      auto gTools = VXD::GeoCache::getInstance().getGeoTools();
      if (gTools->getNumberOfPXDLayers() == 0) {
        B2FATAL("Missing geometry for PXD.");
      }

      m_nPXDSensors = gTools->getNumberOfPXDSensors();

      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional();

      // register collection(s) as optional, your detector might be excluded in DAQ
      m_clusters.isOptional();

      string leaflist =
        "meanOccupancies[40]/F:maxOccupancies[40]/F:doseRates[40]/F:softPhotonFluxes[40]/F:hardPhotonFluxes[40]/F:chargedFluxes[40]/F:segmentDoseRates[240]/F:segmentSoftPhotonFluxes[240]/F:segmentHardPhotonFluxes[240]/F:segmentChargedFluxes[240]/F:averageRate/F:numEvents/I:valid/O";

      // set branch address
      tree->Branch("pxd", &m_rates, leaflist.c_str());

      // check parameters
      if (m_integrationTime <= 0) B2FATAL("invalid integration time window for PXD: " << m_integrationTime);

      // set fractions of active channels
      setActivePixels();
    }

    void PXDHitRateCounter::clear()
    {
      m_buffer.clear();
    }

    void PXDHitRateCounter::accumulate(unsigned timeStamp)
    {
      // check if data are available
      if ((not m_digits.isValid()) or (not m_clusters.isValid())) return;

      // get buffer element
      auto& rates = m_buffer[timeStamp];

      // increment event counter
      rates.numEvents++;

      // accumulate hits
      rates.averageRate += m_digits.getEntries();

      // Use the same indexing of sensors as for PXDDQM
      auto gTools = VXD::GeoCache::getInstance().getGeoTools();

      // accumulate dose rate and event occupancy
      float occupancies[40] = {0};
      for (const PXDDigit& storeDigit : m_digits) {
        VxdID sensorID = storeDigit.getSensorID();
        int index = gTools->getPXDSensorIndex(storeDigit.getSensorID());
        int vBin = PXD::PXDGainCalibrator::getInstance().getBinV(sensorID, storeDigit.getVCellID(), 6);
        double ADUToEnergy =  PXD::PXDGainCalibrator::getInstance().getADUToEnergy(sensorID, storeDigit.getUCellID(),
                              storeDigit.getVCellID());
        double hitEnergy = storeDigit.getCharge() * ADUToEnergy;
        rates.doseRates[index] += (hitEnergy / Unit::J);
        occupancies[index] += 1.0;
        rates.segmentDoseRates[vBin + index * 6] += (hitEnergy / Unit::J);
      }

      for (int index = 0; index < m_nPXDSensors; index++) {
        if (m_activePixels[index] > 0) {
          occupancies[index] /= m_activePixels[index];
        }
        rates.meanOccupancies[index] += occupancies[index];
        if (rates.maxOccupancies[index] < occupancies[index]) {
          rates.maxOccupancies[index] = occupancies[index];
        }
      }

      // accumulate fluxes
      for (const PXDCluster& cluster : m_clusters) {
        // Update if we have a new sensor
        VxdID sensorID = cluster.getSensorID();
        int index = gTools->getPXDSensorIndex(sensorID);
        auto info = getInfo(sensorID);

        auto cluster_uID = info.getUCellID(cluster.getU());
        auto cluster_vID = info.getVCellID(cluster.getV());
        int vBin = PXD::PXDGainCalibrator::getInstance().getBinV(sensorID, cluster_vID, 6);
        double ADUToEnergy =  PXD::PXDGainCalibrator::getInstance().getADUToEnergy(sensorID, cluster_uID, cluster_vID);
        double clusterEnergy = cluster.getCharge() * ADUToEnergy;

        if (cluster.getSize() == 1 && clusterEnergy < 10000 * Unit::eV && clusterEnergy > 6000 * Unit::eV) {
          rates.softPhotonFluxes[index] += 1.0;
          rates.segmentSoftPhotonFluxes[vBin + index * 6] += 1.0;
        } else if (cluster.getSize() == 1 && clusterEnergy > 10000 * Unit::eV) {
          rates.hardPhotonFluxes[index] += 1.0;
          rates.segmentHardPhotonFluxes[vBin + index * 6] += 1.0;
        } else if (cluster.getSize() > 1 && clusterEnergy > 10000 * Unit::eV) {
          rates.chargedFluxes[index] += 1.0;
          rates.segmentChargedFluxes[vBin + index * 6] += 1.0;
        }
      }

      // set flag to true to indicate the rates are valid
      rates.valid = true;
    }

    void PXDHitRateCounter::normalize(unsigned timeStamp)
    {
      // copy buffer element
      m_rates = m_buffer[timeStamp];

      if (not m_rates.valid) return;

      if (m_rates.numEvents == 0) return;

      // Average number of PXDDigits per 1Hz
      m_rates.averageRate /= m_rates.numEvents;

      // Total integration time of PXD per 1Hz
      double currentComponentTime = m_rates.numEvents * m_integrationTime;

      //Pointer to GeoTools instance
      auto gTools = VXD::GeoCache::getInstance().getGeoTools();

      // Compute sensor level averages per 1Hz
      for (int index = 0; index < m_nPXDSensors; index++) {
        VxdID sensorID = gTools->getSensorIDFromPXDIndex(index);
        auto info = getInfo(sensorID);
        double currentSensorMass = m_activeAreas[index] * info.getThickness() * c_densitySi;
        double currentSensorArea = m_activeAreas[index];

        m_rates.meanOccupancies[index] /= m_rates.numEvents;
        if (currentSensorArea > 0) {
          m_rates.doseRates[index] *= (1.0 / (currentComponentTime / Unit::s)) * (1000 / currentSensorMass);
          m_rates.softPhotonFluxes[index] *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));
          m_rates.hardPhotonFluxes[index] *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));
          m_rates.chargedFluxes[index] *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));

          // Loop over v segements
          for (int vBin = 0; vBin < 6; ++vBin)  {
            double currentSegmentMass = m_segmentActiveAreas[vBin + index * 6] * info.getThickness() * c_densitySi;
            double currentSegmentArea = m_segmentActiveAreas[vBin + index * 6];
            if (currentSegmentArea > 0) {
              m_rates.segmentDoseRates[vBin + index * 6] *= (1.0 / (currentComponentTime / Unit::s)) * (1000 / currentSegmentMass);
              m_rates.segmentSoftPhotonFluxes[vBin + index * 6] *= (1.0 / currentSegmentArea) * (1.0 / (currentComponentTime / Unit::s));
              m_rates.segmentHardPhotonFluxes[vBin + index * 6] *= (1.0 / currentSegmentArea) * (1.0 / (currentComponentTime / Unit::s));
              m_rates.segmentChargedFluxes[vBin + index * 6] *= (1.0 / currentSegmentArea) * (1.0 /
                                                                (currentComponentTime / Unit::s));
            }
          }
        }
      }
    }

    void PXDHitRateCounter::setActivePixels()
    {
      //Pointer to GeoTools instance
      auto gTools = VXD::GeoCache::getInstance().getGeoTools();

      // Initialize active areas and active fractions for all sensors
      for (int index = 0; index < m_nPXDSensors; index++) {
        VxdID sensorID = gTools->getSensorIDFromPXDIndex(index);
        auto info = getInfo(sensorID);

        // Compute nominal number of pixel per sensor
        m_activePixels[index] = info.getUCells() * info.getVCells();
        // Compute nominal area of sensor
        m_activeAreas[index] = info.getWidth() * info.getLength();

        // Loop over v segements
        for (int vBin = 0; vBin < 6; ++vBin)  {
          // Compute nominal number of pixel per segment
          // Sgements have same number of rows, but due to pitch change a different length
          m_segmentActivePixels[vBin + index * 6] = info.getUCells() * info.getVCells() / 6;
          // Compute position of v segment
          double v = info.getVCellPosition(vBin * info.getVCells() / 6);
          // Compute nominal area of segment
          m_segmentActiveAreas[vBin + index * 6] = info.getWidth() * info.getVPitch(v) * info.getVCells() / 6;
        }

        if (m_maskDeadPixels) {
          for (int ui = 0; ui < info.getUCells(); ++ui) {
            for (int vi = 0; vi < info.getVCells(); ++vi) {
              if (PXD::PXDPixelMasker::getInstance().pixelDead(sensorID, ui, vi)
                  || !PXD::PXDPixelMasker::getInstance().pixelOK(sensorID, ui, vi)) {
                m_activePixels[index] -= 1;
                m_activeAreas[index] -= info.getVPitch(info.getVCellPosition(vi)) * info.getUPitch();
                int vBin = PXD::PXDGainCalibrator::getInstance().getBinV(sensorID, vi, 6);
                m_segmentActivePixels[vBin + index * 6] -= 1;
                m_segmentActiveAreas[vBin + index * 6] -= info.getVPitch(info.getVCellPosition(vi)) * info.getUPitch();
              }
            }
          }
        }
      }
    }

  } // Background namespace
} // Belle2 namespace

