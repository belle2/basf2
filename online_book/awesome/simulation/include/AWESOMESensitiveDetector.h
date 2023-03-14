/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* AWESOME headers. */
#include <online_book/awesome/dataobjects/AWESOMESimHit.h>

/* Basf2 headers. */
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
    StoreArray<AWESOMESimHit> m_SimHits;

    /** Relation array between MCParticles and AWESOMESimHits. */
    RelationArray m_MCParticlesToSimHits{m_MCParticles, m_SimHits};

  };

}
