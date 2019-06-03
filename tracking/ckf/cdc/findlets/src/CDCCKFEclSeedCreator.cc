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

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "seedDeadRegionPhi"),
                                m_param_seedDeadRegionPhi,
                                "Area in phi around seed in which additional showers are ignored",
                                m_param_seedDeadRegionPhi);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "seedDeadRegionTheta"),
                                m_param_seedDeadRegionTheta,
                                "Area in theta around seed in which additional showers are ignored",
                                m_param_seedDeadRegionTheta);
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
  std::vector<ECLShower> goodShowers;

  // TODO: Improve this loop. It is not necessary to create the "goodShowers" first.
  // Avoid Bremsstrahlung showers by simple dR criterion
  for (auto& shower : m_inputECLshowers) {
    if (shower.getHypothesisId() != ECLShower::c_nPhotons) {
      continue;
    }

    const double Eclus  = shower.getEnergy();
    if (Eclus < m_param_minimalEnRequirement) {
      continue;
    }

    bool addShower = true;
    for (auto& shower2 : m_inputECLshowers) {
      if (shower2.getHypothesisId() == ECLShower::c_nPhotons && Eclus < shower2.getEnergy()) {
        if (std::abs(TVector2::Phi_mpi_pi(shower2.getPhi() - shower.getPhi())) < m_param_seedDeadRegionPhi
            && std::abs(shower2.getTheta() - shower.getTheta()) < m_param_seedDeadRegionTheta) {
          addShower = false;
          break;
        }
      }
    }
    if (addShower) {
      goodShowers.push_back(shower);
    }

  }

  // loop over all showers and create seed objects
  for (auto& shower : m_inputECLshowers) {
    if (shower.getHypothesisId() != ECLShower::c_nPhotons) {
      continue;
    }

    const double Eclus  = shower.getEnergy();
    if (Eclus < m_param_minimalEnRequirement) {
      continue;
    }

    // find the good showers selected above
    bool match = false;
    for (auto& goodShower : goodShowers) {
      if (std::abs(goodShower.getEnergy() - shower.getEnergy()) < 0.0001) {
        match = true;
      }
    }
    if (!match) {
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
    TVector3 mom(Eclus * sinTheta * cosPhi, Eclus * sinTheta * sinPhi, Eclus * cosTheta);

    // Shower assumed to be in 12cm depth
    // pos = pos - 12. / mom.Mag() * mom;

    // Find center of circular trajectory
    double rad = std::abs(mom.Pt() / (29.9792458 * 1e-4 * 1. *
                                      1.5)); // factor 1. for charge // speed of light in [cm per ns] // const magnetic field

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
    double covArray[6];
    shower.getCovarianceMatrixAsArray(covArray);
    // Calculate uncertainties on position from ECLShower
    double dx2 = rClus * cosTheta * cosPhi * rClus * cosTheta * cosPhi * covArray[5]
                 + rClus * sinTheta * sinPhi * rClus * sinTheta * sinPhi * covArray[2];
    double dy2 = rClus * cosTheta * sinPhi * rClus * cosTheta * sinPhi * covArray[5]
                 + rClus * sinTheta * cosPhi * rClus * sinTheta * cosPhi * covArray[2];
    double dz2 = rClus * sinTheta * rClus * sinTheta * covArray[5];

    // As no helix extrapolation is done this is only a very rough momentum estimate
    // Also, Bremsstrahlung is not considered so far
    // Set cov matrix to high values!
    /*
    double dpx2 = Eclus * cosTheta * cosPhi * Eclus * cosTheta * cosPhi * covArray[5]
                 + Eclus * sinTheta * sinPhi * Eclus * sinTheta * sinPhi * covArray[2]
                 + sinTheta * cosPhi * covArray[0];
    double dpy2 = Eclus * cosTheta * sinPhi * Eclus * cosTheta * sinPhi * covArray[5]
                 + Eclus * sinTheta * cosPhi * Eclus * sinTheta * cosPhi * covArray[2]
                 + sinTheta * sinPhi * covArray[0];
    double dpz2 = Eclus * sinTheta * Eclus * sinTheta * covArray[5]
                 + cosTheta * covArray[0];
    */
    // double dpx2 = 0.25 * 0.25 * mom.X() * mom.X();
    // double dpy2 = 0.25 * 0.25 * mom.Y() * mom.Y();
    double dpx2 = std::abs(mom1.X() - mom2.X()) / 2.0 * std::abs(mom1.X() - mom2.X()) / 4.0;
    double dpy2 = std::abs(mom1.Y() - mom2.Y()) / 2.0 * std::abs(mom1.Y() - mom2.Y()) / 4.0;
    double dpz2 = 0.25 * 0.25 * mom.Z() * mom.Z();

    cov(0, 0) = dx2;
    cov(1, 1) = dy2;
    cov(2, 2) = dz2;
    cov(3, 3) = dpx2;
    cov(4, 4) = dpy2;
    cov(5, 5) = dpz2;
    // cov(3, 4) = -sqrt(dpx2*dpy2);
    // cov(4, 3) = -sqrt(dpx2*dpy2);

    genfit::SharedPlanePtr planeNeg(new genfit::DetPlane(pos, pos));
    genfit::SharedPlanePtr planePos(new genfit::DetPlane(pos, pos));
    msopNeg.setPosMomCov(pos, mom, cov);
    msopNeg.setPlane(planeNeg);
    msopPos.setPosMomCov(pos, mom, cov);
    msopPos.setPlane(planePos);

    //B2INFO("Theta: " << thetaClus * 180 / M_PI);
    //B2INFO("En: " << Eclus);
    //B2INFO("- Pos: " << pos.X() << " (" << sqrt(dx2) << "), " << pos.Y() << " (" << sqrt(dy2) << "), " << pos.Z() << " (" << sqrt(dz2) << ")");
    //B2INFO("- Mom: " << mom.X() << " (" << sqrt(dpx2) << "), " << mom.Y() << " (" << sqrt(dpy2) << "), " << mom.Z() << " (" << sqrt(dpz2) << ")");
    //B2INFO("- Mom(Old): " << mom.X() << " (" << sqrt(0.25 * 0.25 * mom.X() * mom.X()) << "), " << mom.Y() << " (" << sqrt(0.25 * 0.25 * mom.Y() * mom.Y()) << "), " << mom.Z() << " (" << sqrt(dpz2) << ")");

    // create CDCCKF states
    CDCCKFState seedStateNeg(eclSeedNeg, msopNeg);
    seeds.push_back({seedStateNeg});
    CDCCKFState seedStatePos(eclSeedPos, msopPos);
    seeds.push_back({seedStatePos});
  }
}
