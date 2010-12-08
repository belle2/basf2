/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCPARTICLEGENERATOR_H_
#define MCPARTICLEGENERATOR_H_

#include <G4VPrimaryGenerator.hh>

#include <string>

class G4Event;

namespace Belle2 {

  namespace Simulation {

    /**
     * The MCParticleGenerator class generates the Geant4 primary particles using the MCParticle list from the DataStore.
     */
    class MCParticleGenerator: public G4VPrimaryGenerator {

    public:

      /** The constructor of the MCParticleGenerator class.
       * @param mcCollectionName The name of the MCParticle collection from which the MCParticles should be read.
       */
      MCParticleGenerator(const std::string& mcCollectionName);

      /** The destructor of the MCParticleGenerator class. */
      virtual ~MCParticleGenerator();

      /**
       * This method is called for each event by Geant4 and creates the primary particles from the MCParticle list.
       *
       * @param event Pointer to the G4Event.
       */
      virtual void GeneratePrimaryVertex(G4Event* event);


    protected:

      std::string m_mcCollectionName; /**< The name of the MCParticle collection from which the MCParticles should be read.*/

    };

  } //end of Simulation namespace

} //end of Belle2 namespace

#endif /* MCPARTICLEGENERATOR_H_ */
