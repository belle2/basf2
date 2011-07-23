/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSENSITIVEDETECTOR_H
#define BKLMSENSITIVEDETECTOR_H

#include <bklm/hitbklm/BKLMSimHit.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

#include <TRandom3.h>

namespace Belle2 {

  //! The Class for BKLM Sensitive Detector
  //! Each qualified simulation step is saved into a StoreArray of BKLMSimHits.
  class BKLMSensitiveDetector: public Simulation::SensitiveDetectorBase {

  public:

    //! Constructor
    BKLMSensitiveDetector(G4String name);

    //! Destructor
    ~BKLMSensitiveDetector() {}

    //! Initialize storage of hits for one event
    void Initialize(G4HCofThisEvent*);

    //! Process each step in the BKLM
    G4bool ProcessHits(G4Step*, G4TouchableHistory*);

    //! Process each event
    void EndOfEvent(G4HCofThisEvent*);

    //! Overlay one random background event
    //void AddbgOne( bool );

  private:

    //! Find and record matching RPC strips for each simHit
    void convertHitToRPCStrips(BKLMSimHit*);

    //! Flag to say whether background study will be done or not
    bool m_DoBackgroundStudy;

    //! PDG encoding for neutron
    int m_NeutronPID;

    //! maximum permissible hit time (based on overflow of LeCroy 1877 TDC)
    double m_HitTimeMax;

    //! Random number generator
    TRandom3* m_Random;

    //! Hit number within one event
    int m_HitNumber;

    //! Flag to enforce once-only initializations in Initialize()
    bool m_FirstCall;

  };

} // end of namespace Belle2

#endif // BKLMSENSITIVEDETECTOR_H
