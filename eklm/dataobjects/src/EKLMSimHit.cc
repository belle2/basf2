/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/dataobjects/EKLMSimHit.h>
#include <framework/logging/Logger.h>
#include "G4VPhysicalVolume.hh"
#include<iostream>
#include<fstream>

using namespace Belle2;

ClassImp(Belle2::EKLMSimHit)

EKLMSimHit::EKLMSimHit():
  EKLMHitBase(),
  m_energy(0.),
  m_momentum(TVector3(0., 0., 0.)),
  m_pv(NULL),
  m_pvName("not initialized"),
  m_volType(0)
{}

EKLMSimHit::EKLMSimHit(const EKLMStepHit* stepHit)
  : EKLMHitBase((EKLMHitBase)*stepHit),
    m_energy(stepHit->getEnergy()),
    m_momentum(*(stepHit->getMomentum())),
    m_pv(stepHit->getVolume()),
    m_pvName(m_pv->GetName()),
    m_volType(stepHit->getVolumeType())
{}

const G4VPhysicalVolume* EKLMSimHit::getVolume() const
{
  return m_pv;
}

void EKLMSimHit::setVolume(const G4VPhysicalVolume* pv)
{
  m_pv = pv;
}

bool  EKLMSimHit::getVolType() const
{
  return m_volType;
}


EKLMStripID EKLMSimHit::getID() const
{
  EKLMStripID str;
  str.endcap = m_Endcap;
  str.layer = m_Layer;
  str.sector = m_Sector;
  str.plane = m_Plane;
  str.strip = m_Strip;
  return str;
}


void  EKLMSimHit::setVolType(int vt)
{
  m_volType = vt;
}


const TVector3*   EKLMSimHit::getMomentum() const
{
  return & m_momentum;
}

void  EKLMSimHit::setMomentum(const TVector3& p)
{
  m_momentum = p;
}

void  EKLMSimHit::setMomentum(const TVector3* p)
{
  m_momentum = *p;
}

double  EKLMSimHit::getEnergy() const
{
  return m_energy;
}

void  EKLMSimHit::setEnergy(double e)
{
  m_energy = e;
}

int EKLMSimHit::getPlane() const
{
  return m_Plane;
}
void EKLMSimHit::setPlane(int plane)
{
  m_Plane = plane;
}
int EKLMSimHit::getStrip() const
{
  return m_Strip;
}
void EKLMSimHit::setStrip(int strip)
{
  m_Strip = strip;
}


void EKLMSimHit::Save(char* filename)
{
  std::ofstream save_hit(filename, std::fstream::app);
  save_hit << '\n';
  save_hit << "EKLM Hit: \n" ;
  save_hit << "Global position: (" << m_GlobalPosition.x() << "," << m_GlobalPosition.y() << "," << m_GlobalPosition.z() << ")\n"  ;
  save_hit << "Time: " << m_Time << '\n' ;
  save_hit << "Energy Deposition: " <<  m_EDep << '\n' ;
  save_hit << "PDG code: " << m_PDG << '\n';
  save_hit.close();
}



//} //end of Belle2 namespace
