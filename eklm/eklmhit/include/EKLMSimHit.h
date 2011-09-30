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
    EKLMSimHit() {};

    //! Constructor with initial values
    EKLMSimHit(G4VPhysicalVolume *pv, G4ThreeVector global_pos,
               G4ThreeVector local_pos, G4double time, G4int PDGcode,
               G4double eDep);

    //! Destructor
    ~EKLMSimHit() {};

    //! returns physical volume
    G4VPhysicalVolume *getPV();

    //! returns global position of the hit
    G4ThreeVector getGlobalPos();

    //! returns local position of the hit
    G4ThreeVector getLocalPos();

    //! returns hit time
    G4double getTime();

    //! returns energy deposition
    G4double getEDep();

    //! returns PDG code of the particle
    G4int getPDGCode();

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
