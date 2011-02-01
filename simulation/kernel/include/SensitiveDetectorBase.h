/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SENSITIVEDETECTOR_H_
#define SENSITIVEDETECTOR_H_

#include <generators/dataobjects/MCParticle.h>

#include <G4VSensitiveDetector.hh>
#include <G4Step.hh>
#include <TObject.h>

#include <string>


namespace Belle2 {

  namespace Simulation {

    /**
     * The base sensitive detector class.
     * It provides support for automatic relation creation and
     * offers additional, convenient methods for the user.
     */
    class SensitiveDetectorBase : public G4VSensitiveDetector {

    public:

      /**
       * Constructor.
       */
      SensitiveDetectorBase(G4String name);

      /**
       * Destructor.
       */
      virtual ~SensitiveDetectorBase();

      /**
       * Adds a new relation collection for the relation Hit->MCParticle.
       *
       * @param hitMCParticleColName The name of the collection storing the relations of hits to MCParticles.
       */
      void addRelationCollection(const std::string& hitMCParticleColName);

      /**
       * Returns the name of the collection storing the relations between hits and MCParticles.
       *
       * @return The name of the collection storing the relations between hits and MCParticles.
       */
      std::string getRelationCollectionName() { return m_hitMCParticleColName; };

      /**
       * Sets the SeenInDetector flag for the MCParticle which created the step.
       * Each subdetector should set its flag to mark MCParticles that traversed the subdetector.
       *
       * @param step Pointer to the step representing the track which created the hit.
       * @param subdetectorBit The subdetector specific bit. See the MCParticle class for the list of available bits.
       */
      void setSeenInDetectorFlag(G4Step* step, MCParticle::StatusBit subdetectorBit);


    protected:

      std::string m_hitMCParticleColName; /**< The name of the collection storing the relations of hits to MCParticles.*/
    };

  }  // end namespace Simulation
} // end namespace Belle2

#endif /* SENSITIVEDETECTOR_H_ */
