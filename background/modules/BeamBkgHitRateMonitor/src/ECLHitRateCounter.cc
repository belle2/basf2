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

#include <numeric>

using namespace std;


namespace Belle2 {
  namespace Background {

    void ECLHitRateCounter::initialize(TTree* tree)
    {
      segmentECL();
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional();
      m_dsps.isOptional();

      // set branch address
      tree->Branch("ecl", &m_rates, "averageRate/F:numEvents/I:valid/O:averageDspBkgRate[16]/F:numEventsSegments[16]/I:validDspRate/O");

      //ECL calibration

      DBObjPtr<ECLCrystalCalib> m_ElectronicsCalib("ECLCrystalElectronics"), m_ECLECalib("ECLCrystalEnergy");
      if (m_ElectronicsCalib.hasChanged()) {
        electronicsCalib = m_ElectronicsCalib->getCalibVector();
      }
      if (m_ECLECalib.hasChanged()) {
        energyCalib = m_ECLECalib->getCalibVector();
      }
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

      /* or count selected ones only
      for(const auto& digit: m_digits) {
      // select digits to count (usualy only good ones)
         rates.averageRate += 1;
      }
      */


      //calculate rates using waveforms
      //The background rate for a crystal is calculated as
      //rate = pedestal_squared / (average_photon_energy_squared * time_constant)
      //where time_constant=2.53 us and average_photon_energy_squared = 1 MeV
      if (m_dsps.getEntries() == 8736) {
        for (auto& aECLDsp : m_dsps) {

          int nadc = aECLDsp.getNADCPoints();
          int cellid = aECLDsp.getCellId();
          int segment_number = findSegment(cellid);
          int crysID = cellid - 1;
          std::vector<int> dspAv = aECLDsp.getDspA();
          rates.numEventsSegments[segment_number]++;

          //finding the pedestal value
          double sum31 = std::accumulate(dspAv.begin(), dspAv.begin() + nadc, 0.0);
          double dspMean = sum31 / nadc;

          double wpsum = 0;
          for (int v = 0; v < nadc; v++) {
            wpsum += pow(dspAv[v] - dspMean, 2);
          }

          double dspRMS = sqrt(wpsum / nadc);
          double dspEnergy = dspRMS * abs(electronicsCalib[crysID] * energyCalib[crysID]);
          //calculating the backgorund rate
          double dspBkgRate = (pow(dspEnergy, 2)) / (2.53 * 1e-12);

          //hit rate for segment in ECL, which is later normalized per 1Hz
          rates.averageDspBkgRate[segment_number] += dspBkgRate;

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
  } // Background namespace
} // Belle2 namespace
