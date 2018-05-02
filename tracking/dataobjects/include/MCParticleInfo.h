/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCPARTICLEINFO_H_
#define MCPARTICLEINFO_H_

#include <TVector3.h>
#include <mdst/dataobjects/MCParticle.h>

namespace Belle2 {

  /** This struct is used by the TrackingPerformanceEvaluation Module to save information
   * of reconstructed tracks
   */
  class MCParticleInfo {

  public :

    /// Constructor
    MCParticleInfo(const MCParticle& the_mcParticle, const TVector3& the_magField);

    /// Getter for x component of momentum.
    double getPx() { return m_mcParticle.getMomentum().Px(); };
    /// Getter for y component of momentum.
    double getPy() { return m_mcParticle.getMomentum().Py(); };
    /// Getter for z component of momentum.
    double getPz() { return m_mcParticle.getMomentum().Pz(); };
    /// Getter for transverse momentum.
    double getPt() { return m_mcParticle.getMomentum().Pt(); };
    /// Getter for magnitut of momentum.
    double getP() { return m_mcParticle.getMomentum().Mag(); };
    /// Getter for energy.
    double getEnergy() { return m_mcParticle.getEnergy(); };

    /// Getter for x component of vertex.
    double getX() { return m_mcParticle.getVertex().X(); };
    /// Getter for y component of vertex.
    double getY() { return m_mcParticle.getVertex().Y(); };
    /// Getter for z component of vertex.
    double getZ() { return m_mcParticle.getVertex().Z(); };

    /// Getter for theta of momentum vector.
    double getPtheta() { return m_mcParticle.getMomentum().Theta(); };
    /// Getter for phi of momentum vector.
    double getPphi() { return m_mcParticle.getMomentum().Phi(); };

    /// Getter for electric charge of particle.
    double getCharge() { return m_mcParticle.getCharge(); };

    /// Getter for D0.
    double getD0();
    /// Getter for Z0.
    double getZ0();
    /// Getter for Phi.
    double getPhi();
    /// Getter for Omega.
    double getOmega();
    /// Getter for Theta.
    double getCotTheta();
    /// Getter for Lambda.
    double getLambda();

    /// Getter for Chi.
    double getChi();

  private:

    /// Reference to MC particle.
    const MCParticle& m_mcParticle;
    /// Member variable for z component of B field.
    double m_myBz;
    /// Member variable for particle's electric charge.
    double m_charge;
  };
}

#endif /* MCPARTICLEINFO_H_ */
