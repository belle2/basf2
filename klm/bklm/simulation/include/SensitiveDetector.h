/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMSimHit.h>
#include <klm/dataobjects/bklm/BKLMSimHitPosition.h>
#include <klm/dbobjects/bklm/BKLMSimulationPar.h>

/* Belle 2 headers. */
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dataobjects/MCParticle.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {

  class BkgSensitiveDetector;

  namespace bklm {

    class Module;
    class GeometryPar;

    //! Class for the BKLM Sensitive Detector
    //! Each qualified simulation step is saved into a StoreArray of BKLMSimHits.
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {

    public:

      //! Constructor
      explicit SensitiveDetector(const G4String& name);

      //! Process each step in the BKLM
      bool step(G4Step*, G4TouchableHistory*) override;

    private:

      //! Find the ranges of matching RPC strips for each simulated hit
      void convertHitToRPCStrips(const CLHEP::Hep3Vector&, const Module*, int&, int&, int&, int&);

      //! Flag to enforce once-only initializations in Initialize()
      bool m_FirstCall;

      //! maximum permissible hit time (based on overflow of LeCroy 1877 TDC)
      double m_HitTimeMax;

      //! Section depth.
      static constexpr int m_DepthSection = 2;

      //! Sector depth.
      static constexpr int m_DepthSector = 3;

      //! Layer depth.
      static constexpr int m_DepthLayer = 5;

      //! Plane depth.
      static constexpr int m_DepthPlane = 9;

      //! Scintillator depth.
      static constexpr int m_DepthScintillator = 10;

      //! Pointer to a sensitive-detector object used for beam-background steps
      BkgSensitiveDetector* m_BkgSensitiveDetector;

      //! Pointer to GeometryPar singleton
      GeometryPar* m_GeoPar;

      //! Simulation parameters (from DB)
      DBObjPtr<BKLMSimulationPar> m_SimPar;

      //! MC particles.
      StoreArray<MCParticle> m_MCParticles;

      //! BKLM simulated hits.
      StoreArray<BKLMSimHit> m_SimHits;

      //! BKLM simulated hit positions.
      StoreArray<BKLMSimHitPosition> m_SimHitPositions;

      //! Relation array between MCPartices and BKLMSimHits.
      RelationArray m_MCParticlesToSimHits{m_MCParticles, m_SimHits};

    };

  } // end of namespace bklm

} // end of namespace Belle2
