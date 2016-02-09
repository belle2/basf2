/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPSampleTimeCalibrator/TOPSampleTimeCalibratorModule.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>

// Root
#include "TFile.h"

#include <iostream>
#include <sstream>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPSampleTimeCalibrator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPSampleTimeCalibratorModule::TOPSampleTimeCalibratorModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Sample time calibrator (under development - not usable yet!)");
    //    setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

    // Add parameters
    addParam("histogramFileName", m_histogramFileName,
             "Output root file for histograms", string(""));
    addParam("timeCutLow", m_timeCutLow, "time cut: low", 0.0);
    addParam("timeCutHigh", m_timeCutHigh, "time cut: high", 50.0);
    addParam("adcCutLow", m_adcCutLow, "ADC cut: low", 0.0);
    addParam("adcCutHigh", m_adcCutHigh, "ADC cut: high", 1.0e6);
    addParam("calChannel", m_calChannel, "calibration channel", 7);

    // initialize array
    for (int i = 0; i < c_NumModules; i++) {
      for (int k = 0; k < c_NumChannels; k++) {
        m_profile[i][k] = 0;
      }
    }

  }

  TOPSampleTimeCalibratorModule::~TOPSampleTimeCalibratorModule()
  {
  }

  void TOPSampleTimeCalibratorModule::initialize()
  {

    StoreArray<TOPDigit>::required();

    if (!m_topgp->isInitialized()) {
      GearDir content("/Detector/DetectorComponent[@name='TOP']/Content");
      m_topgp->Initialize(content);
    }
    if (!m_topgp->isInitialized())
      B2FATAL("Component TOP not found in Gearbox");

  }

  void TOPSampleTimeCalibratorModule::beginRun()
  {
  }

  void TOPSampleTimeCalibratorModule::event()
  {

    StoreArray<TOPDigit> digits;

    // fill profile histograms
    const TOPDigit* firstCalSignal = 0;
    for (const auto& digit : digits) {
      if (digit.getHitQuality() == TOPDigit::c_Junk) continue;
      double time = digit.getTime();
      if (time == 0) firstCalSignal = &digit;
      if (time < m_timeCutLow) continue;
      if (time > m_timeCutHigh) continue;
      double adc = digit.getADC();
      if (adc < m_adcCutLow) continue;
      if (adc > m_adcCutHigh) continue;

      int moduleID = digit.getModuleID();
      auto channel = digit.getChannel();
      if (channel % 8 != (unsigned) m_calChannel) continue;

      if (!firstCalSignal) continue;
      if (firstCalSignal->getChannel() != channel) continue;

      auto prof = m_profile[moduleID - 1][channel];
      if (!prof) {
        stringstream ss;
        ss << "module" << moduleID << "chan" << channel;
        string name;
        ss >> name;
        string title = "Second calibration signal: time vs. sample number " + name;
        prof = new TProfile(name.c_str(), title.c_str(), c_NumSamples, 0,
                            c_NumSamples, m_timeCutLow, m_timeCutHigh);
        m_profile[moduleID - 1][channel] = prof;
      }
      unsigned tdc = firstCalSignal->getTDC() / 16;
      unsigned win = firstCalSignal->getFirstWindow();
      unsigned sample = (tdc + (win % 4) * 64) % 256;
      prof->Fill(sample, time);
    }

  }


  void TOPSampleTimeCalibratorModule::endRun()
  {
  }

  void TOPSampleTimeCalibratorModule::terminate()
  {

    // write histograms to file
    if (!m_histogramFileName.empty()) {
      TFile* file = new TFile(m_histogramFileName.c_str(), "RECREATE");
      if (file->IsZombie()) {
        B2ERROR("Couldn't open file '" << m_histogramFileName << "' for writing!");
      } else {
        file->cd();
        for (int i = 0; i < c_NumModules; i++) {
          for (int k = 0; k < c_NumChannels; k++) {
            TProfile* prof = m_profile[i][k];
            if (prof) prof->Write();
          }
        }
      }
      file->Close();
    }

    // find sample times


  }

  void TOPSampleTimeCalibratorModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

