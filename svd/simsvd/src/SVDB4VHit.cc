/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/simsvd/SVDB4VHit.h>
#include <svd/hitsvd/SVDSimHit.h>

//DataStore classes
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  G4Allocator<SVDB4VHit> SVDB4VHitAllocator;

  SVDB4VHit::SVDB4VHit(G4ThreeVector posIn, G4ThreeVector posOut, G4double theta,
                       G4ThreeVector momIn, G4int PDGcode, G4int trackID,
                       G4double energyDep, G4double stepLength, G4double globalTime,
                       G4String volumeName) :
      m_posIn(posIn), m_posOut(posOut), m_theta(theta), m_momIn(momIn),
      m_PDGcode(PDGcode), m_trackID(trackID), m_energyDep(energyDep),
      m_stepLength(stepLength), m_globalTime(globalTime), m_volumeName(volumeName)
  {

  }


  SVDB4VHit::~SVDB4VHit()
  {

  }


  void SVDB4VHit::Save(G4int iHit)
  {
    StoreArray<SVDSimHit> SVDArray("SVDSimHitArray");
    new(SVDArray->AddrAt(iHit)) SVDSimHit(*this);
  }

} // end of namespace Belle2
