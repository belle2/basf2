/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRACKINGACTION_H_
#define TRACKINGACTION_H_

#include <mdst/dataobjects/MCParticleGraph.h>
#include <simulation/dataobjects/MCParticleTrajectory.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <G4UserTrackingAction.hh>

namespace Belle2 {

  namespace Simulation {

    /**
     * The Tracking Action class.
     */
    class TrackingAction : public G4UserTrackingAction {

    public:

      /**
       * Constructor.
       * @param Reference to the MCParticle graph which is used to add secondary particles.
       */
      explicit TrackingAction(MCParticleGraph& mcParticleGraph);

      /**
       * Destructor.
       */
      virtual ~TrackingAction();

      /**
       * Checks if the particle associated to the track is already in the MCParticle list. If not, the
       * particle is added and as UserInformation attached to the track.
       *
       * @param track Pointer to the Geant4 track which will be transported through the geometry.
       */
      void PreUserTrackingAction(const G4Track* track);

      /**
       * Updates the data of the MCParticle associated with the Geant4 track.
       *
       * @param track Pointer to the Geant4 track which was transported through the geometry.
       */
      void PostUserTrackingAction(const G4Track* track);

      /**
       * Set ignore flag for optical photons
       * if set to true, optical photons will not be stored in MCParticles collection
       * @param ignore flag
       */
      void setIgnoreOpticalPhotons(bool ignore = true) {m_ignoreOpticalPhotons = ignore;}

      /**
       * Set ignore flag for low energy Geant-produced secondary particles
       * if set to true, secondaries with kinetic energy above SecondariesEnergyCut will be stored in the MCParticles collection
       * @param ignore flag
       */
      void setIgnoreSecondaries(bool ignore = true) {m_ignoreSecondaries = ignore;}

      /**
       * Set kinetic energy cut for secondaries
       * @param cut_MeV kinetic energy in MeV
       */
      void setSecondariesEnergyCut(double cut_MeV) {m_secondariesEnergyCut = cut_MeV;}

      /**
       * Set ignore flag for low energy breamsstrahlung photons
       * if set to true, breamsstrahlung photons with kinetic energy above BremsStrahlungPhotonsEnergyCut will be stored in the MCParticles collection,
       * even if IgnoreSecondaries flag is true.
       * @param ignore flag
       */
      void setIgnoreBremsstrahlungPhotons(bool ignore = true) {m_ignoreBremsstrahlungPhotons = ignore;}

      /**
       * Set kinetic energy cut for bremsstrahlung photons
       * @param cut_MeV kinetic energy in MeV
       */
      void setBremsstrahlungPhotonsEnergyCut(double cut_MeV) {m_bremsstrahlungPhotonsEnergyCut = cut_MeV;}

      /**
       * Set ignore flag for e+ or e- coming from gamma conversions into a pair
       * if set to true, e+ or e- from pair conversions with kinetic energy above PairConversionsEnergyCut will be stored in the MCParticles collection,
       * even if IgnoreSecondaries flag is true.
       * @param ignore flag
       */
      void setIgnorePairConversions(bool ignore = true) {m_ignorePairConversions = ignore;}

      /**
       * Set kinetic energy cut for e+ e- pair conversions
       * @param cut_MeV kinetic energy in MeV
       */
      void setPairConversionsEnergyCut(double cut_MeV) {m_pairConversionsEnergyCut = cut_MeV;}

      /** Sets the trajectory option to enable storing of the simulated particle trajectories */
      void setStoreTrajectories(int store, double distanceTolerance);


    protected:

      MCParticleGraph& m_mcParticleGraph; /**< Reference to the MCParticle graph which is updated by the tracking action. */

      bool m_ignoreOpticalPhotons;              /**< do not store optical photons in MCParticles */
      bool m_ignoreSecondaries;                 /**< do not store secondaries in MCParticles */
      double m_secondariesEnergyCut;            /**< kinetic energy cut for stored secondaries [MeV] */
      bool m_ignoreBremsstrahlungPhotons;       /**< do not store bremsstrahlung photons in MCParticles */
      double m_bremsstrahlungPhotonsEnergyCut;  /**< kinetic energy cut for stored bremsstrahlung photons [MeV] */
      bool m_ignorePairConversions;             /**< do not store e+ or e- from pair conversions in MCparticles */
      double m_pairConversionsEnergyCut;        /**< kinetic energy cut for stored e+ or e- from pair conversions [MeV] */


      int m_storeTrajectories;    /**< Store trajectories for 0=none, 1=primary or 2=all particles */
      double m_distanceTolerance;  /**< distance tolerance to merge trajectory points */

      /** Store array for the Trajectories */
      StoreArray<MCParticleTrajectory> m_storeMCTrajectories;
      /** RelationArry for the relation between MCParticles and Trajectories */
      RelationArray m_relMCTrajectories;
    };

  }  // end namespace Simulation
} // end namespace Belle2

#endif /* TRACKINGACTION_H_ */
