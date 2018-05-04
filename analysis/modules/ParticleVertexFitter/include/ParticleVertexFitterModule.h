/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Luigi Li Gioi, Anze Zupanc                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLEVERTEXFITTERMODULE_H
#define PARTICLEVERTEXFITTERMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

// DataStore
#include <framework/database/DBObjPtr.h>

// DataObjects
#include <framework/dbobjects/BeamParameters.h>

// kfitter
#include <analysis/KFit/MassFitKFit.h>
#include <analysis/KFit/FourCFitKFit.h>
#include <analysis/KFit/MassVertexFitKFit.h>
#include <analysis/KFit/VertexFitKFit.h>
#include <analysis/KFit/MakeMotherKFit.h>

// rave
#include <analysis/VertexFitting/RaveInterface/RaveSetup.h>
#include <analysis/VertexFitting/RaveInterface/RaveVertexFitter.h>
#include <analysis/VertexFitting/RaveInterface/RaveKinematicVertexFitter.h>

namespace Belle2 {

  class Particle;

  /**
   * Vertex fitter module
   */
  class ParticleVertexFitterModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleVertexFitterModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    std::string m_listName;       /**< particle list name */
    double m_confidenceLevel;     /**< required fit confidence level */
    double m_Bfield;              /**< magnetic field from data base */
    std::string m_vertexFitter;   /**< Vertex Fitter name */
    std::string m_fitType;        /**< type of the kinematic fit */
    std::string m_withConstraint; /**< additional constraint on vertex */
    std::string m_decayString;    /**< daughter particles selection */
    bool m_updateDaughters;       /**< flag for daughters update */
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

    /**
     * Unconstrained vertex fit using Kfitter
     * @param p pointer to particle
     * @return true for successfull fit
     */
    bool doKVertexFit(Particle* p, bool ipProfileConstraint, bool ipTubeConstraint);

    /**
     * Mass-constrained vertex fit using Kfitter
     * @param p pointer to particle
     * @return true for successfull fit
     */
    bool doKMassVertexFit(Particle* p);

    /**
     * Mass fit using Kfitter
     * @param p pointer to particle
     * @return true for successfull fit
     */
    bool doKMassFit(Particle* p);

    /**
     * FourC fit using Kfitter
     * @param p pointer to particle
     * @return true for successfull fit
     */
    bool doKFourCFit(Particle* p);

    /**
     * Update mother particle after unconstrained vertex fit using Kfitter
     * @param kv reference to Kfitter VertexFit object
     * @param p pointer to particle
     * @return true for successfull construction of mother
     */
    bool makeKVertexMother(analysis::VertexFitKFit& kv, Particle* p);


    /**
     * Update mother particle after mass-constrained vertex fit using Kfitter
     * @param kv reference to Kfitter MassVertexFit object
     * @param p pointer to particle
     * @return true for successfull construction of mother
     */
    bool makeKMassVertexMother(analysis::MassVertexFitKFit& kv, Particle* p);


    /**
     * Update mother particle after mass fit using Kfitter
     * @param kv reference to Kfitter MassFit object
     * @param p pointer to particle
     * @return true for successfull construction of mother
     */
    bool makeKMassMother(analysis::MassFitKFit& kv, Particle* p);

    /**
     * Update mother particle after FourC fit using Kfitter
     * @param kv reference to Kfitter MassFit object
     * @param p pointer to particle
     * @return true for successfull construction of mother
     */
    bool makeKFourCMother(analysis::FourCFitKFit& kv, Particle* p);

    /**
    * update the map of daughter and tracks, find out wich tracks belong to each daugther.
    * @param ui store the tracks ID of each daughter
    * @param l represent the tracks ID
    * @param p pointer to particle
     */
    void updateMapofTrackandDaughter(std::vector<unsigned>& ui, unsigned& l, const Particle* daughter);

    /**
     * Adds given particle's child to the FourCFitKFit.
     * @param kv reference to Kfitter FourCFit object
     * @param particle pointer to particle
     */
    bool addChildofParticletoKfitter(analysis::FourCFitKFit& kv, const Particle* particle);

    /**
     * Adds IPProfile constraint to the vertex fit using kfitter.
     */
    void addIPProfileToKFitter(analysis::VertexFitKFit& kv);

    /**
     * Fills valid particle's children (with valid error matrix) in the vector of Particles that will enter the fit.
     * Pi0 particles are treated separately so they are filled to another vector.
     */
    bool fillFitParticles(const Particle* mother, std::vector<unsigned>& fitChildren, std::vector<unsigned>& pi0Children);

    /**
     * Performs mass refit of pi0 assuming that pi0 originates from the point given by VertexFit.
     */
    bool redoPi0MassFit(Particle* pi0Temp, const Particle* pi0Orig, const analysis::VertexFitKFit& kv) ;

    /**
     * Fit using Rave
     * @param p pointer to particle
     * @return true for successfull fit and update of mother
     */
    bool doRaveFit(Particle* mother);

    /**  check if all the Daughters (o grand-daugthers) are selected for the vertex fit*/
    bool allSelectedDaughters(const Particle* mother, std::vector<const Particle*> tracksVertex);

    /**  calculate iptube constraint (quasi cilinder along boost direction) for RAVE fit*/
    void findConstraintBoost(double cut);
  };

} // Belle2 namespace

#endif
