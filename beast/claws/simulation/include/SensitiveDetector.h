/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SENSITIVEDETECTOR_H
#define SENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <beast/claws/dataobjects/CLAWSSimHit.h>
#include <beast/claws/simulation/SensorTraversal.h>
#include <G4EmSaturation.hh>

#include <stack>
namespace Belle2 {
  /** Namespace to encapsulate code needed for the CLAWS detector */
  namespace claws {

    /** Sensitive Detector implementation of the CLAWS detector */
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {
    public:
      /** Constructor */
      SensitiveDetector();
    protected:
      /** Step processing method
       * @param step the G4Step with the current step information
       * @return true if a Hit has been created, false if the hit was ignored
       */
      bool step(G4Step* step, G4TouchableHistory*) override;
    private:
      /** finish a track */
      bool finishTrack();

      /** needed to call Birk's law*/
      G4EmSaturation* m_saturationEngine;

      /** store array of the MCParticles */
      StoreArray<MCParticle>  m_mcParticles;
      /** store array of the SimHits */
      StoreArray<CLAWSSimHit>  m_simHits;
      /** relation array of the MCParticle -> SimHit relation */
      RelationArray m_relMCSimHit{m_mcParticles, m_simHits};

      /** Stack of tracks to keep track of particles */
      std::stack<SensorTraversal> m_tracks;
    };

  } //claw namespace
} //Belle2 namespace

#endif
