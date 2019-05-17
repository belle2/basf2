/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Andrea Fodor                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/ECLHitRateCounter.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <numeric>

using namespace std;


namespace Belle2 {
  namespace Background {

    void ECLHitRateCounter::initialize(TTree* tree)
    {
      segmentECL();
      findElectronicsNoise();
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional();
      m_dsps.isOptional();

      // set branch address
      tree->Branch("ecl", &m_rates, "averageRate/F:numEvents/I:valid/O:averageDspBkgRate[16]/F:numEventsSegments[16]/I:validDspRate/O");

      //ECL calibration
      DBObjPtr<ECLCrystalCalib> ECLElectronicsCalib("ECLCrystalElectronics"), ECLECalib("ECLCrystalEnergy");
      m_ADCtoEnergy.resize(8736);
      if (ECLElectronicsCalib) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] = ECLElectronicsCalib->getCalibVector()[i];
      if (ECLECalib) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] *= ECLECalib->getCalibVector()[i];
    }

    void ECLHitRateCounter::clear()
    {
      m_buffer.clear();
    }

    void ECLHitRateCounter::accumulate(unsigned timeStamp)
    {
      // check if data are available
      if (not m_digits.isValid()) return;

      // get buffer element
      auto& rates = m_buffer[timeStamp];

      // increment event counter
      rates.numEvents++;

      // accumulate hits
      /* either count all */
      rates.averageRate += m_digits.getEntries();
      rates.valid = true;

      //calculate rates using waveforms
      //The background rate for a crystal is calculated as
      //rate = rms_pedestal_squared / (average_photon_energy_squared * time_constant)
      //where time_constant=2.53 us and average_photon_energy_squared = 1 MeV
      if (m_dsps.getEntries() == 8736) {
        for (auto& aECLDsp : m_dsps) {

          int nadc = aECLDsp.getNADCPoints();
          int cellID = aECLDsp.getCellId();
          int segmentNumber = findECLSegment(cellID);
          int crysID = cellID - 1;
          std::vector<int> dspAv = aECLDsp.getDspA();
          rates.numEventsSegments[segmentNumber]++;

          //finding the pedestal value
          double dspMean = (std::accumulate(dspAv.begin(), dspAv.begin() + nadc, 0.0)) / nadc;
          double wpsum = 0;
          for (int v = 0; v < nadc; v++) {
            wpsum += pow(dspAv[v] - dspMean, 2);
          }
          double dspRMS = sqrt(wpsum / nadc);
          double dspSigma = dspRMS * abs(m_ADCtoEnergy[crysID]);

          //finding the corresponding electronics noise
          float sigmaNoise = m_noiseMap.find(cellID)->second;

          //calculating the background rate per second
          double dspBkgRate = ((pow(dspSigma, 2)) - (pow(sigmaNoise, 2))) / (2.53 * 1e-12);
          if (dspBkgRate < 0) {
            dspBkgRate = 0;
          }

          //hit rate for segment in ECL, which is later normalized per 1Hz
          rates.averageDspBkgRate[segmentNumber] += dspBkgRate;

        }

        // set flag to true to indicate the rates are valid
        rates.validDspRate = true;
      }
    }

    void ECLHitRateCounter::normalize(unsigned timeStamp)
    {
      // copy buffer element
      m_rates = m_buffer[timeStamp];

      if (m_rates.valid) {
        // normalize
        m_rates.normalizeDigits();
      }

      if (m_rates.validDspRate) {
        m_rates.normalizeDsps();
      }

      // optionally: convert to MHz, correct for the masked-out channels etc.

    }


    void ECLHitRateCounter::segmentECL()
    {
      m_geometry = Belle2::ECL::ECLGeometryPar::Instance();
      for (int cid = 1; cid < 8737; cid++) {
        m_geometry->Mapping(cid);
        const B2Vector3D position = m_geometry->GetCrystalPos(cid - 1);
        const double phi = position.Phi();
        const double z = position.Z();

        if (cid < 1297) {
          if (phi > 0.7853 && phi < 2.356) {
            m_segmentMap.insert(std::pair<int, int>(cid, 0));
          } else if (phi >= 2.356 || phi <= -2.356) {
            m_segmentMap.insert(std::pair<int, int>(cid, 1));
          } else if (phi > -2.356 && phi < -0.7853) {
            m_segmentMap.insert(std::pair<int, int>(cid, 2));
          } else {
            m_segmentMap.insert(std::pair<int, int>(cid, 3));
          }
        } else if (cid > 1152 && cid < 7777) {
          if (z > 0) {
            if (phi > 0.7853 && phi < 2.356) {
              m_segmentMap.insert(std::pair<int, int>(cid, 4));
            } else if (phi >= 2.356 || phi <= -2.356) {
              m_segmentMap.insert(std::pair<int, int>(cid, 5));
            } else if (phi > -2.356 && phi < -0.7853) {
              m_segmentMap.insert(std::pair<int, int>(cid, 6));
            } else {
              m_segmentMap.insert(std::pair<int, int>(cid, 7));
            }
          } else {
            if (phi > 0.7853 && phi < 2.356) {
              m_segmentMap.insert(std::pair<int, int>(cid, 8));
            } else if (phi >= 2.356 || phi <= -2.356) {
              m_segmentMap.insert(std::pair<int, int>(cid, 9));
            } else if (phi > -2.356 && phi < -0.7853) {
              m_segmentMap.insert(std::pair<int, int>(cid, 10));
            } else {
              m_segmentMap.insert(std::pair<int, int>(cid, 11));
            }
          }
        } else {
          if (phi > 0.7853 && phi < 2.356) {
            m_segmentMap.insert(std::pair<int, int>(cid, 12));
          } else if (phi >= 2.356 || phi <= -2.356) {
            m_segmentMap.insert(std::pair<int, int>(cid, 13));
          } else if (phi > -2.356 && phi < -0.7853) {
            m_segmentMap.insert(std::pair<int, int>(cid, 14));
          } else {
            m_segmentMap.insert(std::pair<int, int>(cid, 15));
          }
        }
      }
    }

    void ECLHitRateCounter::findElectronicsNoise()
    {
      std::string fileName = FileSystem::findFile("ecl/data/sigmaMeanElectronics.root");
      if (fileName.empty()) {
        B2WARNING("ECLHitRateCounter: file with electronics noise data not found");
        for (int i = 1; i < 8737; i++) {
          m_noiseMap[i] = 0;
        }
        return;
      }
      TFile noiseFile(fileName.c_str(), "READ");
      TH1F* h_Noise = dynamic_cast<TH1F*>(noiseFile.Get("sigma_noise"));
      if (not h_Noise) {
        B2WARNING("ECLHitRateCounter: histogram with electronics noise data not found");
        for (int i = 1; i < 8737; i++) {
          m_noiseMap[i] = 0;
        }
        return;
      } else {

        for (int i = 1; i < 8737; i++) {
          m_noiseMap[i] = h_Noise->GetBinContent(i);
        }

        noiseFile.Close();
      }
    }


  } // Background namespace
} // Belle2 namespace
