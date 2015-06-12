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
#include <top/calibration/ASICChannelConstants.h>

#include "TFile.h"
#include <TClonesArray.h>
#include <sstream>

using namespace std;

namespace Belle2 {
  using namespace TOP;

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
    addParam("outputFileName", m_outputFileName, "Output root file to emulate DB",
             string("WFCalibration.root"));

    addParam("histogramFileName", m_histogramFileName, "Output root file for histograms",
             string(""));

    addParam("barID", m_barID, "ID of TOP module to calibrate");

    addParam("runLow", m_runLow, "IOV:  run lowest", 0);
    addParam("runHigh", m_runHigh, "IOV:  run highest", 0);

    for (int i = 0; i < c_NumChannels; i++) {
      for (int k = 0; k < c_NumWindows; k++) {
        m_profile[i][k] = 0;
      }
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
      unsigned channel = waveform.getHardwareChannelID();
      unsigned window = waveform.getStorageWindow();
      if (channel < c_NumChannels and window < c_NumWindows) {
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
          prof->Fill(i, adc);
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

    TClonesArray constants("Belle2::TOP::ASICChannelConstants");

    for (int channel = 0; channel < c_NumChannels; channel++) {
      new(constants[channel]) ASICChannelConstants(m_barID, channel, numWindows);
      auto* channelConstants = static_cast<ASICChannelConstants*>(constants[channel]);
      for (int window = 0; window < c_NumWindows; window++) {
        TProfile* prof = m_profile[channel][window];
        if (prof) {
          ASICPedestals pedestals(window);
          pedestals.setPedestals(prof);
          bool ok = channelConstants->setPedestals(pedestals);
          if (!ok) {
            B2ERROR("TOPWFCalibratorModule: can't set pedestals for channel "
                    << channel << " storage window " << window);
          }
        }
      }
    }

    Database::setGlobalTag(m_outputFileName);

    auto expNo = evtMetaData->getExperiment();
    IntervalOfValidity iov(expNo, m_runLow, expNo, m_runHigh);
    Database::Instance().storeData("ASICChannelConstants", &constants, iov);


    int all = 0;
    int incomplete = 0;
    for (int channel = 0; channel < constants.GetEntriesFast(); channel++) {
      const auto* chan = static_cast<ASICChannelConstants*>(constants[channel]);
      all++;
      if (chan->getNumofGoodWindows() != chan->getNumofWindows()) incomplete++;
    }

    B2RESULT("TOPWFCalibratorModule: bar ID = " << m_barID <<
             ", number of calibrated channels " << all <<
             " (" << incomplete << " are incomplete)");

  }


} // end Belle2 namespace

