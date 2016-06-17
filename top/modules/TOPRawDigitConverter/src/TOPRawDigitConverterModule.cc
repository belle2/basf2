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

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// Dataobject classes
#include <top/dataobjects/TOPRawDigit.h>
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
             "if true, use sample time calibration (needs DB)", true);
    addParam("useChannelT0Calibration", m_useChannelT0Calibration,
             "if true, use channel T0 calibration (needs DB)", true);
    addParam("useModuleT0Calibration", m_useModuleT0Calibration,
             "if true, use module T0 calibration (needs DB)", true);
    addParam("useCommonT0Calibration", m_useCommonT0Calibration,
             "if true, use common T0 calibration (needs DB)", true);

    if (m_useSampleTimeCalibration) m_timebase = new DBObjPtr<TOPCalTimebase>;
    if (m_useChannelT0Calibration) m_channelT0 = new DBObjPtr<TOPCalChannelT0>;
    if (m_useModuleT0Calibration) m_moduleT0 = new DBObjPtr<TOPCalModuleT0>;
    if (m_useCommonT0Calibration) m_commonT0 = new DBObjPtr<TOPCalCommonT0>;

  }


  TOPRawDigitConverterModule::~TOPRawDigitConverterModule()
  {
    if (m_timebase) delete m_timebase;
    if (m_channelT0) delete m_channelT0;
    if (m_moduleT0) delete m_moduleT0;
    if (m_commonT0) delete m_commonT0;
  }


  void TOPRawDigitConverterModule::initialize()
  {

    StoreArray<TOPRawDigit> rawDigits(m_inputRawDigitsName);
    rawDigits.isRequired();

    StoreArray<TOPDigit> digits(m_outputDigitsName);
    digits.registerInDataStore();
    digits.registerRelationTo(rawDigits);

    // equidistant sample times in case calibration is not required
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_sampleTimes.setTimeAxis(geo->getNominalTDC().getSyncTimeBase());
    m_sampleDivisions = (0x1 << geo->getNominalTDC().getSubBits());

  }


  void TOPRawDigitConverterModule::beginRun()
  {

    StoreObjPtr<EventMetaData> evtMetaData;

    // check if calibrations are available when needed - if not, terminate

    if (m_useSampleTimeCalibration) {
      if (!(*m_timebase).isValid()) {
        B2FATAL("Sample time calibration requested but not available for run "
                << evtMetaData->getRun());
      }
    }
    if (m_useChannelT0Calibration) {
      if (!(*m_channelT0).isValid()) {
        B2FATAL("Channel T0 calibration requested but not available for run "
                << evtMetaData->getRun());
      }
    }
    if (m_useModuleT0Calibration) {
      if (!(*m_moduleT0).isValid()) {
        B2FATAL("Module T0 calibration requested but not available for run "
                << evtMetaData->getRun());
      }
    }
    if (m_useCommonT0Calibration) {
      if (!(*m_commonT0).isValid()) {
        B2FATAL("Common T0 calibration requested but not available for run "
                << evtMetaData->getRun());
      }
    }

  }


  void TOPRawDigitConverterModule::event()
  {

    // get mappers
    const auto& feMapper = TOPGeometryPar::Instance()->getFrontEndMapper();
    const auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();

    StoreArray<TOPRawDigit> rawDigits(m_inputRawDigitsName);
    StoreArray<TOPDigit> digits(m_outputDigitsName);
    digits.clear();

    for (const auto& rawDigit : rawDigits) {
      auto scrodID = rawDigit.getScrodID();
      const auto* feemap = feMapper.getMap(scrodID);
      if (!feemap) {
        B2ERROR("No front-end map available for SCROD " << scrodID);
        continue;
      }
      auto moduleID = feemap->getModuleID();
      auto boardstack = feemap->getBoardstackNumber();
      auto channel = chMapper.getChannel(boardstack,
                                         rawDigit.getCarrierNumber(),
                                         rawDigit.getASICNumber(),
                                         rawDigit.getASICChannel());
      auto pixelID = chMapper.getPixelID(channel);
      double rawTime = rawDigit.getCFDLeadingTime();
      int tdc = int(rawTime * m_sampleDivisions);
      const auto* sampleTimes = &m_sampleTimes; // equidistant sample times
      if (m_useSampleTimeCalibration) {
        sampleTimes = (*m_timebase)->getSampleTimes(scrodID, channel % 128);
        if (!sampleTimes->isCalibrated()) {
          B2ERROR("No sample time calibration available for SCROD " << scrodID
                  << " channel " << channel % 128 << " - raw digit not converted");
          continue;
        }
      }
      float time = sampleTimes->getTimeDifference(rawDigit.getASICWindow(), rawTime);

      if (m_useChannelT0Calibration) time -= (*m_channelT0)->getT0(moduleID, channel);
      if (m_useModuleT0Calibration) time -= (*m_moduleT0)->getT0(moduleID);
      if (m_useCommonT0Calibration) time -= (*m_commonT0)->getT0();

      auto* digit = digits.appendNew(moduleID, pixelID, tdc);
      digit->setTime(time);
      digit->setADC(rawDigit.getValuePeak());
      digit->setPulseWidth(rawDigit.getFWHM());
      digit->setChannel(channel);
      digit->setFirstWindow(rawDigit.getASICWindow());
      digit->addRelationTo(&rawDigit);
    }

  }


  void TOPRawDigitConverterModule::endRun()
  {
  }

  void TOPRawDigitConverterModule::terminate()
  {
  }


} // end Belle2 namespace

