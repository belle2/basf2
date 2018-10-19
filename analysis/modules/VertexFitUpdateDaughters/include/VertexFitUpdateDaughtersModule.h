/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VERTEXFITUPDATEDAUGHTERSMODULE_H
#define VERTEXFITUPDATEDAUGHTERSMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

// DataStore
#include <framework/database/DBObjPtr.h>

// DataObjects
#include <framework/dbobjects/BeamParameters.h>

// rave
#include <analysis/VertexFitting/RaveInterface/RaveSetup.h>
#include <analysis/VertexFitting/RaveInterface/RaveVertexFitter.h>
#include <analysis/VertexFitting/RaveInterface/RaveKinematicVertexFitter.h>

namespace Belle2 {
  /**
   * Partial Update Daughters
   *
   * a way of performing vertex fits and update daughters   *
   */
  class VertexFitUpdateDaughtersModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    VertexFitUpdateDaughtersModule();

    /** sdf */
    virtual ~VertexFitUpdateDaughtersModule();

    /** sdf */
    virtual void initialize() override;

    /** sdf */
    virtual void beginRun() override;

    /** sdf */
    virtual void event() override;

    /** sdfy */
    virtual void endRun() override;

    /** sdf */
    virtual void terminate() override;


  private:

    std::string m_listName;  /**< name of particle list */
    double m_confidenceLevel;     /**< required fit confidence level */
    double m_Bfield;              /**< magnetic field from data base */
    std::string m_withConstraint; /**< additional constraint on vertex */
    std::string m_decayString;  /**< daughter particles selection */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of decays to look for. */
    TVector3 m_BeamSpotCenter;    /**< Beam spot position */
    TMatrixDSym m_beamSpotCov;    /**< Beam spot covariance matrix */
    DBObjPtr<BeamParameters> m_beamParams;/**< Beam parameters */

    /**
     * Main steering routine
     * @param p pointer to particle
     * @return true for successfull fit and prob(chi^2,ndf) > m_confidenceLevel
     */
    bool doVertexFit(Particle* p);

    /**  calculate iptube constraint (quasi cilinder along boost direction) for RAVE fit*/
    void findConstraintBoost(double cut);

  };
}

#endif /* VERTEXFITUPDATEDAUGHTERSMODULE_H */
