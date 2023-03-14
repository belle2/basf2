/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/recoTracksReverter/RecoTracksReverterModule.h>

using namespace Belle2;

REG_MODULE(RecoTracksReverter);

RecoTracksReverterModule::RecoTracksReverterModule() :
  Module()
{
  setDescription("Revert the RecoTracks, including changing of the seed position, reverting the momentum and revising the hits Order");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("inputStoreArrayName", m_inputStoreArrayName,
           "Name of the input StoreArray");
  addParam("outputStoreArrayName", m_outputStoreArrayName,
           "Name of the output StoreArray");
}

void RecoTracksReverterModule::initialize()
{
  m_inputRecoTracks.isRequired(m_inputStoreArrayName);

  m_outputRecoTracks.registerInDataStore(m_outputStoreArrayName, DataStore::c_ErrorIfAlreadyRegistered);
  RecoTrack::registerRequiredRelations(m_outputRecoTracks);

  m_outputRecoTracks.registerRelationTo(m_inputRecoTracks);

}

void RecoTracksReverterModule::event()
{
  for (const RecoTrack& recoTrack : m_inputRecoTracks) {

    if (not recoTrack.wasFitSuccessful()) {
      continue;
    }

    // get the cut from DB
    if (!m_flipCutsFromDB.isValid()) continue;
    double mvaFlipCut = (*m_flipCutsFromDB).getFirstCut();

    if (recoTrack.getFlipQualityIndicator() < mvaFlipCut) continue;
    Track* track = recoTrack.getRelatedFrom<Belle2::Track>();
    if (!track) continue;

    const auto& measuredStateOnPlane = recoTrack.getMeasuredStateOnPlaneFromLastHit();
    const ROOT::Math::XYZVector& currentPosition = ROOT::Math::XYZVector(measuredStateOnPlane.getPos());
    const ROOT::Math::XYZVector& currentMomentum = ROOT::Math::XYZVector(measuredStateOnPlane.getMom());
    const double& currentCharge = measuredStateOnPlane.getCharge();

    RecoTrack* newRecoTrack = m_outputRecoTracks.appendNew(currentPosition, -currentMomentum, -currentCharge,
                                                           recoTrack.getStoreArrayNameOfPXDHits(), recoTrack.getStoreArrayNameOfSVDHits(), recoTrack.getStoreArrayNameOfCDCHits(),
                                                           recoTrack.getStoreArrayNameOfBKLMHits(), recoTrack.getStoreArrayNameOfEKLMHits(),
                                                           recoTrack.getStoreArrayNameOfRecoHitInformation());
    newRecoTrack->addHitsFromRecoTrack(&recoTrack, newRecoTrack->getNumberOfTotalHits(), true);
    newRecoTrack->addRelationTo(&recoTrack);
  }
}

