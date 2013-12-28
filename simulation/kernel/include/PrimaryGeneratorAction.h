/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      virtual void GeneratePrimaries(G4Event* event);


    protected:

      MCParticleGenerator* m_mcParticleGenerator; /**< The instance of the MCParticle generator. */
    };

  } //end of Simulation namespace

} //end of Belle2 namespace


#endif /* PRIMARYGENERATORACTION_H_ */
