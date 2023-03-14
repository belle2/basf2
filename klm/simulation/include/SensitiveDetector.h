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
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMSimHit.h>
#include <klm/dbobjects/bklm/BKLMSimulationPar.h>

/* Basf2 headers. */
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
       * Process each step and store KLMSimHits.
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

      /** Simulation hits. */
      StoreArray<KLMSimHit> m_KLMSimHits;

      /** Relation array between MCPartices and KLMSimHits. */
      RelationArray m_MCParticlesToKLMSimHits{m_MCParticles, m_KLMSimHits};

    };

  }

}
