/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/StereoHitFinder.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::ChooseableFilter<StereoHitFilterFactory>;

StereoHitFinder::StereoHitFinder() : Super()
{
  addProcessingSignalListener(&m_rlWireHitCreator);
  addProcessingSignalListener(&m_matcher);
  addProcessingSignalListener(&m_filterSelector);
  addProcessingSignalListener(&m_singleMatchSelector);
  addProcessingSignalListener(&m_adder);
  addProcessingSignalListener(&m_szFitter);
}

std::string StereoHitFinder::getDescription()
{
  return "Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.";
}

void StereoHitFinder::beginEvent()
{
  Super::beginEvent();

  m_rlTaggedWireHits.clear();
  m_relations.clear();
}

void StereoHitFinder::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_rlWireHitCreator.exposeParameters(moduleParamList, prefix);
  m_matcher.exposeParameters(moduleParamList, prefix);
  m_filterSelector.exposeParameters(moduleParamList, prefix);
  m_singleMatchSelector.exposeParameters(moduleParamList, prefix);
  m_adder.exposeParameters(moduleParamList, prefix);
  m_szFitter.exposeParameters(moduleParamList, prefix);
}

void StereoHitFinder::apply(std::vector<CDCWireHit>& inputWireHits, std::vector<CDCTrack>& tracks)
{
  m_rlTaggedWireHits.reserve(2 * inputWireHits.size());
  m_relations.reserve(2 * inputWireHits.size() * tracks.size());

  m_rlWireHitCreator.apply(inputWireHits, m_rlTaggedWireHits);
  m_matcher.apply(tracks, m_rlTaggedWireHits, m_relations);
  m_filterSelector.apply(m_relations);
  m_singleMatchSelector.apply(m_relations);
  m_adder.apply(m_relations);

  m_szFitter.apply(tracks);
}
