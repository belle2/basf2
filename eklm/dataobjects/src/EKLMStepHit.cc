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


using namespace Belle2;
using namespace std;

ClassImp(EKLMStepHit);

int EKLMStepHit::getPDG() const
{
  return m_PDG;
}


void  EKLMStepHit::setPDG(int PDG)
{
  m_PDG = PDG;
}


double EKLMStepHit::getTime() const
{
  return m_t;
}


void EKLMStepHit::setTime(double t)
{
  m_t = t;
}


double EKLMStepHit::getEnergy() const
{
  return m_E;
}


void EKLMStepHit::setEnergy(double E)
{
  m_E = E;
}

const TVector3 * EKLMStepHit::getPosition() const
{
  return &m_position;
}

void EKLMStepHit::setPosition(TVector3 & position)
{
  m_position = position;
}

const TVector3 * EKLMStepHit::getMomentum() const
{
  return & m_momentum;
}


void EKLMStepHit::setMomentum(TVector3  &  momentum)
{
  m_momentum = momentum;
}

double EKLMStepHit::getEDep() const
{
  return m_energyDeposit;
}

void EKLMStepHit::setEDep(double edep)
{
  m_energyDeposit = edep;
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


void EKLMStepHit::setName(string &name)
{
  m_pvName = name;
}


const G4VPhysicalVolume* EKLMStepHit::getVolume()  const
{
  return m_pv;
}

void EKLMStepHit::setVolume(const G4VPhysicalVolume * vol)
{
  m_pv = vol;
}


void EKLMStepHit::increaseEDep(double e)
{
  m_energyDeposit += e;
}


