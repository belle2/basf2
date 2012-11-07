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

#include <generators/dataobjects/MCParticleGraph.h>

#include <G4UserTrackingAction.hh>
#include <G4Event.hh>

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
      TrackingAction(MCParticleGraph& mcParticleGraph);

      /**
       * Destructor.
       */
      virtual ~TrackingAction();

      /**
       * ??
       * Checks if the particle associated to the track is already in the MCParticle list. If not, the
       * particle is added and as UserInformation attached to the track.
       *
       * @param track Pointer to the Geant4 track which will be transported through the geometry.
       */
      void PreUserTrackingAction(const G4Track* track);

      /**
       * ??
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
      void setIgnoreOpticalPhotons(bool ignore = true) {m_IgnoreOpticalPhotons = ignore;}

      /**
       * Set ignore flag for low energy Geant-produced secondary particles
       * if set to true, secondaries below KineticEnergyCut will not be stored in MCParticles collection
       * @param ignore flag
       */
      void setIgnoreSecondaries(bool ignore = true) {m_IgnoreSecondaries = ignore;}

      /**
       * Set kinetic energy cut for secondaries
       * @param cut_MeV kinetic energy in MeV
       */
      void setKineticEnergyCut(double cut_MeV) {m_EnergyCut = cut_MeV;}


    protected:

      MCParticleGraph& m_mcParticleGraph; /**< Reference to the MCParticle graph which is updated by the tracking action. */
      bool m_IgnoreOpticalPhotons; /**< do not store optical photons in MCParticles */
      bool m_IgnoreSecondaries; /**< do not store low energy secondaries in MCParticles */
      double m_EnergyCut; /**< kinetic energy cut for secondaries [MeV] */

    };

  }  // end namespace Simulation
} // end namespace Belle2

#endif /* TRACKINGACTION_H_ */
