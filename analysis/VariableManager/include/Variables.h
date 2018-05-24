/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Thomas Keck                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
     * return cosine of angle between momentum and vertex vector in xy-plane (vector connecting ip and fitted vertex) of this particle
     */
    double cosAngleBetweenMomentumAndVertexVectorInXYPlane(const Particle* part);

    /**
     * return cosine of angle between momentum and vertex vector (vector connecting ip and fitted vertex) of this particle
     */
    double cosAngleBetweenMomentumAndVertexVector(const Particle* part);

    /**
     * cosine of the angle between momentum the particle and a true B particle. Is somewhere between -1 and 1
     * if only a massless particle like a neutrino is missing in the reconstruction.
     */
    double cosThetaBetweenParticleAndTrueB(const Particle* part);

    /**
     * If the given particle has two daughters: cosine of the angle between the line defined by the momentum difference
     * of the two daughters in the frame of the given particle (mother) and the momentum of the given particle in the lab frame.
     * If the given particle has three daughters: cosine of the angle between the normal vector of the plane
     * defined by the momenta of the three daughters in the frame of the given particle (mother) and the momentum of the given particle in the lab frame.
     * Else: 0.
     */
    double cosHelicityAngle(const Particle* part);

    /**
     * To be used for the decay pi0 -> e+ e- gamma: cosine of the angle between the momentum of the gamma in the frame
     * of the given particle (mother) and the momentum of the given particle in the lab frame.
     * Else: 0.
     */
    double cosHelicityAnglePi0Dalitz(const Particle* part);

    /**
     * return distance relative to interaction point
     */
    double particleDistance(const Particle* part);

    /**
     * return significance of distance relative to interaction point
     * (distance relative to interaction point)/ ( error on distance measurement )
     */
    double particleDistanceSignificance(const Particle* part);

    /**
     * return position in x relative to interaction point
     */
    double particleDX(const Particle* part);

    /**
     * return position in y relative to interaction point
     */
    double particleDY(const Particle* part);

    /**
     * return position in z relative to interaction point
     */
    double particleDZ(const Particle* part);

    /**
     * return transverse distance relative to interaction point
     */
    double particleDRho(const Particle* part);

    /**
     * return vertex azimuthal angle
     */
    double particleDPhi(const Particle* part);

    /**
     * return vertex polar angle
     */
    double particleDCosTheta(const Particle* part);

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
    double particleInvariantMass(const Particle* part);

    /**
     * return mass (determined from particle's daughter 4-momentum vectors under proton mass assumption)
     */
    double particleInvariantMassLambda(const Particle* part);

    /**
     * return uncertainty of the invariant mass (determined from particle's daughter 4-momentum vectors)
     */
    double particleInvariantMassError(const Particle* part);

    /**
     * return signed deviation of particle's invariant mass from its nominal mass
     */
    double particleInvariantMassSignificance(const Particle* part);

    /**
     * return signed deviation of particle's invariant mass (determined from particle's daughter 4-momentum vectors) from its nominal mass
     */
    double particleInvariantMassBeforeFitSignificance(const Particle* part);

    /**
     * Returns the cosine of the angle between the momentum of the particle and the Thrust of the event in the CM system
     */
    double cosToThrustOfEvent(const Particle* part);

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
     * return StoreArray index (0-based) of the MDST object from which the Particle was created
     */
    double particleMdstArrayIndex(const Particle* part);

    /**
     * return unique identifier for identification of Particles that are constructed from the same object in the detector (Track, energy deposit, ...)
     */
    double particleMdstSource(const Particle* part);

    /**
     * return prob(chi^2,ndf) of fit
     */
    double particlePvalue(const Particle* part);

    /**
     * return number of daughter particles
     */
    double particleNDaughters(const Particle* part);

    /**
     * return flavor type
     */
    double particleFlavorType(const Particle* part);

    /**
     * return charge
     */
    double particleCharge(const Particle* part);

    /**
    <<<<<<< Updated upstream
    =======
     * return 1 if Particle is related to initial MCParticle, 0 if Particle is related to non-initial MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCInitialParticle(const Particle* particle);

    /**
     * return 1 if Particle is related to virtual MCParticle, 0 if Particle is related to non-virtual MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCVirtualParticle(const Particle* particle);

    /**
     * return 1 if Particle is related to FSR MCParticle, 0 if Particle is related to non-FSR MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCFSRParticle(const Particle* particle);

    /**
     * return 1 if Particle is related to Photos MCParticle, 0 if Particle is related to non-Photos MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCPhotosParticle(const Particle* particle);

    /**
     * return 1 if Particle is related to ISR MCParticle, 0 if Particle is related to non-ISR MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCISRParticle(const Particle* particle);

    /**
     * return 1 if Particle is correctly reconstructed (SIGNAL), 0 otherwise
     */
    double isSignal(const Particle* part);

    /**
     * return 1 if Particle is almost correctly reconstructed (SIGNAL), 0 otherwise.
     * Misidentification of charged FSP is allowed.
     */
    double isExtendedSignal(const Particle* part);

    /**
     * return 1 if Particle is correctly reconstructed (SIGNAL including misssing neutrino), 0 otherwise
     */
    double isSignalAcceptMissingNeutrino(const Particle* part);

    /**
     * check the PDG code of a particles MC mother
     */
    double genMotherPDG(const Particle* particle);

    /**
     * check the array index of a particle's MC mother
     */
    double genMotherIndex(const Particle* particle);

    /**
     * generated momentum of a particles MC mother
     */
    double genMotherP(const Particle* particle);

    /**
     * check the array index of a particle's related MCParticle
     */
    double genParticleIndex(const Particle* particle);

    /**
     * return PDG code of matched MCParticle
     */
    double particleMCMatchPDGCode(const Particle* particle);

    /**
     * return combination of MCMatching::MCErrorFlags flags.
     */
    double particleMCErrors(const Particle* particle);

    /**
     * return the weight of the Particle -> MCParticle relation (only for the first Relation = largest weight)
     */
    double particleMCMatchWeight(const Particle* particle);

    /**
     * return the number of relations of this Particle to MCParticle
     */
    double particleNumberOfMCMatch(const Particle* particle);

    /**
     * return 1 if Particle is related to primary MCParticle, 0 if Particle is related to non-primary MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCPrimaryParticle(const Particle* particle);

    /**
     * return the true momentum transfer to lepton pair in a B (semi-) leptonic B meson decay
     */
    double particleMCMomentumTransfer2(const Particle* part);

    /**
     * return decay time of matched MCParticle (-999.0 if the particle is not matched)
     * note this is the delta time between decay of the particle and collision
     */
    double particleMCMatchDecayTime(const Particle* particle);

    /**
     * return life time of matched mc particle in CMS frame.
     */
    double particleMCMatchLifeTime(const Particle* particle);

    /**
     * return px of matched MCParticle (-999.0 if the particle is not matched)
     */
    double particleMCMatchPX(const Particle* particle);

    /**
     * return py of matched MCParticle (-999.0 if the particle is not matched)
     */
    double particleMCMatchPY(const Particle* particle);

    /**
     * return pz of matched MCParticle (-999.0 if the particle is not matched)
     */
    double particleMCMatchPZ(const Particle* particle);

    /**
     * return decay x-Vertex of matched MCParticle (-999.0 if the particle is not matched)
     */
    double particleMCMatchDX(const Particle* particle);

    /**
     * return decay y-Vertex of matched MCParticle (-999.0 if the particle is not matched)
     */
    double particleMCMatchDY(const Particle* particle);

    /**
     * return decay z-Vertex of matched MCParticle (-999.0 if the particle is not matched)
     */
    double particleMCMatchDZ(const Particle* particle);

    /**
     * return Energy of matched MCParticle (-999.0 if the particle is not matched)
     */
    double particleMCMatchE(const Particle* particle);

    /**
     * return total momentum of matched MCParticle (-999.0 if the particle is not matched)
     */
    double particleMCMatchP(const Particle* particle);

    /**
     * return recoiling mass against the particles appended as particle's daughters.
     * MC truth values are used in the calculations.
     */
    double particleMCRecoilMass(const Particle* particle);

    /**
    >>>>>>> Stashed changes
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
     * return invarian mass squared of the system recoiling against given Particle
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
     * return a random number between 0 and 1
     */
    double random(const Particle*);

    /**
     * returns the theta angle (lab) that is back-to-back (cms) to the particle
     */
    double b2bTheta(const Particle* particle);

    /**
     * returns the phi angle (lab) that is back-to-back (cms) to the particle
     */
    double b2bPhi(const Particle* particle);

  }
} // Belle2 namespace
