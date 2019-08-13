/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/TOPLaserCalibratorCollector/TOPLaserCalibratorCollectorModule.h>

//ROOT
#include <TTree.h>
#include <TMath.h>

//Framework
#include <framework/gearbox/Const.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/DataStore.h>

//TOP
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPSimHit.h>

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
  //  hitTree->Branch<short>("sample", &m_sample);
  //  hitTree->Branch<short>("window", &m_window);
  hitTree->Branch<int>("event", &m_event);

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

  // first loops over the TOPDigit to find all the pairs of digit that satisfy the pulser contitions
  if (m_useReferencePulse) {
    for (auto digit1 : m_TOPDigitArray) {

      if (digit1.getHitQuality() != 4 ||  m_refChannel != digit1.getChannel())
        continue; // remove photons and everything not on the ref channel

      for (auto digit2 : m_TOPDigitArray) {
        if (digit1.getModuleID() != digit2.getModuleID())
          continue; // not in the same module
        if (digit2.getHitQuality() != 4 ||  m_refChannel != digit2.getChannel())
          continue; // remove photons and everything not on the ref channel

        // If I reach this point, I have to pulser-like hits on the ref channel of the same slot
        if (TMath::Abs(TMath::Abs(digit1.getTime() - digit2.getTime()) - m_pulserDeltaT) < m_pulserDeltaTTolerance) {
          if (digit1.getTime() < digit2.getTime())
            refTimes[digit1.getModuleID() - 1] = digit1.getTime();
          else
            refTimes[digit1.getModuleID() - 1] = digit2.getTime();
        }

      }
    }
  }

  TTree*  hitTree = getObjectPtr<TTree>("hitTree");

  // first loops over the TOPDigit to find all the pairs of digit that satisfy the pulser contitions
  for (auto& digit : m_TOPDigitArray) {
    if (digit.getHitQuality() == 0)
      continue; // remove the bad hits
    m_channel = digit.getChannel();
    m_slot = digit.getModuleID(); // this is 1-based
    m_dVdt = 0.5 * TMath::Sqrt(-2.*TMath::Log(0.5)) * digit.getPulseHeight() / digit.getPulseWidth();

    if (m_refSlot > 0)
      m_refTime = refTimes[m_refSlot - 1];
    else
      m_refTime = refTimes[m_slot - 1];

    m_amplitude = digit.getPulseHeight() ;
    m_width =  digit.getPulseWidth();

    if (m_storeMCTruth) {
      const auto* simHit = digit.getRelated<TOPSimHit>();
      if (simHit)
        m_hitTime = simHit->getTime();
    } else
      m_hitTime = digit.getTime() - m_refTime;

    m_window = 0; // FIXME
    m_sample = 0; // FIXME
    hitTree->Fill();
  }
  m_event++;
}
