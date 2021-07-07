/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// Own include
#include <top/modules/TOPDigitizer/TOPDigitizerModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/modules/TOPDigitizer/TimeDigitizer.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// ROOT
#include <TRandom.h>

#include <map>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(TOPDigitizer)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPDigitizerModule::TOPDigitizerModule() : Module()
  {
    // Set description()
    setDescription("Digitize TOPSimHits");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("timeZeroJitter", m_timeZeroJitter,
             "r.m.s of T0 jitter [ns]", 15e-3);
    addParam("electronicJitter", m_electronicJitter,
             "r.m.s of electronic jitter [ns], "
             "if negative the one from TOPNominalTDC is used. "
             "This parameter is ignored in the full waveform digitization.", -1.0);
    addParam("darkNoise", m_darkNoise,
             "uniformly distributed dark noise (hits per module)", 0.0);
    addParam("ADCx0", m_ADCx0,
             "pulse height distribution parameter [ADC counts]", 204.1);
    addParam("ADCp1", m_ADCp1,
             "pulse height distribution parameter (must be non-negative)", 1.0);
    addParam("ADCp2", m_ADCp2,
             "pulse height distribution parameter (must be non-negative)", 1.025);
    addParam("ADCmax", m_ADCmax,
             "pulse height distribution upper bound [ADC counts]", 2000.0);
    addParam("rmsNoise", m_rmsNoise,
             "r.m.s of noise [ADC counts]", 9.7);
    addParam("threshold", m_threshold,
             "pulse height threshold [ADC counts]", 40);
    addParam("hysteresis", m_hysteresis,
             "pulse height threshold hysteresis [ADC counts]", 10);
    addParam("thresholdCount", m_thresholdCount,
             "minimal number of samples above threshold", 3);
    addParam("useWaveforms", m_useWaveforms,
             "if true, use full waveform digitization", true);
    addParam("allChannels", m_allChannels,
             "if true, make waveforms for all 8192 channels "
             "(note: this will slow-down digitization)", false);
    addParam("useDatabase", m_useDatabase,
             "if true, use channel dependent constants from database (incl. time base)",
             true);
    addParam("useSampleTimeCalibration", m_useSampleTimeCalibration,
             "if true, use only time base calibration from database "
             "(has no effect if useDatabase = True)", false);
    addParam("simulateTTS", m_simulateTTS,
             "if true, simulate time transition spread. "
             "Should be always switched ON, except for some dedicated timing studies.",
             true);
    addParam("minWidthXheight", m_minWidthXheight,
             "minimal product of width and height [ns * ADC counts]", 100.0);
    addParam("lookBackWindows", m_lookBackWindows,
             "number of look back windows, if positive override the number from database", 0);
  }


  void TOPDigitizerModule::initialize()
  {
    // input from datastore
    m_simHits.isRequired();
    m_simCalPulses.isOptional();
    m_mcParticles.isOptional();
    m_simClockState.isOptional();

    // output to datastore
    m_rawDigits.registerInDataStore();
    m_digits.registerInDataStore();
    m_digits.registerRelationTo(m_simHits);
    m_digits.registerRelationTo(m_mcParticles);
    m_digits.registerRelationTo(m_rawDigits);
    m_waveforms.registerInDataStore(DataStore::c_DontWriteOut);
    m_rawDigits.registerRelationTo(m_waveforms, DataStore::c_Event,
                                   DataStore::c_DontWriteOut);

    // geometry and nominal data
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    if (m_electronicJitter < 0) {
      m_electronicJitter = geo->getNominalTDC().getTimeJitter();
    }

    // set pile-up and double hit resolution times (needed for BG overlay)
    TOPDigit::setDoubleHitResolution(geo->getNominalTDC().getDoubleHitResolution());
    TOPDigit::setPileupTime(geo->getNominalTDC().getPileupTime());

    // default sample times (equidistant)
    m_syncTimeBase = geo->getNominalTDC().getSyncTimeBase();
    m_sampleTimes.setTimeAxis(m_syncTimeBase); // equidistant time base

    // default pulse height generator
    m_pulseHeightGenerator = PulseHeightGenerator(m_ADCx0, m_ADCp1, m_ADCp2, m_ADCmax);

  }


  void TOPDigitizerModule::beginRun()
  {
    StoreObjPtr<EventMetaData> evtMetaData;

    // check availability of mappers
    const auto& channelMapper = TOPGeometryPar::Instance()->getChannelMapper();
    if (not channelMapper.isValid()) {
      B2FATAL("No valid channel mapper found for run "
              << evtMetaData->getRun()
              << " of experiment " << evtMetaData->getExperiment());
    }
    const auto& frontEndMapper = TOPGeometryPar::Instance()->getFrontEndMapper();
    if (not frontEndMapper.isValid()) {
      B2FATAL("No valid front-end mapper found for run "
              << evtMetaData->getRun()
              << " of experiment " << evtMetaData->getExperiment());
    }

    // check availability of constants in database
    if (m_useDatabase) {
      if (not m_timebases.isValid()) {
        B2FATAL("Sample time calibration constants requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
      if (not m_channelT0.isValid()) {
        B2FATAL("Channel T0 calibration constants requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
      if (not m_asicShift.isValid()) {
        B2FATAL("ASIC shifts calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
      if (not m_moduleT0.isValid()) {
        B2FATAL("Module T0 calibration constants requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
      if (not m_commonT0.isValid()) {
        B2FATAL("Common T0 calibration constants requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
      if (not m_pulseHeights.isValid()) {
        B2FATAL("Pulse height calibration constants requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
      if (not m_thresholds.isValid()) {
        B2FATAL("Channel thresholds requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
      if (not m_noises.isValid()) {
        B2FATAL("Channel noise levels requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
      if (m_timeWalk.isValid()) {
        TimeDigitizer::setTimeWalk(&m_timeWalk);
      } else {
        TimeDigitizer::setTimeWalk(0);
        // B2FATAL("Time-walk parameters requested but not available for run "
        B2WARNING("Time-walk parameters not available for run "
                  << evtMetaData->getRun()
                  << " of experiment " << evtMetaData->getExperiment());
      }
    } else if (m_useSampleTimeCalibration) {
      if (not m_timebases.isValid()) {
        B2FATAL("Sample time calibration constants requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }

    // check availability of front-end settings
    if (not m_feSetting.isValid()) {
      B2FATAL("Front-end settings are not available for run "
              << evtMetaData->getRun()
              << " of experiment " << evtMetaData->getExperiment());
    }

    // pass a parameter to TimeDigitizer
    TimeDigitizer::setReadoutWindows(m_feSetting->getReadoutWindows());
    if ((evtMetaData->getExperiment() > 0 and evtMetaData->getExperiment() < 5) or
        evtMetaData->getExperiment() == 1002) {
      TimeDigitizer::maskSamples(true); // phase-2: mask samples at window boundaries
    } else {
      TimeDigitizer::maskSamples(false); // phase-3: no masking
    }

  }

  void TOPDigitizerModule::event()
  {

    // get or generate revo9 count
    unsigned revo9cnt = 0;
    if (m_simClockState.isValid()) {
      revo9cnt = m_simClockState->getRevo9Count();
    } else {
      revo9cnt = gRandom->Integer(11520);
    }

    // from revo9 count determine trigger time offset and the number of offset windows
    double SSTfrac = (revo9cnt % 6) / 6.0;
    double offset = m_feSetting->getOffset() / 24.0;
    double trgTimeOffset = (SSTfrac + offset) * m_syncTimeBase;  // in [ns]
    int offsetWindows = m_feSetting->getWindowShift(revo9cnt);
    TimeDigitizer::setOffsetWindows(offsetWindows);

    // from revo9 and write depths determine reference window, phase and storage depth
    int SSTcnt = revo9cnt / 6;
    int refWindow = SSTcnt * 2;  // same as lastWriteAddr
    const auto& writeDepths = m_feSetting->getWriteDepths();
    if (writeDepths.empty()) {
      B2ERROR("TOPDigitzer: vector of write depths is empty. No digitization possible");
      return;
    }
    int lastDepth = writeDepths.back();
    unsigned phase = 0;
    for (auto depth : writeDepths) {
      SSTcnt -= depth;
      if (SSTcnt < 0) break;
      phase++;
      refWindow = SSTcnt * 2;
      lastDepth = depth;
    }
    unsigned storageDepth = lastDepth * 2;
    TimeDigitizer::setStorageDepth(storageDepth);

    // from reference window and lookback determine first of the readout windows
    int lookBackWindows = m_feSetting->getLookbackWindows();
    if (m_lookBackWindows > 0) lookBackWindows = m_lookBackWindows;
    lookBackWindows -= m_feSetting->getExtraWindows();
    int window = refWindow - lookBackWindows;
    if (window < 0) window += storageDepth;
    TimeDigitizer::setFirstWindow(window);

    // geometry and nominal data
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    // time range for digitization
    double timeMin = geo->getSignalShape().getTMin() + offsetWindows * m_syncTimeBase / 2;
    double timeMax = geo->getSignalShape().getTMax() +
                     (m_feSetting->getReadoutWindows() + offsetWindows) * m_syncTimeBase / 2;

    // simulate start time jitter
    double startTimeJitter = gRandom->Gaus(0, m_timeZeroJitter);

    // get electronic efficiency
    double electronicEfficiency = geo->getNominalTDC().getEfficiency();

    // define pixels with time digitizers
    std::map<unsigned, TimeDigitizer> pixels;
    typedef std::map<unsigned, TimeDigitizer>::iterator Iterator;

    // add simulated hits to time digitizers

    for (const auto& simHit : m_simHits) {
      if (not m_useWaveforms) {
        // simulate electronic efficiency
        if (gRandom->Rndm() > electronicEfficiency) continue;
      }
      // do spatial digitization
      double x = simHit.getX();
      double y = simHit.getY();
      int pmtID = simHit.getPmtID();
      int moduleID = simHit.getModuleID();
      if (not geo->isModuleIDValid(moduleID)) continue;
      int pixelID = geo->getModule(moduleID).getPMTArray().getPixelID(x, y, pmtID);
      if (pixelID == 0) continue;

      // add start time jitter and generated TTS to photon time
      double time = simHit.getTime() + startTimeJitter;
      if (m_simulateTTS) {
        const auto& tts = TOPGeometryPar::Instance()->getTTS(moduleID, pmtID);
        time += tts.generateTTS();
      }

      // get time offset for a given pixel
      auto timeOffset = getTimeOffset(trgTimeOffset, moduleID, pixelID);

      // time range cut (to speed up digitization)
      if (time + timeOffset.value < timeMin + timeOffset.timeShift) continue;
      if (time + timeOffset.value > timeMax + timeOffset.timeShift) continue;

      // generate pulse height
      double pulseHeight = generatePulseHeight(moduleID, pixelID);
      auto hitType = TimeDigitizer::c_Hit;

      // add time and pulse height to digitizer of a given pixel
      TimeDigitizer digitizer(moduleID, pixelID, timeOffset.value, timeOffset.error,
                              timeOffset.windowShift, m_rmsNoise, m_sampleTimes);
      if (not digitizer.isValid()) continue;
      unsigned id = digitizer.getUniqueID();
      Iterator it = pixels.insert(pair<unsigned, TimeDigitizer>(id, digitizer)).first;
      it->second.addTimeOfHit(time, pulseHeight, hitType, &simHit);
    }

    // add calibration pulses

    for (const auto& simCalPulses : m_simCalPulses) {
      auto moduleID = simCalPulses.getModuleID();
      auto pixelID = simCalPulses.getPixelID();
      auto pulseHeight = simCalPulses.getAmplitude();
      auto time = simCalPulses.getTime();
      auto hitType = TimeDigitizer::c_CalPulse;

      // get time offset for a given pixel
      auto timeOffset = getTimeOffset(trgTimeOffset, moduleID, pixelID);

      // time range cut (to speed up digitization)
      if (time + timeOffset.value < timeMin + timeOffset.timeShift) continue;
      if (time + timeOffset.value > timeMax + timeOffset.timeShift) continue;

      // add time and pulse height to digitizer of a given pixel
      TimeDigitizer digitizer(moduleID, pixelID, timeOffset.value, timeOffset.error,
                              timeOffset.windowShift, m_rmsNoise, m_sampleTimes);
      if (not digitizer.isValid()) continue;
      unsigned id = digitizer.getUniqueID();
      Iterator it = pixels.insert(pair<unsigned, TimeDigitizer>(id, digitizer)).first;
      it->second.addTimeOfHit(time, pulseHeight, hitType);
    }

    // add randomly distributed dark noise (maybe not needed anymore?)

    if (m_darkNoise > 0) {
      int numModules = geo->getNumModules();
      for (int moduleID = 1; moduleID <= numModules; moduleID++) {
        int numPixels = geo->getModule(moduleID).getPMTArray().getNumPixels();
        int numHits = gRandom->Poisson(m_darkNoise);
        for (int i = 0; i < numHits; i++) {
          int pixelID = int(gRandom->Rndm() * numPixels) + 1;
          double time = (timeMax - timeMin) * gRandom->Rndm() + timeMin;
          double pulseHeight = generatePulseHeight(moduleID, pixelID);
          auto hitType = TimeDigitizer::c_Hit;
          auto timeOffset = getTimeOffset(trgTimeOffset, moduleID, pixelID);
          time += timeOffset.timeShift;
          time -= timeOffset.value;
          TimeDigitizer digitizer(moduleID, pixelID, timeOffset.value, timeOffset.error,
                                  timeOffset.windowShift, m_rmsNoise, m_sampleTimes);
          if (not digitizer.isValid()) continue;
          unsigned id = digitizer.getUniqueID();
          Iterator it = pixels.insert(pair<unsigned, TimeDigitizer>(id, digitizer)).first;
          it->second.addTimeOfHit(time, pulseHeight, hitType);
        }
      }
    }

    // if making waveforms for all channels, add missing digitizers.

    if (m_allChannels) {
      int numModules = geo->getNumModules();
      for (int moduleID = 1; moduleID <= numModules; moduleID++) {
        int numPixels = geo->getModule(moduleID).getPMTArray().getNumPixels();
        for (int pixelID = 1; pixelID <= numPixels; pixelID++) {
          auto timeOffset = getTimeOffset(trgTimeOffset, moduleID, pixelID);
          TimeDigitizer digitizer(moduleID, pixelID, timeOffset.value, timeOffset.error,
                                  timeOffset.windowShift, m_rmsNoise, m_sampleTimes);
          if (not digitizer.isValid()) continue;
          unsigned id = digitizer.getUniqueID();
          pixels.insert(pair<unsigned, TimeDigitizer>(id, digitizer));
        }
      }
    }

    // replace equidistant time base with calibrated one if available

    if (m_useDatabase or m_useSampleTimeCalibration) {
      for (auto& pixel : pixels) {
        auto& digitizer = pixel.second;
        const auto* sampleTimes = m_timebases->getSampleTimes(digitizer.getScrodID(),
                                                              digitizer.getChannel());
        if (not sampleTimes) continue;
        if (sampleTimes->isCalibrated()) digitizer.setSampleTimes(sampleTimes);
      }
    }

    // replace default noise level with channel dependent one if available,

    if (m_useDatabase) {
      for (auto& pixel : pixels) {
        auto& digitizer = pixel.second;
        auto moduleID = digitizer.getModuleID();
        auto channel = digitizer.getChannel();
        auto rmsNoise = m_noises->getNoise(moduleID, channel);
        if (rmsNoise > 0) {
          digitizer.setNoise(rmsNoise);
        }
      }
    }

    // digitize in time

    for (auto& pixel : pixels) {
      const auto& digitizer = pixel.second;
      int threshold = m_threshold;
      if (m_useDatabase) { // use channel dependent ones
        threshold = m_thresholds->getThr(digitizer.getModuleID(), digitizer.getChannel());
        if (threshold <= 0) threshold = m_threshold; // not available, use the default
      }
      if (m_useWaveforms) {
        digitizer.digitize(m_waveforms, m_rawDigits, m_digits,
                           threshold, m_hysteresis, m_thresholdCount);
      } else {
        digitizer.digitize(m_rawDigits, m_digits,
                           threshold, m_thresholdCount, m_electronicJitter);
      }
    }

    // set additional info

    for (auto& rawDigit : m_rawDigits) {
      rawDigit.setRevo9Counter(revo9cnt);
      rawDigit.setPhase(phase);
      rawDigit.setLastWriteAddr(refWindow);
      rawDigit.setLookBackWindows(lookBackWindows);
      rawDigit.setOfflineFlag();
    }

    for (auto& waveform : m_waveforms) {
      waveform.setRevo9Counter(revo9cnt);
      waveform.setOffsetWindows(offsetWindows);
    }

    // set calibration flags

    if (m_useDatabase) {
      for (auto& digit : m_digits) {
        if (m_channelT0->isCalibrated(digit.getModuleID(), digit.getChannel())) {
          digit.addStatus(TOPDigit::c_ChannelT0Calibrated);
        }
        if (m_moduleT0->isCalibrated(digit.getModuleID())) {
          digit.addStatus(TOPDigit::c_ModuleT0Calibrated);
        }
        if (m_commonT0->isCalibrated()) {
          digit.addStatus(TOPDigit::c_CommonT0Calibrated);
        }
      }
    }

    // cut on product of pulse width and height, as for data in TOPRawDigitConverter

    for (auto& digit : m_digits) {
      if (digit.getPulseWidth() * digit.getPulseHeight() < m_minWidthXheight)
        digit.setHitQuality(TOPDigit::c_Junk);
    }

  }


  TOPDigitizerModule::TimeOffset TOPDigitizerModule::getTimeOffset(double trgOffset,
      int moduleID,
      int pixelID)
  {
    double timeOffset = trgOffset;
    double calErrorSq = 0;
    int winShift = 0;
    double timeShift = 0;
    if (m_useDatabase) {
      const auto& channelMapper = TOPGeometryPar::Instance()->getChannelMapper();
      auto channel = channelMapper.getChannel(pixelID);
      if (m_channelT0->isCalibrated(moduleID, channel)) {
        timeOffset += m_channelT0->getT0(moduleID, channel);
        double err = m_channelT0->getT0Error(moduleID, channel);
        calErrorSq += err * err;
      }
      auto asic = channel / 8;
      if (m_asicShift->isCalibrated(moduleID, asic)) {
        timeOffset += m_asicShift->getT0(moduleID, asic);
        winShift = lround(m_asicShift->getT0(moduleID, asic) / m_syncTimeBase * 2);
        timeShift = winShift * m_syncTimeBase / 2;
      }
      if (m_moduleT0->isCalibrated(moduleID)) {
        timeOffset += m_moduleT0->getT0(moduleID);
        double err = m_moduleT0->getT0Error(moduleID);
        calErrorSq += err * err;
      }
      if (m_commonT0->isCalibrated()) {
        timeOffset += m_commonT0->getT0();
        double err = m_commonT0->getT0Error();
        calErrorSq += err * err;
      }
    }
    return TimeOffset(timeOffset, calErrorSq, winShift, timeShift);
  }


  double TOPDigitizerModule::generatePulseHeight(int moduleID, int pixelID) const
  {
    if (m_useDatabase) {
      const auto& channelMapper = TOPGeometryPar::Instance()->getChannelMapper();
      auto channel = channelMapper.getChannel(pixelID);
      if (m_pulseHeights->isCalibrated(moduleID, channel)) {
        const auto& par = m_pulseHeights->getParameters(moduleID, channel);
        PulseHeightGenerator generator(par.x0, par.p1, par.p2, m_ADCmax);
        return generator.generate();
      }
    }

    return m_pulseHeightGenerator.generate();
  }

} // end Belle2 namespace

