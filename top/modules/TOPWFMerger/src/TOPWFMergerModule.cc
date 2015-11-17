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
#include <top/modules/TOPWFMerger/TOPWFMergerModule.h>

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
#include <top/dataobjects/TOPWaveform.h>

#include <iostream>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPWFMerger)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPWFMergerModule::TOPWFMergerModule() : Module()

  {
    // set module description
    setDescription("TOP waveform merger + pedestal subtraction, gain correction");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("outlayerCut", m_outlayerCut, " ", 2.0);

  }

  TOPWFMergerModule::~TOPWFMergerModule()
  {
  }

  void TOPWFMergerModule::initialize()
  {

    StoreArray<TOPRawWaveform> rawWaveforms;
    rawWaveforms.isRequired();

    StoreArray<TOPWaveform> waveforms;
    waveforms.registerInDataStore();
    waveforms.registerRelationTo(rawWaveforms);

  }

  void TOPWFMergerModule::beginRun()
  {

    if (!m_asicChannels.hasChanged()) return;

    m_map.clear();
    for (const auto& asic : m_asicChannels) {
      auto key = getKey(asic.getBarID(), asic.getChannelID());
      m_map[key] = &asic;
    }

    B2INFO("TOPWFMerger: new ASIC calibration map of size " << m_map.size() << " created");

  }


  void TOPWFMergerModule::event()
  {

    StoreArray<TOPRawWaveform> rawWaveforms;
    StoreArray<TOPWaveform> waveforms;

    typedef std::vector<const TOPRawWaveform*> RawWaveforms;
    std::map<unsigned, RawWaveforms> map;
    for (const auto& rawWaveform : rawWaveforms) {
      auto key = getKey(rawWaveform.getBarID(), rawWaveform.getChannelID());
      map[key].push_back(&rawWaveform);
    }

    for (const auto& element : map) {
      auto key = element.first;
      const auto* calibration = m_map[key]; // ASIC channel calibration constants
      if (!calibration) {
        B2WARNING("TOPWFMerger: no calibration available for " << key);
        continue;
      }
      int numWindows = calibration->getNumofWindows();
      auto barID = element.second[0]->getBarID();
      auto pixelID = element.second[0]->getPixelID();
      auto channelID = element.second[0]->getChannelID();
      auto* waveform = waveforms.appendNew(barID, pixelID, channelID);
      int prevWindow = element.second[0]->getStorageWindow() - 1;
      for (const auto& rawWaveform : element.second) {
        int window = rawWaveform->getStorageWindow();
        int diff = window - prevWindow;
        prevWindow = window;
        if (diff != 1 and diff != (1 - numWindows))
          waveform = waveforms.appendNew(barID, pixelID, channelID);
        bool ok = appendRawWavefrom(rawWaveform, calibration, waveform);
        if (ok) waveform->addRelationTo(rawWaveform);
      }
    }

    return;

    //-- test ----->
    for (const auto& waveform : waveforms) {
      cout << waveform.getBarID() << " ";
      cout << waveform.getChannelID() << " ";
      for (const auto& raw : waveform.getRelationsTo<TOPRawWaveform>())
        cout << raw.getStorageWindow() << " ";
      cout << endl;
    }
    //<-- test -----

  }


  void TOPWFMergerModule::endRun()
  {
  }

  void TOPWFMergerModule::terminate()
  {
  }


  bool TOPWFMergerModule::appendRawWavefrom(const TOPRawWaveform* rawWaveform,
                                            const TOPASICChannel* calibration,
                                            TOPWaveform* waveform)
  {
    if (!rawWaveform) return false;
    if (!calibration) return false;
    if (!waveform) return false;

    auto window = rawWaveform->getStorageWindow();
    const auto* pedestals = calibration->getPedestals(window);
    if (!pedestals) {
      B2WARNING("TOPWFMerger: no calibration available for bar " <<
                calibration->getBarID() << " channel " <<
                calibration->getChannelID() << " window " << window);
      return false;
    }
    const auto* gains = calibration->getGains(window);
    if (!gains) {
      B2ERROR("TOPWFMerger: gains not given");
      return false;
    }

    if (rawWaveform->getBarID() != calibration->getBarID())
      B2FATAL("TOPWFMerger: bug (barID)");
    if (rawWaveform->getChannelID() != calibration->getChannelID())
      B2FATAL("TOPWFMerger: bug (channelID)");
    if (window != pedestals->getASICWindowID())
      B2FATAL("TOPWFMerger: bug (windowID)");

    auto offset = getPedestalOffset(rawWaveform, pedestals);

    unsigned i = 0;
    for (const auto& rawADC : rawWaveform->getWaveform()) {
      auto gain = gains->getValue(i);
      TOPWaveform::WFSample sample;
      sample.adc = (rawADC - pedestals->getValue(i) - offset) * gain;
      sample.err = (pedestals->getError(i)) * gain;
      sample.time = calibration->getSampleTime(window, i);
      waveform->appendSample(sample);
      i++;
    }

    return true;
  }


  float TOPWFMergerModule::getPedestalOffset(const TOPRawWaveform* rawWaveform,
                                             const TOPASICPedestals* pedestals)
  {

    if (m_outlayerCut <= 0) return 0;

    std::vector<TOPWaveform::WFSample> samples;
    unsigned i = 0;
    for (const auto& rawADC : rawWaveform->getWaveform()) {
      TOPWaveform::WFSample sample;
      sample.adc = rawADC - pedestals->getValue(i);
      sample.err = pedestals->getError(i);
      if (sample.err > 0) samples.push_back(sample);
      i++;
    }
    if (samples.empty()) return 0;

    double cut = 1000;
    double offset = 0;
    for (int iter = 0; iter < 3; iter++) {
      double sumy = 0;
      double sume = 0;
      for (const auto& sample : samples) {
        if (fabs(sample.adc - offset) / sample.err > cut) continue;
        double errSq = sample.err * sample.err;
        sumy += sample.adc / errSq;
        sume += 1 / errSq;
      }
      if (sume > 0) offset = sumy / sume;
      cut = m_outlayerCut;
    }

    return offset;
  }



} // end Belle2 namespace

