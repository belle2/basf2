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

EKLMStepHit::EKLMStepHit(
  const TVector3 momentum,
  const double E ,
  const int  trID,
  const int  ptrID
)
  :
  EKLMHitBase()
{
  m_energy = E;
  m_momentum = momentum;
  m_trackID = trID;
  m_parentTrackID = ptrID;
}


EKLMStepHit::EKLMStepHit(
  const int Endcap,
  const int Layer,
  const int Sector,
  const int Plane,
  const int Strip,
  const int PDG,
  const double Time,
  const double EDep,
  const TVector3 GlobalPosition,
  const TVector3 LocalPosition,
  const TVector3 momentum,
  const double E ,
  const int  trID,
  const int  ptrID,
  const int volID
) :
  EKLMHitBase(Endcap, Layer, Sector, PDG, Time, EDep, GlobalPosition,
              LocalPosition)
{
  m_Plane = Plane;
  m_Strip = Strip;
  m_energy = E;
  m_momentum = momentum;
  m_trackID = trID;
  m_parentTrackID = ptrID;
  m_volid = volID;
}


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

int EKLMStepHit::getVolumeID() const
{
  return m_volid;
}

void EKLMStepHit::setVolumeID(int id)
{
  m_volid = id;
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

