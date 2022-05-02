/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/TOPChannelMasker/TOPChannelMaskerModule.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/geometry/TOPGeometryPar.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

//-----------------------------------------------------------------
///                 Register the Module
//-----------------------------------------------------------------
  REG_MODULE(TOPChannelMasker);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  TOPChannelMaskerModule::TOPChannelMaskerModule() : Module()
  {
    // Set module properties
    setDescription("Masks dead, hot and uncalibrated channels from the reconstruction");

    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("maskUncalibratedChannelT0", m_maskUncalibratedChannelT0,
             "if true, mask channelT0-uncalibrated channels", true);
    addParam("maskUncalibratedTimebase", m_maskUncalibratedTimebase,
             "if true, mask timebase-uncalibrated channels ", true);
  }

  void TOPChannelMaskerModule::initialize()
  {
    // register data objects
    m_digits.isRequired();
    m_eventAsicMask.isOptional();
  }

  void TOPChannelMaskerModule::beginRun()
  {

    if (not m_channelMask.isValid()) {
      B2FATAL("channel mask not available");
    }
    if (not m_channelT0.isValid()) {
      B2FATAL("channel T0 calibration not available");
    }
    if (not m_timebase.isValid()) {
      B2FATAL("timebase calibration not available");
    }

  }

  void TOPChannelMaskerModule::event()
  {

    // have those payloads changed?

    bool pmtInstalled = m_pmtInstalled.hasChanged();
    bool pmtQEData = m_pmtQEData.hasChanged();
    bool channelRQE = m_channelRQE.hasChanged();
    bool thresholdEff = m_thresholdEff.hasChanged();

    // if at least one then pass pixel relative efficiencies to the reconstructon code

    if (pmtInstalled or pmtQEData or channelRQE or thresholdEff) {
      TOPRecoManager::setChannelEffi();
    }

    // have asic masks changed?

    bool asicMasksChanged = false;
    if (m_eventAsicMask.isValid()) {
      if (m_eventAsicMask->get() != m_savedAsicMask.get()) {
        m_savedAsicMask.set(m_eventAsicMask->get());
        asicMasksChanged = true;
      }
    }

    // have channel masks or calibration changed?

    bool channelMaskChanged = m_channelMask.hasChanged();
    bool channelT0Changed = m_channelT0.hasChanged();
    bool timebaseChanged = m_timebase.hasChanged();

    // if at least one then pass the new masking to the reconstruction code

    if (channelMaskChanged or asicMasksChanged or
        (m_maskUncalibratedChannelT0 and channelT0Changed) or
        (m_maskUncalibratedTimebase and timebaseChanged)) {

      TOPRecoManager::setChannelMask(m_channelMask, m_savedAsicMask);
      if (m_maskUncalibratedChannelT0) {
        TOPRecoManager::setUncalibratedChannelsOff(m_channelT0);
      }
      if (m_maskUncalibratedTimebase) {
        TOPRecoManager::setUncalibratedChannelsOff(m_timebase);
      }
    }

    // now flag actual data Cherenkov hits as coming from masked channels

    for (auto& digit : m_digits) {
      // if already set switch the state back to c_Good (e.g. for digits read from file)
      if (digit.getHitQuality() == TOPDigit::c_Masked or
          digit.getHitQuality() == TOPDigit::c_Uncalibrated) {
        digit.setHitQuality(TOPDigit::c_Good);
      }
      if (digit.getHitQuality() != TOPDigit::c_Good) continue;

      // now do the new masking of c_Good
      auto slotID = digit.getModuleID();
      auto channel = digit.getChannel();
      if (not m_channelMask->isActive(slotID, channel)) {
        digit.setHitQuality(TOPDigit::c_Masked);
        continue;
      }
      if (not m_savedAsicMask.isActive(slotID, channel)) {
        digit.setHitQuality(TOPDigit::c_Masked);
        const unsigned maxCount = 10; // at HLT this means (10 * number-of-processes) messages before being suppressed
        if (m_errorCount < maxCount) {
          B2ERROR("Unexpected hit found in a channel that is masked-out by firmware"
                  << LogVar("slotID", slotID) << LogVar("channel", channel));
        } else if (m_errorCount == maxCount) {
          B2ERROR("Unexpected hit found in a channel that is masked-out by firmware"
                  << LogVar("slotID", slotID) << LogVar("channel", channel)
                  << LogVar("... message will be suppressed now, errorCount", m_errorCount));
        }
        m_errorCount++;
        continue;
      }
      if (m_maskUncalibratedChannelT0 and not m_channelT0->isCalibrated(slotID, channel)) {
        digit.setHitQuality(TOPDigit::c_Uncalibrated);
        continue;
      }
      if (m_maskUncalibratedTimebase) {
        const auto& fe_mapper = TOPGeometryPar::Instance()->getFrontEndMapper();
        const auto* fe = fe_mapper.getMap(slotID, channel / 128);
        if (not fe) {
          B2ERROR("No front-end map found" << LogVar("slotID", slotID) << LogVar("channel", channel));
          digit.setHitQuality(TOPDigit::c_Uncalibrated);
          continue;
        }
        auto scrodID = fe->getScrodID();
        const auto* sampleTimes = m_timebase->getSampleTimes(scrodID, channel);
        if (not sampleTimes->isCalibrated()) {
          digit.setHitQuality(TOPDigit::c_Uncalibrated);
        }
      }
    }

  }

} // end Belle2 namespace

