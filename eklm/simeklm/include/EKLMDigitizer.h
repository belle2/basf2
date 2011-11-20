/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Timofey Uglov                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDIGITIZER_H
#define EKLMDIGITIZER_H

#include <eklm/eklmutils/EKLMutils.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMStripHit.h>
#include <map>
#include <vector>
#include  "CLHEP/Vector/ThreeVector.h"



namespace Belle2 {


  //! Digitize EKLMSimHits  to get EKLM StripHits
  class EKLMDigitizer {

  public:

    //! Constructor
    EKLMDigitizer() {};

    //! Destructor
    ~EKLMDigitizer() {};

    //! Read hits from the store, sort sim hits and fill m_HitStripMap
    void readAndSortSimHits();

    //! merges hits from the same strip. Creates EKLMStripHits
    void mergeSimHitsToStripHits();

    //! Operator new
    void *operator new(size_t);

    //! Operator delete
    void operator delete(void *aEKLMDigitizer);

  private:

    //! std::map for hits sorting according strip name
    std::map<G4VPhysicalVolume *, std::vector<EKLMSimHit*> > m_HitStripMap;

    //! vector of EKLMStripHits
    std::vector<EKLMStripHit*> m_HitVector;

    //! sim hits vector
    std::vector<EKLMSimHit*> m_simHitsVector;


  };

  //! Extern allocator
  extern G4Allocator<EKLMDigitizer> EKLMDigitizerAllocator;

} // end of namespace Belle2

#endif
