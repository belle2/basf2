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
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
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
             "if true, use sample time calibration (needs DB)", true);
    addParam("useChannelT0Calibration", m_useChannelT0Calibration,
             "if true, use channel T0 calibration (needs DB)", true);
    addParam("useModuleT0Calibration", m_useModuleT0Calibration,
             "if true, use module T0 calibration (needs DB)", true);
    addParam("useCommonT0Calibration", m_useCommonT0Calibration,
             "if true, use common T0 calibration (needs DB)", true);
    addParam("subtractOffset", m_subtractOffset,
             "if true, subtract offset defined for nominal TDC (required for MC)", false);
    addParam("calibrationChannel", m_calibrationChannel,
             "calpulse selection: ASIC channel (use -1 to turn off the selection)", -1);
    addParam("calpulseWidthMin", m_calpulseWidthMin,
             "calpulse selection: minimal width [ns]", 0.0);
    addParam("calpulseWidthMax", m_calpulseWidthMax,
             "calpulse selection: maximal width [ns]", 0.0);
    addParam("calpulseHeightMin", m_calpulseHeightMin,
             "calpulse selection: minimal height [ns]", 0);
    addParam("calpulseHeightMax", m_calpulseHeightMax,
             "calpulse selection: maximal height [ns]", 0);
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

    // either common T0 or TDC offset
    if (m_useCommonT0Calibration and m_subtractOffset)
      B2ERROR("One can use either common T0 from DB or offset of nominal TDC");

    // alocate DB objects (steering values passed from py after constructor is called!)
    if (m_useSampleTimeCalibration) m_timebase = new DBObjPtr<TOPCalTimebase>;
    if (m_useChannelT0Calibration) m_channelT0 = new DBObjPtr<TOPCalChannelT0>;
    if (m_useModuleT0Calibration) m_moduleT0 = new DBObjPtr<TOPCalModuleT0>;
    if (m_useCommonT0Calibration) m_commonT0 = new DBObjPtr<TOPCalCommonT0>;

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
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    StoreArray<TOPRawDigit> rawDigits(m_inputRawDigitsName);
    StoreArray<TOPDigit> digits(m_outputDigitsName);
    digits.clear();

    for (const auto& rawDigit : rawDigits) {
      if (rawDigit.getErrorFlags() != 0) continue;
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
      double rawTime = rawDigit.getCFDLeadingTime(); // time in [samples]
      int tdc = int(rawTime * m_sampleDivisions);
      const auto* sampleTimes = &m_sampleTimes; // equidistant sample times
      if (m_useSampleTimeCalibration) {
        sampleTimes = (*m_timebase)->getSampleTimes(scrodID, channel % 128);
        if (!sampleTimes->isCalibrated()) {
          B2WARNING("No sample time calibration available for SCROD " << scrodID
                    << " channel " << channel % 128 << " - raw digit not converted");
          continue;
        }
      }
      auto window = rawDigit.getASICWindow();
      double time = sampleTimes->getTimeDifference(window, rawTime); // time in [ns]
      if (m_useChannelT0Calibration) time -= (*m_channelT0)->getT0(moduleID, channel);
      if (m_useModuleT0Calibration) time -= (*m_moduleT0)->getT0(moduleID);
      if (m_useCommonT0Calibration) time -= (*m_commonT0)->getT0();
      if (m_subtractOffset) time -= geo->getNominalTDC().getOffset();
      double width = sampleTimes->getDeltaTime(window, rawDigit.getCFDFallingTime(),
                                               rawDigit.getCFDLeadingTime()); // in [ns]
      auto* digit = digits.appendNew(moduleID, pixelID, tdc);
      digit->setTime(time);
      // digit->setTimeError(timeError); TODO!
      digit->setADC(rawDigit.getValuePeak());
      digit->setIntegral(rawDigit.getIntegral());
      digit->setPulseWidth(width);
      digit->setChannel(channel);
      digit->setFirstWindow(window);
      digit->addRelationTo(&rawDigit);
      if (!rawDigit.isFEValid() or rawDigit.isPedestalJump())
        digit->setHitQuality(TOPDigit::c_Junk);
      const auto* waveform = rawDigit.getRelated<TOPRawWaveform>();
      if (waveform) {
        if (!waveform->areWindowsInOrder()) digit->setHitQuality(TOPDigit::c_Junk);
      }
    }

    // select and flag calpulses, if calibration channel given
    unsigned calibrationChannel = m_calibrationChannel;
    if (calibrationChannel < 8) {
      for (auto& digit : digits) {
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;
        if (digit.getASICChannel() != calibrationChannel) continue;
        if (digit.getADC() < m_calpulseHeightMin) continue;
        if (digit.getADC() > m_calpulseHeightMax) continue;
        if (digit.getPulseWidth() < m_calpulseWidthMin) continue;
        if (digit.getPulseWidth() > m_calpulseWidthMax) continue;
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

