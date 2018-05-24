/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
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
     * return 1 if the charge of the particle is wrong. 0 in
     * all other cases
     */
    double isWrongCharge(const Particle* particle);

    /**
     * Return 1 if the charged final state particle comes from a
     * cloned track, 0 if not a clone or not from a track, and
     * returns NAN if MCParticle not found (like for data or if
     * not MCMatched)");
     */
    double isCloneTrack(const Particle* particle);

    /**
     * Return 1 if the particle is a clone track or has a clone
     * track as a daughter, 0 otherwise.
     */
    double isOrHasCloneTrack(const Particle* particle);

    /**
     * return 1 if the particle was misidentified (note that this
     * can occur if the wrong charge is assigned). 0 all other cases
     */
    double isMisidentified(const Particle* particle);

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

    /*
     * return the true momentum transfer to lepton pair in a B (semi-) leptonic B meson decay
     */
    double particleMCMomentumTransfer2(const Particle* part);

    /**
     * return decay time of matched MCParticle (-999.0 if the particle is not matched)
     * note this is the delta time between decay of the particle and collision
     */
    double particleMCMatchDecayTime(const Particle* particle);


    /** return life time of matched mc particle in CMS frame. */
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
     * return the ID of the generated decay of positive tau in a tau pair event.
     */
    int tauPlusMcMode(const Particle*);

    /**
     * return the ID of the generated decay of negative tau in a tau pair event.
     */
    int tauMinusMcMode(const Particle*);

    /**
     * check that neutrals were seen in ECL, and charged were seen in SVD
     */
    double isReconstructible(const Particle*);

    /**
     * was theicle seen in the PXD
     */
    double seenInPXD(const Particle*);

    /**
     * was theicle seen in the SVD
     */
    double seenInSVD(const Particle*);

    /**
     * was theicle seen in the CDC
     */
    double seenInCDC(const Particle*);

    /**
     * was theicle seen in the TOP
     */
    double seenInTOP(const Particle*);

    /**
     * was theicle seen in the ECL
     */
    double seenInECL(const Particle*);

    /**
     * was theicle seen in the ARICH
     */
    double seenInARICH(const Particle*);

    /**
     * was theicle seen in the KLM
     */
    double seenInKLM(const Particle*);
  }
}
