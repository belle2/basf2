/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPDoublePulseGenerator/TOPDoublePulseGeneratorModule.h>
#include <top/geometry/TOPGeometryPar.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// datastore classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>

// root
#include <TRandom.h>


using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPDoublePulseGenerator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPDoublePulseGeneratorModule::TOPDoublePulseGeneratorModule() : Module()

  {
    // set module description
    setDescription("Generator of calibration double pulses");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("moduleIDs", m_moduleIDs,
             "list of slots for which to generate cal pulse, empty list means all slots.",
             m_moduleIDs);
    m_asicChannels.push_back(0);
    addParam("asicChannels", m_asicChannels,
             "ASIC calibration channels (0 - 7), empty list means all channels.",
             m_asicChannels);
    addParam("timeDifference", m_timeDifference,
             "time difference between first and second pulse [ns].", 21.87);
    addParam("timeResolution", m_timeResolution,
             "sigma of time difference [ns].", 40.0e-3);
    addParam("sampleTimeIntervals", m_sampleTimeIntervals,
             "vector of 256 sample time intervals to construct sample times. "
             "If empty, equidistant intervals will be used.", m_sampleTimeIntervals);
    addParam("useDatabase", m_useDatabase,
             "if true, use sample times from database instead of sampleTimeIntervals.",
             false);
    addParam("storageWindows", m_storageWindows,
             "number of storage windows (old FW used 64 out of 512)", (unsigned) 512);

  }

  TOPDoublePulseGeneratorModule::~TOPDoublePulseGeneratorModule()
  {
    if (m_timebase) delete m_timebase;
  }

  void TOPDoublePulseGeneratorModule::initialize()
  {
    // Output

    StoreArray<TOPDigit> digits;
    digits.registerInDataStore();

    // prepare vectors to loop on

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    if (m_moduleIDs.empty()) {
      for (const auto& module : geo->getModules()) {
        m_moduleIDs.push_back(module.getModuleID());
      }
    } else {
      for (auto moduleID : m_moduleIDs) {
        if (!geo->isModuleIDValid(moduleID))
          B2ERROR("Invalid module ID found in input list: " << moduleID);
      }
    }

    if (m_asicChannels.empty()) {
      for (unsigned ch = 0; ch < 8; ch++) m_asicChannels.push_back(ch);
    } else {
      for (unsigned ch : m_asicChannels) {
        if (ch > 7)
          B2ERROR("Invalid ASIC channel found in input list: " << ch);
      }
    }

    // set sample times

    double syncTimeBase = geo->getNominalTDC().getSyncTimeBase();
    m_sampleTimes.setTimeAxis(syncTimeBase); // equidistant
    m_sampleDivisions = (0x1 << geo->getNominalTDC().getSubBits());

    if (m_useDatabase) {
      m_timebase = new DBObjPtr<TOPCalTimebase>;
    } else if (m_sampleTimeIntervals.empty()) {
      B2INFO("TOPDoublePulseGenerator: using equidistant sample times");
    } else if (m_sampleTimeIntervals.size() == 256) {
      std::vector<double> timeAxis;
      timeAxis.push_back(0);
      for (auto dt : m_sampleTimeIntervals) timeAxis.push_back(dt + timeAxis.back());
      double rescale = 2 * syncTimeBase / timeAxis.back();
      for (auto& t : timeAxis) t *= rescale;
      m_sampleTimes.setTimeAxis(timeAxis, syncTimeBase); // given by steering
      B2INFO("TOPDoublePulseGenerator: using sample times from steering");
    } else {
      B2ERROR("sampleTimeIntervals: size must be 256 or empty");
    }

  }


  void TOPDoublePulseGeneratorModule::beginRun()
  {
    StoreObjPtr<EventMetaData> evtMetaData;
    if (m_useDatabase) {
      if (!(*m_timebase).isValid()) {
        B2FATAL("Sample time calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }

  }


  void TOPDoublePulseGeneratorModule::event()
  {

    StoreArray<TOPDigit> digits;

    const auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();
    const auto& feMapper = TOPGeometryPar::Instance()->getFrontEndMapper();

    for (auto moduleID : m_moduleIDs) {
      for (unsigned asic = 0; asic < 64; asic++) {
        for (auto asicChannel : m_asicChannels) {
          unsigned channel = asic * 8 + asicChannel;
          auto pixelID = chMapper.getPixelID(channel);

          // sample times: from steering or database
          const TOPSampleTimes* sampleTimes = &m_sampleTimes;
          if (m_useDatabase) {
            unsigned scrodID = 0;
            int bs = asic / 16;
            const auto* feMap = feMapper.getMap(moduleID, bs);
            if (feMap) scrodID = feMap->getScrodID();
            sampleTimes = (*m_timebase)->getSampleTimes(scrodID, channel % 128);
            if (!sampleTimes->isCalibrated()) {
              B2WARNING("No sample time calibration available for SCROD " << scrodID
                        << " channel " << channel % 128 << " - equidistant will be used");
            }
          }

          // first calpulse
          double time = gRandom->Rndm() * sampleTimes->getTimeRange();
          unsigned window = gRandom->Rndm() * m_storageWindows;
          double sample = sampleTimes->getSample(window, time);
          int tdc = int(sample * m_sampleDivisions);
          auto* digit = digits.appendNew(moduleID, pixelID, tdc);
          digit->setFirstWindow(window);
          digit->setTime(time);
          digit->setChannel(channel);
          digit->setHitQuality(TOPDigit::c_CalPulse);

          // second calpulse
          time += gRandom->Gaus(m_timeDifference, m_timeResolution);
          sample = sampleTimes->getSample(window, time);
          tdc = int(sample * m_sampleDivisions);
          digit = digits.appendNew(moduleID, pixelID, tdc);
          digit->setFirstWindow(window);
          digit->setTime(time);
          digit->setChannel(channel);
          digit->setHitQuality(TOPDigit::c_CalPulse);
        }
      }
    }

  }


  void TOPDoublePulseGeneratorModule::endRun()
  {
  }

  void TOPDoublePulseGeneratorModule::terminate()
  {
  }


} // end Belle2 namespace

