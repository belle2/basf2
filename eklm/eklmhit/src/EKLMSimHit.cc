/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/eklmhit/EKLMSimHit.h>
#include <framework/logging/Logger.h>
#include<iostream>
#include<fstream>

using namespace Belle2;

ClassImp(Belle2::EKLMSimHit)

G4Allocator<EKLMSimHit> EKLMSimHitAllocator;

EKLMSimHit::EKLMSimHit(G4VPhysicalVolume *pv, G4ThreeVector global_pos,
                       G4ThreeVector local_pos, G4double time,
                       G4int PDGcode, G4double eDep)
{
  m_pv = pv;
  m_global_pos = global_pos;
  m_local_pos = local_pos;
  m_time = time;
  m_PDGcode = PDGcode;
  m_eDep = eDep;
}

G4VPhysicalVolume* EKLMSimHit::getPV()
{
  return m_pv;
}

G4ThreeVector EKLMSimHit::getGlobalPos()
{
  return m_global_pos;
}

G4ThreeVector EKLMSimHit::getLocalPos()
{
  return m_local_pos;
}

G4double EKLMSimHit::getTime()
{
  return m_time;
}

G4double EKLMSimHit::getEDep()
{
  return m_eDep;
}

G4int EKLMSimHit::getPDGCode()
{
  return m_PDGcode;
}

void EKLMSimHit::Save(char* filename)
{
  std::ofstream save_hit(filename, std::fstream::app);
  save_hit << '\n';
  save_hit << "EKLM Hit: \n" ;
  save_hit << "Global position: " << m_global_pos << '\n'  ;
  save_hit << "Local position: " << m_local_pos << '\n'  ;
  save_hit << "Time: " << m_time << '\n' ;
  save_hit << "Energy Deposition: " <<  m_eDep << '\n' ;
  save_hit << "PDG code: " << m_PDGcode << '\n';
  save_hit.close();
}



//} //end of Belle2 namespace
