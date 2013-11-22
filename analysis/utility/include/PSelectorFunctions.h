/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PSELECTORFUNCTIONS_H
#define PSELECTORFUNCTIONS_H


namespace Belle2 {

  class Particle;

  namespace analysis {

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum magnitude
     */
    double particleP(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component x
     */
    double particlePx(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component y
     */
    double particlePy(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component z
     */
    double particlePz(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return transverse momentum
     */
    double particlePt(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return  momentum cosine of polar angle
     */
    double particleCosTheta(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum azimuthal angle
     */
    double particlePhi(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum magnitude in CMS
     */
    double particlePStar(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component x in CMS
     */
    double particlePxStar(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component y in CMS
     */
    double particlePyStar(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component z in CMS
     */
    double particlePzStar(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return transverse momentum in CMS
     */
    double particlePtStar(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum cosine of polar angle in CMS
     */
    double particleCosThetaStar(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum azimuthal angle in CMS
     */
    double particlePhiStar(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return position in x relative to interaction point
     */
    double particleDX(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return position in y relative to interaction point
     */
    double particleDY(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return position in z relative to interaction point
     */
    double particleDZ(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return transverse distance relative to interaction point
     */
    double particleDRho(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return mass
     */
    double particleMass(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return mass minus nominal mass
     */
    double particleDMass(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return released energy in decay
     */
    double particleQ(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return released energy in decay minus nominal one
     */
    double particleDQ(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return beam constrained mass
     */
    double particleMbc(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return energy difference in CMS
     */
    double particleDeltaE(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleElectronId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return muon Id
     */
    double particleMuonId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return pion Id
     */
    double particlePionId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return kaon Id
     */
    double particleKaonId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return proton Id
     */
    double particleProtonId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return prob(chi^2,ndf) of fit
     */
    double particlePvalue(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return number of daughter particles
     */
    double particleNDaughters(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return flavor type
     */
    double particleFlavorType(const Particle* part);

  }
} // Belle2 namespace

#endif



