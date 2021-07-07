/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/ECLHitRateCounter.h>

#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <framework/database/DBObjPtr.h>
#include <framework/geometry/B2Vector3.h>

#include <numeric>

using namespace std;


namespace Belle2 {
  namespace Background {

    void ECLHitRateCounter::initialize(TTree* tree)
    {
      segmentECL();
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_dsps.isOptional();

      // set branch address
      tree->Branch("ecl", &m_rates, "averageDspBkgRate[16]/F:numEvents/I:validDspRate/O");

      //ECL calibration
      DBObjPtr<ECLCrystalCalib> ECLElectronicsCalib("ECLCrystalElectronics"), ECLECalib("ECLCrystalEnergy"),
               ECLWaveformNoise("ECLWaveformRMS");
      m_ADCtoEnergy.resize(8736);
      if (ECLElectronicsCalib) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] = ECLElectronicsCalib->getCalibVector()[i];
      if (ECLECalib) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] *= ECLECalib->getCalibVector()[i];
      m_waveformNoise.resize(8736);
      if (ECLWaveformNoise) for (int i = 0; i < 8736; i++) m_waveformNoise[i] = ECLWaveformNoise->getCalibVector()[i];
    }

    void ECLHitRateCounter::clear()
    {
      m_buffer.clear();
    }

    void ECLHitRateCounter::accumulate(unsigned timeStamp)
    {
      // check if data are available
      if (not m_dsps.isValid()) return;

      //calculate rates using waveforms
      //The background rate for a crystal is calculated as
      //rate = rms_pedestal_squared / (average_photon_energy_squared * time_constant)
      //where time_constant=2.53 us and average_photon_energy_squared = 1 MeV
      if (m_dsps.getEntries() == 8736) {

        // get buffer element
        auto& rates = m_buffer[timeStamp];

        // increment event counter
        rates.numEvents++;

        for (auto& aECLDsp : m_dsps) {

          int nadc = aECLDsp.getNADCPoints();
          int cellID = aECLDsp.getCellId();
          int segmentNumber = findECLSegment(cellID);
          int crysID = cellID - 1;
          std::vector<int> dspAv = aECLDsp.getDspA();

          //finding the pedestal value
          double dspMean = (std::accumulate(dspAv.begin(), dspAv.begin() + nadc, 0.0)) / nadc;
          double wpsum = 0;
          for (int v = 0; v < nadc; v++) {
            wpsum += pow(dspAv[v] - dspMean, 2);
          }
          double dspRMS = sqrt(wpsum / nadc);
          double dspSigma = dspRMS * abs(m_ADCtoEnergy[crysID]);

          //calculating the background rate per second
          double dspBkgRate = ((pow(dspSigma, 2)) - (pow(m_waveformNoise[crysID], 2))) / (2.53 * 1e-12);
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

      if (not m_rates.validDspRate) return;

      // normalize
      m_rates.normalize();

      //get average ret per crystal in segment
      for (int i = 0; i < 16; i++) {
        m_rates.averageDspBkgRate[i] /= m_crystalsInSegment[i];
      }
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
            m_crystalsInSegment[0] += 1;
          } else if (phi >= 2.356 || phi <= -2.356) {
            m_segmentMap.insert(std::pair<int, int>(cid, 1));
            m_crystalsInSegment[1] += 1;
          } else if (phi > -2.356 && phi < -0.7853) {
            m_segmentMap.insert(std::pair<int, int>(cid, 2));
            m_crystalsInSegment[2] += 1;
          } else {
            m_segmentMap.insert(std::pair<int, int>(cid, 3));
            m_crystalsInSegment[3] += 1;
          }
        } else if (cid < 7777) {
          if (z > 0) {
            if (phi > 0.7853 && phi < 2.356) {
              m_segmentMap.insert(std::pair<int, int>(cid, 4));
              m_crystalsInSegment[4] += 1;
            } else if (phi >= 2.356 || phi <= -2.356) {
              m_segmentMap.insert(std::pair<int, int>(cid, 5));
              m_crystalsInSegment[5] += 1;
            } else if (phi > -2.356 && phi < -0.7853) {
              m_segmentMap.insert(std::pair<int, int>(cid, 6));
              m_crystalsInSegment[6] += 1;
            } else {
              m_segmentMap.insert(std::pair<int, int>(cid, 7));
              m_crystalsInSegment[7] += 1;
            }
          } else {
            if (phi > 0.7853 && phi < 2.356) {
              m_segmentMap.insert(std::pair<int, int>(cid, 8));
              m_crystalsInSegment[8] += 1;
            } else if (phi >= 2.356 || phi <= -2.356) {
              m_segmentMap.insert(std::pair<int, int>(cid, 9));
              m_crystalsInSegment[9] += 1;
            } else if (phi > -2.356 && phi < -0.7853) {
              m_segmentMap.insert(std::pair<int, int>(cid, 10));
              m_crystalsInSegment[10] += 1;
            } else {
              m_segmentMap.insert(std::pair<int, int>(cid, 11));
              m_crystalsInSegment[11] += 1;
            }
          }
        } else {
          if (phi > 0.7853 && phi < 2.356) {
            m_segmentMap.insert(std::pair<int, int>(cid, 12));
            m_crystalsInSegment[12] += 1;
          } else if (phi >= 2.356 || phi <= -2.356) {
            m_segmentMap.insert(std::pair<int, int>(cid, 13));
            m_crystalsInSegment[13] += 1;
          } else if (phi > -2.356 && phi < -0.7853) {
            m_segmentMap.insert(std::pair<int, int>(cid, 14));
            m_crystalsInSegment[14] += 1;
          } else {
            m_segmentMap.insert(std::pair<int, int>(cid, 15));
            m_crystalsInSegment[15] += 1;
          }
        }
      }
    }

  } // Background namespace
} // Belle2 namespace
