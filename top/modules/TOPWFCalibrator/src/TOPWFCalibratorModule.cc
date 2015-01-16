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

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPRawWaveform.h>

#include "TFile.h"
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
    addParam("outputFileName", m_outputFileName, "Output root file",
             string("WFCalibration.root"));

    addParam("barID", m_barID, "ID of TOP module to calibrate");

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

    for (auto & waveform : waveforms) {
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
        for (const auto & adc : waveform.getWaveform()) {
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

    TFile* file = new TFile(m_outputFileName.c_str(), "RECREATE");
    file->cd();
    for (int i = 0; i < c_NumChannels; i++) {
      for (int k = 0; k < c_NumWindows; k++) {
        TProfile* prof = m_profile[i][k];
        if (prof) prof->Write();
      }
    }
    file->Close();

  }


} // end Belle2 namespace

