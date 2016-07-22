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
#include <topcaf/modules/TOPCAFtoTOPDigit/TOPCAFtoTOPDigitModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// Dataobject classes
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include <top/dataobjects/TOPDigit.h>
#include <framework/dataobjects/EventMetaData.h>

#include <top/geometry/TOPGeometryPar.h>


using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCAFtoTOPDigit)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCAFtoTOPDigitModule::TOPCAFtoTOPDigitModule() : Module()

  {
    // set module description
    setDescription("Converts TOPCAFDigits to TOPDigits and applies time calibration");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("useSampleTimeCalibration", m_useSampleTimeCalibration,
             "if true, use sample time calibration (needs DB)", true);
    addParam("useChannelT0Calibration", m_useChannelT0Calibration,
             "if true, use channel T0 calibration (needs DB)", false);
    addParam("selectGood", m_selectGood,
             "if true, select only those with two cal pulses found, otherwise"
             " those with at least one cal pulse found", true);
  }


  TOPCAFtoTOPDigitModule::~TOPCAFtoTOPDigitModule()
  {
    if (m_timebase) delete m_timebase;
    if (m_channelT0) delete m_channelT0;
  }


  void TOPCAFtoTOPDigitModule::initialize()
  {
    StoreArray<TOPCAFDigit> cafDigits;
    cafDigits.isRequired();

    StoreArray<TOPDigit> digits;
    digits.registerInDataStore();
    digits.registerRelationTo(cafDigits);

    // equidistant sample times in case calibration is not required
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_sampleTimes.setTimeAxis(geo->getNominalTDC().getSyncTimeBase());
    m_sampleDivisions = (0x1 << geo->getNominalTDC().getSubBits());

    if (m_useSampleTimeCalibration) m_timebase = new DBObjPtr<TOPCalTimebase>;
    if (m_useChannelT0Calibration) m_channelT0 = new DBObjPtr<TOPCalChannelT0>;

  }


  void TOPCAFtoTOPDigitModule::beginRun()
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

  }


  void TOPCAFtoTOPDigitModule::event()
  {

    // get mappers
    const auto& feMapper = TOPGeometryPar::Instance()->getFrontEndMapper();
    const auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();

    StoreArray<TOPCAFDigit> cafDigits;
    StoreArray<TOPDigit> digits;
    digits.clear();

    for (const auto& cafDigit : cafDigits) {

      // apply selection criteria
      if (!selectDigit(cafDigit)) continue;

      // collect information about the channel
      auto scrodID = cafDigit.GetScrodID();
      const auto* feemap = feMapper.getMap(scrodID);
      if (!feemap) {
        B2ERROR("No front-end map available for SCROD " << scrodID);
        continue;
      }
      auto moduleID = feemap->getModuleID();
      auto boardstack = feemap->getBoardstackNumber();
      auto channel = chMapper.getChannel(boardstack,
                                         cafDigit.GetCarrier(),
                                         cafDigit.GetASIC(),
                                         cafDigit.GetASICChannel());
      auto pixelID = chMapper.getPixelID(channel);

      // check pixel ID
      if (pixelID != cafDigit.GetPixel()) {
        B2WARNING("pixel ID mismatch: " << pixelID << " " << cafDigit.GetPixel());
      }

      // get sample time calibration constants for that channel
      const auto* sampleTimes = &m_sampleTimes; // equidistant sample times
      if (m_useSampleTimeCalibration) {
        sampleTimes = (*m_timebase)->getSampleTimes(scrodID, channel % 128);
        if (!sampleTimes->isCalibrated()) {
          B2ERROR("No sample time calibration available for SCROD " << scrodID
                  << " channel " << channel % 128 << " - TOPCAFDigit not converted");
          continue;
        }
      }

      // raw times: assumed processed by WaveTimingFast and DoubleCalPulse modules!
      double rawTime = cafDigit.GetTDCBin(); // hit time in TDC bins (samples)
      double asicRefTime = cafDigit.GetQuality(); // cal pulse time in TDC bins (samples)
      int tdc = int(rawTime * m_sampleDivisions);

      // apply sample time and T0 calibration
      auto window = cafDigit.GetASICWindow();
      double time = sampleTimes->getDeltaTime(window, rawTime, asicRefTime); // in [ns]
      if (m_useChannelT0Calibration) time -= (*m_channelT0)->getT0(moduleID, channel);

      // convert signal width to ns
      double width = sampleTimes->getDeltaTime(window, rawTime + cafDigit.GetWidth(),
                                               rawTime); // in [ns]

      // append and set TOPDigit, add relation to TOPCAFDigit
      auto* digit = digits.appendNew(moduleID, pixelID, tdc);
      digit->setTime(time);
      digit->setADC(cafDigit.GetADCHeight());
      digit->setIntegral(cafDigit.GetCharge());
      digit->setPulseWidth(width);
      digit->setChannel(channel);
      digit->setFirstWindow(window);
      if (cafDigit.GetFlag() % 1000 != 0) digit->setHitQuality(TOPDigit::c_CalPulse);
      digit->addRelationTo(&cafDigit);

    }

  }


  void TOPCAFtoTOPDigitModule::endRun()
  {
  }


  void TOPCAFtoTOPDigitModule::terminate()
  {
  }


  bool TOPCAFtoTOPDigitModule::selectDigit(const TOPCAFDigit& cafDigit)
  {
    int flag = cafDigit.GetFlag();
    if (flag <= 0) return false; // no cal pulse found
    if (!m_selectGood) return true; // loose selection required (at least one cal pulse)
    return (flag / 1000 == 2); // strict selection required (two cal pulses found)
  }


} // end Belle2 namespace

