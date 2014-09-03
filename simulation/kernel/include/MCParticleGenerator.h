/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter, Marko Staric                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCPARTICLEGENERATOR_H_
#define MCPARTICLEGENERATOR_H_

#include <mdst/dataobjects/MCParticleGraph.h>

#include <G4VPrimaryGenerator.hh>
#include <G4PrimaryParticle.hh>
#include <G4Event.hh>

#include <string>

namespace Belle2 {

  namespace Simulation {

    /**
     * This class creates Geant4 primary particles from MCParticle list
     * The generator assumes that each particle in the list has exactly one mother.
     */
    class MCParticleGenerator: public G4VPrimaryGenerator {

    public:

      /** The constructor of the MCParticleGenerator class.
       * @param mcCollectionName MCParticle collection from which MCParticles are read.
       * @param mcParticleGraph Reference to the MCParticle graph that will be filled
       */
      MCParticleGenerator(const std::string& mcCollectionName,
                          MCParticleGraph& mcParticleGraph);

      /** The destructor of the MCParticleGenerator class. */
      virtual ~MCParticleGenerator();

      /**
       * Create G4 primary particles from MCParticle list.
       * This method is called for each event.
       *
       * @param event Pointer to the G4Event.
       */
      virtual void GeneratePrimaryVertex(G4Event* event);


    protected:

      std::string m_mcCollectionName;     /**< Name of the MCParticle collection */
      MCParticleGraph& m_mcParticleGraph; /**< Reference to the MCParticle graph */

      /**
       * Takes a MCParticle and creates a primary particle for Geant4.
       * The daughters of the specified MCParticle are added recursively.
       *
       * @param mcParticle a reference to MCParticle
       * @param event a pointer to Geant4 event
       * @param lastG4Mother a pointer to G4 mother particle
       * @param motherIndex mother index from MCParticle graph
       * @param useTime use MCParticle decay time (as given by the generator)
       */
      void addParticle(MCParticle& mcParticle,
                       G4Event* event,
                       G4PrimaryParticle* lastG4Mother,
                       int motherIndex,
                       bool useTime);

    };

  } //end of Simulation namespace

} //end of Belle2 namespace

#endif /* MCPARTICLEGENERATOR_H_ */
