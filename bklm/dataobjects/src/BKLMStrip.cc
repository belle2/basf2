/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dataobjects/BKLMStrip.h>

#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(BKLMStrip)

//! empty constructor
BKLMStrip::BKLMStrip() : TObject()
{
}

//! Constructor with initial values
BKLMStrip::BKLMStrip(bool inRPC, int frontBack, int sector,
                     int layer, char direction, int strip,
                     double tdc, double adc) :
  TObject(),
  m_InRPC(inRPC),
  m_FrontBack(frontBack),
  m_Sector(sector),
  m_Layer(layer),
  m_Direction(direction),
  m_Strip(strip),
  m_TDC(tdc),
  m_ADC(adc)
{
}

//! Copy constructor
BKLMStrip::BKLMStrip(const BKLMStrip& h) :
  m_InRPC(h.m_InRPC),
  m_FrontBack(h.m_FrontBack),
  m_Sector(h.m_Sector),
  m_Layer(h.m_Layer),
  m_Direction(h.m_Direction),
  m_Strip(h.m_Strip),
  m_TDC(h.m_TDC),
  m_ADC(h.m_ADC)
{
}

bool BKLMStrip::match(const BKLMStrip* s) const
{
  if (m_Strip     != s->getStrip()) return false;
  if (m_Direction != s->getDirection()) return false;
  if (m_Layer     != s->getLayer()) return false;
  if (m_Sector    != s->getSector()) return false;
  if (m_FrontBack != s->getFrontBack()) return false;
  if (m_InRPC     != s->getInRPC()) return false;
  return true;
}
