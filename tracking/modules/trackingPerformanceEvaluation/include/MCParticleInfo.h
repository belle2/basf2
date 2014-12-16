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

    MCParticleInfo(const MCParticle& the_mcParticle, const TVector3& the_magField);

    virtual ~ MCParticleInfo();

    double getPx() { return m_mcParticle.getMomentum().Px(); };
    double getPy() { return m_mcParticle.getMomentum().Py(); };
    double getPz() { return m_mcParticle.getMomentum().Pz(); };
    double getPt() { return m_mcParticle.getMomentum().Pt(); };
    double getP() { return m_mcParticle.getMomentum().Mag(); };

    double getX() { return m_mcParticle.getVertex().X(); };
    double getY() { return m_mcParticle.getVertex().Y(); };
    double getZ() { return m_mcParticle.getVertex().Z(); };

    double getPtheta() { return m_mcParticle.getMomentum().Theta(); };
    double getPphi() { return m_mcParticle.getMomentum().Phi(); };

    double getCharge() { return m_mcParticle.getCharge(); };

    double getD0();
    double getZ0();
    double getPhi();
    double getOmega();
    double getCotTheta();

  private:

    const MCParticle& m_mcParticle;
    const TVector3& m_magField;
    double m_charge;

  };

}


#endif /* MCPARTICLEINFO_H_ */
