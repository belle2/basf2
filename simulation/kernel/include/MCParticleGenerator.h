/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCPARTICLEGENERATOR_H_
#define MCPARTICLEGENERATOR_H_

#include <generators/dataobjects/MCParticleGraph.h>

#include <G4VPrimaryGenerator.hh>
#include <G4PrimaryParticle.hh>
#include <G4Event.hh>

#include <string>

class G4Event;

namespace Belle2 {

  namespace Simulation {

    /**
     * The MCParticleGenerator class generates the Geant4 primary particles using the MCParticle list from the DataStore.
     * The generator assumes that each particle in the list has exactly one mother.
     */
    class MCParticleGenerator: public G4VPrimaryGenerator {

    public:

      /** The constructor of the MCParticleGenerator class.
       * @param mcParticleGraph Reference to the MCParticle graph which is filled by this class.
       * @param mcCollectionName The name of the MCParticle collection from which the MCParticles should be read.
       */
      MCParticleGenerator(const std::string& mcCollectionName, MCParticleGraph& mcParticleGraph);

      /** The destructor of the MCParticleGenerator class. */
      virtual ~MCParticleGenerator();

      /**
       * This method is called for each event by Geant4 and creates the primary particles from the MCParticle list.
       *
       * @param event Pointer to the G4Event.
       */
      virtual void GeneratePrimaryVertex(G4Event* event);


    protected:

      std::string m_mcCollectionName;     /**< The name of the MCParticle collection from which the MCParticles should be read.*/
      MCParticleGraph& m_mcParticleGraph; /**< Reference to the MCParticle graph which is filled by this class. */

      /**
       * Takes a MCParticle and creates a primary particle for Geant4.
       * The daughters of the specified MCParticle are added recursively.
       *
       * @param motherIndex Graph MCParticle
       * @return Returns a pointer to the created primary particle. NULL if the primary particle could not be created.
       */
      void addParticle(MCParticle &mcParticle, G4Event* event, G4PrimaryParticle* lastG4Mother, int motherIndex, bool useTime);

    };

  } //end of Simulation namespace

} //end of Belle2 namespace

#endif /* MCPARTICLEGENERATOR_H_ */
