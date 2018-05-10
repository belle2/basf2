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
#include <top/modules/TOPDigitizer/TOPDigitizerModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/modules/TOPDigitizer/TimeDigitizer.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
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
             "r.m.s of T0 jitter [ns]", 25e-3);
    addParam("electronicJitter", m_electronicJitter,
             "r.m.s of electronic jitter [ns], "
             "if negative the one from TOPNominalTDC is used. "
             "This parameter is ignored in the full waveform digitization.", -1.0);
    addParam("darkNoise", m_darkNoise,
             "uniformly distributed dark noise (hits per module)", 0.0);
    addParam("ADCx0", m_ADCx0,
             "pulse height distribution parameter [ADC counts]", 3.0);
    addParam("ADCp1", m_ADCp1,
             "pulse height distribution parameter (must be non-negative)", 3.85);
    addParam("ADCp2", m_ADCp2,
             "pulse height distribution parameter (must be non-negative)", 0.544);
    addParam("ADCmax", m_ADCmax,
             "pulse height distribution upper bound [ADC counts]", 2000.0);
    addParam("rmsNoise", m_rmsNoise,
             "r.m.s of noise [ADC counts]", 9.0);
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
    addParam("storageDepth", m_storageDepth, "ASIC analog storage depth", (unsigned) 508);

  }


  TOPDigitizerModule::~TOPDigitizerModule()
  {
    if (m_timebases) delete m_timebases;
    if (m_pulseHeights) delete m_pulseHeights;
    if (m_thresholds) delete m_thresholds;
    if (m_noises) delete m_noises;
  }

  void TOPDigitizerModule::initialize()
  {
    // input from datastore
    m_simHits.isRequired();
    m_simCalPulses.isOptional();
    m_mcParticles.isOptional();

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
    double syncTimeBase = geo->getNominalTDC().getSyncTimeBase();
    m_sampleTimes.setTimeAxis(syncTimeBase); // equidistant time base

    // default pulse height generator
    m_pulseHeightGenerator = PulseHeightGenerator(m_ADCx0, m_ADCp1, m_ADCp2, m_ADCmax);

    // constants from database
    if (m_useDatabase) {
      m_timebases = new DBObjPtr<TOPCalTimebase>;
      m_pulseHeights = new DBObjPtr<TOPCalChannelPulseHeight>;
      m_thresholds = new DBObjPtr<TOPCalChannelThreshold>;
      m_noises = new DBObjPtr<TOPCalChannelNoise>;
    } else if (m_useSampleTimeCalibration) {
      m_timebases = new DBObjPtr<TOPCalTimebase>;
    }

    // time range for digitization
    m_timeMin = geo->getNominalTDC().getTimeMin() + geo->getSignalShape().getTMin();
    m_timeMax = geo->getNominalTDC().getTimeMax() + geo->getSignalShape().getTMax();

  }

  void TOPDigitizerModule::beginRun()
  {
    StoreObjPtr<EventMetaData> evtMetaData;

    // check availability of constants in database
    if (m_useDatabase) {
      if (!(*m_timebases).isValid()) {
        B2FATAL("Sample time calibration constants requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
      if (!(*m_pulseHeights).isValid()) {
        B2FATAL("Pulse height calibration constants requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
      if (!(*m_thresholds).isValid()) {
        B2FATAL("Channel thresholds requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
      if (!(*m_noises).isValid()) {
        B2FATAL("Channel noise levels requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    } else if (m_useSampleTimeCalibration) {
      if (!(*m_timebases).isValid()) {
        B2FATAL("Sample time calibration constants requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }

  }

  void TOPDigitizerModule::event()
  {

    // generate first window number
    unsigned window = gRandom->Integer(m_storageDepth);

    // simulate start time jitter
    double startTimeJitter = gRandom->Gaus(0, m_timeZeroJitter);

    // get nominal TTS and electronic efficiency
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    const auto& tts = geo->getNominalTTS();
    double electronicEfficiency = geo->getNominalTDC().getEfficiency();

    // define pixels with time digitizers
    std::map<unsigned, TimeDigitizer> pixels;
    typedef std::map<unsigned, TimeDigitizer>::iterator Iterator;

    // add simulated hits to time digitizers

    for (const auto& simHit : m_simHits) {
      // simulate electronic efficiency
      if (!m_useWaveforms) {
        if (gRandom->Rndm() > electronicEfficiency) continue;
      }
      // do spatial digitization
      double x = simHit.getX();
      double y = simHit.getY();
      int pmtID = simHit.getPmtID();
      int moduleID = simHit.getModuleID();
      if (!geo->isModuleIDValid(moduleID)) continue;
      int pixelID = geo->getModule(moduleID).getPMTArray().getPixelID(x, y, pmtID);
      if (pixelID == 0) continue;

      // add start time jitter and generated TTS to photon time
      double time = simHit.getTime() + startTimeJitter;
      if (m_simulateTTS) time += tts.generateTTS();

      // time range cut (to speed up digitization)
      if (time < m_timeMin) continue;
      if (time > m_timeMax) continue;

      // generate pulse height
      double pulseHeight = generatePulseHeight(moduleID, pixelID);
      auto hitType = TimeDigitizer::c_Hit;

      // add time and pulse height to digitizer of a given pixel
      TimeDigitizer digitizer(moduleID, pixelID, window, m_storageDepth,
                              m_rmsNoise, m_sampleTimes);
      if (!digitizer.isValid()) continue;
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
      TimeDigitizer digitizer(moduleID, pixelID, window, m_storageDepth,
                              m_rmsNoise, m_sampleTimes);
      if (!digitizer.isValid()) continue;
      unsigned id = digitizer.getUniqueID();
      Iterator it = pixels.insert(pair<unsigned, TimeDigitizer>(id, digitizer)).first;
      it->second.addTimeOfHit(time, pulseHeight, hitType);
    }

    // add randomly distributed dark noise (maybe not needed anymore?)

    if (m_darkNoise > 0) {
      int numModules = geo->getNumModules();
      double timeMin = geo->getNominalTDC().getTimeMin();
      double timeMax = geo->getNominalTDC().getTimeMax();
      for (int moduleID = 1; moduleID <= numModules; moduleID++) {
        int numPixels = geo->getModule(moduleID).getPMTArray().getNumPixels();
        int numHits = gRandom->Poisson(m_darkNoise);
        for (int i = 0; i < numHits; i++) {
          int pixelID = int(gRandom->Rndm() * numPixels) + 1;
          double time = (timeMax - timeMin) * gRandom->Rndm() + timeMin;
          double pulseHeight = generatePulseHeight(moduleID, pixelID);
          auto hitType = TimeDigitizer::c_Hit;
          TimeDigitizer digitizer(moduleID, pixelID, window, m_storageDepth,
                                  m_rmsNoise, m_sampleTimes);
          if (!digitizer.isValid()) continue;
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
          TimeDigitizer digitizer(moduleID, pixelID, window, m_storageDepth,
                                  m_rmsNoise, m_sampleTimes);
          if (!digitizer.isValid()) continue;
          unsigned id = digitizer.getUniqueID();
          pixels.insert(pair<unsigned, TimeDigitizer>(id, digitizer));
        }
      }
    }

    // replace equidistant time base with calibrated one if available

    if (m_timebases) {
      for (auto& pixel : pixels) {
        auto& digitizer = pixel.second;
        const auto* sampleTimes = (*m_timebases)->getSampleTimes(digitizer.getScrodID(),
                                                                 digitizer.getChannel());
        if (!sampleTimes) continue;
        if (sampleTimes->isCalibrated()) digitizer.setSampleTimes(sampleTimes);
      }
    }

    // replace default noise level with channel dependent one if available

    if (m_noises) {
      for (auto& pixel : pixels) {
        auto& digitizer = pixel.second;
        auto rmsNoise = (*m_noises)->getNoise(digitizer.getModuleID(),
                                              digitizer.getChannel());
        if (rmsNoise > 0) {
          digitizer.setNoise(rmsNoise);
        }
      }
    }

    // digitize in time

    for (auto& pixel : pixels) {
      const auto& digitizer = pixel.second;
      int threshold = m_threshold;
      if (m_thresholds) { // use channel dependent ones
        threshold = (*m_thresholds)->getThr(digitizer.getModuleID(),
                                            digitizer.getChannel());
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

  }


  void TOPDigitizerModule::endRun()
  {

  }

  void TOPDigitizerModule::terminate()
  {

  }

  double TOPDigitizerModule::generatePulseHeight(int moduleID, int pixelID) const
  {
    if (m_pulseHeights) {
      const auto& channelMapper = TOPGeometryPar::Instance()->getChannelMapper();
      if (!channelMapper.isValid()) {
        B2ERROR("TOPDigitizer: no valid channel mapper found");
        return 0;
      }
      auto channel = channelMapper.getChannel(pixelID);
      if ((*m_pulseHeights)->isCalibrated(moduleID, channel)) {
        const auto& par = (*m_pulseHeights)->getParameters(moduleID, channel);
        PulseHeightGenerator generator(par.x0, par.p1, par.p2, m_ADCmax);
        return generator.generate();
      }
    }

    return m_pulseHeightGenerator.generate();
  }

} // end Belle2 namespace

