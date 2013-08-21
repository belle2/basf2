/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dataobjects/BKLMHit1d.h>
#include <bklm/dataobjects/BKLMDigit.h>

#include <framework/logging/Logger.h>

#include <cmath>
#include <algorithm>

using namespace Belle2;

ClassImp(BKLMHit1d)

//! empty constructor
BKLMHit1d::BKLMHit1d() : RelationsObject()
{
}

//! Constructor with a cluster of continuous parallel BKLMDigits
BKLMHit1d::BKLMHit1d(const std::vector<BKLMDigit*>& digits) :
  RelationsObject()
{
  m_Time = 0.0;
  m_EDep = 0.0;
  m_StripAve = 0.0;
  m_StripErr = 0.0;
  m_StripCount = 0;

  if (digits.size() > 0) {
    const BKLMDigit* pDigit = digits.front();
    m_Status = pDigit->getStatus();
    m_IsForward = pDigit->isForward();
    m_Sector = pDigit->getSector();
    m_Layer = pDigit->getLayer();
    m_IsPhiReadout = pDigit->isPhiReadout();
    m_StripMin = pDigit->getStrip();
    m_StripMax = pDigit->getStrip();
    m_ModuleID = pDigit->getModuleID();
  } else {
    B2WARNING("BKLMHit1d:  Attempt to create a 1D hit with no BKLMDigits")
    m_Status = STATUS_UNUSED;
    m_IsForward = true;
    m_Sector = 0;
    m_Layer = 0;
    m_IsPhiReadout = false;
    m_StripMin = 0;
    m_StripMax = 0;
    m_ModuleID = 0;
  }

  for (std::vector<BKLMDigit*>::const_iterator iDigit = digits.begin(); iDigit != digits.end(); ++iDigit) {
    if ((*iDigit)->getModuleID() != m_ModuleID) {
      B2WARNING("BKLMHit1d: Attempt to combine non-parallel BKLMDigits")
      continue;
    }
    m_Status |= (*iDigit)->getStatus();
    m_Time += (*iDigit)->getTime();
    m_EDep += (*iDigit)->getEDep();
    int strip = (*iDigit)->getStrip();
    m_StripAve += strip;
    m_StripMin = std::min(m_StripMin, strip);
    m_StripMax = std::max(m_StripMax, strip);
    ++m_StripCount;
  }

  if (m_StripCount > 0) {
    double nStrips = (double)(m_StripCount);
    m_Time /= nStrips;
    m_StripAve /= nStrips;
    m_StripErr = nStrips / sqrt(12.0);
  }
}

//! Copy constructor
BKLMHit1d::BKLMHit1d(const BKLMHit1d& h) :
  RelationsObject(h),
  m_Status(h.m_Status),
  m_IsForward(h.m_IsForward),
  m_Sector(h.m_Sector),
  m_Layer(h.m_Layer),
  m_IsPhiReadout(h.m_IsPhiReadout),
  m_ModuleID(h.m_ModuleID),
  m_StripMin(h.m_StripMin),
  m_StripMax(h.m_StripMax),
  m_StripCount(h.m_StripCount),
  m_StripAve(h.m_StripAve),
  m_StripErr(h.m_StripErr),
  m_Time(h.m_Time),
  m_EDep(h.m_EDep)
{
}

