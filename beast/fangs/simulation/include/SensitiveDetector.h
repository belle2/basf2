/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SENSITIVEDETECTOR_H
#define SENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <beast/fangs/dataobjects/FANGSSimHit.h>
#include <beast/fangs/simulation/SensorTraversal.h>

#include <stack>
namespace Belle2 {
  /** Namespace to encapsulate code needed for the FANGS detector */
  namespace fangs {

    /** Sensitive Detector implementation of the FANGS detector */
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

      /** store array of the MCParticles */
      StoreArray<MCParticle>  m_mcParticles;
      /** store array of the SimHits */
      StoreArray<FANGSSimHit>  m_simHits;
      /** relation array of the MCParticle -> SimHit relation */
      RelationArray m_relMCSimHit{m_mcParticles, m_simHits};

      /** Stack of tracks to keep track of particles */
      std::stack<SensorTraversal> m_tracks;
    };

  } //claw namespace
} //Belle2 namespace

#endif
