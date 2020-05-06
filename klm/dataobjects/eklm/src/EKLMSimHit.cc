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
