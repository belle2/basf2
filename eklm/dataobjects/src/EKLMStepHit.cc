/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMStepHit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(EKLMStepHit);

EKLMStepHit::EKLMStepHit(
  const TVector3 momentum,
  const float E ,
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

float EKLMStepHit::getEnergy() const
{
  return m_energy;
}

void EKLMStepHit::setEnergy(float E)
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

