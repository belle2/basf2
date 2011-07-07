/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSIMHIT_H
#define BKLMSIMHIT_H

#include <TObject.h>

//Geant4 classes
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

namespace Belle2 {

  //! Store one simulation hit
  class BKLMSimHit : public G4VHit {

  public:

    //! constructor needed to make the class storable
    inline BKLMSimHit() {}

    //! Constructor with initial values
    inline BKLMSimHit(G4ThreeVector hitPos, G4double hitTime, G4double deltaE,
                      G4bool inRPCGas, G4bool decayed,
                      G4int trackID,
                      G4int primaryPID, G4int parentPID, G4int ancestorPID, G4int firstPID) :
        m_hitPos(hitPos),
        m_hitTime(hitTime),
        m_deltaE(deltaE),
        m_inRPCGas(inRPCGas),
        m_decayed(decayed),
        m_trackID(trackID),
        m_primaryPID(primaryPID),
        m_parentPID(parentPID),
        m_ancestorPID(ancestorPID),
        m_firstPID(firstPID) {}

    //! Destructor
    ~BKLMSimHit() {}

    //! returns position of the hit
    inline G4ThreeVector getHitPos() const { return m_hitPos; }

    //! returns hit time
    inline G4double getHitTime() const     { return m_hitTime; }

    //! returns energy deposition
    inline G4double getDeltaE() const      { return m_deltaE; }

    //! returns flag whether hit is in RPC gas (true) or scintillator (false)
    inline G4bool getInRPCGas() const      { return m_inRPCGas; }

    //! returns flag whether particle decayed in this hit
    inline G4bool getDecayed() const       { return m_decayed; }

    //! returns PDG code of the particle
    inline G4int getPrimaryPID() const     { return m_primaryPID; }

    //! returns PDG code of the particle's parent
    inline G4int getParentPID() const      { return m_parentPID; }

    //! returns PDG code of the particle's oldest ancestor
    inline G4int getAncestorPID() const    { return m_ancestorPID; }

    //! returns PDG code of the particle's generator ancestor
    inline G4int getFirstPID() const       { return m_firstPID; }

    void Print() {}

    void Draw() {}

    //! dumps hit into ASCII file
    void Save(char* filename);

    //inline void* operator new( size_t );

    //inline void operator delete( void *aHit );

  private:

    //! global-frame hit position
    G4ThreeVector m_hitPos;

    //! global hit time
    G4double m_hitTime;

    //! energy deposition
    G4double m_deltaE;

    //! flag to say whether the hit is in RPCgas (true) or scintillator (false)
    G4bool m_inRPCGas;

    //! flag to say whether the track decayed in this hit
    G4bool m_decayed;

    //! GEANT4 track identifier
    G4int m_trackID;

    //! PDG code of the track's particle
    G4int m_primaryPID;

    //! PDG code of the track's particle's parent
    G4int m_parentPID;

    //! PDG code of the track's oldest ancestor according to GEANT4
    G4int m_ancestorPID;

    //! PDG code of the track's generated ancestor
    G4int m_firstPID;

    //! Needed to make root object storable
    ClassDef(BKLMSimHit, 1);

  };

  /*
  extern G4Allocator<BKLMSimHit> BKLMSimHitAllocator;

  inline void* BKLMSimHit::operator new( size_t ) {
    void *aHit;
    aHit = (void*)BKLMSimHitAllocator.MallocSingle();
    return aHit;
  }

  inline void BKLMSimHit::operator delete( void *aHit ) {
    BKLMSimHitAllocator.FreeSingle( (BKLMSimHit*)aHit );
  }
  */

  //! define collection of BKLMSimHits
  typedef G4THitsCollection<BKLMSimHit> BKLMSimHitsCollection;

} // end of namespace Belle2

#endif //BKLMSIMHIT_H
