/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dataobjects/BKLMRawHit.h>

#include <framework/logging/Logger.h>

using namespace Belle2;

// empty constructor for ROOT - do not use this
BKLMRawHit::BKLMRawHit() :
  RelationsObject(),
  m_Status(0),
  m_Crate(0),
  m_Slot(0),
  m_Channel(0),
  m_TDC(0),
  m_ADC(0)
{
}

// Constructor with initial values
BKLMRawHit::BKLMRawHit(unsigned int status, int crate, int slot, int channel,
                       int tdc, int adc) :
  RelationsObject(),
  m_Status(status),
  m_Crate(crate),
  m_Slot(slot),
  m_Channel(channel),
  m_TDC(tdc),
  m_ADC(adc)
{
}

// Copy constructor
BKLMRawHit::BKLMRawHit(const BKLMRawHit& h) :
  RelationsObject(h),
  m_Status(h.m_Status),
  m_Crate(h.m_Crate),
  m_Slot(h.m_Slot),
  m_Channel(h.m_Channel),
  m_TDC(h.m_TDC),
  m_ADC(h.m_ADC)
{
}

// Assignment operator
BKLMRawHit& BKLMRawHit::operator=(const BKLMRawHit& h)
{
  m_Status = h.m_Status;
  m_Crate = h.m_Crate;
  m_Slot = h.m_Slot;
  m_Channel = h.m_Channel;
  m_TDC = h.m_TDC;
  m_ADC = h.m_ADC;
  return *this;
}

bool BKLMRawHit::match(const BKLMRawHit* s) const
{
  if (m_Crate   != s->getCrate()) return false;
  if (m_Slot    != s->getSlot()) return false;
  if (m_Channel != s->getChannel()) return false;
  return true;
}
