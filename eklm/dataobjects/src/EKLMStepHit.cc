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

EKLMStepHit::EKLMStepHit()
{
}

EKLMStepHit::~EKLMStepHit()
{
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

