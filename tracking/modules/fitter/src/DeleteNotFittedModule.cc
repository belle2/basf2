/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/fitter/DeleteNotFittedModule.h>
#include <framework/datastore/StoreArray.h>
#include <genfit/TrackCand.h>
#include <mdst/dataobjects/TrackFitResult.h>

using namespace Belle2;

REG_MODULE(CopyFittedTracks)
REG_MODULE(CopyNotFittedTracks)

BaseFilterFittedTracksModule::BaseFilterFittedTracksModule() : Module()
{
  // Set module properties
  setDescription("Copy all TrackCands with a TrackFitResult to the StoreArray");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("inputStoreArrayNameForTrackCands", m_param_inputStoreArrayNameForTrackCands, "Input StoreArray name for the TrackCands.",
           std::string("TrackCands"));
  addParam("inputStoreArrayNameForRecoTracks", m_param_inputStoreArrayNameForRecoTracks, "Input StoreArray name for the RecoTracks.",
           std::string("RecoTracks"));
  addParam("outputStoreArrayNameForTrackCands", m_param_outputStoreArrayNameForTrackCands,
           "Output StoreArray name for the TrackCands.",
           std::string("FittedTrackCands"));
  addParam("outputStoreArrayNameForRecoCands", m_param_outputStoreArrayNameForRecoTracks,
           "Output StoreArray name for the TrackCands.",
           std::string("FittedRecoTracks"));

}

void BaseFilterFittedTracksModule::initialize()
{
  StoreArray<genfit::TrackCand> outputTrackCands(m_param_outputStoreArrayNameForTrackCands);
  outputTrackCands.registerInDataStore();
  StoreArray<RecoTrack> outputRecoTracks(m_param_outputStoreArrayNameForRecoTracks);
  outputRecoTracks.registerInDataStore();

  StoreArray<genfit::TrackCand> inputTrackCands(m_param_inputStoreArrayNameForTrackCands);
  inputTrackCands.isRequired();
  StoreArray<RecoTrack> inputRecoTracks(m_param_inputStoreArrayNameForRecoTracks);
  inputRecoTracks.isRequired();

  StoreArray<TrackFitResult> trackFitResults;
  trackFitResults.isRequired();

  outputTrackCands.registerRelationTo(inputTrackCands);
  outputTrackCands.registerRelationTo(trackFitResults);
  outputRecoTracks.registerRelationTo(outputTrackCands);
  outputRecoTracks.registerRelationTo(trackFitResults);
}

void BaseFilterFittedTracksModule::event()
{
  StoreArray<genfit::TrackCand> outputTrackCands(m_param_outputStoreArrayNameForTrackCands);
  outputTrackCands.create(false);
  StoreArray<RecoTrack> outputRecoTracks(m_param_outputStoreArrayNameForRecoTracks);
  outputRecoTracks.create(false);

  StoreArray<genfit::TrackCand> inputTrackCands(m_param_inputStoreArrayNameForTrackCands);
  StoreArray<RecoTrack> inputRecoTracks(m_param_inputStoreArrayNameForRecoTracks);

  for (const RecoTrack& recoTrack : inputRecoTracks) {
    if (copyRecoTrack(recoTrack)) {
      RecoTrack* copiedRecoTrack = outputRecoTracks.appendNew(recoTrack);
      TrackFitResult* relatedTrackFitResult = recoTrack.getRelated<TrackFitResult>("TrackFitResults");
      if (relatedTrackFitResult) {
        copiedRecoTrack->addRelationTo(relatedTrackFitResult);
      }
      genfit::TrackCand* relatedTrackCand = recoTrack.getRelated<genfit::TrackCand>(m_param_inputStoreArrayNameForTrackCands);
      if (relatedTrackCand) {
        genfit::TrackCand* newTrackCand = outputTrackCands.appendNew(*relatedTrackCand);
        copiedRecoTrack->addRelationTo(newTrackCand);
      }
    }
  }
}


