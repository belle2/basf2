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
#include <top/modules/TOPDoublePulseGenerator/TOPCalPulseGeneratorModule.h>
#include <top/geometry/TOPGeometryPar.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/logging/Logger.h>

// root
#include <TRandom.h>


using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCalPulseGenerator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCalPulseGeneratorModule::TOPCalPulseGeneratorModule() : Module()

  {
    // set module description
    setDescription("Realistic generator of calibration double pulses. "
                   "Needs TOPDigitizer to digitize");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("moduleIDs", m_moduleIDs,
             "list of slots for which to generate cal pulse, empty list means all slots.",
             m_moduleIDs);
    m_asicChannels.push_back(0);
    addParam("asicChannels", m_asicChannels,
             "ASIC calibration channels (0 - 7), empty list means all channels.",
             m_asicChannels);
    // default for these three below are set according to laser run 8/414
    addParam("amplitude", m_amplitude, "amplitude of cal pulse [ADC counts]", 600.0);
    addParam("delay", m_delay, "delay of cal pulse [ns]", 10.5);
    addParam("windowSize", m_windowSize, "size of time window in which to generate cal pulses [ns]", 8.0);
  }


  void TOPCalPulseGeneratorModule::initialize()
  {
    // Output

    m_calPulses.registerInDataStore();

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

  }


  void TOPCalPulseGeneratorModule::event()
  {
    const auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();

    double time = m_delay + gRandom->Uniform(m_windowSize);
    for (auto moduleID : m_moduleIDs) {
      for (unsigned asic = 0; asic < 64; asic++) {
        for (auto asicChannel : m_asicChannels) {
          unsigned channel = asic * 8 + asicChannel;
          auto pixelID = chMapper.getPixelID(channel);
          m_calPulses.appendNew(moduleID, channel, pixelID, time, m_amplitude);
        }
      }
    }

  }

} // end Belle2 namespace

