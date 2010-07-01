/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/simpxd/PXDB4VHit.h>
//#include <hitcdc/SimHitCDC.h>

//DataStore classes
//#include <datastore/StoreObjPtr.h>
//#include <datastore/StoreDefs.h>
//#include <datastore/StoreArray.h>

namespace Belle2 {

  G4Allocator<PXDB4VHit> PXDB4VHitAllocator;

  PXDB4VHit::PXDB4VHit(G4ThreeVector posIn, G4ThreeVector posOut, G4ThreeVector momIn, G4int PDGcode,
                       G4int trackID, G4double energyDep, G4double stepLength, G4double globalTime,
                       G4String volumeName) :
      m_posIn(posIn), m_posOut(posOut), m_momIn(momIn), m_PDGcode(PDGcode),
      m_trackID(trackID), m_energyDep(energyDep), m_stepLength(stepLength),
      m_globalTime(globalTime), m_volumeName(volumeName)
  {

  }


  PXDB4VHit::~PXDB4VHit()
  {

  }


  void PXDB4VHit::Save(G4int iHit)
  {

  }

} // end of namespace Belle2
