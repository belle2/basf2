/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/eklm/EKLMSimHit.h>

using namespace Belle2;

EKLMSimHit::EKLMSimHit()
{
  m_trackID = -1;
  m_parentTrackID = -1;
  m_volid = -1;
  m_Plane = -1;
  m_Strip = -1;
}

EKLMSimHit::~EKLMSimHit()
{
}

int EKLMSimHit::getTrackID() const
{
  return m_trackID;
}

void EKLMSimHit::setTrackID(int track)
{
  m_trackID = track;
}

int EKLMSimHit::getParentTrackID() const
{
  return m_parentTrackID;
}

void EKLMSimHit::setParentTrackID(int track)
{
  m_parentTrackID = track;
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

