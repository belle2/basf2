/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module`
#include <ecl/modules/eclAutocovarianceCalibrationC1Collector/eclAutocovarianceCalibrationC1Collector.h>

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
REG_MODULE(eclAutocovarianceCalibrationC1Collector);
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclAutocovarianceCalibrationC1CollectorModule::eclAutocovarianceCalibrationC1CollectorModule()
{
  // Set module properties
  setDescription("Module to export histogram of noise level of ECL waveforms in random trigger events");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void eclAutocovarianceCalibrationC1CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclAutocovarianceCalibrationC1Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  PPVsCrysID = new TH2F("PPVsCrysID", "Peak to peak amplitude for each crystal;crystal ID;Peak to peak Amplitud (ADC)",
                        ECLElementNumbers::c_NCrystals, 0,
                        ECLElementNumbers::c_NCrystals, 2000, 0, 2000);
  registerObject<TH2F>("PPVsCrysID", PPVsCrysID);

  m_eclDsps.registerInDataStore();
}



void eclAutocovarianceCalibrationC1CollectorModule::collect()
{

  //Checking how many waveforms saved in event
  const int NumDsp = m_eclDsps.getEntries();

  //Random Trigger Events have waveform for each crystal
  if (NumDsp == ECLElementNumbers::c_NCrystals) {

    for (auto& aECLDsp : m_eclDsps) {

      const int id = aECLDsp.getCellId() - 1;

      //Peak to peak amplitude used to gauge noise level
      float PeakToPeak = (float) aECLDsp.computePeaktoPeakAmp();

      // I avoid using getObjectPtr<TH2>("PPVsCrysID")->Fill(id, PeakToPeak); to improve on run time as getObjectPtr is  very slow
      PPVsCrysID->Fill(id, PeakToPeak);

    }
  }
}

void eclAutocovarianceCalibrationC1CollectorModule::closeRun()
{
  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {
    for (int j = 0; j < 2000; j++) {
      getObjectPtr<TH2>("PPVsCrysID")->SetBinContent(i + 1, j + 1, PPVsCrysID->GetBinContent(i + 1, j + 1));
    }
  }
}
