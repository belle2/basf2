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
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPTimeZero.h>

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
    setDescription("Module merges raw waveforms, performs pedestal subtraction "
                   "and gain correction, and converts waveforms to hits.");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("outlayerCut", m_outlayerCut, "outlayer removal cut value in number of "
             "sigma for pedestal offset determination", 3.0);
    addParam("threshold", m_threshold, "threshold value in number of sigma "
             "to discriminate signal against noise", 5.0);
    addParam("hysteresis", m_hysteresis,
             "comparator hysteresis in number of sigma", 1.0);
    addParam("minWidth", m_minWidth,
             "minimal required width of digital pulse in number of samples "
             "(accept pulse if width > minWidth)", 2);
    addParam("fraction", m_fraction,
             "the fraction for constant fraction discrimination", 0.5);
    addParam("useFTSW", m_useFTSW,
             "add or not add FTSW time to hit times when making TOPDigits", true);

  }


  TOPWFMergerModule::~TOPWFMergerModule()
  {
  }

  void TOPWFMergerModule::initialize()
  {

    StoreArray<TOPRawWaveform> rawWaveforms;
    rawWaveforms.isRequired();

    StoreObjPtr<TOPTimeZero> timeZero;
    if (m_useFTSW) {
      timeZero.isRequired();
    } else {
      timeZero.isOptional();
    }

    StoreArray<TOPWaveform> waveforms;
    waveforms.registerInDataStore();
    waveforms.registerRelationTo(rawWaveforms);

    StoreArray<TOPDigit> digits;
    digits.registerInDataStore();
    digits.registerRelationTo(waveforms);

    if (m_fraction >= 1) B2ERROR("TOPWFMerger: fraction must be less that 1");
    if (m_fraction <= 0) B2ERROR("TOPWFMerger: fraction must be greater that 0");

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

    // group together waveforms belonging to the same module and channel

    typedef std::vector<const TOPRawWaveform*> RawWaveforms;
    std::map<unsigned, RawWaveforms> map;
    for (const auto& rawWaveform : rawWaveforms) {
      auto key = getKey(rawWaveform.getBarID(), rawWaveform.getChannelID());
      map[key].push_back(&rawWaveform);
    }

    // merge raw waveforms, do pedestal subtraction and gain correction

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

    // convert to hits

    double t0 = 0;
    if (m_useFTSW) {
      StoreObjPtr<TOPTimeZero> timeZero;
      t0 = timeZero->getTime();
    }

    StoreArray<TOPDigit> digits;
    for (auto& waveform : waveforms) {
      int nDig = waveform.setDigital(m_threshold, m_threshold - m_hysteresis, m_minWidth);
      if (nDig == 0) continue;
      int nHit = waveform.convertToHits(m_fraction);
      if (nHit == 0) continue;
      auto barID = waveform.getBarID();
      auto pixelID = waveform.getPixelID();
      auto channelID = waveform.getChannelID();
      auto firstWindow = waveform.getFirstWindow();
      auto refWindow = waveform.getReferenceWindow();
      const auto& hits = waveform.getHits();
      for (const auto& hit : hits) {
        auto tdc = m_topgp->getTDCcount(hit.time);
        auto* digit = digits.appendNew(barID, pixelID, tdc);
        digit->setTime(hit.time + t0);
        digit->setADC(hit.height);
        digit->setPulseWidth(hit.width);
        digit->setChannelID(channelID);
        digit->setFirstWindow(firstWindow);
        digit->setReferenceWindow(refWindow);
        digit->addRelationTo(&waveform);
      }
    }

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
      if (sample.err == 0) sample.adc = 0; // undefined pedestal
      sample.time = calibration->getSampleTime(window, i);
      waveform->appendSample(sample);
      i++;
    }
    waveform->appendWindowNumber(rawWaveform->getStorageWindow());
    waveform->setReferenceWindow(rawWaveform->getReferenceWindow());

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

    const int niter = 4;
    double cut = m_outlayerCut * (1 << (niter - 1));
    double offset = 0;
    for (int iter = 0; iter < niter; iter++) {
      double sumy = 0;
      double sume = 0;
      for (const auto& sample : samples) {
        if (fabs(sample.adc - offset) / sample.err > cut) continue;
        double errSq = sample.err * sample.err;
        sumy += sample.adc / errSq;
        sume += 1 / errSq;
      }
      if (sume > 0) offset = sumy / sume;
      cut /= 2;
    }

    return offset;
  }



} // end Belle2 namespace

