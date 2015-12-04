/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPWFCalibrator/TOPWFCalibratorModule.h>
#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework - Database
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPRawWaveform.h>

// database classes
#include <framework/database/DBStore.h>
#include <top/dbobjects/TOPASICChannel.h>

#include "TFile.h"
#include <TClonesArray.h>
#include <iostream>
#include <sstream>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPWFCalibrator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPWFCalibratorModule::TOPWFCalibratorModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Calibration of waveforms (under development)");

    // Add parameters
    addParam("histogramFileName", m_histogramFileName, "Output root file for histograms",
             string(""));

    addParam("barID", m_barID, "ID of TOP module to calibrate");

    addParam("runLow", m_runLow, "IOV:  run lowest", 0);
    addParam("runHigh", m_runHigh, "IOV:  run highest", 0);

    for (int i = 0; i < c_NumChannels; i++) {
      for (int k = 0; k < c_NumWindows; k++) {
        m_profile[i][k] = 0;
      }
      m_baseline[i] = 0;
    }

  }

  TOPWFCalibratorModule::~TOPWFCalibratorModule()
  {
  }

  void TOPWFCalibratorModule::initialize()
  {

    StoreArray<TOPRawWaveform>::required();

  }

  void TOPWFCalibratorModule::beginRun()
  {
  }

  void TOPWFCalibratorModule::event()
  {

    StoreArray<TOPRawWaveform> waveforms;

    for (auto& waveform : waveforms) {
      if (waveform.getBarID() != m_barID) continue;
      unsigned channel = waveform.getChannel();
      unsigned window = waveform.getStorageWindow();
      if (channel < c_NumChannels and window < c_NumWindows) {
        auto baseline = m_baseline[channel];
        if (!baseline) {
          stringstream ss;
          ss << "chan" << channel;
          string name;
          ss >> name;
          string title = "Pedestal's average per window " + name;
          int size = c_NumWindows;
          baseline = new TProfile(name.c_str(), title.c_str(), size, 0, size, "s");
          m_baseline[channel] = baseline;
        }
        double average = 0;
        for (const auto& adc : waveform.getWaveform()) average += adc;
        average /= waveform.getSize();
        baseline->Fill(window, average);

        auto prof = m_profile[channel][window];
        if (!prof) {
          stringstream ss;
          ss << "chan" << channel << "win" << window;
          string name;
          ss >> name;
          string title = "Pedestals " + name;
          int size = waveform.getSize();
          prof = new TProfile(name.c_str(), title.c_str(), size, 0, size, "s");
          m_profile[channel][window] = prof;
        }
        int i = 0;
        for (const auto& adc : waveform.getWaveform()) {
          prof->Fill(i, adc - average);
          i++;
        }

      }
    }

  }


  void TOPWFCalibratorModule::endRun()
  {
  }

  void TOPWFCalibratorModule::terminate()
  {

    StoreObjPtr<EventMetaData> evtMetaData;

    if (!m_histogramFileName.empty()) {
      TFile* file = new TFile(m_histogramFileName.c_str(), "RECREATE");
      if (file->IsZombie()) {
        B2ERROR("Couldn't open file '" << m_histogramFileName << "' for writing!");
      } else {
        file->cd();
        for (int i = 0; i < c_NumChannels; i++) {
          for (int k = 0; k < c_NumWindows; k++) {
            TProfile* prof = m_profile[i][k];
            if (prof) prof->Write();
          }
          TProfile* prof = m_baseline[i];
          if (prof) prof->Write();
        }
      }
      file->Close();
    }

    unsigned numWindows = 0;
    for (unsigned i = 0; i < c_NumChannels; i++) {
      for (unsigned k = 0; k < c_NumWindows; k++) {
        TProfile* prof = m_profile[i][k];
        if (!prof) continue;
        if (k > numWindows) numWindows = k;
      }
    }
    numWindows++;
    B2INFO("TOPWFCalibratorModule: number of active ASIC storage windows: " <<
           numWindows);

    TClonesArray constants("Belle2::TOPASICChannel");
    const auto name = DBStore::arrayName<TOPASICChannel>("");

    int badPed = 0;
    int badSampl = 0;
    for (int channel = 0; channel < c_NumChannels; channel++) {
      new(constants[channel]) TOPASICChannel(m_barID, channel, numWindows);
      auto* channelConstants = static_cast<TOPASICChannel*>(constants[channel]);
      auto* baseline = m_baseline[channel];
      for (int window = 0; window < c_NumWindows; window++) {
        TProfile* prof = m_profile[channel][window];
        if (prof) {
          TOPASICPedestals pedestals(window);
          double average = 0;
          if (baseline) average = baseline->GetBinContent(window + 1);
          int bad = pedestals.setPedestals(prof, average);
          if (bad > 0) {
            badPed++;
            badSampl += bad;
            B2INFO("TOPWFCalibratorModule: " << bad
                   << " bad pedestal(s) found for channel "
                   << channel << " window " << window);
          }
          bool ok = channelConstants->setPedestals(pedestals);
          if (!ok) {
            B2ERROR("TOPWFCalibratorModule: can't set pedestals for channel "
                    << channel << " storage window " << window);
          }
        }
      }
    }

    auto expNo = evtMetaData->getExperiment();
    IntervalOfValidity iov(expNo, m_runLow, expNo, m_runHigh);
    Database::Instance().storeData(name, &constants, iov);

    int all = 0;
    int incomplete = 0;
    for (int channel = 0; channel < constants.GetEntriesFast(); channel++) {
      const auto* chan = static_cast<TOPASICChannel*>(constants[channel]);
      all++;
      if (chan->getNumofGoodWindows() != chan->getNumofWindows()) incomplete++;
    }

    B2RESULT("TOPWFCalibratorModule: bar ID = " << m_barID <<
             ", number of calibrated channels " << all <<
             " incomplete: " << incomplete <<
             " bad samples: " << badSampl <<
             " in " << badPed << " windows");

  }


} // end Belle2 namespace

