/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSIMHIT_H
#define EKLMSIMHIT_H

#include <simulation/simkernel/B4VHit.h>

//Geant4 classes
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"



namespace Belle2 {

  class EKLMSimHit  : public B4VHit {

  public:

    //! Constructor with initial values

    inline EKLMSimHit(G4ThreeVector pos, G4double time, G4int PDGcode,  G4double eDep) {
      m_pos = pos;
      m_time = time;
      m_PDGcode = PDGcode;
      m_eDep = eDep;
    };

    //! Destructor
    ~EKLMSimHit() {};

    inline G4ThreeVector getPos() const
    {return m_pos;}

    inline G4double getTime() const
    {return m_time;}

    inline G4double getEDep() const
    {return m_eDep;}

    inline G4int getPDGCode() const
    {return m_PDGcode;}



    void Print();
    void Save(char * filename);

  private:
    G4ThreeVector m_pos;
    G4double m_time;
    G4double m_eDep;
    G4int m_PDGcode;
  };

  typedef G4THitsCollection<EKLMSimHit> EKLMSimHitsCollection;

} // end of namespace Belle2

#endif //EKLMSIMHIT_H
