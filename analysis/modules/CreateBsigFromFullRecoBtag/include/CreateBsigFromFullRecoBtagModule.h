/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CREATEBSIGFROMFULLRECOBTAGMODULE_H
#define CREATEBSIGFROMFULLRECOBTAGMODULE_H

#include <framework/core/Module.h>
#include <string>
//#include <vector>
//#include <tuple>
//#include <memory>
#include <analysis/DecayDescriptor/DecayDescriptor.h>


// DataStore
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// DataObjects
#include <framework/dbobjects/BeamParameters.h>
#include <analysis/dataobjects/ParticleList.h>

// rave
#include <analysis/raveInterface/RaveSetup.h>
#include <analysis/raveInterface/RaveVertexFitter.h>
#include <analysis/raveInterface/RaveKinematicVertexFitter.h>

namespace Belle2 {
  /**
   * Create a B particle from a Bbar particle
   *
   * set the direction of the recoil B   *
   */
  class CreateBsigFromFullRecoBtagModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    CreateBsigFromFullRecoBtagModule();

    /** sdf */
    virtual ~CreateBsigFromFullRecoBtagModule();

    /** sdf */
    virtual void initialize();

    /** sdf */
    virtual void beginRun();

    /** sdf */
    virtual void event();

    /** sdfy */
    virtual void endRun();

    /** sdf */
    virtual void terminate();


  private:

    std::string m_listName;  /**< name of particle list */
    double m_confidenceLevel;     /**< required fit confidence level */
    double m_Bfield;              /**< magnetic field from data base */
    std::string m_listOutput; /**< Bsig output list */
    std::string m_antiListOutput; /**< Bsig output list */
    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */

    TVector3 m_BeamSpotCenter;    /**< Beam spot position */
    TMatrixDSym m_beamSpotCov;    /**< Beam spot covariance matrix */
    DBObjPtr<BeamParameters> m_beamParams;/**< Beam parameters */

    bool m_isSelfConjugatedParticle; /**< flag that indicates whether an anti-particle mother does not exist and should not be reconstructed as well*/

    /**
     * Main steering routine
     * @param p pointer to particle
     * @return true for successfull fit and prob(chi^2,ndf) > m_confidenceLevel
     */
    bool doVertexFit(Particle* p);


  };
}

#endif /* CREATEBSIGFROMFULLRECOBTAGMODULE_H */
