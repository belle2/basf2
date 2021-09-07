/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

// framework - DataStore
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/geometry/B2Vector3.h>

// DataObjects
#include <analysis/dataobjects/ParticleList.h>
#include <mdst/dbobjects/BeamSpot.h>

// KFit
#include <analysis/VertexFitting/KFit/MassFitKFit.h>
#include <analysis/VertexFitting/KFit/FourCFitKFit.h>
#include <analysis/VertexFitting/KFit/MassFourCFitKFit.h>
#include <analysis/VertexFitting/KFit/MassPointingVertexFitKFit.h>
#include <analysis/VertexFitting/KFit/MassVertexFitKFit.h>
#include <analysis/VertexFitting/KFit/VertexFitKFit.h>
#include <analysis/VertexFitting/KFit/MakeMotherKFit.h>

// Rave
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

    StoreObjPtr<ParticleList> m_plist; /**< particle list */
    std::string m_listName;       /**< particle list name */
    double m_confidenceLevel;     /**< required fit confidence level */
    double m_Bfield;              /**< magnetic field from data base */
    std::string m_vertexFitter;   /**< Vertex Fitter name */
    std::string m_fitType;        /**< type of the kinematic fit */
    std::string m_withConstraint; /**< additional constraint on vertex */
    std::string m_decayString;    /**< daughter particles selection */
    bool m_updateDaughters;       /**< flag for daughters update */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of decays to look for. */
    bool m_hasCovMatrix = false;      /**< flag for mother covariance matrix (PseudoFitter)*/
    B2Vector3D m_BeamSpotCenter;    /**< Beam spot position */
    TMatrixDSym m_beamSpotCov;    /**< Beam spot covariance matrix */
    DBObjPtr<BeamSpot> m_beamSpotDB;/**< Beam spot database object */
    double m_smearing;            /**< smearing width applied to IP tube */
    std::vector<int> m_massConstraintList; /**< PDG codes of the particles to be mass constraint (massfourC)*/
    std::vector<std::string> m_massConstraintListParticlename; /**< Name of the particles to be mass constraint (massfourC)*/

    /**
     * Main steering routine
     * @param p pointer to particle
     * @return true for successful fit and prob(chi^2,ndf) > m_confidenceLevel
     */
    bool doVertexFit(Particle* p);

    /**
     * Unconstrained vertex fit using KFit
     * @param p pointer to particle
     * @param ipProfileConstraint flag for IP profile constraint
     * @param ipTubeConstraint flag for IP tube constraint
     * @return true for successful fit
     */
    bool doKVertexFit(Particle* p, bool ipProfileConstraint, bool ipTubeConstraint);

    /**
     * Mass-constrained vertex fit using KFit
     * @param p pointer to particle
     * @return true for successful fit
     */
    bool doKMassVertexFit(Particle* p);

    /**
     * Mass-constrained vertex fit with additional pointing constraint using KFit
     * @param p pointer to particle
     * @return true for successful fit
     */
    bool doKMassPointingVertexFit(Particle* p);

    /**
     * Mass fit using KFit
     * @param p pointer to particle
     * @return true for successful fit
     */
    bool doKMassFit(Particle* p);

    /**
     * FourC fit using KFit
     * @param p pointer to particle
     * @return true for successful fit
     */
    bool doKFourCFit(Particle* p);

    /**
     * MassFourC fit using KFit
     * @param p pointer to particle
     * @return true for successful fit
     */
    bool doKMassFourCFit(Particle* p);

    /**
     * Update mother particle after unconstrained vertex fit using KFit
     * @param kv reference to KFit VertexFit object
     * @param p pointer to particle
     * @return true for successful construction of mother
     */
    bool makeKVertexMother(analysis::VertexFitKFit& kv, Particle* p);

    /**
     * Update mother particle after mass-constrained vertex fit using KFit
     * @param kv reference to KFit MassVertexFit object
     * @param p pointer to particle
     * @return true for successful construction of mother
     */
    bool makeKMassVertexMother(analysis::MassVertexFitKFit& kv, Particle* p);

    /**
     * Update mother particle after mass-constrained vertex fit with additional pointing constraint using KFit
     * @param kv reference to KFit MassPointingVertexFit object
     * @param p pointer to particle
     * @return true for successful construction of mother
     */
    bool makeKMassPointingVertexMother(analysis::MassPointingVertexFitKFit& kv, Particle* p);

    /**
     * Update mother particle after mass fit using KFit
     * @param kv reference to KFit MassFit object
     * @param p pointer to particle
     * @return true for successful construction of mother
     */
    bool makeKMassMother(analysis::MassFitKFit& kv, Particle* p);

    /**
     * Update mother particle after FourC fit using KFit
     * @param kv reference to KFit MassFit object
     * @param p pointer to particle
     * @return true for successful construction of mother
     */
    bool makeKFourCMother(analysis::FourCFitKFit& kv, Particle* p);

    /**
     * Update mother particle after MassFourC fit using KFit
     * @param kv reference to KFit MassFit object
     * @param p pointer to particle
     * @return true for successful construction of mother
     */
    bool makeMassKFourCMother(analysis::MassFourCFitKFit& kv, Particle* p);

    /**
     * update the map of daughter and tracks, find out which tracks belong to each daughter.
     * @param l represent the tracks ID
     * @param pars map of all parameters
     * @param pard vector of parameters
     * @param allparticles vector of all particles
     * @param daughter pointer to particle
     */
    void updateMapOfTrackAndDaughter(unsigned& l, std::vector<std::vector<unsigned>>& pars, std::vector<unsigned>& pard,
                                     std::vector<Particle*>&  allparticles, const Particle* daughter);

    /**
     * Adds given particle's child to the FourCFitKFit.
     * @param kv reference to KFit FourCFit object
     * @param particle pointer to particle
     */
    bool addChildofParticletoKFit(analysis::FourCFitKFit& kv, const Particle* particle);

    /**
     * Adds given particle's child to the MassFourCFitKFit.
     * @param kf reference to KFit FourCFit object
     * @param particle pointer to particle
     * @param particleId vector of daughters track id
     */
    bool addChildofParticletoMassKFit(analysis::MassFourCFitKFit& kf, const Particle* particle, std::vector<unsigned>& particleId);

    /**
     * Adds IPProfile constraint to the vertex fit using KFit.
     */
    void addIPProfileToKFit(analysis::VertexFitKFit& kv);

    /**
     * Adds IPTube constraint to the vertex fit using KFit.
     */
    void addIPTubeToKFit(analysis::VertexFitKFit& kv);

    /**
     * Fills valid particle's children (with valid error matrix) in the vector of Particles that will enter the fit.
     * Particles formed from two photons (e.g. pi0) are treated separately so they are filled to another vector.
     */
    bool fillFitParticles(const Particle* mother, std::vector<const Particle*>& fitChildren,
                          std::vector<const Particle*>& twoPhotonChildren);

    /**
     * Combines preFit particle and vertex information from vertex fit kv to create new postFit particle.
     * A mass refit of this new particle is performed assuming that it originates from the point given by VertexFit.
     */
    bool redoTwoPhotonDaughterMassFit(Particle* postFit, const Particle* preFit, const analysis::VertexFitKFit& kv) ;

    /**
     * Fit using Rave
     * @param mother pointer to particle
     * @return true for successful fit and update of mother
     */
    bool doRaveFit(Particle* mother);

    /**  check if all the Daughters (o grand-daughters) are selected for the vertex fit*/
    bool allSelectedDaughters(const Particle* mother, const std::vector<const Particle*>& tracksVertex);

    /**  calculate iptube constraint (quasi cylinder along boost direction) for RAVE fit*/
    void findConstraintBoost(double cut);

    /** smear beam spot covariance */
    void smearBeamSpot(double width);
  };

} // Belle2 namespace

