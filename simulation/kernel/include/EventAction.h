/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef EVENTACTION_H_
#define EVENTACTION_H_

#include <mdst/dataobjects/MCParticleGraph.h>

#include <G4UserEventAction.hh>
class G4Event;

namespace Belle2 {

  class MCParticleGraph;

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
       * @param createRelation If set to true the relation collection Hit -> MCParticle is created.
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


    protected:

      std::string m_mcCollectionName;     /**< The name of the MCParticle collection to which the MCParticles should be written.*/
      MCParticleGraph& m_mcParticleGraph; /**< Reference to the MCParticle graph which is converted to a MCParticle list by this class. */

    };

  }  // end namespace Simulation
} // end namespace Belle2

#endif /* EVENTACTION_H_ */
