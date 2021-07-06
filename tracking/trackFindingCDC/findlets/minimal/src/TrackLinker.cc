/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  outputTracks.reserve(outputTracks.size() + m_trackPaths.size());
  for (const std::vector<const CDCTrack*>& trackPath : m_trackPaths) {
    outputTracks.push_back(CDCTrack::condense(trackPath));
  }
}
