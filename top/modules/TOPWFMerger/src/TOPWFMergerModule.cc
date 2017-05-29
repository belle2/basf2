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
#include <top/geometry/TOPGeometryPar.h>


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

  using namespace TOP;

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
    addParam("activeWindows", m_activeWindows,
             "number of active ASIC windows (used for logger only)", 64);
    addParam("useSampleTimeCalibration", m_useSampleTimeCalibration,
             "if true, use sample time calibration constants from database, "
             "otherwise use equidistant time axis", false);

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

    // equidistant sample times in case calibration is not required
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_sampleTimes.setTimeAxis(geo->getNominalTDC().getSyncTimeBase());
    m_sampleDivisions = (0x1 << geo->getNominalTDC().getSubBits());

  }

  void TOPWFMergerModule::beginRun()
  {

    if (m_asicChannels.hasChanged()) {

      m_pedestalMap.clear();
      for (const auto& asic : m_asicChannels) {
        auto key = getKey(asic.getModuleID(), asic.getChannel());
        m_pedestalMap[key] = &asic;
      }

      B2INFO("TOPWFMerger: new ASIC pedestal calibration map of size "
             << m_pedestalMap.size() << " created");
    }
    if (m_pedestalMap.empty()) {
      B2FATAL("TOPWFMerger: cannot proceed - no ASIC pedestals available");
      return;
    }

  }


  void TOPWFMergerModule::event()
  {

    StoreArray<TOPRawWaveform> rawWaveforms;
    StoreArray<TOPWaveform> waveforms;

    // group together waveforms belonging to the same module and channel

    typedef std::vector<const TOPRawWaveform*> RawWaveforms;
    std::map<unsigned, RawWaveforms> map;
    for (const auto& rawWaveform : rawWaveforms) {
      auto key = getKey(rawWaveform.getModuleID(), rawWaveform.getChannel());
      map[key].push_back(&rawWaveform);
    }

    // merge raw waveforms, do pedestal subtraction and gain correction

    for (const auto& element : map) {
      auto key = element.first;
      const auto* calibration = m_pedestalMap[key];
      if (!calibration) {
        B2WARNING("TOPWFMerger: no pedestal calibration available for module " <<
                  getScrodID(key) << " channel " << getChannel(key));
        continue;
      }
      int numWindows = calibration->getNumofWindows();
      auto moduleID = element.second[0]->getModuleID();
      auto pixelID = element.second[0]->getPixelID();
      auto channel = element.second[0]->getChannel();
      auto scrodID = element.second[0]->getScrodID();
      auto* waveform = waveforms.appendNew(moduleID, pixelID, channel, scrodID);
      int prevWindow = element.second[0]->getStorageWindow() - 1;
      for (const auto& rawWaveform : element.second) {
        int window = rawWaveform->getStorageWindow();
        int diff = window - prevWindow;
        prevWindow = window;
        if (diff != 1 and diff != (1 - numWindows))
          waveform = waveforms.appendNew(moduleID, pixelID, channel, scrodID);
        bool ok = appendRawWavefrom(rawWaveform, calibration, waveform);
        if (ok) waveform->addRelationTo(rawWaveform);
      }
    }

    // convert to hits

    float t0 = 0;
    if (m_useFTSW) {
      StoreObjPtr<TOPTimeZero> timeZero;
      t0 = timeZero->getTime();
    }

    StoreArray<TOPDigit> digits;
    for (auto& waveform : waveforms) {
      int nDig = waveform.setDigital(m_threshold,
                                     m_threshold - m_hysteresis,
                                     m_minWidth);
      if (nDig == 0) continue;
      int nHit = waveform.convertToHits(m_fraction);
      if (nHit == 0) continue;
      auto moduleID = waveform.getModuleID();
      auto pixelID = waveform.getPixelID();
      auto channel = waveform.getChannel();
      auto firstWindow = waveform.getFirstWindow();
      const auto* sampleTimes = &m_sampleTimes; // default calibration
      if (m_useSampleTimeCalibration and m_timebase.isValid()) {
        auto scrodID = waveform.getScrodID();
        sampleTimes = m_timebase->getSampleTimes(scrodID, channel % 128);
        if (!sampleTimes->isCalibrated()) {
          B2ERROR("No sample time calibration available for SCROD " << scrodID
                  << " channel " << channel % 128);
        }
      }

      const auto& hits = waveform.getHits();
      for (const auto& hit : hits) {
        float time = sampleTimes->getTime(firstWindow, hit.time);
        auto* digit = digits.appendNew(moduleID, pixelID, hit.time);
        digit->setTime(time + t0);
        digit->setTimeError(hit.timeErr);  // TODO: convert to [ns]
        digit->setPulseHeight(hit.height);
        digit->setIntegral(hit.area);
        digit->setPulseWidth(hit.width);  // TODO: convert to [ns], give FWHM!
        digit->setChannel(channel);
        digit->setFirstWindow(firstWindow);
        digit->addRelationTo(&waveform);
      }
    }

  }


  void TOPWFMergerModule::endRun()
  {
  }

  void TOPWFMergerModule::terminate()
  {
    if (m_falseWindows > 0)
      B2ERROR("TOPWFMerger: " << m_falseWindows <<
              " ASIC windows found with window number > " << m_activeWindows - 1 <<
              " and no pedestals");

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
      if (window < (unsigned) m_activeWindows) {
        B2WARNING("TOPWFMerger: no calibration available for module " <<
                  calibration->getModuleID() << " channel " <<
                  calibration->getChannel() << " window " << window);
      } else {
        m_falseWindows++;
      }
      return false;
    }
    const auto* gains = calibration->getGains(window);
    if (!gains) {
      B2ERROR("TOPWFMerger: gains not given");
      return false;
    }

    if (rawWaveform->getModuleID() != calibration->getModuleID())
      B2FATAL("TOPWFMerger: bug (module ID)");
    if (rawWaveform->getChannel() != calibration->getChannel())
      B2FATAL("TOPWFMerger: bug (channel number)");
    if (window != pedestals->getASICWindow())
      B2FATAL("TOPWFMerger: bug (ASIC window number)");

    auto offset = getPedestalOffset(rawWaveform, pedestals);

    unsigned i = 0;
    for (const auto& rawADC : rawWaveform->getWaveform()) {
      auto gain = gains->getValue(i);
      TOPWaveform::WFSample sample;
      sample.adc = (rawADC - pedestals->getValue(i) - offset) * gain;
      sample.err = (pedestals->getError(i)) * gain;
      if (sample.err == 0) sample.adc = 0; // undefined pedestal
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

