/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSIMHIT_H
#define EKLMSIMHIT_H


#include <TObject.h>

#include "G4VPhysicalVolume.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

#include "eklm/eklmhit/EKLMHitBase.h"

#include "G4VHit.hh"
namespace Belle2 {

  //! Class to handle simulation hits
  //  class EKLMSimHit  : public G4VHit  {
  class EKLMSimHit : public EKLMHitBase  {

  public:

    //! constructor needed to make the class storable
    inline EKLMSimHit() {};

    //! Constructor with initial values
    inline EKLMSimHit(G4VPhysicalVolume *pv, G4ThreeVector global_pos,
                      G4ThreeVector local_pos, G4double time, G4int PDGcode,
                      G4double eDep) {
      this->m_pv = pv;
      this->m_global_pos = global_pos;
      this->m_local_pos = local_pos;
      this->m_time = time;
      this->m_PDGcode = PDGcode;
      this->m_eDep = eDep;
    };

    //! Destructor
    ~EKLMSimHit() {};

    //! returns physical volume
    inline G4VPhysicalVolume *getPV() const
    {return m_pv;}

    //! returns global position of the hit
    inline G4ThreeVector getGlobalPos() const
    {return m_global_pos;}

    //! returns local position of the hit
    inline G4ThreeVector getLocalPos() const
    {return m_local_pos;}

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
    //! Physical volume
    G4VPhysicalVolume *m_pv; //! {ROOT streamer directive}

    //! hit position (in global reference frame)
    G4ThreeVector m_global_pos;

    //! hit position (in local reference frame)
    G4ThreeVector m_local_pos;

    //!hit time
    G4double m_time;

    //! energy depostion
    G4double m_eDep;

    //! PDG code of the track particle
    G4int m_PDGcode;

    //! Needed to make root object storable
    ClassDef(Belle2::EKLMSimHit, 1);

  };

  //! define collections of Sim Hits
  //  typedef G4THitsCollection<EKLMSimHit> EKLMSimHitsCollection;

} // end of namespace Belle2

#endif //EKLMSIMHIT_H
