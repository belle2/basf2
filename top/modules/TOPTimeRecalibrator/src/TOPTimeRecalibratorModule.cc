/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPTimeRecalibrator/TOPTimeRecalibratorModule.h>
#include <top/geometry/TOPGeometryPar.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>


using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPTimeRecalibrator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPTimeRecalibratorModule::TOPTimeRecalibratorModule() : Module()

  {
    // set module description
    setDescription("Utility module for re-calibrating time of TOPDigits. "
                   "Useful for checking new calibrations on existing cDST files. "
                   "Note that pulseWidth and timeError are not changed "
                   "although they also depend on time calibration.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("useSampleTimeCalibration", m_useSampleTimeCalibration,
             "if true, use sample time calibration", true);
    addParam("useChannelT0Calibration", m_useChannelT0Calibration,
             "if true, use channel T0 calibration", true);
    addParam("useModuleT0Calibration", m_useModuleT0Calibration,
             "if true, use module T0 calibration", true);
    addParam("useCommonT0Calibration", m_useCommonT0Calibration,
             "if true, use common T0 calibration", true);
    addParam("subtractBunchTime", m_subtractBunchTime,
             "if true, subtract reconstructed bunch time", true);

  }

  void TOPTimeRecalibratorModule::initialize()
  {

    // registration of objects in datastore
    m_digits.isRequired();
    m_recBunch.isRequired();

    // equidistant sample times in case calibration is not required
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_syncTimeBase = geo->getNominalTDC().getSyncTimeBase();
    m_sampleTimes.setTimeAxis(m_syncTimeBase);

  }

  void TOPTimeRecalibratorModule::beginRun()
  {
    StoreObjPtr<EventMetaData> evtMetaData;

    // check if calibrations are available when needed - if not, terminate

    if (m_useSampleTimeCalibration) {
      if (!m_timebase.isValid()) {
        B2FATAL("Sample time calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }
    if (m_useChannelT0Calibration) {
      if (!m_channelT0.isValid()) {
        B2FATAL("Channel T0 calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }
    if (m_useModuleT0Calibration) {
      if (!m_moduleT0.isValid()) {
        B2FATAL("Module T0 calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }
    if (m_useCommonT0Calibration) {
      if (!m_commonT0.isValid()) {
        B2FATAL("Common T0 calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }

  }

  void TOPTimeRecalibratorModule::event()
  {
    int revo9cnt = m_recBunch->getRevo9Counter();
    double SSTfrac = (revo9cnt % 6) / 6.0;
    double timeOffset = SSTfrac * m_syncTimeBase;  // in [ns], to be subtracted
    const auto& feMapper = TOPGeometryPar::Instance()->getFrontEndMapper();
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    for (auto& digit : m_digits) {

      // save MC offset status
      bool offsetStatus = digit.hasStatus(TOPDigit::c_OffsetSubtracted);

      // reset status bits
      unsigned short statusBits = 0;
      digit.setStatus(statusBits);

      // get what's needed from a digit
      double rawTimeLeading = digit.getRawTime();
      auto window = digit.getFirstWindow();
      auto moduleID = digit.getModuleID();
      auto channel = digit.getChannel();

      // convert raw time to time using equidistant or calibrated time base
      const auto* sampleTimes = &m_sampleTimes; // equidistant sample times
      if (m_useSampleTimeCalibration) {
        auto bs = channel / 128;
        const auto* feemap = feMapper.getMap(moduleID, bs);
        if (!feemap) {
          B2ERROR("No front-end map available."
                  << LogVar("slot", moduleID)
                  << LogVar("boardstack", bs));
          continue;
        }
        auto scrodID = feemap->getScrodID();
        sampleTimes = m_timebase->getSampleTimes(scrodID, channel % 128);
        if (sampleTimes->isCalibrated()) {
          statusBits |= TOPDigit::c_TimeBaseCalibrated;
        }
      }
      double time = sampleTimes->getTime(window, rawTimeLeading) - timeOffset;

      // apply other calibrations
      if (m_useChannelT0Calibration) {
        const auto& cal = m_channelT0;
        if (cal->isCalibrated(moduleID, channel)) {
          time -= cal->getT0(moduleID, channel);
          statusBits |= TOPDigit::c_ChannelT0Calibrated;
        }
      }
      if (m_useModuleT0Calibration) {
        const auto& cal = m_moduleT0;
        if (cal->isCalibrated(moduleID)) {
          time -= cal->getT0(moduleID);
          statusBits |= TOPDigit::c_ModuleT0Calibrated;
        }
      }
      if (m_useCommonT0Calibration) {
        const auto& cal = m_commonT0;
        if (cal->isCalibrated()) {
          time -= cal->getT0();
          statusBits |= TOPDigit::c_CommonT0Calibrated;
        }
      }

      // subtract bunch time
      if (m_subtractBunchTime and m_recBunch->isReconstructed()) {
        time -= m_recBunch->getTime();
        statusBits |= TOPDigit::c_EventT0Subtracted;
      }

      // subtract offset used in MC if status bit was set in this digit
      if (offsetStatus) {
        time -= geo->getNominalTDC().getOffset();
        statusBits |= TOPDigit::c_OffsetSubtracted;
      }

      // set re-calibrated time and status bits
      digit.setTime(time);
      digit.setStatus(statusBits);
    }

  }


} // end Belle2 namespace

