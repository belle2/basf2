/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Dey                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>


// DataStore
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// DataObjects
#include <framework/dbobjects/BeamParameters.h>
#include <analysis/dataobjects/ParticleList.h>

// rave
#include <analysis/VertexFitting/RaveInterface/RaveSetup.h>
#include <analysis/VertexFitting/RaveInterface/RaveVertexFitter.h>
#include <analysis/VertexFitting/RaveInterface/RaveKinematicVertexFitter.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
namespace Belle2 {
  /**
   * Create a B particle from a Bbar particle
   *
   * set the direction of the recoil B   *
   */
  class BtubeCreatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    BtubeCreatorModule();

    /** destructor */
    virtual ~BtubeCreatorModule();

    /** declare data store elements */
    virtual void initialize() override;

    /** process event */
    virtual void event() override;

  private:

    std::string m_listName;  /**< name of particle list */
    double m_confidenceLevel;     /**< required fit confidence level */
    double m_Bfield;              /**< magnetic field from data base */
    //    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */
    //std::string m_listOutput;
    TVector3 m_BeamSpotCenter;    /**< Beam spot position */
    TMatrixDSym m_beamSpotCov;    /**< Beam spot covariance matrix */
    DBObjPtr<BeamParameters> m_beamParams;/**< Beam parameters */
    /**
     * Main steering routine
     * @param p pointer to particle
     * @return true for successfull fit and prob(chi^2,ndf) > m_confidenceLevel
     */
    bool doVertexFit(Particle* p);
    bool doRaveFit(Particle* q, Particle* r);
    bool m_verbose;
  };
}
