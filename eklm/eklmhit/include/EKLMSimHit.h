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


#include <TObject.h>
#include <simulation/simkernel/B4VHit.h>


//Geant4 classes
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"



namespace Belle2 {

  //! Class to handle simulation hits
  class EKLMSimHit  :  public B4VHit  {

  public:

    //! constructor needed to make the class storable
    inline EKLMSimHit() {};

    //! Constructor with initial values
    inline EKLMSimHit(G4ThreeVector pos, G4double time, G4int PDGcode,  G4double eDep) {
      m_pos = pos;
      m_time = time;
      m_PDGcode = PDGcode;
      m_eDep = eDep;
    };

    //! Destructor
    ~EKLMSimHit() {};

    //! returns position of the hit
    inline G4ThreeVector getPos() const
    {return m_pos;}

    //! returns hit time
    inline G4double getTime() const
    {return m_time;}

    //! returns energy deposition
    inline G4double getEDep() const
    {return m_eDep;}

    //! returns PDG code of the particle
    inline G4int getPDGCode() const
    {return m_PDGcode;}

    //! dumps hit into ASCII file
    void Save(char * filename);

  private:
    //! hit position (in global reference frame)
    G4ThreeVector m_pos;

    //!hit time
    G4double m_time;

    //! energy depostion
    G4double m_eDep;

    //! PDG code of the track particle
    G4int m_PDGcode;

    ClassDef(EKLMSimHit, 1);

  };

  typedef G4THitsCollection<EKLMSimHit> EKLMSimHitsCollection;

} // end of namespace Belle2

#endif //EKLMSIMHIT_H
