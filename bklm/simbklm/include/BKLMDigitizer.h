/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMDIGITIZER_H
#define BKLMDIGITIZER_H

#include "G4VDigitizerModule.hh"
#include "G4ThreeVector.hh"

#include <bklm/bklmhit/BKLMSimHit.h>
#include <bklm/bklmhit/BKLMSimDigi.h>
#include <vector>

namespace Belle2 {

  //! Digitize BKLMSimHits to get BKLM StripHits
  class BKLMDigitizer : public G4VDigitizerModule {

  public:

    //! Constructor
    BKLMDigitizer(G4String name);

    //! Destructor
    ~BKLMDigitizer() {}

    //! Convert all SimHits to SimDigis
    void Digitize();

    //! Store all SimHits and SimDigis
    G4bool Store();

    //! Operator new
    //inline void *operator new( size_t );

    //! Operator delete
    //inline void operator delete( void* aBKLMDigitizer );

  private:

    //! name of hits collection
    G4String m_hitsName;

    //! name of digits collection
    G4String m_digiName;

    //! convert one SimHit to SimDigi(s)
    G4bool hitToDigi(const BKLMSimHit*, G4int&, G4int&, G4int&, G4int&,
                     G4double&, G4int&, std::vector<G4int>&, std::vector<G4int>&);

  };

  /*
  //! Allocator
  extern G4Allocator<BKLMDigitizer> BKLMDigitizerAllocator;

  //! Operator new
  inline void* BKLMDigitizer::operator new( size_t )
  {
    void* aBKLMDigitizer;
    aBKLMDigitizer = (void*)BKLMDigitizerAllocator.MallocSingle();
    return aBKLMDigitizer;
  }

  //! Operator delete
  inline void BKLMDigitizer::operator delete( void* aBKLMDigitizer )
  {
    BKLMDigitizerAllocator.FreeSingle( (BKLMDigitizer*)aBKLMDigitizer );
  }
  */

} // end of namespace Belle2

#endif // BKLMDIGITIZER_H
