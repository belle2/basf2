/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* AWESOME headers. */
#include <awesome/dataobjects/AwesomeSimHit.h>

/* Belle2 headers. */
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

/* Geant4 headers. */
#include <G4Step.hh>
#include <G4TouchableHistory.hh>

namespace Belle2::AWESOME {

  /** Sensitive Detector implementation of the AWESOME detector */
  class AWESOMESensitiveDetector : public Simulation::SensitiveDetectorBase {

  public:

    /** Constructor */
    AWESOMESensitiveDetector();

    /**
     * Step processing method.
     * @param step The G4Step with the current step information.
     * @return true if a hit was created, false otherwise.
     */
    bool step(G4Step* step, G4TouchableHistory*) override;

  private:

    /** MC particles. */
    StoreArray<MCParticle> m_MCParticles;

    /** AWESOME simulated hits. */
    StoreArray<AwesomeSimHit> m_SimHits;

    /** Relation array between MCParticles and AwesomeSimHits. */
    RelationArray m_MCParticlesToSimHits{m_MCParticles, m_SimHits};

  };

}
