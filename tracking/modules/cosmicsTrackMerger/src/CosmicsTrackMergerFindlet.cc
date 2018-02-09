/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/cosmicsTrackMerger/CosmicsTrackMergerFindlet.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CosmicsTrackMergerFindlet::CosmicsTrackMergerFindlet()
{
  this->addProcessingSignalListener(&m_trackRelationCreator);
}

std::string CosmicsTrackMergerFindlet::getDescription()
{
  return "Links tracks by extraction of track paths in a cellular automaton.";
}

void CosmicsTrackMergerFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_trackRelationCreator.exposeParameters(moduleParamList, prefix);
  m_cellularPathFinder.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter("inputRecoTracks", m_param_inputRecoTracks,
                                "Store Array name of the input reco tracks.", m_param_inputRecoTracks);
  moduleParamList->addParameter("outputRecoTracks", m_param_outputRecoTracks,
                                "Store Array name of the output reco tracks.", m_param_outputRecoTracks);
}

void CosmicsTrackMergerFindlet::initialize()
{
  m_inputTracks.isRequired(m_param_inputRecoTracks);

  m_outputTracks.registerInDataStore(m_param_outputRecoTracks);
  RecoTrack::registerRequiredRelations(m_outputTracks);

  Super::initialize();
}

void CosmicsTrackMergerFindlet::apply()
{
  m_inputTrackVector.clear();

  // Read the tracks from the Store Array
  for (const RecoTrack& recoTrack : m_inputTracks) {
    m_inputTrackVector.emplace_back(&recoTrack);
  }

  std::sort(m_inputTrackVector.begin(), m_inputTrackVector.end());

  // Obtain the tracks as pointers
  std::vector<const CellularRecoTrack*> trackPtrs = as_pointers<const CellularRecoTrack>(m_inputTrackVector);

  // Create linking relations
  m_trackRelations.clear();
  m_trackRelationCreator.apply(trackPtrs, m_trackRelations);

  // Find linking paths
  m_trackPaths.clear();
  m_cellularPathFinder.apply(trackPtrs, m_trackRelations, m_trackPaths);

  // Write out the tracks to the data store
  // TODO: other possibility would be to use relations for this!
  // One would need to extend the RelatedTrackCombiner for this
  for (const std::vector<const CellularRecoTrack*>& trackPath : m_trackPaths) {
    // We use the first track to give us the momentum seed etc.
    const RecoTrack* firstTrack = *(trackPath.front());
    RecoTrack* newRecoTrack = firstTrack->copyToStoreArray(m_outputTracks);

    unsigned int numberOfAddedHits = 0;
    for (const CellularRecoTrack* cellularRecoTrack : trackPath) {
      numberOfAddedHits += newRecoTrack->addHitsFromRecoTrack(*cellularRecoTrack, numberOfAddedHits);
    }
  }
}
