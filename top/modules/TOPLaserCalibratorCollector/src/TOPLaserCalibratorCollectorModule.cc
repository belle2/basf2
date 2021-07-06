/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/TOPLaserCalibratorCollector/TOPLaserCalibratorCollectorModule.h>

//ROOT
#include <TTree.h>
#include <TMath.h>

//TOP
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPSimHit.h>

#include <algorithm>
#include <vector>

using namespace Belle2;

REG_MODULE(TOPLaserCalibratorCollector)

TOPLaserCalibratorCollectorModule::TOPLaserCalibratorCollectorModule()
{
  // Set module properties
  setDescription("Collector module for the TOP  ChannelT0 calibration and the quality monitoring using laser and pulser data");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("useReferencePulse", m_useReferencePulse, "Use the pulser as reference", bool(false));
  addParam("refChannel", m_refChannel, "Channel to be used as reference");
  addParam("refSlot", m_refSlot, "Slot to be used as reference");
  addParam("pulserDeltaT", m_pulserDeltaT, "Approximate time difference between the two calpulses, in ns", float(21.8));
  addParam("pulserDeltaTTolerance", m_pulserDeltaTTolerance, "Window around the nominal deltaT used to select a double pulse, in ns ",
           float(2.));
  addParam("storeMCTruth", m_storeMCTruth, " Store the TOPSimHits information instead of the  TOPDigit one. \
                                             If this option is used, useReferencePulse will be automatically set to false.");

}



void TOPLaserCalibratorCollectorModule::prepare()
{

  // Create the output trees

  auto hitTree = new TTree("hitTree", "hitTree");
  hitTree->Branch<short>("channel", &m_channel);
  hitTree->Branch<short>("slot", &m_slot);
  hitTree->Branch<float>("hitTime", &m_hitTime);
  hitTree->Branch<float>("dVdt", &m_dVdt);
  hitTree->Branch<float>("refTime", &m_refTime);
  hitTree->Branch<float>("amplitude", &m_amplitude);
  hitTree->Branch<float>("width", &m_width);
  hitTree->Branch<short>("sample", &m_sample);
  hitTree->Branch<short>("window", &m_window);
  hitTree->Branch<int>("event", &m_event);
  hitTree->Branch<bool>("refTimeValid", &m_refTimeValid);

  registerObject<TTree>("hitTree", hitTree);

  m_TOPDigitArray.isRequired();

  if (m_storeMCTruth)
    m_useReferencePulse = false;

  m_event = 0;
}


/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void TOPLaserCalibratorCollectorModule::collect()
{
  float refTimes[16] = {0.}; // Reference time for each slot
  std::vector<bool> refTimesValid(16, true);

  // first loop over TOPDigits to find all the pairs of digits that satisfy the double-pulse conditions
  if (m_useReferencePulse) {

    std::vector<float> calPulseTimes[16];
    for (const auto& digit : m_TOPDigitArray) {
      if (digit.getHitQuality() != TOPDigit::c_CalPulse or digit.getChannel() != m_refChannel)
        continue; // remove photons and everything not on the ref channel
      calPulseTimes[digit.getModuleID() - 1].push_back(digit.getTime());
    }

    for (int i = 0; i < 16; i++) {
      refTimesValid[i] = false;
      auto& calTimes = calPulseTimes[i];
      if (calTimes.size() < 2) continue;
      std::sort(calTimes.begin(), calTimes.end());
      for (unsigned k = 0; k < calTimes.size() - 1; k++) {
        auto t1 = calTimes[k];
        auto t2 = calTimes[k + 1];
        if (fabs(fabs(t2 - t1) - m_pulserDeltaT) < m_pulserDeltaTTolerance) {
          refTimes[i] = t1;
          refTimesValid[i] = true;
          break;
        }
      }
    }

  }

  TTree*  hitTree = getObjectPtr<TTree>("hitTree");

  // then fill the tree with good digits (photons and cal pulses)
  for (const auto& digit : m_TOPDigitArray) {
    if (digit.getHitQuality() == TOPDigit::c_Junk) continue; // remove the bad hits
    m_channel = digit.getChannel();
    m_slot = digit.getModuleID(); // this is 1-based
    m_dVdt = 0.5 * TMath::Sqrt(-2.*TMath::Log(0.5)) * digit.getPulseHeight() / digit.getPulseWidth();

    if (m_refSlot > 0) {
      m_refTime = refTimes[m_refSlot - 1];
      m_refTimeValid = refTimesValid[m_refSlot - 1];
    } else {
      m_refTime = refTimes[m_slot - 1];
      m_refTimeValid = refTimesValid[m_slot - 1];
    }

    m_amplitude = digit.getPulseHeight() ;
    m_width =  digit.getPulseWidth();

    if (m_storeMCTruth) {
      const auto* simHit = digit.getRelated<TOPSimHit>();
      if (not simHit) continue; // no tree entry if MC truth doesn't exist
      m_hitTime = simHit->getTime();
    } else {
      m_hitTime = digit.getTime() - m_refTime;
    }

    m_window = -1;
    const auto* rawDigit = digit.getRelated<TOPRawDigit>();
    if (rawDigit) m_window = rawDigit->getASICWindow(); // window from which the feature is extracted
    m_sample = digit.getModulo256Sample(); // sample number refered in TBC
    hitTree->Fill();
  }
  m_event++;
}
