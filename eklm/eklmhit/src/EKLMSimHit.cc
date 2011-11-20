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

EKLMSimHit::EKLMSimHit()
{
  m_pv = NULL;
  m_global_pos = TVector3(0, 0, 0);
  m_local_pos = TVector3(0, 0, 0);
  m_time = 0;
  m_PDGcode = 0;
  m_eDep = 0;
}

EKLMSimHit::EKLMSimHit(G4VPhysicalVolume *pv, TVector3 global_pos,
                       TVector3 local_pos, G4double time,
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

void EKLMSimHit::setPV(G4VPhysicalVolume* pv)
{
  m_pv = pv;
}

TVector3 EKLMSimHit::getGlobalPos()
{
  return m_global_pos;
}

void EKLMSimHit::setGlobalPos(const TVector3 & gp)
{
  m_global_pos = gp;
}

TVector3 EKLMSimHit::getLocalPos()
{
  return m_local_pos;
}

void setLocalPos(TVector3 lp)
{
  //  m_local_pos=lp;
}

G4double EKLMSimHit::getTime()
{
  return m_time;
}
void EKLMSimHit::setTime(double t)
{
  m_time = t;
}

G4double EKLMSimHit::getEDep()
{
  return m_eDep;
}

void EKLMSimHit::setEDep(double e)
{
  m_eDep = e;
}

G4int EKLMSimHit::getPDGCode()
{
  return m_PDGcode;
}

void EKLMSimHit::setPDGCode(int pdg)
{
  m_PDGcode = pdg;
}


G4int EKLMSimHit::getTrackID()
{
  return m_trackID;
}

void  EKLMSimHit::setTrackID(G4int id)
{
  m_trackID = id;
}


G4int EKLMSimHit::getParentTrackID()
{
  return  m_parentTrackID;
}

void EKLMSimHit::setParentTrackID(G4int id)
{
  m_parentTrackID = id;
}

bool  EKLMSimHit::getVolType()
{
  return m_volType;
}


void  EKLMSimHit::setVolType(int vt)
{
  m_volType = vt;
}


TVector3  EKLMSimHit::getMomentum()
{
  return m_momentum;
}

void  EKLMSimHit::setMomentum(const TVector3 &p)
{
  m_momentum = p;
}

double  EKLMSimHit::getEnergy()
{
  return m_energy;
}

void  EKLMSimHit::setEnergy(double e)
{
  m_energy = e;
}


void EKLMSimHit::Save(char* filename)
{
  std::ofstream save_hit(filename, std::fstream::app);
  save_hit << '\n';
  save_hit << "EKLM Hit: \n" ;
  save_hit << "Global position: (" << m_global_pos.x() << "," << m_global_pos.y() << "," << m_global_pos.z() << ")\n"  ;
  save_hit << "Local position: (" << m_local_pos.x() << "," << m_local_pos.y() << "," << m_local_pos.z() << ")\n"  ;
  save_hit << "Time: " << m_time << '\n' ;
  save_hit << "Energy Deposition: " <<  m_eDep << '\n' ;
  save_hit << "PDG code: " << m_PDGcode << '\n';
  save_hit.close();
}



//} //end of Belle2 namespace
