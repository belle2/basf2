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

// OrcaKinFit
#include <analysis/OrcaKinFit/BaseFitter.h>
#include <analysis/OrcaKinFit/TextTracer.h>

// Constraints
#include <analysis/OrcaKinFit/MomentumConstraint.h>
#include <analysis/OrcaKinFit/RecoilMassConstraint.h>
#include <analysis/OrcaKinFit/MassConstraint.h>

// Fitobjects
#include <analysis/OrcaKinFit/ParticleFitObject.h>

// analysis dataobjects
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/ParticleList.h>

// framework datastore
#include <framework/datastore/StoreObjPtr.h>

// ROOT
#include <Math/Vector4D.h>
#include <TMatrixFSym.h>

// CLHEOP
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>

namespace Belle2 {
  class Particle;
  namespace OrcaKinFit {

    /**
     * Kinematic fitter module
     */
    class ParticleKinematicFitterModule : public Module {

    public:

      /**
       * Constructor
       */
      ParticleKinematicFitterModule();

      /**
       * Initialize the Module.
       * This method is called at the beginning of data processing.
       */
      virtual void initialize() override;

      /**
       * termination.
       */
      virtual void terminate() override;

      /**
       * Event processor.
       */
      virtual void event() override;

    private:

      // module parameters
      std::string m_listName;            /**< particle list name */
      std::string m_kinematicFitter;     /**< Kinematic Fitter name */
      std::string m_orcaFitterEngine;    /**< Orca Fitter Engine name */
      std::string m_orcaTracer;          /**< Tracer (None, Text or ROOT) */
      std::string m_orcaConstraint;      /**< Constraint (softBeam, hardBeam (default)) */
      bool m_debugFitter;                /**< activate internal debugging (for New and Newton fitter only)*/
      int m_debugFitterLevel;            /**< internal debugging level (for New and Newton fitter only)*/
      bool m_addUnmeasuredPhoton;        /**< add one unmeasured photon to the fit (costs 3 constraints)*/
      bool m_add3CPhoton;                /**< add one photon with unmeasured energy to the fit (costs 1 constraints)*/
      bool m_updateMother;               /**< update mother kinematics*/
      bool m_updateDaughters;            /**< update daughter kinematics*/
      double m_recoilMass;               /**< Recoil mass for RecoilMass constraint */
      double m_invMass;                  /**< Invariant mass for Mass constraint */

      // internal variables
      TextTracer* m_textTracer;                           /**< internal text output variable */
      StoreObjPtr<EventExtraInfo> m_eventextrainfo;       /**< StoreObjPtr for the EventExtraInfo in this mode */
      std::vector <double> m_unmeasuredLeptonFitObject;   /**< unmeasured fit object */
      std::vector <double> m_unmeasuredGammaFitObject;    /**< unmeasured fit object */
      StoreObjPtr<ParticleList> m_plist;                  /**< StoreObjPtr for the particle list */

      // hard constraints
      MomentumConstraint m_hardConstraintPx;  /**< hard beam constraint px */
      MomentumConstraint m_hardConstraintPy;  /**< hard beam constraint py */
      MomentumConstraint m_hardConstraintPz;  /**< hard beam constraint pz */
      MomentumConstraint m_hardConstraintE;   /**< hard beam constraint E */

      RecoilMassConstraint m_hardConstraintRecoilMass;  /**< hard recoil mass constraint */

      MassConstraint m_hardConstraintMass;  /**< hard mass constraint */

      // UNUSED YET
      std::string m_decayString;         /**< daughter particles selection */
      DecayDescriptor m_decaydescriptor; /**< Decay descriptor of decays to look for. */

      /**
       * Main steering routine for any kinematic fitter
       * @param p pointer to particle
       * @return true for successful fit
       */
      bool doKinematicFit(Particle* p);

      /**
       * Kinematic fit using OrcaKinFit
       * @param p pointer to particle
       * @return true for successful fit
       */
      bool doOrcaKinFitFit(Particle* p);

      /**
       * Fills valid particle's children (with valid error matrix) in the vector of Particles that will enter the fit.
       */
      bool fillFitParticles(Particle* mother, std::vector<Particle*>& particleChildren);

      /**
       * Added four vectors and calculated a covariance matrix for a combined particles
       * @param mother pointer to particle
      */
      bool AddFour(Particle* mother);

      /**
       * Adds given particle to the OrcaKinFit.
       * @param fitter reference to OrcaKinFit fitter object
       * @param particle pointer to particle
       * @param index used to name particles: particle_%index
       */
      void addParticleToOrcaKinFit(BaseFitter& fitter, Particle* particle, const int index);

      /**
       * Adds Orca fit object to the constraints.
       * @param fitobject reference to OrcaKinFit fit object
       */
      void addFitObjectToConstraints(ParticleFitObject& fitobject);

      /**
       * Adds Orca fit object to the constraints.
       * @param fitter reference to OrcaKinFit fitter object
       */
      void addConstraintsToFitter(BaseFitter& fitter);

      /**
       * Adds tracer to the fitter.
       * @param fitter reference to OrcaKinFit fitter object
       */
      void addTracerToFitter(BaseFitter& fitter);

      /**
       * Sets constraints, this is not connect to particles or a fitter at this stage
       */
      void setConstraints();

      /**
       * Get constraints (at whatever stage before/after fitting)
       */
      ROOT::Math::PxPyPzEVector getLorentzVectorConstraints();


      /**
       * Resets all objects associated with the OrcaKinFit fitter.
       * @param fitter reference to OrcaKinFit fitter object
       */
      void resetFitter(BaseFitter& fitter);

      /**
       * Adds an unmeasured gamma (E, phi, theta) to the fit (-3C)
       * stored as EventExtraInfo TODO
       * @param fitter reference to OrcaKinFit fitter object
       */
      void addUnmeasuredGammaToOrcaKinFit(BaseFitter& fitter);

      /**
       * Update the mother: momentum is sum of daughters TODO update covariance matrix
       * @param fitter reference to OrcaKinFit fitter object
       * @param particleChildren list of daughter particles
       * @param mother mother particle
       */
      void updateOrcaKinFitMother(BaseFitter& fitter, std::vector<Particle*>& particleChildren, Particle* mother);


      /**
       * Update the daughters: momentum is sum of daughters TODO update covariance matrix
       * @param fitter reference to OrcaKinFit fitter object
       * @param mother mother particle
       */
      bool updateOrcaKinFitDaughters(BaseFitter& fitter, Particle* mother);

      /**
       * update the map of daughter and tracks, find out which tracks belong to each daughter.
       * @param l represent the tracks ID
       * @param pars map of all parameters
       * @param pard vector of parameters
       * @param allparticles vector of all particles
       * @param daughter pointer to particle
       */
      void updateMapOfTrackAndDaughter(unsigned& l,  std::vector<std::vector<unsigned>>& pars, std::vector<unsigned>& pard,
                                       std::vector<Particle*>&  allparticles, const Particle* daughter);

      /**
       * store fit object information as ExtraInfo
       * @param prefix can be used to distinguish e.g. "Fitted" and "Measured"
       * @param fitter reference to OrcaKinFit fitter object
       * @param particleChildren list of all particle children
       * @param mother mother particle
       */
      bool storeOrcaKinFitParticles(const std::string& prefix, BaseFitter& fitter, std::vector<Particle*>& particleChildren,
                                    Particle* mother);


      /**
       * Returns particle's 4x4 momentum-error matrix as a HepSymMatrix
       * @param particle pointer to particle
       */
      CLHEP::HepSymMatrix getCLHEPMomentumErrorMatrix(Particle* particle);

      /**
       * Returns particle's 7x7 momentum-vertex-error matrix as a HepSymMatrix
       * @param particle pointer to particle
       */
      CLHEP::HepSymMatrix getCLHEPMomentumVertexErrorMatrix(Particle* particle);

      /**
       * Returns particle's 4-momentum as a HepLorentzVector
       * @param particle pointer to particle
       */
      CLHEP::HepLorentzVector getCLHEPLorentzVector(Particle* particle);

      /**
       * @param fitobject reference to OrcaKinFit fit object
       * Returns particle's 4-momentum as a PxPyPzEVector
       */
      ROOT::Math::PxPyPzEVector getLorentzVector(ParticleFitObject* fitobject);

      /**
       * Returns fit object error on the parameter ilocal
       * @param fitobject reference to OrcaKinFit fit object
       * @param ilocal internal local ID
       */
      float getFitObjectError(ParticleFitObject* fitobject, int ilocal);

      /**
       * Returns covariance matrix
       * @param fitobject reference to OrcaKinFit fit object
       */
      TMatrixFSym getFitObjectCovMat(ParticleFitObject* fitobject);

      /**
       * Returns covariance matrix
       * @param fitobject reference to OrcaKinFit fit object
       */
      TMatrixFSym getCovMat7(ParticleFitObject* fitobject);

      /**
       * Returns particle's 7x7 momentum-error matrix as a TMatrixFSym
       */
      TMatrixFSym getTMatrixFSymMomentumErrorMatrix();

      /**
       * Returns particle's 7x7 momentum-vertex-error matrix as a TMatrixFSym
       */
      TMatrixFSym getTMatrixFSymMomentumVertexErrorMatrix();

    };

  }// end OrcaKinFit namespace
} // Belle2 namespace

