/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dataobjects/BKLMDigit.h>

#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(BKLMDigit)

//! empty constructor
BKLMDigit::BKLMDigit() : RelationsObject()
{
}

//! Constructor with initial values
BKLMDigit::BKLMDigit(unsigned int status, bool isForward, int sector,
                     int layer, bool isPhiReadout, int strip,
                     double time, double energy) :
  RelationsObject(),
  m_Status(status),
  m_IsForward(isForward),
  m_Sector(sector),
  m_Layer(layer),
  m_IsPhiReadout(isPhiReadout),
  m_Strip(strip),
  m_Time(time),
  m_Energy(energy)
{
}

//! Copy constructor
BKLMDigit::BKLMDigit(const BKLMDigit& h) :
  RelationsObject(h),
  m_Status(h.m_Status),
  m_IsForward(h.m_IsForward),
  m_Sector(h.m_Sector),
  m_Layer(h.m_Layer),
  m_IsPhiReadout(h.m_IsPhiReadout),
  m_Strip(h.m_Strip),
  m_Time(h.m_Time),
  m_Energy(h.m_Energy)
{
}

bool BKLMDigit::match(const BKLMDigit* s) const
{
  if (m_Strip        != s->getStrip()) return false;
  if (m_Layer        != s->getLayer()) return false;
  if (m_Sector       != s->getSector()) return false;
  if (m_IsForward    != s->isForward()) return false;
  if (m_IsPhiReadout != s->isPhiReadout()) return false;
  if (m_Status       != s->getStatus()) return false;
  return true;
}
