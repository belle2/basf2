/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <iostream>
#include <fstream>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMSimHit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(Belle2::EKLMSimHit)

EKLMSimHit::EKLMSimHit():
  EKLMHitBase(),
  m_energy(0.),
  m_momentum(TVector3(0., 0., 0.)),
  m_volid(0)
{
}

EKLMSimHit::EKLMSimHit(const EKLMStepHit* stepHit)
  : EKLMHitBase((EKLMHitBase)*stepHit),
    m_energy(stepHit->getEnergy()),
    m_momentum(*(stepHit->getMomentum())),
    m_volid(stepHit->getVolumeID())
{
}

int EKLMSimHit::getVolumeID() const
{
  return m_volid;
}

void EKLMSimHit::setVolumeID(int id)
{
  m_volid = id;
}

const TVector3* EKLMSimHit::getMomentum() const
{
  return & m_momentum;
}

void EKLMSimHit::setMomentum(const TVector3& p)
{
  m_momentum = p;
}

void EKLMSimHit::setMomentum(const TVector3* p)
{
  m_momentum = *p;
}

float EKLMSimHit::getEnergy() const
{
  return m_energy;
}

void EKLMSimHit::setEnergy(float e)
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

void EKLMSimHit::Print()
{
  printf("EKLM Hit:\nGlobal position: (%f, %f, %f)\nTime: %f\n"
         "Energy Deposition: %f\nPDG code: %d\n",
         m_globalX, m_globalY, m_globalZ, m_Time, m_EDep, m_PDG);
}

