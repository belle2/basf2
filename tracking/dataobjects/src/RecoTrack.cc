#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/MCParticle.h>

#include <genfit/TrackCand.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/WireTrackCandHit.h>

#include <framework/dataobjects/Helix.h>

using namespace Belle2;

RecoTrack::RecoTrack(const TVector3& seedPosition, const TVector3& seedMomentum, const short int seedCharge,
                     const std::string& storeArrayNameOfCDCHits,
                     const std::string& storeArrayNameOfSVDHits,
                     const std::string& storeArrayNameOfPXDHits,
                     const std::string& storeArrayNameOfBKLMHits,
                     const std::string& storeArrayNameOfEKLMHits,
                     const std::string& storeArrayNameOfRecoHitInformation) :
  m_charge(seedCharge),
  m_storeArrayNameOfCDCHits(storeArrayNameOfCDCHits),
  m_storeArrayNameOfSVDHits(storeArrayNameOfSVDHits),
  m_storeArrayNameOfPXDHits(storeArrayNameOfPXDHits),
  m_storeArrayNameOfBKLMHits(storeArrayNameOfBKLMHits),
  m_storeArrayNameOfEKLMHits(storeArrayNameOfEKLMHits),
  m_storeArrayNameOfRecoHitInformation(storeArrayNameOfRecoHitInformation)
{
  m_genfitTrack.setStateSeed(seedPosition, seedMomentum);
  // TODO Set the covariance seed (that should be done by the tracking package)
  TMatrixDSym covSeed(6);
  covSeed(0, 0) = 1e-3;
  covSeed(1, 1) = 1e-3;
  covSeed(2, 2) = 4e-3;
  covSeed(3, 3) = 0.01e-3;
  covSeed(4, 4) = 0.01e-3;
  covSeed(5, 5) = 0.04e-3;
  m_genfitTrack.setCovSeed(covSeed);
}

RecoTrack* RecoTrack::createFromTrackCand(const genfit::TrackCand& trackCand,
                                          const std::string& storeArrayNameOfRecoTracks,
                                          const std::string& storeArrayNameOfCDCHits,
                                          const std::string& storeArrayNameOfSVDHits,
                                          const std::string& storeArrayNameOfPXDHits,
                                          const std::string& storeArrayNameOfBKLMHits,
                                          const std::string& storeArrayNameOfEKLMHits,
                                          const std::string& storeArrayNameOfRecoHitInformation,
                                          const bool recreateSortingParameters
                                         )
{

  StoreArray<RecoTrack> recoTracks(storeArrayNameOfRecoTracks);
  StoreArray<RecoHitInformation> recoHitInformations(storeArrayNameOfRecoHitInformation);
  StoreArray<UsedCDCHit> cdcHits(storeArrayNameOfCDCHits);
  StoreArray<UsedSVDHit> svdHits(storeArrayNameOfSVDHits);
  StoreArray<UsedPXDHit> pxdHits(storeArrayNameOfPXDHits);
  StoreArray<UsedBKLMHit> bklmHits(storeArrayNameOfBKLMHits);
  StoreArray<UsedEKLMHit> eklmHits(storeArrayNameOfEKLMHits);

  // Set the tracking parameters
  const TVector3& position = trackCand.getPosSeed();
  const TVector3& momentum = trackCand.getMomSeed();
  const short int charge = trackCand.getChargeSeed();
  const double time = trackCand.getTimeSeed();

  RecoTrack* newRecoTrack = recoTracks.appendNew(position, momentum, charge,
                                                 cdcHits.getName(), svdHits.getName(),
                                                 pxdHits.getName(),
                                                 bklmHits.getName(), eklmHits.getName(),
                                                 recoHitInformations.getName());
  newRecoTrack->setTimeSeed(time);

  // TODO Set the covariance seed (that should be done by the tracking package)
  TMatrixDSym covSeed(6);
  covSeed(0, 0) = 1e-3;
  covSeed(1, 1) = 1e-3;
  covSeed(2, 2) = 4e-3;
  covSeed(3, 3) = 0.01e-3;
  covSeed(4, 4) = 0.01e-3;
  covSeed(5, 5) = 0.04e-3;
  newRecoTrack->m_genfitTrack.setCovSeed(covSeed);

  for (unsigned int hitIndex = 0; hitIndex < trackCand.getNHits(); hitIndex++) {
    genfit::TrackCandHit* trackCandHit = trackCand.getHit(hitIndex);
    const int detID = trackCandHit->getDetId();
    const int hitID = trackCandHit->getHitId();
    const unsigned int sortingParameter = recreateSortingParameters ? hitIndex : static_cast<const unsigned int>
                                          (trackCandHit->getSortingParameter());
    if (detID == Const::CDC) {
      UsedCDCHit* cdcHit = cdcHits[hitID];
      // Special case for CDC hits, we add a right-left information
      const genfit::WireTrackCandHit* wireHit = dynamic_cast<const genfit::WireTrackCandHit*>(trackCandHit);
      if (not wireHit) {
        B2FATAL("CDC hit is not a wire hit. The RecoTrack can not handle such a case.");
      }
      if (wireHit->getLeftRightResolution() > 0) {
        newRecoTrack->addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::c_right);
      } else if (wireHit->getLeftRightResolution() < 0) {
        newRecoTrack->addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::c_left);
      } else {
        newRecoTrack->addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::c_undefinedRightLeftInformation);
      }

    } else if (detID == Const::SVD) {
      UsedSVDHit* svdHit = svdHits[hitID];
      newRecoTrack->addSVDHit(svdHit, sortingParameter);
    } else if (detID == Const::PXD) {
      UsedPXDHit* pxdHit = pxdHits[hitID];
      newRecoTrack->addPXDHit(pxdHit, sortingParameter);
    } else if (detID == Const::BKLM) {
      UsedBKLMHit* bklmHit = bklmHits[hitID];
      newRecoTrack->addBKLMHit(bklmHit, sortingParameter);
    } else if (detID == Const::EKLM) {
      UsedEKLMHit* eklmHit = eklmHits[hitID];
      newRecoTrack->addEKLMHit(eklmHit, sortingParameter);
    }
  }

  return newRecoTrack;
}

genfit::TrackCand RecoTrack::createGenfitTrackCand() const
{
  genfit::TrackCand createdTrackCand;

  // Set the trajectory parameters
  createdTrackCand.setPosMomSeed(getPositionSeed(), getMomentumSeed(), getChargeSeed());
  createdTrackCand.setCovSeed(getSeedCovariance());
  createdTrackCand.setTimeSeed(getTimeSeed());

  // Add the hits
  mapOnHits<UsedCDCHit>(m_storeArrayNameOfCDCHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedCDCHit * const hit) {
    if (hitInformation.getRightLeftInformation() == RecoHitInformation::c_left) {
      createdTrackCand.addHit(new genfit::WireTrackCandHit(Const::CDC, hit->getArrayIndex(), -1,
                                                           hitInformation.getSortingParameter(), -1));
    } else if (hitInformation.getRightLeftInformation() == RecoHitInformation::c_right) {
      createdTrackCand.addHit(new genfit::WireTrackCandHit(Const::CDC, hit->getArrayIndex(), -1,
                                                           hitInformation.getSortingParameter(), 1));
    } else {
      createdTrackCand.addHit(new genfit::WireTrackCandHit(Const::CDC, hit->getArrayIndex(), -1,
                                                           hitInformation.getSortingParameter(), 0));
    }
  });
  mapOnHits<UsedSVDHit>(m_storeArrayNameOfSVDHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedSVDHit * const hit) {
    createdTrackCand.addHit(Const::SVD, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });
  mapOnHits<UsedPXDHit>(m_storeArrayNameOfPXDHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedPXDHit * const hit) {
    createdTrackCand.addHit(Const::PXD, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });
  mapOnHits<UsedBKLMHit>(m_storeArrayNameOfBKLMHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedBKLMHit * const hit) {
    createdTrackCand.addHit(Const::BKLM, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });
  mapOnHits<UsedEKLMHit>(m_storeArrayNameOfEKLMHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedEKLMHit * const hit) {
    createdTrackCand.addHit(Const::EKLM, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });

  createdTrackCand.sortHits();

  // Set the MC Particle
  const MCParticle* relatedMCParticle = getRelatedTo<MCParticle>();
  if (relatedMCParticle) {
    createdTrackCand.setMcTrackId(relatedMCParticle->getArrayIndex());
  }

  // Add the hits
  return createdTrackCand;
}

size_t RecoTrack::addHitsFromRecoTrack(const RecoTrack* recoTrack, const unsigned int sortingParameterOffset)
{
  size_t hitsCopied = 0;

  for (auto* pxdHit : recoTrack->getPXDHitList()) {
    auto recoHitInfo = recoTrack->getRecoHitInformation(pxdHit);
    assert(recoHitInfo);
    hitsCopied += addPXDHit(pxdHit, recoHitInfo->getSortingParameter() + sortingParameterOffset, recoHitInfo->getFoundByTrackFinder());
  }

  for (auto* svdHit : recoTrack->getSVDHitList()) {
    auto recoHitInfo = recoTrack->getRecoHitInformation(svdHit);
    assert(recoHitInfo);
    hitsCopied += addSVDHit(svdHit, recoHitInfo->getSortingParameter() + sortingParameterOffset, recoHitInfo->getFoundByTrackFinder());
  }

  for (auto* cdcHit : recoTrack->getCDCHitList()) {
    auto recoHitInfo = recoTrack->getRecoHitInformation(cdcHit);
    assert(recoHitInfo);
    hitsCopied += addCDCHit(cdcHit, recoHitInfo->getSortingParameter() + sortingParameterOffset,
                            recoHitInfo->getRightLeftInformation(),
                            recoHitInfo->getFoundByTrackFinder());
  }

  for (auto* bklmHit : recoTrack->getBKLMHitList()) {
    auto recoHitInfo = recoTrack->getRecoHitInformation(bklmHit);
    assert(recoHitInfo);
    hitsCopied += addBKLMHit(bklmHit, recoHitInfo->getSortingParameter() + sortingParameterOffset,
                             recoHitInfo->getFoundByTrackFinder());
  }

  for (auto* eklmHit : recoTrack->getEKLMHitList()) {
    auto recoHitInfo = recoTrack->getRecoHitInformation(eklmHit);
    assert(recoHitInfo);
    hitsCopied += addEKLMHit(eklmHit, recoHitInfo->getSortingParameter() + sortingParameterOffset,
                             recoHitInfo->getFoundByTrackFinder());
  }

  return hitsCopied;
}


bool RecoTrack::wasFitSuccessful(const genfit::AbsTrackRep* representation) const
{
  checkDirtyFlag();

  if (getRepresentations().empty()) {
    return false;
  }

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

  // make sure we only consider fitted if the Kalman method was used
  const genfit::KalmanFitStatus* kfs = dynamic_cast<const genfit::KalmanFitStatus*>(fs);
  if (not kfs) {
    return false;
  }

  return true;
}

void RecoTrack::prune()
{
  // "Delete" all RecoHitInfromation but the first and the last.
  // Copy is intended!
  std::vector<RelationEntry> relatedRecoHitInformations = getRelationsWith<RecoHitInformation>
                                                          (m_storeArrayNameOfRecoHitInformation).relations();
  std::sort(relatedRecoHitInformations.begin(), relatedRecoHitInformations.end() , [](const RelationEntry & lhs,
  const RelationEntry & rhs) {
    return dynamic_cast<RecoHitInformation*>(lhs.object)->getSortingParameter() > dynamic_cast<RecoHitInformation*>
           (rhs.object)->getSortingParameter();
  });

  // "Prune" all RecoHitInformation but the first and the last.
  for (unsigned int i = 1; i < relatedRecoHitInformations.size() - 1; ++i) {
    dynamic_cast<RecoHitInformation*>(relatedRecoHitInformations[i].object)->setFlag(RecoHitInformation::RecoHitFlag::c_pruned);
    dynamic_cast<RecoHitInformation*>(relatedRecoHitInformations[i].object)->setCreatedTrackPointID(-1);
  }


  // Genfits prune method fails, if the number of hits is too small.
  if (getHitPointsWithMeasurement().size() >= 2) {
    m_genfitTrack.prune("FL");
  }
}

genfit::Track& RecoTrackGenfitAccess::getGenfitTrack(RecoTrack& recoTrack)
{
  return recoTrack.m_genfitTrack;
}

const genfit::MeasuredStateOnPlane& RecoTrack::getMeasuredStateOnPlaneClosestTo(const TVector3& closestPoint,
    const genfit::AbsTrackRep* representation)
{
  checkDirtyFlag();
  const unsigned int numberOfPoints = m_genfitTrack.getNumPointsWithMeasurement();

  assert(numberOfPoints > 0);

  const genfit::MeasuredStateOnPlane* nearestStateOnPlane = nullptr;
  double minimalDistance2 = 0;
  for (unsigned int hitIndex = 0; hitIndex < numberOfPoints; hitIndex++) {
    const genfit::MeasuredStateOnPlane& measuredStateOnPlane = m_genfitTrack.getFittedState(hitIndex, representation);

    const double currentDistance2 = (measuredStateOnPlane.getPos() - closestPoint).Mag2();

    if (not nearestStateOnPlane or currentDistance2 < minimalDistance2) {
      nearestStateOnPlane = &measuredStateOnPlane;
      minimalDistance2 = currentDistance2;
    }
  }
  return *nearestStateOnPlane;
}


void RecoTrack::deleteFittedInformation()
{
  // Delete all fitted information for all representations
  for (unsigned int i = 0; i < getRepresentations().size(); i++) {
    m_genfitTrack.deleteTrackRep(i);
  }
}

/// Helper function to get the seed or the measured state on plane from a track
std::tuple<const TVector3&, const TVector3&, short> RecoTrack::extractTrackState() const
{
  if (not wasFitSuccessful()) {
    return std::make_tuple(getPositionSeed(), getMomentumSeed(), getChargeSeed());
  } else {
    const auto& measuredStateOnPlane = getMeasuredStateOnPlaneFromFirstHit();
    return std::make_tuple(measuredStateOnPlane.getPos(), measuredStateOnPlane.getMom(), measuredStateOnPlane.getCharge());
  }
}

RecoTrack* RecoTrack::copyToStoreArrayUsing(StoreArray<RecoTrack>& storeArray,
                                            const TVector3& position, const TVector3& momentum, short charge,
                                            const TMatrixDSym& covariance, double timeSeed) const
{
  RecoTrack* newRecoTrack = storeArray.appendNew(position, momentum, charge,
                                                 getStoreArrayNameOfCDCHits(), getStoreArrayNameOfSVDHits(), getStoreArrayNameOfPXDHits(),
                                                 getStoreArrayNameOfBKLMHits(), getStoreArrayNameOfEKLMHits(), getStoreArrayNameOfRecoHitInformation());

  newRecoTrack->setTimeSeed(timeSeed);
  newRecoTrack->setSeedCovariance(covariance);

  return newRecoTrack;
}

RecoTrack* RecoTrack::copyToStoreArrayUsingSeeds(StoreArray<RecoTrack>& storeArray) const
{
  return copyToStoreArrayUsing(storeArray, getPositionSeed(), getMomentumSeed(), getChargeSeed(), getSeedCovariance(), getTimeSeed());
}

RecoTrack* RecoTrack::copyToStoreArray(StoreArray<RecoTrack>& storeArray) const
{
  if (wasFitSuccessful()) {
    const auto& mSoP = getMeasuredStateOnPlaneFromFirstHit();
    return copyToStoreArrayUsing(storeArray, mSoP.getPos(), mSoP.getMom(), static_cast<short>(mSoP.getCharge()),
                                 mSoP.get6DCov(), mSoP.getTime());
  } else {
    return copyToStoreArrayUsingSeeds(storeArray);
  }
}