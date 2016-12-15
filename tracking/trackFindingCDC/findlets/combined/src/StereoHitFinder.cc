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

using namespace Belle2;
using namespace TrackFindingCDC;

StereoHitFinder::StereoHitFinder() : Super()
{
  addProcessingSignalListener(&m_rlWireHitCreator);
  addProcessingSignalListener(&m_matcher);
  addProcessingSignalListener(&m_selector);
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
  m_selector.exposeParameters(moduleParamList, prefix);
  m_adder.exposeParameters(moduleParamList, prefix);
  m_szFitter.exposeParameters(moduleParamList, prefix);
}

void StereoHitFinder::apply(std::vector<CDCWireHit>& inputWireHits, std::vector<CDCTrack>& tracks)
{
  B2WARNING("Number of tracks: " << tracks.size());
  B2WARNING("Number of wire hits: " << inputWireHits.size());

  m_rlTaggedWireHits.reserve(2 * inputWireHits.size());
  m_relations.reserve(2 * inputWireHits.size() * tracks.size());

  m_rlWireHitCreator.apply(inputWireHits, m_rlTaggedWireHits);

  B2WARNING("Number of rlWireHits: " << m_rlTaggedWireHits.size());

  m_matcher.apply(tracks, m_rlTaggedWireHits, m_relations);

  B2WARNING("Number of relations: " << m_relations.size());

  m_selector.apply(m_relations);

  B2WARNING("Number of relations after selection: " << m_relations.size());

  m_adder.apply(m_relations);


  m_szFitter.apply(tracks);
}
