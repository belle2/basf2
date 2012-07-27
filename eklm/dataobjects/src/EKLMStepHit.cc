/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/dataobjects/EKLMStepHit.h>
#include <framework/logging/Logger.h>


using namespace Belle2;
using namespace std;

ClassImp(EKLMStepHit);



double EKLMStepHit::getEnergy() const
{
  return m_energy;
}


void EKLMStepHit::setEnergy(double E)
{
  m_energy = E;
}

const TVector3* EKLMStepHit::getMomentum() const
{
  return & m_momentum;
}


void EKLMStepHit::setMomentum(TVector3&    momentum)
{
  m_momentum = momentum;
}

int EKLMStepHit::getTrackID() const
{
  return m_trackID;
}

void EKLMStepHit::setTrackID(int track)
{
  m_trackID = track;
}


int EKLMStepHit::getParentTrackID() const
{
  return m_parentTrackID;
}


void EKLMStepHit::setParentTrackID(int track)
{
  m_parentTrackID = track;
}


string EKLMStepHit::getName()  const
{
  return m_pvName;
}


void EKLMStepHit::setName(string& name)
{
  m_pvName = name;
}

EKLMStripID EKLMStepHit::getID() const
{
  EKLMStripID str;
  str.endcap = m_Endcap;
  str.layer = m_Layer;
  str.sector = m_Sector;
  str.plane = m_Plane;
  str.strip = m_Strip;
  return str;
}



const G4VPhysicalVolume* EKLMStepHit::getVolume()  const
{
  return m_pv;
}

void EKLMStepHit::setVolume(const G4VPhysicalVolume* vol)
{
  m_pv = vol;
}

int  EKLMStepHit::getVolumeType()  const
{
  return m_volType;
}

void EKLMStepHit::setVolumeType(int type)
{
  if (type != 0 && type != 1 && type != 2)
    B2ERROR("Wrong Volume Type!");
  m_volType = type;
}


int EKLMStepHit::getPlane() const
{
  return m_Plane;
}
void EKLMStepHit::setPlane(int plane)
{
  m_Plane = plane;
}
int EKLMStepHit::getStrip() const
{
  return m_Strip;
}
void EKLMStepHit::setStrip(int strip)
{
  m_Strip = strip;
}
