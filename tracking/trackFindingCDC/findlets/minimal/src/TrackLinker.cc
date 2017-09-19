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

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

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

void TrackLinker::exposeParams(ParamList* paramList, const std::string& prefix)
{
  m_trackRelationCreator.exposeParams(paramList, prefix);
}

void TrackLinker::apply(const std::vector<CDCTrack>& inputTracks, std::vector<CDCTrack>& outputTracks)
{
  // Create linking relations
  m_trackRelations.clear();
  m_trackRelationCreator.apply(inputTracks, m_trackRelations);

  // Find linking paths
  m_trackPaths.clear();
  WeightedNeighborhood<const CDCTrack> trackNeighborhood(m_trackRelations);
  m_cellularPathFinder.apply(inputTracks, trackNeighborhood, m_trackPaths);

  // Put the linked tracks together
  for (const std::vector<const CDCTrack*>& trackPath : m_trackPaths) {
    outputTracks.push_back(CDCTrack::condense(trackPath));
  }
}
