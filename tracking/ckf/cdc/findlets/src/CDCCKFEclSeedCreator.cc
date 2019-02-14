/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Simon Kurz, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/findlets/CDCCKFEclSeedCreator.h>

#include <tracking/ckf/general/utilities/SearchDirection.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/ckf/cdc/entities/CDCCKFState.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;

CDCCKFEclSeedCreator::CDCCKFEclSeedCreator() : Super()
{

}

void CDCCKFEclSeedCreator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "inputECLshowersStoreArrayName"),
                                m_param_inputEclShowerStoreArrayName,
                                "StoreArray name of the input Shower Store Array.");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "eclSeedRecoTrackStoreArrayName"),
                                m_param_eclSeedRecoTrackStoreArrayName,
                                "StoreArray name of the output Track Store Array.");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimalEnRequirementCluster"),
                                m_param_minimalEnRequirement,
                                "Minimal energy requirement for the input clusters",
                                m_param_minimalEnRequirement);
}

void CDCCKFEclSeedCreator::initialize()
{
  Super::initialize();

  m_inputECLshowers.isRequired(m_param_inputEclShowerStoreArrayName);

  m_eclSeedRecoTracks.registerInDataStore(m_param_eclSeedRecoTrackStoreArrayName);
  RecoTrack::registerRequiredRelations(m_eclSeedRecoTracks);

  m_eclSeedRecoTracks.registerRelationTo(m_inputECLshowers);
}

void CDCCKFEclSeedCreator::apply(std::vector<CDCCKFPath>& seeds)
{
  for (auto& shower : m_inputECLshowers) {
    // TODO: TrackLoader checks if RecoTrack present in relatedRecoTrackStoreArrayName. Does this have to be done?
    // Also does something with relationCheckForDirection, is this necessary?

    // choose photon hypothesis since electrons not used?! Talk to Torben.
    if (shower.getHypothesisId() != ECLShower::c_nPhotons) {
      return;
    }

    const double Eclus  = shower.getEnergy();
    if (Eclus < m_param_minimalEnRequirement) {
      continue;
    }

    // TODO: how is R defined? Is that actually what I want?
    const double thetaClus  = shower.getTheta();
    const double phiClus  = shower.getPhi();
    const double rClus  = shower.getR();

    const double sinTheta = sin(thetaClus);
    const double cosTheta = cos(thetaClus);
    const double sinPhi = sin(phiClus);
    const double cosPhi = cos(phiClus);

    const TVector3 pos(rClus * sinTheta * cosPhi, rClus * sinTheta * sinPhi, rClus * cosTheta);
    const TVector3 mom(Eclus * sinTheta * cosPhi, Eclus * sinTheta * sinPhi, Eclus * cosTheta);

    // electron and positron hypothesis
    RecoTrack* eclSeedNeg = m_eclSeedRecoTracks.appendNew(pos, mom, -1);
    eclSeedNeg->addRelationTo(&shower);
    RecoTrack* eclSeedPos = m_eclSeedRecoTracks.appendNew(pos, mom, +1);
    eclSeedPos->addRelationTo(&shower);

    // define MeasuredStateOnPlane
    genfit::AbsTrackRep* repNeg = RecoTrackGenfitAccess::createOrReturnRKTrackRep(*eclSeedNeg, -211);
    genfit::MeasuredStateOnPlane msopNeg(repNeg);
    genfit::AbsTrackRep* repPos = RecoTrackGenfitAccess::createOrReturnRKTrackRep(*eclSeedPos, 211);
    genfit::MeasuredStateOnPlane msopPos(repPos);

    // set position, momentum, cov, sharedPlanePtr
    TMatrixDSym cov(6);
    // TODO: check values of covariance matrix
    // for now: neglect correlations
    double covArray[6];
    shower.getCovarianceMatrixAsArray(covArray);
    double dx2 = rClus * cosTheta * cosPhi * rClus * cosTheta * cosPhi * covArray[5]
                 + rClus * sinTheta * sinPhi * rClus * sinTheta * sinPhi * covArray[2];
    // + what to do about R?
    double dy2 = rClus * cosTheta * sinPhi * rClus * cosTheta * sinPhi * covArray[5]
                 + rClus * sinTheta * cosPhi * rClus * sinTheta * cosPhi * covArray[2];
    // + what to do about R?
    double dz2 = rClus * sinTheta * rClus * sinTheta * covArray[5];
    // + what to do about R?
    // As no helix extrapolation is done this is only a very rough momentum estimate
    // Set cov matrix to high values!
    double dpx2 = 0.25 * mom.X() * mom.X();
    double dpy2 = 0.25 * mom.Y() * mom.Y();
    double dpz2 = 0.25 * mom.Z() * mom.Z();
    cov(0, 0) = dx2;
    cov(1, 1) = dy2;
    cov(2, 2) = dz2;
    cov(3, 3) = dpx2;
    cov(4, 4) = dpy2;
    cov(5, 5) = dpz2;
    // TODO: this is a shared pointer, so do I need to create two instances for Neg/Pos hypothesis?
    genfit::SharedPlanePtr plane(new genfit::DetPlane(pos, pos));
    msopNeg.setPosMomCov(pos, mom, cov);
    msopNeg.setPlane(plane);
    msopPos.setPosMomCov(pos, mom, cov);
    msopPos.setPlane(plane);

    //B2INFO("Pos: " << pos.X() << " (" << sqrt(dx2) << "), " << pos.Y() << " (" << sqrt(dy2) << "), " << pos.Z() << " (" << sqrt(dz2) << ")");
    //B2INFO("Mom: " << mom.X() << " (" << sqrt(dpx2) << "), " << mom.Y() << " (" << sqrt(dpy2) << "), " << mom.Z() << " (" << sqrt(dpz2) << ")");

    // create CDCCKF states
    CDCCKFState seedStateNeg(eclSeedNeg, msopNeg);
    seeds.push_back({seedStateNeg});
    CDCCKFState seedStatePos(eclSeedPos, msopPos);
    seeds.push_back({seedStatePos});
  }
}
