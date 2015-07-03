#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

ClassImp(RecoTrack);

RecoTrack* RecoTrack::createFromTrackCand(genfit::TrackCand* trackCand,
                                          const std::string& storeArrayNameOfRecoTracks,
                                          const std::string& storeArrayNameOfCDCHits,
                                          const std::string& storeArrayNameOfSVDHits,
                                          const std::string& storeArrayNameOfPXDHits,
                                          const std::string& storeArrayNameOfRecoHitInformation
                                         )
{

  StoreArray<RecoTrack> recoTracks(storeArrayNameOfRecoTracks);
  StoreArray<CDCHit> cdcHits(storeArrayNameOfCDCHits);
  StoreArray<SVDHit> svdHits(storeArrayNameOfSVDHits);
  StoreArray<PXDHit> pxdHits(storeArrayNameOfPXDHits);

  // Set the tracking parameters
  const TVector3& position = trackCand->getPosSeed();
  const TVector3& momentum = trackCand->getMomSeed();
  const short int charge = trackCand->getChargeSeed();
  // TODO: Get the magnetic field!
  const double bZ = 1.5;
  RecoTrack* newRecoTrack = recoTracks.appendNew(position, momentum, charge, bZ,
                                                 storeArrayNameOfCDCHits, storeArrayNameOfSVDHits,
                                                 storeArrayNameOfPXDHits, storeArrayNameOfRecoHitInformation);

  for (unsigned int hitIndex = 0; hitIndex < trackCand->getNHits(); hitIndex++) {
    genfit::TrackCandHit* trackCandHit = trackCand->getHit(hitIndex);
    const int detID = trackCandHit->getDetId();
    const int hitID = trackCandHit->getHitId();
    const double sortingParameter = trackCandHit->getSortingParameter();
    if (detID == Const::CDC) {
      CDCHit* cdcHit = cdcHits[hitID];
      newRecoTrack->addCDCHit(cdcHit, sortingParameter);
    } else if (detID == Const::SVD) {
      SVDHit* svdHit = svdHits[hitID];
      newRecoTrack->addSVDHit(svdHit, sortingParameter);
    } else if (detID == Const::PXD) {
      PXDHit* pxdHit = pxdHits[hitID];
      newRecoTrack->addPXDHit(pxdHit, sortingParameter);
    }
  }

  return newRecoTrack;
}

genfit::TrackCand* RecoTrack::createGenfitTrackCand() const
{
  genfit::TrackCand* createdTrackCand = new genfit::TrackCand();

  // Set the trajectory parameters
  createdTrackCand->setPosMomSeed(getPerigee(), getMomentum(), getCharge());

  // Add the hits
  mapOnHits<CDCHit>(m_storeArrayNameOfCDCHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const CDCHit * const hit) {
    createdTrackCand->addHit(Const::CDC, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });
  mapOnHits<SVDHit>(m_storeArrayNameOfSVDHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const SVDHit * const hit) {
    createdTrackCand->addHit(Const::SVD, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });
  mapOnHits<PXDHit>(m_storeArrayNameOfPXDHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const PXDHit * const hit) {
    createdTrackCand->addHit(Const::PXD, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });

  // Add the hits
  return createdTrackCand;
}
