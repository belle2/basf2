/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SENSITIVEDETECTOR_H
#define SENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {

  class BKLMSimHit;
  class BkgSensitiveDetector;

  namespace bklm {

    class Module;
    class GeometryPar;
    class SimulationPar;

    //! Class for the BKLM Sensitive Detector
    //! Each qualified simulation step is saved into a StoreArray of BKLMSimHits.
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {

    public:

      //! Constructor
      explicit SensitiveDetector(const G4String& name);

      //! Process each step in the BKLM
      bool step(G4Step*, G4TouchableHistory*) override;

      //! Tidy up at the end of each event
      void EndOfEvent(G4HCofThisEvent*) override {}

    private:

      //! Find the ranges of matching RPC strips for each simulated hit
      void convertHitToRPCStrips(const CLHEP::Hep3Vector&, const Module*, int&, int&, int&, int&);

      //! Flag to enforce once-only initializations in Initialize()
      bool m_FirstCall;

      //! maximum permissible hit time (based on overflow of LeCroy 1877 TDC)
      double m_HitTimeMax;

      //! Pointer to a sensitive-detector object used for beam-background steps
      BkgSensitiveDetector* m_BkgSensitiveDetector;

      //! Pointer to GeometryPar singleton
      GeometryPar* m_GeoPar;

      //! Pointer to SimulationPar singleton
      SimulationPar* m_SimPar;

    };

  } // end of namespace bklm

} // end of namespace Belle2
#endif // SENSITIVEDETECTOR_H
