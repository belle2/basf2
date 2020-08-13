/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Sasha Glazov                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AsicBackgroundDetector.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <cdc/dataobjects/CDCHit.h>
#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;
using namespace TrackFindingCDC;
using std::vector;
using std::map;
using std::pair;
using std::sort;

void AsicBackgroundDetector::initialize()
{
  Super::initialize();
  // database:
  m_channelMapFromDB = std::make_unique<DBArray<CDCChannelMap>> ();

  if ((*m_channelMapFromDB).isValid()) {
    B2DEBUG(100, "CDC Channel map is  valid");
  } else {
    B2FATAL("CDC Channel map is not valid");
  }
}

void AsicBackgroundDetector::beginRun()
{
  Super::beginRun();

  // Load map from DB:
  for (const auto& cm : (*m_channelMapFromDB)) {
    const int isl = cm.getISuperLayer();
    const int il = cm.getILayer();
    const int iw = cm.getIWire();
    const int iBoard = cm.getBoardID();
    const int iCh = cm.getBoardChannel();
    const WireID wireId(isl, il, iw);
    m_map[wireId.getEWire()] = std::pair<int, int>(iBoard, iCh);
  }
}

std::string AsicBackgroundDetector::getDescription()
{
  return "Marks hits as background using ASIC-based filter.";
}

void AsicBackgroundDetector::apply(std::vector<CDCWireHit>& wireHits)
{

  map< pair<int, int>, vector<CDCWireHit*>> groupedByAsic;
  for (CDCWireHit& wireHit : wireHits) {
    auto eWire = wireHit.getWireID().getEWire();
    B2ASSERT("Channel map NOT found for the channel", m_map.count(eWire) > 0);
    auto board = m_map[eWire].first;
    auto channel = m_map[eWire].second;
    auto asicID = pair<int, int>(board, channel / 8);  // ASIC are groups of 8 channels
    groupedByAsic[asicID].push_back(&wireHit);
  };
  for (auto& [asicID, asicList] :  groupedByAsic) {
    applyAsicFilter(asicList);
  };

  return;
}

void AsicBackgroundDetector::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimalHitNumberASIC"),
                                m_minimal_hit_number,
                                "Required number of hits per ASIC for background check",
                                m_minimal_hit_number);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "deviationFromMedianTDCinASIC"),
                                m_deviation_from_median,
                                "Flag hits as cross talk if TDC does not deviate from median more than this value",
                                m_deviation_from_median);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimalNSignalASIC"),
                                m_nsignal_max,
                                "Flag background hits only when ASIC contains no more than this number of signal hits",
                                m_nsignal_max);
}

void AsicBackgroundDetector::applyAsicFilter(std::vector<CDCWireHit*>& wireHits)
{

  if (wireHits.size() < m_minimal_hit_number) {
    return;
  };

  if (wireHits.size() > 8) {
    B2ERROR("Number of hits per asic should not exceed 8, observe too many hits." << LogVar("nHits", wireHits.size()));
    /// This is abnormal situation, detected for few runs, related to CDC unpacker. Hits are to be marked as background.
    for (auto& hit : wireHits) {
      (*hit)->setBackgroundFlag();
      (*hit)->setTakenFlag();
    }
    return;
  }

  B2ASSERT("Number of hits per asic should be above 0 and can not exceed 8",
           (wireHits.size() <= 8) && (wireHits.size() > 0));

  // compute median time:
  vector<short> times;
  for (auto& hit : wireHits) {
    short tdc = hit->getHit()->getTDCCount();
    times.push_back(tdc);
  }
  sort(times.begin(), times.end());
  int mid = times.size() / 2;
  double median = times.size() % 2 == 0 ? (times[mid] + times[mid - 1]) / 2 : times[mid];


  size_t nbg = 0;
  int adcOnMedian = 0;
  int adcOffMedian = 0;
  for (auto& hit : wireHits) {
    int adc = hit->getHit()->getADCCount();
    if (abs(hit->getHit()->getTDCCount() - median) < m_deviation_from_median) {
      nbg++;
      if (adc > adcOnMedian) adcOnMedian = adc;
    } else {
      if (adc > adcOffMedian) adcOffMedian = adc;
    }
  }

  if ((nbg < times.size())     // at least 1 hit with different TDC ("signal")
      && (nbg > 1) // more than one candidate hits
      && (nbg > times.size() - m_nsignal_max) // a few background hits
      && (adcOnMedian < adcOffMedian) // triggered by large ADC "signal"
     ) {

    // mark hits too close to the median time as background:
    for (auto& hit : wireHits) {
      if (abs(hit->getHit()->getTDCCount() - median) < m_deviation_from_median) {
        (*hit)->setBackgroundFlag();
        (*hit)->setTakenFlag();
      }
    }
  }
}
