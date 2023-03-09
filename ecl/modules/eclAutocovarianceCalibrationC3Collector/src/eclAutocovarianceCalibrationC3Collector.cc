/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module`
#include <ecl/modules/eclAutocovarianceCalibrationC3Collector/eclAutocovarianceCalibrationC3Collector.h>

//Root
#include <TH1F.h>

//Framework
#include <framework/dataobjects/EventMetaData.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(eclAutocovarianceCalibrationC3Collector)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclAutocovarianceCalibrationC3CollectorModule::eclAutocovarianceCalibrationC3CollectorModule() : CalibrationCollectorModule(),
  m_ECLAutocovarianceCalibrationC3Threshold("ECLAutocovarianceCalibrationC1Threshold")
{
  // Set module properties
  setDescription("Module to export histogram of noise in waveforms from random trigger events");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("BaselineLimit", m_BaselineLimit, "Number of waveforms required to compute baseline", 10000);
}

void eclAutocovarianceCalibrationC3CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclAutocovarianceCalibrationC3Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  auto BaselineVsCrysID = new TH1F("BaselinevsCrysID", "Baseline for each crystal;crystal ID;Baseline (ADC)", 8736, 0, 8736);
  registerObject<TH1F>("BaselineVsCrysID", BaselineVsCrysID);

  m_PeakToPeakThresholds = m_ECLAutocovarianceCalibrationC3Threshold->getCalibVector();

  m_eclDsps.registerInDataStore();
  m_eclDigits.registerInDataStore();

  m_Baseline.assign(8736, vector < float >(31, 0));
  m_BaselineLimit = 10000;
  m_counter.assign(8736, 0);
  m_BaselineComputed.assign(8736, 0);

}


void eclAutocovarianceCalibrationC3CollectorModule::collect()
{

  const int NumDsp = m_eclDsps.getEntries();

  //Random Trigger Event
  if (NumDsp == 8736) {

    //B2INFO(NumDsp);
    //if (NumDsp >0) {

    for (auto& aECLDsp : m_eclDsps) {

      const int id = aECLDsp.getCellId() - 1;

      if (m_BaselineComputed[id] == false) {

        int minADC = aECLDsp.getDspA()[0];
        int maxADC = aECLDsp.getDspA()[0];

        for (int i = 0; i < 31; i++) {

          if (aECLDsp.getDspA()[i] < minADC) minADC = aECLDsp.getDspA()[i];
          if (aECLDsp.getDspA()[i] > maxADC) maxADC = aECLDsp.getDspA()[i];

        }

        int PeakToPeak = maxADC - minADC;

        //B2INFO(PeakToPeak<<" "<<m_PeakToPeakThresholds[id]);

        //low noise waveform
        if (PeakToPeak < m_PeakToPeakThresholds[id]) {

          m_counter[id]++;

          for (int i = 0; i < 31; i++) m_Baseline[id][i] += (float)aECLDsp.getDspA()[i];

        }

        if (m_counter[id] == m_BaselineLimit) {

          float baseline = 0.0;

          for (int i = 0; i < 31; i++)  baseline += m_Baseline[id][i];
          baseline /= (float)m_counter[id];

          getObjectPtr<TH1F>("BaselineVsCrysID")->SetBinContent(id + 1, baseline);

          m_BaselineComputed[id] = true;

          B2INFO(id << " " << baseline);

        }
      }
    }
  }
}
