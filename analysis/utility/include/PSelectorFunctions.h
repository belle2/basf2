/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PSELECTORFUNCTIONS_H
#define PSELECTORFUNCTIONS_H

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

namespace Belle2 {

  namespace analysis {

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
     * return  momentum cosine of polar angle
     */
    double particleCosTheta(const Particle* part);

    /**
     * return momentum azimuthal angle
     */
    double particlePhi(const Particle* part);

    /**
     * return momentum magnitude in CMS
     */
    double particleP_CMS(const Particle* part);

    /**
     * return energy in CMS
     */
    double particleE_CMS(const Particle* part);

    /**
     * return momentum component x in CMS
     */
    double particlePx_CMS(const Particle* part);

    /**
     * return momentum component y in CMS
     */
    double particlePy_CMS(const Particle* part);

    /**
     * return momentum component z in CMS
     */
    double particlePz_CMS(const Particle* part);

    /**
     * return transverse momentum in CMS
     */
    double particlePt_CMS(const Particle* part);

    /**
     * return momentum cosine of polar angle in CMS
     */
    double particleCosTheta_CMS(const Particle* part);

    /**
     * return momentum azimuthal angle in CMS
     */
    double particlePhi_CMS(const Particle* part);

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
     * return uncertainty of the invariant mass (determined from particle's daughter 4-momentum vectors)
     */
    double particleInvariantMassError(const Particle* part);

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
     * return electron Id
     */
    double particleElectronId(const Particle* part);

    /**
     * return electron Id
     */
    double particleElectrondEdxId(const Particle* part);

    /**
     * return electron Id
     */
    double particleElectronTOPId(const Particle* part);

    /**
     * return electron Id
     */
    double particleElectronARICHId(const Particle* part);
    /**
     * return muon Id
     */
    double particleMuonId(const Particle* part);

    /**
     * return electron Id
     */
    double particleMuondEdxId(const Particle* part);

    /**
     * return electron Id
     */
    double particleMuonTOPId(const Particle* part);

    /**
     * return electron Id
     */
    double particleMuonARICHId(const Particle* part);

    /**
     * return pion Id
     */
    double particlePionId(const Particle* part);

    /**
     * return electron Id
     */
    double particlePiondEdxId(const Particle* part);

    /**
     * return electron Id
     */
    double particlePionTOPId(const Particle* part);

    /**
     * return electron Id
     */
    double particlePionARICHId(const Particle* part);

    /**
     * return kaon Id
     */
    double particleKaonId(const Particle* part);

    /**
     * return electron Id
     */
    double particleKaondEdxId(const Particle* part);

    /**
     * return electron Id
     */
    double particleKaonTOPId(const Particle* part);

    /**
     * return electron Id
     */
    double particleKaonARICHId(const Particle* part);

    /**
     * return proton Id
     */
    double particleProtonId(const Particle* part);

    /**
     * return electron Id
     */
    double particleProtondEdxId(const Particle* part);

    /**
     * return electron Id
     */
    double particleProtonTOPId(const Particle* part);

    /**
     * return electron Id
     */
    double particleProtonARICHId(const Particle* part);


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
     * NeuroBayesifys this function
     * @param part cconst pointer to Particle
     * @return function value, except if information from ARICH is not available which is set to -999
     */
    template<double(*T)(const Particle*)>
    double NeuroBayesifyARICH(const Particle* particle)
    {

      const PIDLikelihood* pid = particle->getRelatedTo<PIDLikelihood>();
      if (!pid)
        return -999;

      Const::PIDDetectorSet set = Const::ARICH;
      if (not pid->isAvailable(set))
        return -999;
      return T(particle);
    }

    /**
     * NeuroBayesifys this function
     * @return function value, except if information from TOP is not available which is set to -999
     */
    template<double(*T)(const Particle*)>
    double NeuroBayesifyTOP(const Particle* particle)
    {

      const PIDLikelihood* pid = particle->getRelated<PIDLikelihood>();
      if (!pid)
        return -999;

      Const::PIDDetectorSet set = Const::TOP;
      if (not pid->isAvailable(set))
        return -999;
      return T(particle);
    }

    /**
     * return 1 if ARICH Id is missing
     */
    double particleMissingARICHId(const Particle*);

    /**
     * return 1 if TOPId is missing
     */
    double particleMissingTOPId(const Particle*);

    /**
     * return 1 if Particle is correctly reconstructed (SIGNAL), 0 otherwise
     */
    double isSignal(const Particle* particle);

    /**
     * return absolute PDG code of matched MCParticle
     */
    double particleAbsMCMatchPDGCode(const Particle* particle);

    /**
     * return PDG code of matched MCParticle
     */
    double particleMCMatchPDGCode(const Particle* particle);

    /**
     * return ORed combination of MCMatching::MCMatchStatus flags.
     */
    double particleMCMatchStatus(const Particle* particle);

    /**
     * return number of remaining tracks as given by the related RestOfEvent object
     *
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     */
    double nROETracks(const Particle* particle);

    /**
     * return number of remaining ECL clusters as given by the related RestOfEvent object
     *
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     */
    double nROEClusters(const Particle* particle);

    /**
     * return magnitude of 3-momentum recoiling against given Particle
     */
    double recoilMomentum(const Particle* particle);

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
     * return extra energy in the calorimeter that is not associated to the given Particle
     *
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     */
    double extraEnergy(const Particle* particle);

    /**
     * return 1/2/3 if the ECL Cluster is detected in the forward/barrel/backward region
     */
    double eclClusterDetectionRegion(const Particle* particle);

    /**
     * Return 1 if ECLCluster passes the following selection criteria:
     * Forward  : E > 125 MeV && E9/E25>0.7
     * Barrel   : E > 100 MeV
     * Backward : E > 150 MeV
     */
    double goodGamma(const Particle* particle);

    /**
     * return ratio of energies in inner 3x3 and 5x5 cells
     */
    double eclClusterE9E25(const Particle* particle);

    /**
     *
     * return number of hits associated to this cluster
     */
    double eclClusterNHits(const Particle* particle);

    /**
     * return 1/0 if charged track is/is not Matched to this cluster
     */
    double eclClusterTrackMatched(const Particle* particle);

    /**
     * return cosine of the angle between the mother momentum vector and the direction of the first daughter in the mother's rest frame
     */
    double particleDecayAngle(const Particle* particle);

    /**
     * return cosine of the angle between the first two daughters, in lab frame
     */
    double particleDaughterAngle(const Particle* particle);

  }
} // Belle2 namespace

#endif
