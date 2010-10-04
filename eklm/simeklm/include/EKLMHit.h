/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHIT_H
#define EKLMHIT_H

#include <simulation/simkernel/B4VHit.h>

//Geant4 classes
#include "G4THitsCollection.hh"
#include "G4VHit.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

namespace Belle2 {

  //! The Class for EKLM Hit
  /*! Implementation of EKLM hit for BelleII experiment.
   */

  class EKLMHit : public B4VHit {

  public:

    //! Constructor with initial values
    inline EKLMHit(G4String StripName, G4ThreeVector pos, G4double time, G4int decayed,
                   G4int trackID, G4int parentID, G4int PDGcode,
                   G4int motherPDGcode, G4int firstID,  G4double eDep)
        :
        m_StripName(StripName),
        m_pos(pos),
        m_time(time),
        m_decayed(decayed),
        m_trackID(trackID),
        m_parentID(parentID),
        m_PDGcode(PDGcode),
        m_motherPDGcode(motherPDGcode),
        m_firstID(firstID),
        m_eDep(eDep) {};

    //! Destructor
    ~EKLMHit() {};

    //! Operator new
    inline void *operator new(size_t);

    //! Operator delete
    inline void operator delete(void *aEKLMHit);

    //! Print hits information
    void Print();

    //! Save hits into ASCII file
    void Save(FILE *oFile);


  private:
    G4String m_StripName;
    G4ThreeVector m_pos;
    G4double m_time;
    G4int m_decayed;
    G4int m_trackID;
    G4int m_parentID;
    G4int m_PDGcode;
    G4int m_motherPDGcode;
    G4int m_firstID;
    G4double m_eDep;


  };

  typedef G4THitsCollection<EKLMHit> EKLMHitsCollection;

  extern G4Allocator<EKLMHit> EKLMHitAllocator;


  // Operator new
  inline void* EKLMHit::operator new(size_t)
  {
    void *aEKLMHit;
    aEKLMHit = (void *) EKLMHitAllocator.MallocSingle();
    return aEKLMHit;
  }

  // Operator delete
  inline void EKLMHit::operator delete(void *aEKLMHit)
  {
    EKLMHitAllocator.FreeSingle((EKLMHit*) aEKLMHit);
  }

} // end of namespace Belle2

#endif //EKLMHIT_H
