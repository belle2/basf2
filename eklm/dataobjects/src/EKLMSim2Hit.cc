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
#include <eklm/dataobjects/EKLMSim2Hit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(Belle2::EKLMSim2Hit)

EKLMSim2Hit::EKLMSim2Hit()
{
  setMomentum(CLHEP::HepLorentzVector(0, 0, 0, 0));
  m_volid = 0;
}

EKLMSim2Hit::EKLMSim2Hit(const EKLMSimHit* simHit)
  : EKLMHitBase((EKLMHitBase)*simHit),
    EKLMHitCoord((EKLMHitCoord)*simHit),
    EKLMHitMomentum((EKLMHitMomentum)*simHit)
{
  setVolumeID(simHit->getVolumeID());
  setPlane(simHit->getPlane());
  setStrip(simHit->getStrip());
}

int EKLMSim2Hit::getVolumeID() const
{
  return m_volid;
}

void EKLMSim2Hit::setVolumeID(int id)
{
  m_volid = id;
}

int EKLMSim2Hit::getPlane() const
{
  return m_Plane;
}

void EKLMSim2Hit::setPlane(int plane)
{
  m_Plane = plane;
}

int EKLMSim2Hit::getStrip() const
{
  return m_Strip;
}

void EKLMSim2Hit::setStrip(int strip)
{
  m_Strip = strip;
}

