/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/bklm/geometry/Module.h>
#include <klm/dataobjects/bklm/BKLMSimHit.h>
#include <klm/dataobjects/bklm/BKLMSimHitPosition.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMSimHit.h>
#include <klm/dbobjects/bklm/BKLMSimulationPar.h>

/* Belle 2 headers. */
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dataobjects/MCParticle.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {

  class BkgSensitiveDetector;

  namespace KLM {

    /**
     * KLM sensitive-detector class.
     */
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {

    public:

      /**
       * Constructor.
       * @param[in] name        Sensitive-detector name.
       * @param[in] subdetector Subdetector.
       */
      SensitiveDetector(const G4String& name, KLMSubdetectorNumber subdetector);

      /**
       * Process each step and store BKLMSimHits or EKLMSimHits.
       */
      bool step(G4Step*, G4TouchableHistory*) override;

    private:

      /**
       * Step in BKLM.
       */
      bool stepBKLM(G4Step*, G4TouchableHistory*);

      /**
       * Step in EKLM.
       */
      bool stepEKLM(G4Step*, G4TouchableHistory*);

      /**
       * Find the ranges of matching RPC strips for BKLM hits.
       */
      void convertHitToRPCStrips(const CLHEP::Hep3Vector&, const bklm::Module*, int&, int&, int&, int&);

      /** Subdetector. */
      KLMSubdetectorNumber m_Subdetector;

      /** Flag to enforce once-only initializations in Initialize(). */
      bool m_FirstCall;

      /** Maximum permissible hit time based on overflow of LeCroy 1877 TDC. */
      double m_HitTimeMax;

      /** All hits with time large than m_ThresholdHitTime will be dropped. */
      double m_ThresholdHitTime;

      /** Section depth. */
      static constexpr int m_DepthSection = 2;

      /** Sector depth. */
      static constexpr int m_DepthSector = 3;

      /** Layer depth. */
      static constexpr int m_DepthLayer = 5;

      /** Plane depth. */
      static constexpr int m_DepthPlane = 9;

      /** Scintillator depth. */
      static constexpr int m_DepthScintillator = 10;

      /**
       * Pointer to a sensitive-detector object
       * used for beam-background steps.
       */
      BkgSensitiveDetector* m_BkgSensitiveDetector;

      /** Pointer to GeometryPar singleton. */
      bklm::GeometryPar* m_GeoPar;

      /** EKLM element numbers. */
      const EKLMElementNumbers* m_ElementNumbers;

      /** Simulation parameters (from DB). */
      DBObjPtr<BKLMSimulationPar> m_SimPar;

      /** MC particles. */
      StoreArray<MCParticle> m_MCParticles;

      /** BKLM simulation hits. */
      StoreArray<BKLMSimHit> m_BKLMSimHits;

      /** EKLM simulation hits. */
      StoreArray<EKLMSimHit> m_EKLMSimHits;

      /** BKLM simulation hit positions. */
      StoreArray<BKLMSimHitPosition> m_BKLMSimHitPositions;

      /** Relation array between MCPartices and BKLMSimHits. */
      RelationArray m_MCParticlesToBKLMSimHits{m_MCParticles, m_BKLMSimHits};

      /** Relation array between MCPartices and EKLMSimHits. */
      RelationArray m_MCParticlesToEKLMSimHits{m_MCParticles, m_EKLMSimHits};

    };

  }

}