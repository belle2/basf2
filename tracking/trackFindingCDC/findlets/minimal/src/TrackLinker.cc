/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackLinker.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackLinker::TrackLinker()
{
  this->addProcessingSignalListener(&m_trackRelationCreator);
}

std::string TrackLinker::getDescription()
{
  return "Links tracks by extraction of track paths in a cellular automaton.";
}

void TrackLinker::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_trackRelationCreator.exposeParameters(moduleParamList, prefix);
  m_cellularPathFinder.exposeParameters(moduleParamList, prefix);
}

void TrackLinker::apply(const std::vector<CDCTrack>& inputTracks, std::vector<CDCTrack>& outputTracks)
{
  // Obtain the tracks as pointers
  std::vector<const CDCTrack*> trackPtrs = as_pointers<const CDCTrack>(inputTracks);

  // Create linking relations
  m_trackRelations.clear();
  m_trackRelationCreator.apply(trackPtrs, m_trackRelations);

  // Find linking paths
  m_trackPaths.clear();
  m_cellularPathFinder.apply(trackPtrs, m_trackRelations, m_trackPaths);

  // Put the linked tracks together
  for (const std::vector<const CDCTrack*>& trackPath : m_trackPaths) {
    outputTracks.push_back(CDCTrack::condense(trackPath));
  }
}
