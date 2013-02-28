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

EKLMSimHit::EKLMSimHit()
{
  setMomentum(CLHEP::HepLorentzVector(0, 0, 0, 0));
  m_volid = 0;
}

EKLMSimHit::EKLMSimHit(const EKLMStepHit* stepHit)
  : EKLMHitBase((EKLMHitBase)*stepHit)
{
  setMomentum(stepHit->getMomentum());
  setVolumeID(stepHit->getVolumeID());
}

int EKLMSimHit::getVolumeID() const
{
  return m_volid;
}

void EKLMSimHit::setVolumeID(int id)
{
  m_volid = id;
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

