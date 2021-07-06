/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/findlets/CDCCKFEclSeedCreator.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/ckf/cdc/entities/CDCCKFState.h>

#include <framework/core/ModuleParamList.h>

#include <framework/gearbox/Const.h>

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

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "restrictToForwardSeeds"),
                                m_param_restrictToForwardSeeds,
                                "Don't do Ecl seeding in central region to save computing time",
                                m_param_restrictToForwardSeeds);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "tanLambdaForwardNeg"),
                                m_param_tanLambdaForwardNeg,
                                "Up to which (neg) tanLambda value should the seeding be performed",
                                m_param_tanLambdaForwardNeg);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "tanLambdaForwardPos"),
                                m_param_tanLambdaForwardPos,
                                "Up to which (pos) tanLambda value should the seeding be performed",
                                m_param_tanLambdaForwardPos);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "showerDepth"),
                                m_param_showerDepth,
                                "Don't do Ecl seeding in central region to save computing time",
                                m_param_showerDepth);
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
  // loop over all showers and create seed objects
  for (auto& shower : m_inputECLshowers) {
    if (shower.getHypothesisId() != ECLShower::c_nPhotons) {
      continue;
    }

    const double Eclus  = shower.getEnergy();
    if (Eclus < m_param_minimalEnRequirement) {
      continue;
    }

    // Get shower position, momentum
    const double thetaClus  = shower.getTheta();
    const double phiClus  = shower.getPhi();
    const double rClus  = shower.getR();

    const double sinTheta = sin(thetaClus);
    const double cosTheta = cos(thetaClus);
    const double sinPhi = sin(phiClus);
    const double cosPhi = cos(phiClus);

    TVector3 pos(rClus * sinTheta * cosPhi, rClus * sinTheta * sinPhi, rClus * cosTheta);
    const double tanLambda = pos.Z() / pos.Perp();

    // restrict to forward seeds
    if (m_param_restrictToForwardSeeds) {
      if (tanLambda > m_param_tanLambdaForwardNeg && tanLambda < m_param_tanLambdaForwardPos) {
        continue;
      }
    }

    // Correction if shower is assumed to start in a certain depth
    pos = pos - m_param_showerDepth / pos.Mag() * pos;

    TVector3 mom = Eclus / pos.Mag() * pos;

    // Calculate helix trajectory for negative and positive charge seeds
    // Find center of circular trajectory
    // factor 1. for charge // speed of light in [cm per ns] // const magnetic field
    double rad = std::abs(mom.Pt() / (Const::speedOfLight * 1e-4 * 1. * 1.5));

    // Particle would not be able to reach ECL (underestimation of shower energy)
    if (2. * rad < pos.Perp()) {
      rad = pos.Perp() / 2.0 + 1.0;
    }

    // Use pq formula (center of circle has to be on perpendicular line through center of line between (0,0) and seed position)
    double q = pos.Perp();
    double y3 = pos.Y() / 2.0;
    double x3 = pos.X() / 2.0;

    double basex = sqrt(rad * rad - q * q / 4.0) * (-pos.Y()) / q;
    double basey = sqrt(rad * rad - q * q / 4.0) * pos.X() / q;

    double centerx1 = x3 + basex;
    double centery1 = y3 + basey;
    double centerx2 = x3 - basex;
    double centery2 = y3 - basey;

    // vectors for tangent at seed position (perpendicular to radius at seed position)
    double momx1 = pos.Y() - centery1;
    double momy1 = - (pos.X() - centerx1);
    double momx2 = pos.Y() - centery2;
    double momy2 = - (pos.X() - centerx2);

    // two solutions (pointing toward and away from center)
    // make sure that particle moves inwards
    if (momx1 * pos.X() + momy1 * pos.Y() < 0) {
      momx1 = -momx1;
      momy1 = -momy1;
    }
    if (momx2 * pos.X() + momy2 * pos.Y() < 0) {
      momx2 = -momx2;
      momy2 = -momy2;
    }

    // scale to unit length
    double mom1abs = sqrt(momx1 * momx1 + momy1 * momy1);
    double mom2abs = sqrt(momx2 * momx2 + momy2 * momy2);
    momx1 = momx1 / mom1abs;
    momy1 = momy1 / mom1abs;
    momx2 = momx2 / mom2abs;
    momy2 = momy2 / mom2abs;

    TVector3 mom1(momx1 * mom.Perp(), momy1 * mom.Perp(), mom.Z());
    TVector3 mom2(momx2 * mom.Perp(), momy2 * mom.Perp(), mom.Z());

    // Pick the right momentum for positive/negative charge
    // Use cross product if momentum vector is (counter)clockwise wrt position vector
    bool clockwise1 = true;
    bool clockwise2 = true;
    if (pos.Y() * mom1.X() - pos.X() * mom1.Y() > 0) {
      clockwise1 = false;
    }
    if (pos.Y() * mom2.X() - pos.X() * mom2.Y() > 0) {
      clockwise2 = false;
    }

    if (clockwise1 == clockwise2) {
      B2WARNING("Something went wrong during helix extrapolation. Skipping track.");
      continue;
    }

    TVector3 mompos;
    TVector3 momneg;
    if (clockwise1) {
      mompos = mom2;
      momneg = mom1;
    } else {
      mompos = mom1;
      momneg = mom2;
    }

    // electron and positron hypothesis
    RecoTrack* eclSeedNeg = m_eclSeedRecoTracks.appendNew(pos, momneg, -1);
    eclSeedNeg->addRelationTo(&shower);
    RecoTrack* eclSeedPos = m_eclSeedRecoTracks.appendNew(pos, mompos, +1);
    eclSeedPos->addRelationTo(&shower);

    // define MeasuredStateOnPlane (use pion hypothesis)
    genfit::AbsTrackRep* repNeg = RecoTrackGenfitAccess::createOrReturnRKTrackRep(*eclSeedNeg, -Const::pion.getPDGCode());
    genfit::MeasuredStateOnPlane msopNeg(repNeg);
    genfit::AbsTrackRep* repPos = RecoTrackGenfitAccess::createOrReturnRKTrackRep(*eclSeedPos, Const::pion.getPDGCode());
    genfit::MeasuredStateOnPlane msopPos(repPos);

    // set position, momentum, cov, sharedPlanePtr
    TMatrixDSym cov(6);
    double covArray[6];
    shower.getCovarianceMatrixAsArray(covArray);

    // Calculate uncertainties on position from ECLShower
    double dx2 = rClus * cosTheta * cosPhi * rClus * cosTheta * cosPhi * covArray[5]
                 + rClus * sinTheta * sinPhi * rClus * sinTheta * sinPhi * covArray[2];
    double dy2 = rClus * cosTheta * sinPhi * rClus * cosTheta * sinPhi * covArray[5]
                 + rClus * sinTheta * cosPhi * rClus * sinTheta * cosPhi * covArray[2];
    double dz2 = rClus * sinTheta * rClus * sinTheta * covArray[5];

    double dpx2 = std::abs(mom1.X() - mom2.X()) / 4.0 * std::abs(mom1.X() - mom2.X()) / 4.0;
    double dpy2 = std::abs(mom1.Y() - mom2.Y()) / 4.0 * std::abs(mom1.Y() - mom2.Y()) / 4.0;
    double dpz2 = 0.25 * 0.25 * mom.Z() * mom.Z();

    cov(0, 0) = dx2;
    cov(1, 1) = dy2;
    cov(2, 2) = dz2;
    cov(3, 3) = dpx2;
    cov(4, 4) = dpy2;
    cov(5, 5) = dpz2;

    // set properties of genfit objects
    genfit::SharedPlanePtr planeNeg(new genfit::DetPlane(pos, pos));
    genfit::SharedPlanePtr planePos(new genfit::DetPlane(pos, pos));
    msopNeg.setPosMomCov(pos, momneg, cov);
    msopNeg.setPlane(planeNeg);
    msopPos.setPosMomCov(pos, mompos, cov);
    msopPos.setPlane(planePos);

    // create CDCCKF states
    CDCCKFState seedStateNeg(eclSeedNeg, msopNeg);
    seeds.push_back({seedStateNeg});
    CDCCKFState seedStatePos(eclSeedPos, msopPos);
    seeds.push_back({seedStatePos});
  }
}
