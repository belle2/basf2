/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <G4UserEventAction.hh>

#include <fstream>

class G4Event;

namespace Belle2 {

  class EventMetaData;

  namespace Simulation {

    /**
     * The Event Action class.
     * This class is invoked by G4EventManager for each event.
     */
    class EventAction : public G4UserEventAction {

    public:
      /**
       * Constructor.
       * @param mcCollectionName The name of the MCParticle collection.
       * @param mcParticleGraph Reference to the MCParticle graph.
       */
      EventAction(const std::string& mcCollectionName, MCParticleGraph& mcParticleGraph);

      /**
       * Destructor.
       */
      virtual ~EventAction();

      /**
       * This method is invoked before converting the primary particles to G4Track objects.
       *
       * @param event Pointer to the current G4Event object.
       */
      void BeginOfEventAction(const G4Event* event);

      /**
       * This method is invoked at the very end of event processing.
       *
       * @param event The pointer to the G4Event object which allows to access trajectories, hits collections and/or digits collections.
       */
      void EndOfEventAction(const G4Event* event);

      /**
       * This method gets the output stream for the event-history steps
       */
      std::ofstream* getVREventStream() const { return m_VREventStream; }

    protected:

      std::string m_mcCollectionName;     /**< The name of the MCParticle collection to which the MCParticles should be written.*/
      MCParticleGraph& m_mcParticleGraph; /**< Reference to the MCParticle graph which is converted to a MCParticle list by this class. */

    private:

      bool m_writeSimSteps{false}; /**< Flag for writing out the simulation steps. */
      std::ofstream* m_VREventStream{nullptr};  /**< Output stream for writing each step of event's history for the Virtual Reality. */
      StoreObjPtr<EventMetaData> m_evtMetaData; /**< Event metadata. */

    };

  }  // end namespace Simulation
} // end namespace Belle2
