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
#include <climits>
#include <algorithm>

using namespace Belle2;

// empty constructor for ROOT - do not use this
BKLMHit1d::BKLMHit1d() :
  RelationsObject(),
  m_ModuleID(0),
  m_Time(0.0),
  m_EDep(0.0)
{
}

// Constructor with a cluster of contiguous parallel BKLMDigits
BKLMHit1d::BKLMHit1d(const std::vector<BKLMDigit*>& digits) :
  RelationsObject()
{
  m_Time = 0.0;
  m_EDep = 0.0;
  m_ModuleID = 0;

  if (digits.size() == 0) {
    B2WARNING("Attempt to create a 1D hit with no BKLMDigits");
    return;
  }

  int stripMin = INT_MAX;
  int stripMax = INT_MIN;
  int mask = BKLM_MODULEID_MASK | BKLM_PLANE_MASK;
  m_ModuleID = digits.front()->getModuleID();
  for (std::vector<BKLMDigit*>::const_iterator iDigit = digits.begin(); iDigit != digits.end(); ++iDigit) {
    BKLMDigit* digit = *iDigit;
    if (((digit->getModuleID() ^ m_ModuleID) & mask) != 0) {
      B2WARNING("Attempt to combine non-parallel or distinct-module BKLMDigits");
      continue;
    }
    m_Time += digit->getTime();
    m_EDep += digit->getEDep();
    int strip = digit->getStrip();
    stripMin = std::min(stripMin, strip);
    stripMax = std::max(stripMax, strip);
    addRelationTo(digit);
  }

  if (stripMax >= stripMin) {
    m_Time /= ((stripMax - stripMin) + 1.0);
    m_ModuleID = (m_ModuleID & ~(BKLM_STRIP_MASK | BKLM_MAXSTRIP_MASK))
                 | ((stripMin - 1) << BKLM_STRIP_BIT)
                 | ((stripMax - 1) << BKLM_MAXSTRIP_BIT);
  }
}

// Copy constructor
BKLMHit1d::BKLMHit1d(const BKLMHit1d& h) :
  RelationsObject(h),
  m_ModuleID(h.m_ModuleID),
  m_Time(h.m_Time),
  m_EDep(h.m_EDep)
{
}

// Assignment operator
BKLMHit1d& BKLMHit1d::operator=(const BKLMHit1d& h)
{
  m_ModuleID = h.m_ModuleID;
  m_Time = h.m_Time;
  m_EDep = h.m_EDep;
  return *this;
}
