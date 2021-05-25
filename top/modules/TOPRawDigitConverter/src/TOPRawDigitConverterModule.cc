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
#include <top/modules/TOPRawDigitConverter/TOPRawDigitConverterModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// Dataobject classes
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPRawWaveform.h>
#include <top/dataobjects/TOPDigit.h>
#include <framework/dataobjects/EventMetaData.h>

#include <top/geometry/TOPGeometryPar.h>


using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPRawDigitConverter)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPRawDigitConverterModule::TOPRawDigitConverterModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Converts row digits to digits and applies time calibration");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("inputRawDigitsName", m_inputRawDigitsName,
             "name of TOPRawDigit store array", string(""));
    addParam("outputDigitsName", m_outputDigitsName,
             "name of TOPDigit store array", string(""));
    addParam("useSampleTimeCalibration", m_useSampleTimeCalibration,
             "if true, use sample time calibration", true);
    addParam("useAsicShiftCalibration", m_useAsicShiftCalibration,
             "if true, use ASIC shifts calibration", true);
    addParam("useChannelT0Calibration", m_useChannelT0Calibration,
             "if true, use channel T0 calibration", true);
    addParam("useModuleT0Calibration", m_useModuleT0Calibration,
             "if true, use module T0 calibration", true);
    addParam("useCommonT0Calibration", m_useCommonT0Calibration,
             "if true, use common T0 calibration", true);
    addParam("useTimeWalkCalibration", m_useTimeWalkCalibration,
             "if true, use time-walk calibration", true);
    addParam("pedestalRMS", m_pedestalRMS,
             "r.m.s of pedestals [ADC counts], "
             "if positive, timeError will be estimated from FE data. "
             "This is the default value used if r.m.s is not available from DB.", 9.7);
    addParam("minPulseWidth", m_minPulseWidth,
             "minimal pulse width [ns] to flag digit as good", 1.0);
    addParam("maxPulseWidth", m_maxPulseWidth,
             "maximal pulse width [ns] to flag digit as good", 10.0);
    addParam("minWidthXheight", m_minWidthXheight,
             "minimal product of width and height [ns * ADC counts]", 100.0);
    addParam("storageDepth", m_storageDepth,
             "ASIC analog storage depth of Interim FE format (ignored in other formats)",
             (unsigned) 508);
    addParam("lookBackWindows", m_lookBackWindows,
             "number of look back windows, if positive override the number from database",
             0);
    addParam("setPhase", m_setPhase,
             "if true, set (override) phase in TOPRawDigits", true);
    addParam("calibrationChannel", m_calibrationChannel,
             "calpulse selection: ASIC channel (use -1 to turn off the selection)", -1);
    addParam("calpulseWidthMin", m_calpulseWidthMin,
             "calpulse selection: minimal width [ns]", 0.0);
    addParam("calpulseWidthMax", m_calpulseWidthMax,
             "calpulse selection: maximal width [ns]", 0.0);
    addParam("calpulseHeightMin", m_calpulseHeightMin,
             "calpulse selection: minimal height [ADC counts]", 0);
    addParam("calpulseHeightMax", m_calpulseHeightMax,
             "calpulse selection: maximal height [ADC counts]", 0);
    addParam("calpulseTimeMin", m_calpulseTimeMin,
             "calpulse selection (ON if max > min): minimal time [ns]", 0.0);
    addParam("calpulseTimeMax", m_calpulseTimeMax,
             "calpulse selection (ON if max > min): maximal time [ns]", 0.0);
  }


  TOPRawDigitConverterModule::~TOPRawDigitConverterModule()
  {
  }


  void TOPRawDigitConverterModule::initialize()
  {

    // registration of objects in datastore
    m_rawDigits.isRequired(m_inputRawDigitsName);
    m_eventDebugs.isOptional();
    m_digits.registerInDataStore(m_outputDigitsName);
    m_digits.registerRelationTo(m_rawDigits);
    m_asicMask.registerInDataStore();

    // equidistant sample times in case calibration is not required
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_syncTimeBase = geo->getNominalTDC().getSyncTimeBase();
    m_sampleTimes.setTimeAxis(m_syncTimeBase);

  }


  void TOPRawDigitConverterModule::beginRun()
  {

    StoreObjPtr<EventMetaData> evtMetaData;

    // check if calibrations are available when needed - if not, terminate

    if (m_useSampleTimeCalibration) {
      if (not m_timebase.isValid()) {
        B2FATAL("Sample time calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }
    if (m_useChannelT0Calibration) {
      if (not m_channelT0.isValid()) {
        B2FATAL("Channel T0 calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }
    if (m_useAsicShiftCalibration) {
      if (not m_asicShift.isValid()) {
        B2FATAL("ASIC shifts calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }
    if (m_useModuleT0Calibration) {
      if (not m_moduleT0.isValid()) {
        B2FATAL("Module T0 calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }
    if (m_useCommonT0Calibration) {
      if (not m_commonT0.isValid()) {
        B2FATAL("Common T0 calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }
    if (not m_timeWalk.isValid()) {
      // B2FATAL("Time-walk calibration is not available for run "
      B2WARNING("Time-walk calibration is not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
    }
    if (m_pedestalRMS > 0 and not m_noises.isValid()) {
      B2FATAL("Channel noise levels are not available for run "
              << evtMetaData->getRun()
              << " of experiment " << evtMetaData->getExperiment());
    }

    if (not m_feSetting.isValid()) {
      B2FATAL("Front-end settings are not available for run "
              << evtMetaData->getRun()
              << " of experiment " << evtMetaData->getExperiment());
    }

  }


  void TOPRawDigitConverterModule::event()
  {

    // get mappers

    const auto& feMapper = TOPGeometryPar::Instance()->getFrontEndMapper();
    const auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    // clear TOPDigits

    m_digits.clear();

    // set storage windows in RawDigits if not already done in unpacker

    for (auto& rawDigit : m_rawDigits) {
      const auto* waveform = rawDigit.getRelated<TOPRawWaveform>();
      if (waveform and rawDigit.getStorageWindows().empty()) {
        rawDigit.setStorageWindows(waveform->getStorageWindows());
      }
    }

    // set asic masks (asics and whole boardstacks masked in firmware for this event)

    m_asicMask.create();
    if (m_eventDebugs.getEntries() > 0) {
      std::vector<unsigned short> masks(64, 0xFFFF);
      for (const auto& eventDebug : m_eventDebugs) {
        auto scrodID = eventDebug.getScrodID();
        const auto* feemap = feMapper.getMap(scrodID);
        if (not feemap) {
          B2WARNING("TOPRawDigitConverter: No front-end map available."
                    << LogVar("scrodID", scrodID));
          continue;
        }
        auto moduleID = feemap->getModuleID();
        auto boardstack = feemap->getBoardstackNumber();
        unsigned bs = (moduleID - 1) * 4 + boardstack;
        if (bs < 64) {
          masks[bs] = eventDebug.getAsicMask();
        } else {
          B2ERROR("TOPRawDigitConverter: Invalid global boardstack number."
                  << LogVar("bs", bs));
        }
      }
      m_asicMask->set(masks);
    }

    // convert to TOPDigits

    for (const auto& rawDigit : m_rawDigits) {

      if (rawDigit.getErrorFlags() != 0) continue;

      // determine moduleID, pixedID and channel

      auto scrodID = rawDigit.getScrodID();
      const auto* feemap = feMapper.getMap(scrodID);
      if (not feemap) {
        B2WARNING("TOPRawDigitConverter: No front-end map available."
                  << LogVar("scrodID", scrodID));
        continue;
      }
      auto moduleID = feemap->getModuleID();
      auto boardstack = feemap->getBoardstackNumber();
      auto channel = chMapper.getChannel(boardstack,
                                         rawDigit.getCarrierNumber(),
                                         rawDigit.getASICNumber(),
                                         rawDigit.getASICChannel());
      auto pixelID = chMapper.getPixelID(channel);

      // get raw times

      double rawTimeLeading = rawDigit.getCFDLeadingTime(); // time in [samples]
      double rawTimeFalling = rawDigit.getCFDFallingTime(); // time in [samples]

      // get ASIC window

      int window = rawDigit.getASICWindow();

      // timing alignment: set time origin according to data type

      double timeOffset = 0;
      int storageDepth = m_storageDepth;
      if (rawDigit.getDataType() == TOPRawDigit::c_Interim) {

        // correct raw times for possible window discontinuity
        rawTimeLeading = rawDigit.correctTime(rawTimeLeading, m_storageDepth);
        rawTimeFalling = rawDigit.correctTime(rawTimeFalling, m_storageDepth);

        // set window number: number of look back windows back from the last write address
        int lastWriteAddr = rawDigit.getLastWriteAddr();
        int nback = lastWriteAddr - window;
        if (nback < 0) nback += m_storageDepth;
        int lookBackWindows = m_feSetting->getLookbackWindows();
        if (m_lookBackWindows > 0) lookBackWindows = m_lookBackWindows;
        int nwin = lookBackWindows - nback;
        window -= nwin;
        if (window < 0) window += m_storageDepth;
        if (window >= (int) m_storageDepth) window -= m_storageDepth;

        // add samples to raw time to account for the new window number
        rawTimeLeading += nwin * TOPRawDigit::c_WindowSize;
        rawTimeFalling += nwin * TOPRawDigit::c_WindowSize;
      } else if (rawDigit.getDataType() == TOPRawDigit::c_ProductionDebug) {

        // take revo9 counter and calculate corresponding SST count and its fraction
        int revo9cnt = rawDigit.getRevo9Counter();
        int SSTcnt = revo9cnt / 6;
        double SSTfrac = (revo9cnt % 6) / 6.0;
        double offset = m_feSetting->getOffset() / 24.0;
        timeOffset = (SSTfrac + offset) * m_syncTimeBase;  // in [ns], to be subtracted

        // find reference window
        int refWindow = SSTcnt * 2;  // seems to be the same as lastWriteAddr
        const auto& writeDepths = m_feSetting->getWriteDepths();
        if (writeDepths.empty()) {
          B2ERROR("TOPRawDigitConverter: vector of write depths is empty. Return!");
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
        if (m_setPhase) const_cast<TOPRawDigit&>(rawDigit).setPhase(phase);
        storageDepth = lastDepth * 2;

        if (window >= storageDepth) {
          B2WARNING("TOPRawDigitConverter: window number greater than storage depth."
                    << LogVar("window number", window)
                    << LogVar("storage depth", storageDepth)
                    << LogVar("refWindow", refWindow)
                    << LogVar("phase", phase));
          continue;
        }

        // set window number: number of look back windows back from the reference window
        int deltaWindow = window - refWindow;
        if (deltaWindow > 0) deltaWindow -= storageDepth;
        int lookBackWindows = m_feSetting->getLookbackWindows();
        if (m_lookBackWindows > 0) lookBackWindows = m_lookBackWindows;
        lookBackWindows -= m_feSetting->getExtraWindows();

        int nwin = lookBackWindows + deltaWindow;
        int startWindow = refWindow - lookBackWindows;
        if (startWindow < 0) startWindow += storageDepth;
        window = startWindow;

        // add samples to raw time to account for the new window number
        rawTimeLeading += nwin * TOPRawDigit::c_WindowSize;
        rawTimeFalling += nwin * TOPRawDigit::c_WindowSize;
      }

      // convert raw time to time using equidistant or calibrated time base

      unsigned short statusBits = 0;
      const auto* sampleTimes = &m_sampleTimes; // equidistant sample times
      if (m_useSampleTimeCalibration) {
        sampleTimes = m_timebase->getSampleTimes(scrodID, channel % 128);
        if (sampleTimes->isCalibrated()) {
          statusBits |= TOPDigit::c_TimeBaseCalibrated;
        }
      }
      // time and width in [ns]
      double time = sampleTimes->getTime(window, rawTimeLeading) - timeOffset;
      double width = sampleTimes->getDeltaTime(window, rawTimeFalling, rawTimeLeading);

      // default time uncertainty
      double timeError = geo->getNominalTDC().getTimeJitter();

      if (rawDigit.getDataType() == TOPRawDigit::c_MC) {
        // MC with simplified digitization
        time -= geo->getNominalTDC().getOffset();
        statusBits |= TOPDigit::c_OffsetSubtracted;
      } else {
        // data and MC with full waveform digitization
        if (m_pedestalRMS > 0) {
          double rmsNoise = m_pedestalRMS;
          if (m_noises->isCalibrated(moduleID, channel)) {
            rmsNoise = m_noises->getNoise(moduleID, channel);
          }
          double rawErr = rawDigit.getCFDLeadingTimeError(rmsNoise); // in [samples]
          int sample = static_cast<int>(rawTimeLeading);
          if (rawTimeLeading < 0) sample--;
          timeError = rawErr * sampleTimes->getTimeBin(window, sample); // [ns]
        }

        auto pulseHeight = rawDigit.getValuePeak();
        double timeErrorSq = timeError * timeError;
        if (m_timeWalk.isValid()) timeErrorSq += m_timeWalk->getSigmaSq(pulseHeight);

        if (m_useTimeWalkCalibration and m_timeWalk.isValid()) {
          if (m_timeWalk->isCalibrated()) {
            time -= m_timeWalk->getTimeWalk(pulseHeight);
          }
        }
        if (m_useChannelT0Calibration) {
          const auto& cal = m_channelT0;
          if (cal->isCalibrated(moduleID, channel)) {
            time -= cal->getT0(moduleID, channel);
            double err = cal->getT0Error(moduleID, channel);
            timeErrorSq += err * err;
            statusBits |= TOPDigit::c_ChannelT0Calibrated;
          }
        }
        if (m_useAsicShiftCalibration) {
          auto asic = channel / 8;
          if (m_asicShift->isCalibrated(moduleID, asic)) {
            time -= m_asicShift->getT0(moduleID, asic);
          }
        }
        if (m_useModuleT0Calibration) {
          const auto& cal = m_moduleT0;
          if (cal->isCalibrated(moduleID)) {
            time -= cal->getT0(moduleID);
            double err = cal->getT0Error(moduleID);
            timeErrorSq += err * err;
            statusBits |= TOPDigit::c_ModuleT0Calibrated;
          }
        }
        if (m_useCommonT0Calibration) {
          const auto& cal = m_commonT0;
          if (cal->isCalibrated()) {
            time -= cal->getT0();
            double err = cal->getT0Error();
            timeErrorSq += err * err;
            statusBits |= TOPDigit::c_CommonT0Calibrated;
          }
        }
        timeError = sqrt(timeErrorSq);
      }

      // append new TOPDigit and set it

      auto* digit = m_digits.appendNew(moduleID, pixelID, rawTimeLeading);
      digit->setTime(time);
      digit->setTimeError(timeError);
      digit->setPulseHeight(rawDigit.getValuePeak());
      digit->setIntegral(rawDigit.getIntegral());
      digit->setPulseWidth(width);
      digit->setChannel(channel);
      digit->setFirstWindow(window);
      digit->setStatus(statusBits);
      digit->addRelationTo(&rawDigit);

      if (not rawDigit.isFEValid() or rawDigit.isPedestalJump())
        digit->setHitQuality(TOPDigit::c_Junk);
      if (rawDigit.isAtWindowDiscontinuity(storageDepth))
        digit->setHitQuality(TOPDigit::c_Junk);
      if (digit->getPulseWidth() < m_minPulseWidth or
          digit->getPulseWidth() > m_maxPulseWidth or
          digit->getPulseWidth() * digit->getPulseHeight() < m_minWidthXheight)
        digit->setHitQuality(TOPDigit::c_Junk);
    }

    // if calibration channel given, select and flag cal pulses

    unsigned calibrationChannel = m_calibrationChannel;
    if (calibrationChannel < 8) {
      for (auto& digit : m_digits) {
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;
        if (digit.getASICChannel() != calibrationChannel) continue;
        if (digit.getPulseHeight() < m_calpulseHeightMin) continue;
        if (digit.getPulseHeight() > m_calpulseHeightMax) continue;
        if (digit.getPulseWidth() < m_calpulseWidthMin) continue;
        if (digit.getPulseWidth() > m_calpulseWidthMax) continue;
        if (m_calpulseTimeMax > m_calpulseTimeMin) {
          if (digit.getTime() < m_calpulseTimeMin) continue;
          if (digit.getTime() > m_calpulseTimeMax) continue;
        }
        digit.setHitQuality(TOPDigit::c_CalPulse);
      }
    }

  }


  void TOPRawDigitConverterModule::endRun()
  {
  }

  void TOPRawDigitConverterModule::terminate()
  {
  }


} // end Belle2 namespace

