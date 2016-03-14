#include <tracking/dataobjects/RecoTrack.h>

#include <tracking/trackFitting/measurementCreator/creators/CoordinateMeasurementCreator.h>

#include <genfit/FitStatus.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/RKTrackRep.h>
#include <genfit/TrackCand.h>
#include <genfit/AbsFitter.h>

#include <framework/gearbox/Const.h>

#include <framework/dataobjects/Helix.h>
#include <genfit/WireTrackCandHit.h>

using namespace Belle2;

RecoTrack::RecoTrack(const TVector3& position, const TVector3& momentum, const short int charge,
                     const std::string& storeArrayNameOfCDCHits,
                     const std::string& storeArrayNameOfSVDHits,
                     const std::string& storeArrayNameOfPXDHits,
                     const std::string& storeArrayNameOfRecoHitInformation) :
  m_charge(charge),
  m_storeArrayNameOfCDCHits(storeArrayNameOfCDCHits),
  m_storeArrayNameOfSVDHits(storeArrayNameOfSVDHits),
  m_storeArrayNameOfPXDHits(storeArrayNameOfPXDHits),
  m_storeArrayNameOfRecoHitInformation(storeArrayNameOfRecoHitInformation)
{
  m_genfitTrack.setStateSeed(position, momentum);
}

RecoTrack* RecoTrack::createFromTrackCand(const genfit::TrackCand* trackCand,
                                          const std::string& storeArrayNameOfRecoTracks,
                                          const std::string& storeArrayNameOfCDCHits,
                                          const std::string& storeArrayNameOfSVDHits,
                                          const std::string& storeArrayNameOfPXDHits,
                                          const std::string& storeArrayNameOfRecoHitInformation
                                         )
{

  StoreArray<RecoTrack> recoTracks(storeArrayNameOfRecoTracks);
  StoreArray<UsedCDCHit> cdcHits(storeArrayNameOfCDCHits);
  StoreArray<UsedSVDHit> svdHits(storeArrayNameOfSVDHits);
  StoreArray<UsedPXDHit> pxdHits(storeArrayNameOfPXDHits);

  // Set the tracking parameters
  const TVector3& position = trackCand->getPosSeed();
  const TVector3& momentum = trackCand->getMomSeed();
  const short int charge = trackCand->getChargeSeed();

  RecoTrack* newRecoTrack = recoTracks.appendNew(position, momentum, charge,
                                                 storeArrayNameOfCDCHits, storeArrayNameOfSVDHits,
                                                 storeArrayNameOfPXDHits, storeArrayNameOfRecoHitInformation);

  // TODO Set the covariance seed (that should be done by the tracking package)
  TMatrixDSym covSeed(6);
  covSeed(0, 0) = 1e-3;
  covSeed(1, 1) = 1e-3;
  covSeed(2, 2) = 4e-3;
  covSeed(3, 3) = 0.01e-3;
  covSeed(4, 4) = 0.01e-3;
  covSeed(5, 5) = 0.04e-3;
  newRecoTrack->m_genfitTrack.setCovSeed(covSeed);

  for (unsigned int hitIndex = 0; hitIndex < trackCand->getNHits(); hitIndex++) {
    genfit::TrackCandHit* trackCandHit = trackCand->getHit(hitIndex);
    const int detID = trackCandHit->getDetId();
    const int hitID = trackCandHit->getHitId();
    const unsigned int sortingParameter = static_cast<const unsigned int>(trackCandHit->getSortingParameter());
    if (detID == Const::CDC) {
      UsedCDCHit* cdcHit = cdcHits[hitID];
      // Special case for CDC hits, we add a right-left information
      const genfit::WireTrackCandHit* wireHit = dynamic_cast<const genfit::WireTrackCandHit*>(trackCandHit);
      if (not wireHit) {
        B2FATAL("CDC hit is not a wire hit. The RecoTrack can not handle such a case.");
      }
      if (wireHit->getLeftRightResolution() > 0) {
        newRecoTrack->addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::right);
      } else if (wireHit->getLeftRightResolution() < 0) {
        newRecoTrack->addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::left);
      } else {
        newRecoTrack->addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::undefinedRightLeftInformation);
      }

    } else if (detID == Const::SVD) {
      UsedSVDHit* svdHit = svdHits[hitID];
      newRecoTrack->addSVDHit(svdHit, sortingParameter);
    } else if (detID == Const::PXD) {
      UsedPXDHit* pxdHit = pxdHits[hitID];
      newRecoTrack->addPXDHit(pxdHit, sortingParameter);
    }
  }

  return newRecoTrack;
}

genfit::TrackCand* RecoTrack::createGenfitTrackCand() const
{
  genfit::TrackCand* createdTrackCand = new genfit::TrackCand();

  // Set the trajectory parameters
  createdTrackCand->setPosMomSeed(getPosition(), getMomentum(), getCharge());

  // Add the hits
  mapOnHits<UsedCDCHit>(m_storeArrayNameOfCDCHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedCDCHit * const hit) {
    createdTrackCand->addHit(Const::CDC, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });
  mapOnHits<UsedSVDHit>(m_storeArrayNameOfSVDHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedSVDHit * const hit) {
    createdTrackCand->addHit(Const::SVD, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });
  mapOnHits<UsedPXDHit>(m_storeArrayNameOfPXDHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedPXDHit * const hit) {
    createdTrackCand->addHit(Const::PXD, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });

  // Add the hits
  return createdTrackCand;
}

bool RecoTrack::wasFitSuccessful(const genfit::AbsTrackRep* representation) const
{
  checkDirtyFlag();

  if (not hasTrackFitStatus(representation)) {
    return false;
  }

  const genfit::FitStatus* fs = getTrackFitStatus(representation);
  if (not fs) {
    return false;
  }
  if (not fs->isFitConverged()) {
    return false;
  }
  const genfit::KalmanFitStatus* kfs = dynamic_cast<const genfit::KalmanFitStatus*>(fs);
  if (not kfs) {
    return false;
  }

  return true;
}