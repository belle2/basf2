/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMSimHit.h>

/* Belle 2 headers. */
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2::EKLM {

  /**
   * The Class for EKLM Sensitive Detector.
   * @details
   * In this class, every variables defined in EKLMSimHit will be calculated.
   * EKLMSimHits are saved into hits collection.
   */
  class EKLMSensitiveDetector : public Simulation::SensitiveDetectorBase  {

  public:

    /**
     * Constructor.
     */
    explicit EKLMSensitiveDetector(G4String name);

    /**
     * Destructor.
     */
    ~EKLMSensitiveDetector();

    /**
     * Process each step and calculate variables for EKLMSimHit
     * store EKLMSimHit.
     */
    bool step(G4Step* aStep, G4TouchableHistory* history) override;

  private:

    /** Element numbers. */
    const EKLMElementNumbers* m_ElementNumbers;

    /** MC particles. */
    StoreArray<MCParticle> m_MCParticles;

    /** Simulation hits. */
    StoreArray<EKLMSimHit> m_SimHits;

    /** Relation array between MCPartices and EKLMSimHits. */
    RelationArray m_MCParticlesToSimHits{m_MCParticles, m_SimHits};

    /**
     * All hits with time large  than m_ThresholdHitTime
     * will be dropped.
     */
    G4double m_ThresholdHitTime;

  };

}
