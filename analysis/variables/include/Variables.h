/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/MCParticle.h>
#include <vector>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * return momentum magnitude
     */
    double particleP(const Particle* part);

    /**
     * return energy
     */
    double particleE(const Particle* part);


    /**
     * return momentum component x
     */
    double particlePx(const Particle* part);

    /**
     * return momentum component y
     */
    double particlePy(const Particle* part);

    /**
     * return momentum component z
     */
    double particlePz(const Particle* part);

    /**
     * return transverse momentum
     */
    double particlePt(const Particle* part);

    /**
     * return error of momentum
     */
    double particlePErr(const Particle* part);

    /**
     * return error of momentum component x
     */
    double particlePxErr(const Particle* part);

    /**
     * return error of momentum component y
     */
    double particlePyErr(const Particle* part);

    /**
     * return error of momentum component z
     */
    double particlePzErr(const Particle* part);

    /**
     * return error of transverse momentum
     */
    double particlePtErr(const Particle* part);

    /**
     * return polar angle
     */
    double particleTheta(const Particle* part);

    /**
     * return error of polar angle
     */
    double particleThetaErr(const Particle* part);

    /**
     * return momentum cosine of polar angle
     */
    double particleCosTheta(const Particle* part);

    /**
     * return error of momentum cosine of polar angle
     */
    double particleCosThetaErr(const Particle* part);

    /**
     * return momentum azimuthal angle
     */
    double particlePhi(const Particle* part);

    /**
     * return error of momentum azimuthal angle
     */
    double particlePhiErr(const Particle* part);

    /**
     * return the particle scaled momentum, i.e. the particle's momentum divided by the
     * maximum momentum allowed for a particle of its mass.
     */
    double particleXp(const Particle* part);

    /**
     * return particle's pdg code
     */
    double particlePDGCode(const Particle* part);

    /**
     * return cosine of angle between momentum and vertex vector in particle xy-plane in LAB frame (origin of vertex vector is IP)
     */
    double cosAngleBetweenMomentumAndVertexVectorInXYPlane(const Particle* part);

    /**
     * return cosine of angle between momentum and vertex vector of particle in LAB frame (origin of vertex vector is IP)
     */
    double cosAngleBetweenMomentumAndVertexVector(const Particle* part);

    /**
     * cosine of the angle in CMS between momentum the reconstructed particle and a nominal B particle.
     * It is somewhere between -1 and 1 if only a single massless particle like a neutrino is missing in the reconstruction.
     */
    double cosThetaBetweenParticleAndNominalB(const Particle* part);

    /**
     * Returns the cosine of the angle between the particle and the thrust axis
     * of the event, as calculate by the EventShapeCalculator module.
     */
    double cosToThrustOfEvent(const Particle* part);

    /**
     * return the (i,j)-th element of the MomentumVertex covariance matrix
     * Ordering of the elements in the error matrix is: px, py, pz, E, x, y, z
     */
    double covMatrixElement(const Particle*, const std::vector<double>& element);

    /**
     * return momentum deviation chi^2 value calculated as
     * chi^2 = sum_i (p_i - mc(p_i))^2/sigma(p_i)^2, where sum runs over i = px, py, pz and
     * mc(p_i) is the mc truth value and sigma(p_i) is the estimated error of i-th component of momentum vector
     */
    double momentumDeviationChi2(const Particle*);

    /**
     * return mass (determined from particle's 4-momentum vector)
     */
    double particleMass(const Particle* part);

    /**
     * return mass minus nominal mass
     */
    double particleDMass(const Particle* part);

    /**
     * return mass (determined from particle's daughter 4-momentum vectors)
     */
    double particleInvariantMassFromDaughters(const Particle* part);

    /**
     * return mass (determined from particle's daughter 4-momentum vectors under proton mass assumption)
     */
    double particleInvariantMassLambda(const Particle* part);

    /**
     * return uncertainty of the invariant mass
     */
    double particleInvariantMassError(const Particle* part);

    /**
     * return signed deviation of particle's invariant mass from its nominal mass in units of uncertainty on the invariant mass
     */
    double particleInvariantMassSignificance(const Particle* part);

    /**
     * return mass squared (determined from particle's 4-momentum vector)
     */
    double particleMassSquared(const Particle* part);

    /**
     * return released energy in decay
     */
    double particleQ(const Particle* part);

    /**
     * return released energy in decay minus nominal one
     */
    double particleDQ(const Particle* part);

    /**
     * return beam constrained mass
     */
    double particleMbc(const Particle* part);

    /**
     * return energy difference in CMS
     */
    double particleDeltaE(const Particle* part);

    /**
     * return component x of 3-momentum recoiling against given Particle
     */
    double recoilPx(const Particle* particle);

    /**
     * return component y of 3-momentum recoiling against given Particle
     */
    double recoilPy(const Particle* particle);

    /**
     * return component z of 3-momentum recoiling against given Particle
     */
    double recoilPz(const Particle* particle);

    /**
     * return magnitude of 3-momentum recoiling against given Particle
     */
    double recoilMomentum(const Particle* particle);

    /**
     * returns the polar angle of the missing momentum vector between the beam and the particle in the lab system
     */
    double recoilMomentumTheta(const Particle* part);

    /**
     * returns the azimuthal angle of the missing momentum vector between the beam and the particle in the lab system
     */
    double recoilMomentumPhi(const Particle* part);

    /**
     * returns the squared missing mass of the signal side which is calculated in the CMS frame under the assumption that the signal and
     * tag side are produced back to back and the tag side energy equals the beam energy. The variable must be applied to the Upsilon and
     * the tag side must be the first, the signal side the second daughter!
     */
    double m2RecoilSignalSide(const Particle* part);

    /**
     * returns the impact parameter D of the given particle in the xy plane
     */
    double ImpactXY(const Particle* particle);

    /**
     * return energy recoiling against given Particle
     */
    double recoilEnergy(const Particle* particle);

    /**
     * return invariant mass of the system recoiling against given Particle
     */
    double recoilMass(const Particle* particle);

    /**
     *
     * return invariant mass squared of the system recoiling against given Particle
     */
    double recoilMassSquared(const Particle* particle);

    /**
     * Returns the decay type of recoil particle (meant for B mesons)
     * No related mcparticle = -1
     * Hadronic = 0
     * Direct leptonic = 1
     * Direct semileptonic = 2
     * Lower level leptonic = 3
     */
    double recoilMCDecayType(const Particle* particle);

    /**
     * Helper function: Returns decay type of MCParticle
     */
    void checkMCParticleDecay(MCParticle* mcp, int& decayType, bool recursive);

    /**
     * return always zero
     */
    double False(const Particle*);

    /**
     * return always one
     */
    double True(const Particle*);
    /**
     * return std::numeric_limits<double>::infinity()
     */
    double infinity(const Particle*);

    /**
     * return a random number between 0 and 1 for each candidate
     */
    double random(const Particle*);

    /**
     * return a random number between 0 and 1 for each event
     */
    double eventRandom(const Particle*);

    /**
     * returns the theta angle (lab) that is back-to-back (cms) to the particle
     */
    double b2bTheta(const Particle* particle);

    /**
     * returns the phi angle (lab) that is back-to-back (cms) to the particle
     */
    double b2bPhi(const Particle* particle);

    /**
     * returns the theta angle (lab) that is back-to-back (cms) to the cluster
     */
    double b2bClusterTheta(const Particle* particle);

    /**
     * returns the phi angle (lab) that is back-to-back (cms) to the cluster
     */
    double b2bClusterPhi(const Particle* particle);

    /**
     * returns the longitudinal momentum asymmetry
     * alpha = (p_{L}^{+} - p_{L}^{-}) / (p_{L}^{+} - p_{L}^{-})
     * for the Armenteros plot.
     * The particle (mother) is required to have exactly two daughters.
     * In case the two daughters have same charge it will return
     * alpha = (p_{L}^{d1} - p_{L}^{d2}) / (p_{L}^{d1} - p_{L}^{d2})
     * where d1 is the first daughter, d2 the second daughter.
     */
    double ArmenterosLongitudinalMomentumAsymmetry(const Particle* part);

    /**
     * returns the transverse momentum of the first daughter with
     * respect to the V0 mother
     */
    double ArmenterosDaughter1Qt(const Particle* part);

    /**
     * returns the transverse momentum of the second daughter with
     * respect to the V0 mother
     */
    double ArmenterosDaughter2Qt(const Particle* part);
  }
} // Belle2 namespace
