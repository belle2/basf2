/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/MCParticleGraph.h>

#include <G4VPrimaryGenerator.hh>
#include <G4PrimaryParticle.hh>
#include <G4Event.hh>

#include <string>

class G4VSolid;

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
      /** Pointer to the top level solid to check if particles are inside the simulation volume */
      G4VSolid* m_topSolid{nullptr};
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
      void addParticle(const MCParticle& mcParticle,
                       G4Event* event,
                       G4PrimaryParticle* lastG4Mother,
                       int motherIndex,
                       bool useTime);

      /**
       * Create a simulation vertex for the given particle.
       *
       * This checks if the particle starts inside the simulation volume. If
       * it's inside trivially just create a vertex at the given position and
       * time. If not check if the particle intersects with the simulation
       * volume.
       *
       * If it does intersect and will survive long enough to get there we
       * create a vertex at the simulation volume boundary and also set
       * productionTimeShift to the flight time to get to the simulation volume.
       *
       * @param p Particle to create the vertex for
       * @param productionTimeShift return the flight time of the particle
       *    before reaching the simulation volume
       * @returns a new G4PrimaryVertex or nullptr if the particle didn't
       *    intersect with the simulation volume in time.
       */
      G4PrimaryVertex* determineVertex(const MCParticleGraph::GraphParticle& p, double& productionTimeShift);
    };

  } //end of Simulation namespace

} //end of Belle2 namespace
