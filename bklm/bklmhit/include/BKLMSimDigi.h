/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSIMDIGI_H
#define BKLMSIMDIGI_H

#include "TObject.h"

//Geant4 classes
#include "G4VHit.hh"
#include "G4VDigi.hh"
#include "G4TDigiCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

namespace Belle2 {

  class BKLMSimHit;

  class BKLMSimDigi : public G4VDigi {

  public:

    enum Direction {
      PHISTRIP = 0,
      THETASTRIP = 1
    };

    enum BarrelEndcap {
      BARREL = 0,
      ENDCAP = 1
    };

    enum ForwardBackward {
      FORWARD = 0,
      BACKWARD = 1
    };

    enum Status {
      /// Unused
      UNUSED = 0,
      /// Used in Projected (Extrapolated) Track
      PTRACK = 1,
      /// Used in Shower
      SHOWER = 2,
      /// Used in Independent Track
      ITRACK = 4,
      /// 2d Hit not real (created by mismatch Phi and Z, or Phi and Theta hits).
      MIRROR = 8,
      /// Out of time window cuts
      OOTIME = 16,
      /// Background hits
      BGHITS = 32,
      /// Added for multiplicity study in Monte Carlo
      MCMULT = 64,
      /// Strip hit but inefficient in Monte Carlo
      MCKILL = 128
    };

    inline BKLMSimDigi() :
        m_be(-1),
        m_fb(-1),
        m_sector(-1),
        m_layer(-1),
        m_direction(-1),
        m_stripNo(-1),
        m_time(0.0),
        m_status(0),
        m_hit(NULL),
        m_firstDigi(NULL) {}

    inline BKLMSimDigi(G4int be, G4int fb, G4int sector, G4int layer,
                       G4int direction, G4int stripNo, G4double time,
                       G4int status, const BKLMSimHit* hit) :
        m_be(be),
        m_fb(fb),
        m_sector(sector),
        m_layer(layer),
        m_direction(direction),
        m_stripNo(stripNo),
        m_time(time),
        m_status(status),
        m_hit(hit),
        m_firstDigi(NULL) {}

    ~BKLMSimDigi() {}

    void Draw(void) {}
    void Print(void) {}
    void Store(void) const;

    inline const G4int        GetBE() const { return m_be; }
    inline const G4int        GetFB() const { return m_fb; }
    inline const G4int        GetSector() const { return m_sector; }
    inline const G4int        GetLayer() const { return m_layer; }
    inline const G4int        GetDirection() const { return m_direction; }
    inline const G4int        GetStripNo() const { return m_stripNo; }
    inline const G4double     GetTime() const { return m_time; }
    inline const G4int        GetStatus() const { return m_status; }
    inline const BKLMSimHit*  GetHit() const { return m_hit; }
    inline const BKLMSimDigi* GetFirstDigi() const { return m_firstDigi; }
    inline void               SetFirstDigi(const BKLMSimDigi* digi) { m_firstDigi = digi; }

    //inline void* operator new( size_t );
    //inline void operator delete( void* );

  private:

    G4int              m_be;
    G4int              m_fb;
    G4int              m_sector;
    G4int              m_layer;
    G4int              m_direction;
    G4int              m_stripNo;
    G4double           m_time;
    G4int              m_status;
    const BKLMSimHit*  m_hit;
    const BKLMSimDigi* m_firstDigi;

  };

  /*
  extern G4Allocator<BKLMSimDigi> BKLMSimDigiAllocator;

  inline void* BKLMSimDigi::operator new( size_t ) {
    return (void*)BKLMSimDigiAllocator.MallocSingle();
  }

  inline void BKLMSimDigi::operator delete( void* aDigi ) {
    BKLMSimDigiAllocator.FreeSingle( (BKLMSimDigi*)aDigi );
  }
  */

  typedef G4TDigiCollection<BKLMSimDigi> BKLMSimDigiCollection;

} // end of namespace Belle2

#endif // BKLMSIMDIGI_H
