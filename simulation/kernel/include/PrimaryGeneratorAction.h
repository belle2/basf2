/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef PRIMARYGENERATORACTION_H_
#define PRIMARYGENERATORACTION_H_

#include <mdst/dataobjects/MCParticleGraph.h>

#include <G4VUserPrimaryGeneratorAction.hh>
#include <string>

class G4Event;

namespace Belle2 {

  namespace Simulation {

    class MCParticleGenerator;

    /**
     * The PrimaryGeneratorAction class inherits from G4VuserPrimaryGeneratorAction and specifies how a primary event should be generated.
     * Actual generation of primary particles will be done by a concrete classes of G4VPrimaryGenerator.
     * This class just arranges the way primary particles are generated.
     */
    class PrimaryGeneratorAction: public G4VUserPrimaryGeneratorAction {

    public:

      /** The constructor of the PrimaryGeneratorAction.
       * @param mcParticleGraph Reference to the MCParticle Graph, which is filled by this class.
       * @param mcCollectionName The name of the MCParticle collection that should be used to create the MCParticle generator.
       */
      PrimaryGeneratorAction(const std::string& mcCollectionName, MCParticleGraph& mcParticleGraph);

      /** The destructor of the PrimaryGeneratorAction. */
      virtual ~PrimaryGeneratorAction();

      /**
       * This method is called for each event by Geant4 and creates the primary particles by invoking the generators.
       *
       * @param event Pointer to the G4Event.
       */
      void GeneratePrimaries(G4Event* event);


    protected:

      MCParticleGenerator* m_mcParticleGenerator; /**< The instance of the MCParticle generator. */
    };

  } //end of Simulation namespace

} //end of Belle2 namespace


#endif /* PRIMARYGENERATORACTION_H_ */
