/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/MCParticle.h>

#include <genfit/TrackCand.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/WireTrackCandHit.h>
#include <genfit/RKTrackRep.h>
#include <genfit/MplTrackRep.h>
#include <simulation/monopoles/MonopoleConstants.h>
#include <svd/dataobjects/SVDCluster.h>

#include <cmath>

using namespace Belle2;

RecoTrack::RecoTrack(const ROOT::Math::XYZVector& seedPosition, const ROOT::Math::XYZVector& seedMomentum,
                     const short int seedCharge,
                     const std::string& storeArrayNameOfPXDHits,
                     const std::string& storeArrayNameOfSVDHits,
                     const std::string& storeArrayNameOfCDCHits,
                     const std::string& storeArrayNameOfBKLMHits,
                     const std::string& storeArrayNameOfEKLMHits,
                     const std::string& storeArrayNameOfRecoHitInformation) :
  m_charge(seedCharge),
  m_storeArrayNameOfPXDHits(storeArrayNameOfPXDHits),
  m_storeArrayNameOfSVDHits(storeArrayNameOfSVDHits),
  m_storeArrayNameOfCDCHits(storeArrayNameOfCDCHits),
  m_storeArrayNameOfBKLMHits(storeArrayNameOfBKLMHits),
  m_storeArrayNameOfEKLMHits(storeArrayNameOfEKLMHits),
  m_storeArrayNameOfRecoHitInformation(storeArrayNameOfRecoHitInformation)
{
  m_genfitTrack.setStateSeed(XYZToTVector(seedPosition), XYZToTVector(seedMomentum));
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

void RecoTrack::registerRequiredRelations(
  StoreArray<RecoTrack>& recoTracks,
  std::string const& pxdHitsStoreArrayName,
  std::string const& svdHitsStoreArrayName,
  std::string const& cdcHitsStoreArrayName,
  std::string const& bklmHitsStoreArrayName,
  std::string const& eklmHitsStoreArrayName,
  std::string const& recoHitInformationStoreArrayName)
{
  StoreArray<RecoHitInformation> recoHitInformations(recoHitInformationStoreArrayName);
  recoHitInformations.registerInDataStore();
  recoTracks.registerRelationTo(recoHitInformations);

  StoreArray<RecoHitInformation::UsedPXDHit> pxdHits(pxdHitsStoreArrayName);
  if (pxdHits.isOptional()) {
    pxdHits.registerRelationTo(recoTracks);
    recoHitInformations.registerRelationTo(pxdHits);
  }

  StoreArray<RecoHitInformation::UsedSVDHit> svdHits(svdHitsStoreArrayName);
  if (svdHits.isOptional()) {
    svdHits.registerRelationTo(recoTracks);
    recoHitInformations.registerRelationTo(svdHits);
  }

  StoreArray<RecoHitInformation::UsedCDCHit> cdcHits(cdcHitsStoreArrayName);
  if (cdcHits.isOptional()) {
    cdcHits.registerRelationTo(recoTracks);
    recoHitInformations.registerRelationTo(cdcHits);
  }

  StoreArray<RecoHitInformation::UsedBKLMHit> bklmHits(bklmHitsStoreArrayName);
  if (bklmHits.isOptional()) {
    bklmHits.registerRelationTo(recoTracks);
    recoHitInformations.registerRelationTo(bklmHits);
  }

  StoreArray<RecoHitInformation::UsedEKLMHit> eklmHits(eklmHitsStoreArrayName);
  if (eklmHits.isOptional()) {
    eklmHits.registerRelationTo(recoTracks);
    recoHitInformations.registerRelationTo(eklmHits);
  }
}

RecoTrack* RecoTrack::createFromTrackCand(const genfit::TrackCand& trackCand,
                                          const std::string& storeArrayNameOfRecoTracks,
                                          const std::string& storeArrayNameOfPXDHits,
                                          const std::string& storeArrayNameOfSVDHits,
                                          const std::string& storeArrayNameOfCDCHits,
                                          const std::string& storeArrayNameOfBKLMHits,
                                          const std::string& storeArrayNameOfEKLMHits,
                                          const std::string& storeArrayNameOfRecoHitInformation,
                                          const bool recreateSortingParameters
                                         )
{

  StoreArray<RecoTrack> recoTracks(storeArrayNameOfRecoTracks);
  StoreArray<RecoHitInformation> recoHitInformations(storeArrayNameOfRecoHitInformation);
  StoreArray<UsedPXDHit> pxdHits(storeArrayNameOfPXDHits);
  StoreArray<UsedSVDHit> svdHits(storeArrayNameOfSVDHits);
  StoreArray<UsedCDCHit> cdcHits(storeArrayNameOfCDCHits);
  StoreArray<UsedBKLMHit> bklmHits(storeArrayNameOfBKLMHits);
  StoreArray<UsedEKLMHit> eklmHits(storeArrayNameOfEKLMHits);

  // Set the tracking parameters
  const ROOT::Math::XYZVector& position = ROOT::Math::XYZVector(trackCand.getPosSeed());
  const ROOT::Math::XYZVector& momentum = ROOT::Math::XYZVector(trackCand.getMomSeed());
  const short int charge = trackCand.getChargeSeed();
  const double time = trackCand.getTimeSeed();

  RecoTrack* newRecoTrack = recoTracks.appendNew(position, momentum, charge,
                                                 pxdHits.getName(), svdHits.getName(), cdcHits.getName(),
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
    const unsigned int sortingParameter = recreateSortingParameters ? hitIndex : static_cast<unsigned int>
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
  createdTrackCand.setPosMomSeed(XYZToTVector(getPositionSeed()), XYZToTVector(getMomentumSeed()), getChargeSeed());
  createdTrackCand.setCovSeed(getSeedCovariance());
  createdTrackCand.setTimeSeed(getTimeSeed());

  // Add the hits
  mapOnHits<UsedPXDHit>(m_storeArrayNameOfPXDHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedPXDHit * const hit) {
    createdTrackCand.addHit(Const::PXD, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });
  mapOnHits<UsedSVDHit>(m_storeArrayNameOfSVDHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedSVDHit * const hit) {
    createdTrackCand.addHit(Const::SVD, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });
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

const genfit::TrackPoint* RecoTrack::getCreatedTrackPoint(const RecoHitInformation* recoHitInformation) const
{
  int createdTrackPointID = recoHitInformation->getCreatedTrackPointID();
  if (createdTrackPointID == -1) {
    return nullptr;
  }

  return m_genfitTrack.getPoint(createdTrackPointID);
}

size_t RecoTrack::addHitsFromRecoTrack(const RecoTrack* recoTrack, unsigned int sortingParameterOffset, bool reversed,
                                       std::optional<double> optionalMinimalWeight)
{
  size_t hitsCopied = 0;

  unsigned int maximalSortingParameter = 0;

  if (reversed) {
    const auto& recoHitInformations = recoTrack->getRecoHitInformations();
    const auto sortBySP = [](const RecoHitInformation * lhs, const RecoHitInformation * rhs) {
      return lhs->getSortingParameter() < rhs->getSortingParameter();
    };
    const auto& maximalElement = std::max_element(recoHitInformations.begin(), recoHitInformations.end(), sortBySP);
    if (maximalElement != recoHitInformations.end()) {
      maximalSortingParameter = (*maximalElement)->getSortingParameter();
    }
  }

  // Helper function to add the sorting parameter offset (or reverse the sign of the sorting parameter)
  const auto calculateSortingParameter = [maximalSortingParameter, sortingParameterOffset](unsigned int sortingParameters) {
    if (maximalSortingParameter > 0) {
      return maximalSortingParameter - sortingParameters + sortingParameterOffset;
    }
    return sortingParameters + sortingParameterOffset;
  };

  const auto testHitWeight = [recoTrack, optionalMinimalWeight](const RecoHitInformation * recoHitInformation) {
    if (not optionalMinimalWeight) {
      return true;
    }
    double minimalWeight = *optionalMinimalWeight;
    const genfit::TrackPoint* trackPoint = recoTrack->getCreatedTrackPoint(recoHitInformation);
    if (trackPoint) {
      genfit::KalmanFitterInfo* kalmanFitterInfo = trackPoint->getKalmanFitterInfo();
      if (not kalmanFitterInfo) {
        return false;
      }
      const std::vector<double>& weights = kalmanFitterInfo->getWeights();
      const auto checkWeight = [minimalWeight](const double weight) {
        return weight >= minimalWeight;
      };
      return std::any_of(weights.begin(), weights.end(), checkWeight);
    }
    return true;
  };

  for (auto* pxdHit : recoTrack->getPXDHitList()) {
    auto recoHitInfo = recoTrack->getRecoHitInformation(pxdHit);
    assert(recoHitInfo);
    if (testHitWeight(recoHitInfo)) {
      hitsCopied += addPXDHit(pxdHit, calculateSortingParameter(recoHitInfo->getSortingParameter()),
                              recoHitInfo->getFoundByTrackFinder());
    }
  }

  for (auto* svdHit : recoTrack->getSVDHitList()) {
    auto recoHitInfo = recoTrack->getRecoHitInformation(svdHit);
    assert(recoHitInfo);
    if (testHitWeight(recoHitInfo)) {
      hitsCopied += addSVDHit(svdHit, calculateSortingParameter(recoHitInfo->getSortingParameter()),
                              recoHitInfo->getFoundByTrackFinder());
    }
  }

  for (auto* cdcHit : recoTrack->getCDCHitList()) {
    auto recoHitInfo = recoTrack->getRecoHitInformation(cdcHit);
    assert(recoHitInfo);
    if (testHitWeight(recoHitInfo)) {
      hitsCopied += addCDCHit(cdcHit, calculateSortingParameter(recoHitInfo->getSortingParameter()),
                              recoHitInfo->getRightLeftInformation(),
                              recoHitInfo->getFoundByTrackFinder());
    }
  }

  for (auto* bklmHit : recoTrack->getBKLMHitList()) {
    auto recoHitInfo = recoTrack->getRecoHitInformation(bklmHit);
    assert(recoHitInfo);
    if (testHitWeight(recoHitInfo)) {
      hitsCopied += addBKLMHit(bklmHit, calculateSortingParameter(recoHitInfo->getSortingParameter()),
                               recoHitInfo->getFoundByTrackFinder());
    }
  }

  for (auto* eklmHit : recoTrack->getEKLMHitList()) {
    auto recoHitInfo = recoTrack->getRecoHitInformation(eklmHit);
    assert(recoHitInfo);
    if (testHitWeight(recoHitInfo)) {
      hitsCopied += addEKLMHit(eklmHit, calculateSortingParameter(recoHitInfo->getSortingParameter()),
                               recoHitInfo->getFoundByTrackFinder());
    }
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
  if (not dynamic_cast<const genfit::KalmanFitStatus*>(fs)) {
    return false;
  }

  // make sure there is at least one hit with a valid mSoP
  const unsigned int trackSize = m_genfitTrack.getNumPoints();
  for (unsigned int i = 0; i < trackSize; i++) {
    try {
      m_genfitTrack.getFittedState(i, representation);
      return true;
    } catch (const genfit::Exception& exception) {
      B2DEBUG(29, "Can not get mSoP because of: " << exception.what());
    }
  }

  return false;
}

void RecoTrack::prune()
{
  // "Delete" all RecoHitInfromation but the first and the last.
  // Copy is intended!
  std::vector<RelationEntry> relatedRecoHitInformations = getRelationsWith<RecoHitInformation>
                                                          (m_storeArrayNameOfRecoHitInformation).relations();
  std::sort(relatedRecoHitInformations.begin(), relatedRecoHitInformations.end(), [](const RelationEntry & lhs,
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

genfit::AbsTrackRep* RecoTrackGenfitAccess::createOrReturnRKTrackRep(RecoTrack& recoTrack, int PDGcode)
{
  // try to get the trackRep, if it has already been added
  genfit::AbsTrackRep* trackRepresentation = recoTrack.getTrackRepresentationForPDG(std::abs(PDGcode));

  // not available? create one
  if (trackRepresentation == nullptr) {
    if (PDGcode == Monopoles::c_monopolePDGCode) {
      trackRepresentation = new genfit::MplTrackRep(PDGcode, Monopoles::monopoleMagCharge);
    } else {
      trackRepresentation = new genfit::RKTrackRep(PDGcode);
    }
    RecoTrackGenfitAccess::getGenfitTrack(recoTrack).addTrackRep(trackRepresentation);
  }
  return trackRepresentation;
}

const genfit::MeasuredStateOnPlane& RecoTrack::getMeasuredStateOnPlaneClosestTo(const ROOT::Math::XYZVector& closestPoint,
    const genfit::AbsTrackRep* representation)
{
  checkDirtyFlag();
  const unsigned int numberOfPoints = m_genfitTrack.getNumPointsWithMeasurement();

  assert(numberOfPoints > 0);

  const genfit::MeasuredStateOnPlane* nearestStateOnPlane = nullptr;
  double minimalDistance2 = 0;
  for (unsigned int hitIndex = 0; hitIndex < numberOfPoints; hitIndex++) {
    try {
      const genfit::MeasuredStateOnPlane& measuredStateOnPlane = m_genfitTrack.getFittedState(hitIndex, representation);

      const double currentDistance2 = (ROOT::Math::XYZVector(measuredStateOnPlane.getPos()) - closestPoint).Mag2();

      if (not nearestStateOnPlane or currentDistance2 < minimalDistance2) {
        nearestStateOnPlane = &measuredStateOnPlane;
        minimalDistance2 = currentDistance2;
      }
    } catch (const genfit::Exception& exception) {
      B2DEBUG(50, "Can not get mSoP because of: " << exception.what());
      continue;
    }
  }

  // catch case no hit has measured state on plane (not sure how likely that is) which would lead to undefined behavior
  if (not nearestStateOnPlane) {
    B2WARNING("Non of the track hits had a MeasuredStateOnPlane! Exception thrown.");
    throw  NoStateOnPlaneFound();
  }

  return *nearestStateOnPlane;
}


void RecoTrack::deleteFittedInformation()
{
  // Delete all fitted information for all representations
  for (const genfit::AbsTrackRep* rep : getRepresentations()) {
    deleteFittedInformationForRepresentation(rep);
  }
}

void RecoTrack::deleteFittedInformationForRepresentation(const genfit::AbsTrackRep* rep)
{
  m_genfitTrack.deleteFittedState(rep);
}

genfit::AbsTrackRep* RecoTrack::getTrackRepresentationForPDG(int pdgCode) const
{
  if (pdgCode < 0) {
    B2FATAL("Only positive pdgCode is possible when calling getTrackRepresentationForPDG, got " << pdgCode);
  }

  const std::vector<genfit::AbsTrackRep*>& trackRepresentations = getRepresentations();

  for (genfit::AbsTrackRep* trackRepresentation : trackRepresentations) {
    // Check if the track representation is a RKTrackRep.
    const genfit::RKTrackRep* rkTrackRepresenation = dynamic_cast<const genfit::RKTrackRep*>(trackRepresentation);
    if (rkTrackRepresenation != nullptr) {
      // take the aboslute value of the PDG code as the TrackRep holds the PDG code including the charge (so -13 or 13)
      if (std::abs(rkTrackRepresenation->getPDG()) == pdgCode) {
        return trackRepresentation;
      }
    }
  }

  return nullptr;
}


/// Helper function to get the seed or the measured state on plane from a track
std::tuple<ROOT::Math::XYZVector, ROOT::Math::XYZVector, short> RecoTrack::extractTrackState() const
{
  if (not wasFitSuccessful()) {
    return std::make_tuple(getPositionSeed(), getMomentumSeed(), getChargeSeed());
  } else {
    const auto& measuredStateOnPlane = getMeasuredStateOnPlaneFromFirstHit();
    return std::make_tuple(ROOT::Math::XYZVector(measuredStateOnPlane.getPos()), ROOT::Math::XYZVector(measuredStateOnPlane.getMom()),
                           measuredStateOnPlane.getCharge());
  }
}

RecoTrack* RecoTrack::copyToStoreArrayUsing(StoreArray<RecoTrack>& storeArray,
                                            const ROOT::Math::XYZVector& position, const ROOT::Math::XYZVector& momentum, short charge,
                                            const TMatrixDSym& covariance, double timeSeed) const
{
  RecoTrack* newRecoTrack = storeArray.appendNew(position, momentum, charge,
                                                 getStoreArrayNameOfPXDHits(), getStoreArrayNameOfSVDHits(), getStoreArrayNameOfCDCHits(),
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
    return copyToStoreArrayUsing(storeArray,
                                 ROOT::Math::XYZVector(mSoP.getPos()),
                                 ROOT::Math::XYZVector(mSoP.getMom()),
                                 static_cast<short>(mSoP.getCharge()),
                                 mSoP.get6DCov(), mSoP.getTime());
  } else {
    return copyToStoreArrayUsingSeeds(storeArray);
  }
}

bool RecoTrack::hasTrackFitStatus(const genfit::AbsTrackRep* representation) const
{
  checkDirtyFlag();

  // there might be the case, where the genfit track has no trackreps, even not the cardinal
  // one because no fit attempt was performed. In this case, the "hasFitStatus" call to genfit
  // will fail with an access violation. To prevent that, check for the number of reps here before
  // actually calling genfit's hasFitStatus(...)
  if (m_genfitTrack.getNumReps() == 0)
    return false;

  return m_genfitTrack.hasFitStatus(representation);
}

std::vector<RecoHitInformation*> RecoTrack::getRecoHitInformations(bool getSorted) const
{
  std::vector<RecoHitInformation*> hitList;
  RelationVector<RecoHitInformation> recoHitInformations = getRelationsTo<RecoHitInformation>
                                                           (m_storeArrayNameOfRecoHitInformation);

  hitList.reserve(recoHitInformations.size());
  for (auto& recoHit : recoHitInformations) {
    hitList.push_back(&recoHit);
  }

  // sort the returned vector if requested
  if (getSorted) {
    std::sort(hitList.begin(), hitList.end(), [](const RecoHitInformation * a,
    const RecoHitInformation * b) -> bool {
      return a->getSortingParameter() < b->getSortingParameter();
    });
  }

  return hitList;
}

const genfit::MeasuredStateOnPlane& RecoTrack::getMeasuredStateOnPlaneFromRecoHit(const RecoHitInformation* recoHitInfo,
    const genfit::AbsTrackRep* representation) const
{
  checkDirtyFlag();

  if (!hasTrackFitStatus(representation)) {
    B2FATAL("MeasuredStateOnPlane can not be retrieved for RecoTracks where no fit has been attempted.");
  }

  if (!recoHitInfo->useInFit()) {
    B2FATAL("MeasuredStateOnPlane cannot be provided for RecoHit which was not used in the fit.");
  }

  const auto* hitTrackPoint = getCreatedTrackPoint(recoHitInfo);
  if (not hitTrackPoint) {
    B2FATAL("TrackPoint was requested which has not been created");
  }

  const auto* fittedResult = hitTrackPoint->getFitterInfo(representation);
  if (not fittedResult) {
    throw NoTrackFitResult();
  }

  return fittedResult->getFittedState();
}

const genfit::MeasuredStateOnPlane& RecoTrack::getMeasuredStateOnPlaneFromFirstHit(const genfit::AbsTrackRep* representation) const
{
  const unsigned int trackSize = m_genfitTrack.getNumPoints();
  for (unsigned int i = 0; i < trackSize; i++) {
    try {
      return m_genfitTrack.getFittedState(i, representation);
    } catch (const genfit::Exception& exception) {
      B2DEBUG(28, "Can not get mSoP because of: " << exception.what());
    }
  }

  B2FATAL("There is no single hit with a valid mSoP in this track! Check if the fit failed with wasFitSuccessful before");
}

const genfit::MeasuredStateOnPlane& RecoTrack::getMeasuredStateOnPlaneFromLastHit(const genfit::AbsTrackRep* representation) const
{
  int trackSize = m_genfitTrack.getNumPoints();
  for (int i = -1; i >= -trackSize; i--) {
    try {
      return m_genfitTrack.getFittedState(i, representation);
    } catch (const genfit::Exception& exception) {
      B2DEBUG(28, "Can not get mSoP because of: " << exception.what());
    }
  }

  B2FATAL("There is no single hit with a valid mSoP in this track!");
}

void RecoTrack::estimateArmTime()
{
  m_isArmTimeComputed = true;
  const std::vector<RecoHitInformation*>& recoHits = getRecoHitInformations(true);
  bool svdDone = false;
  int nSVDHits = 0;
  static RecoHitInformation::RecoHitDetector und = RecoHitInformation::RecoHitDetector::c_undefinedTrackingDetector;
  RecoHitInformation::RecoHitDetector SVD = RecoHitInformation::RecoHitDetector::c_SVD;
  RecoHitInformation::RecoHitDetector detIDpre = und;
  RecoHitInformation::RecoHitDetector detIDpost = und;
  float clusterTimeSum = 0;
  float clusterTimeSigma2Sum = 0;
  bool trackArmTimeDone = false;

  // loop over the recoHits of the RecoTrack
  for (const auto& recoHit : recoHits) {
    RecoHitInformation::RecoHitDetector foundin = recoHit->getTrackingDetector();
    if (!svdDone && foundin != SVD) {
      detIDpre = foundin;
      trackArmTimeDone = false;
    }
    if (foundin == SVD) {
      RelationVector<SVDCluster> svdClusters = recoHit->getRelationsTo<SVDCluster>(m_storeArrayNameOfSVDHits);
      clusterTimeSum += svdClusters[0]->getClsTime();
      clusterTimeSigma2Sum += svdClusters[0]->getClsTimeSigma() * svdClusters[0]->getClsTimeSigma();
      nSVDHits += 1;
      svdDone = true;
    } else {
      // Compute the track arm times using SVD cluster times
      if (svdDone && nSVDHits > 1) {
        detIDpost = foundin;
        if (!isOutgoingArm(detIDpre, detIDpost)) {
          m_ingoingArmTime = clusterTimeSum / nSVDHits;
          m_ingoingArmTimeError = std::sqrt(clusterTimeSigma2Sum / (nSVDHits * (nSVDHits - 1)));
          m_hasIngoingArmTime = true;
          m_nSVDHitsOfIngoingArm = nSVDHits;
        } else {
          m_outgoingArmTime = clusterTimeSum / nSVDHits;
          m_outgoingArmTimeError = std::sqrt(clusterTimeSigma2Sum / (nSVDHits * (nSVDHits - 1)));
          m_hasOutgoingArmTime = true;
          m_nSVDHitsOfOutgoingArm = nSVDHits;
        }
        svdDone = false;
        detIDpre = detIDpost;
        detIDpost = und;
        clusterTimeSum = 0;
        nSVDHits = 0;
        trackArmTimeDone = true;
      }
    }

    // When the last recoHit is SVD, it does not enter in the else{} of if (detID == SVD) {...} else {...}
    // where the track arm times are calculated, so they are calculated here.
    // It will not reset all variables because it is run only at the last recoHit
    if (!trackArmTimeDone && (recoHit == recoHits.back()) && nSVDHits > 1) {
      if (!isOutgoingArm(detIDpre, detIDpost)) {
        m_ingoingArmTime = clusterTimeSum / nSVDHits;
        m_ingoingArmTimeError = std::sqrt(clusterTimeSigma2Sum / (nSVDHits * (nSVDHits - 1)));
        m_hasIngoingArmTime = true;
        m_nSVDHitsOfIngoingArm = nSVDHits;
      } else {
        m_outgoingArmTime = clusterTimeSum / nSVDHits;
        m_outgoingArmTimeError = std::sqrt(clusterTimeSigma2Sum / (nSVDHits * (nSVDHits - 1)));
        m_hasOutgoingArmTime = true;
        m_nSVDHitsOfOutgoingArm = nSVDHits;
      }
    }
  }
}

bool RecoTrack::isOutgoingArm(RecoHitInformation::RecoHitDetector pre, RecoHitInformation::RecoHitDetector post)
{
  static RecoHitInformation::RecoHitDetector und = RecoHitInformation::RecoHitDetector::c_undefinedTrackingDetector;
  RecoHitInformation::RecoHitDetector PXD = RecoHitInformation::RecoHitDetector::c_PXD;
  RecoHitInformation::RecoHitDetector CDC = RecoHitInformation::RecoHitDetector::c_CDC;
  bool isOutgoing = true;
  if (pre == PXD && post == CDC) isOutgoing = true;
  else if (pre == und && post == CDC) isOutgoing = true;
  else if (pre == PXD && post == und) isOutgoing = true;
  else if (pre == CDC && post == PXD) isOutgoing = false;
  else if (pre == und && post == PXD) isOutgoing = false;
  else if (pre == CDC && post == und) isOutgoing = false;
  else {
    //TO DO
    B2DEBUG(29,
            "SVD-only? PXD-SVD-PXD??? --- use layer information to determine if the track arm is outgoing or ingoing! Considered --> 'OUT'");
    isOutgoing = true;
  }
  return isOutgoing;
}

void RecoTrack::flipTrackDirectionAndCharge(const genfit::AbsTrackRep* representation)
{
  const genfit::MeasuredStateOnPlane& measuredStateOnPlane = getMeasuredStateOnPlaneFromLastHit(representation);
  const ROOT::Math::XYZVector& fittedPosition = ROOT::Math::XYZVector(measuredStateOnPlane.getPos());
  const ROOT::Math::XYZVector& fittedMomentum = ROOT::Math::XYZVector(measuredStateOnPlane.getMom());
  const double& fittedCharge = measuredStateOnPlane.getCharge();

  // revert the charge and momentum
  setChargeSeed(-fittedCharge);
  setPositionAndMomentum(fittedPosition, -fittedMomentum);
  revertRecoHitInformationSorting();
  swapArmTimes();
  setDirtyFlag();
}

std::string RecoTrack::getInfoHTML() const
{
  std::stringstream out;

  out << "<b>Charge seed</b>=" << getChargeSeed();

  out << "<b>pT seed</b>=" << getMomentumSeed().Rho();
  out << ", <b>pZ seed</b>=" << getMomentumSeed().Z();
  out << "<br>";
  out << "<b>position seed</b>=" << getMomentumSeed().X() << ", " << getMomentumSeed().Y() << ", " << getMomentumSeed().Z();
  out << "<br>";

  for (const genfit::AbsTrackRep* rep : getRepresentations()) {
    out << "<b>was fitted with " << rep->getPDG() << "</b>=" << wasFitSuccessful() << ", ";
  }
  out << "<br>";

  return out.str();
}
