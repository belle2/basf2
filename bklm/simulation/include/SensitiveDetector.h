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

  namespace bklm {

    //! Class for the BKLM Sensitive Detector
    //! Each qualified simulation step is saved into a StoreArray of BKLMSimHits.
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {

    public:

      //! Constructor
      SensitiveDetector();

      //! Process each step in the BKLM
      bool step(G4Step*, G4TouchableHistory*);

      //! Tidy up at the end of each event
      void EndOfEvent(G4HCofThisEvent*) {}

    private:

      //! Find and record matching RPC strips for each simHit
      void convertHitToRPCStrips(BKLMSimHit*);

      //! Flag to say whether background study will be done or not
      bool m_DoBackgroundStudy;

      //! PDG encoding for neutron
      int m_NeutronPID;

      //! maximum permissible hit time (based on overflow of LeCroy 1877 TDC)
      double m_HitTimeMax;

      //! Flag to enforce once-only initializations in Initialize()
      bool m_FirstCall;

    };

  } // end of namespace bklm

} // end of namespace Belle2
#endif // SENSITIVEDETECTOR_H
