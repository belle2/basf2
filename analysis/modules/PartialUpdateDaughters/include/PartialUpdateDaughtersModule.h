/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTIALUPDATEDAUGHTERSMODULE_H
#define PARTIALUPDATEDAUGHTERSMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

// DataStore
#include <framework/database/DBObjPtr.h>

// DataObjects
#include <framework/dbobjects/BeamParameters.h>

// rave
#include <analysis/raveInterface/RaveSetup.h>
#include <analysis/raveInterface/RaveVertexFitter.h>
#include <analysis/raveInterface/RaveKinematicVertexFitter.h>

namespace Belle2 {
  /**
   * Partial Update Daughters
   *
   * a way of performing vertex fits and update daughters   *
   */
  class PartialUpdateDaughtersModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PartialUpdateDaughtersModule();

    /** sdf */
    virtual ~PartialUpdateDaughtersModule();

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

#endif /* PARTIALUPDATEDAUGHTERSMODULE_H */
