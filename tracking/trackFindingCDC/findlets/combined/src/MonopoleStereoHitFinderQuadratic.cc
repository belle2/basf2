/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/MonopoleStereoHitFinderQuadratic.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::ChooseableFilter<StereoHitFilterFactory>;

MonopoleStereoHitFinderQuadratic::MonopoleStereoHitFinderQuadratic() : Super()
{
  addProcessingSignalListener(&m_rlWireHitCreator);
  addProcessingSignalListener(&m_matcher);
  addProcessingSignalListener(&m_filterSelector);
  addProcessingSignalListener(&m_singleMatchSelector);
  addProcessingSignalListener(&m_adder);
  addProcessingSignalListener(&m_inspector);
//   addProcessingSignalListener(&m_szFitter);
}

std::string MonopoleStereoHitFinderQuadratic::getDescription()
{
  return "Tries to add monopole CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.\n"
         "WARNING This findlet is kept here just in case hyperbolic one misbehaves and eats up too much RAM\n"
         "If it doesn't, this one should be removed around release-05 or earlier";
}

void MonopoleStereoHitFinderQuadratic::beginEvent()
{
  Super::beginEvent();

  m_rlTaggedWireHits.clear();
  m_relations.clear();
}

void MonopoleStereoHitFinderQuadratic::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_rlWireHitCreator.exposeParameters(moduleParamList, prefix);
  m_matcher.exposeParameters(moduleParamList, prefix);
  m_filterSelector.exposeParameters(moduleParamList, prefix);
  m_singleMatchSelector.exposeParameters(moduleParamList, prefix);
  m_adder.exposeParameters(moduleParamList, prefix);
  m_inspector.exposeParameters(moduleParamList, prefix);
//   m_szFitter.exposeParameters(moduleParamList, prefix);
}

void MonopoleStereoHitFinderQuadratic::apply(std::vector<CDCWireHit>& inputWireHits, std::vector<CDCTrack>& tracks)
{
  m_rlTaggedWireHits.reserve(2 * inputWireHits.size());
  m_relations.reserve(2 * inputWireHits.size() * tracks.size());

  m_rlWireHitCreator.apply(inputWireHits, m_rlTaggedWireHits);
  m_matcher.apply(tracks, m_rlTaggedWireHits, m_relations);
//   m_filterSelector.apply(m_relations);
  m_singleMatchSelector.apply(m_relations);
  m_adder.apply(m_relations);
  m_inspector.apply(tracks);

  for (auto track : tracks)
    track.sortByArcLength2D();
//   m_szFitter.apply(tracks);
}
