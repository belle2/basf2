/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dataobjects/BKLMHit2d.h>
#include <bklm/dataobjects/BKLMHit1d.h>

#include <framework/logging/Logger.h>

#include <cmath>

using namespace Belle2;

ClassImp(BKLMHit2d)

//! empty constructor
BKLMHit2d::BKLMHit2d() : RelationsObject()
{
}

//! Constructor with orthogonal 1D hits
BKLMHit2d::BKLMHit2d(const BKLMHit1d* hit1, const BKLMHit1d* hit2) :
  RelationsObject()
{
  const BKLMHit1d* hitPhi = hit1;
  const BKLMHit1d* hitZ   = hit2;
  if (hitPhi->isPhiReadout() == hitZ->isPhiReadout()) {
    B2WARNING("Attempt to form a 2D hit from parallel 1D hits")
  } else if (hitZ->isPhiReadout()) {
    hitPhi = hit2;
    hitZ   = hit1;
  }
  m_ModuleID = hitPhi->getModuleID() | (hitZ->getModuleID() & BKLM_STATUS_MASK);
  m_ZStrips = ((hitZ->getStripMin() - 1) << BKLM_ZSTRIP_BIT) | ((hitZ->getStripMax() - 1) << BKLM_ZMAXSTRIP_BIT);

  if (((hitZ->getModuleID() ^ m_ModuleID) & BKLM_MODULEID_MASK) != 0) {
    B2WARNING("Attempt to form a 2D hit from distinct-module 1D hits")
  }

  m_GlobalPosition[0] = 0.0;
  m_GlobalPosition[1] = 0.0;
  m_GlobalPosition[2] = 0.0;
  m_LocalPosition[0] = 0.0;
  m_LocalPosition[1] = 0.0;
  m_LocalPosition[2] = 0.0;
  m_Time = 0.5 * (hitPhi->getTime() + hitZ->getTime());
  m_EDep = hitPhi->getEDep() + hitZ->getEDep();
}

//! Copy constructor
BKLMHit2d::BKLMHit2d(const BKLMHit2d& h) :
  RelationsObject(h),
  m_ModuleID(h.m_ModuleID),
  m_ZStrips(h.m_ZStrips),
  m_Time(h.m_Time),
  m_EDep(h.m_EDep)
{
  m_GlobalPosition[0] = h.m_GlobalPosition[0];
  m_GlobalPosition[1] = h.m_GlobalPosition[1];
  m_GlobalPosition[2] = h.m_GlobalPosition[2];
  m_LocalPosition[0] = h.m_LocalPosition[0];
  m_LocalPosition[1] = h.m_LocalPosition[1];
  m_LocalPosition[2] = h.m_LocalPosition[2];
}

void BKLMHit2d::setLocalPosition(double x, double y, double z)
{
  m_LocalPosition[0] = x;
  m_LocalPosition[1] = y;
  m_LocalPosition[2] = z;
}

void BKLMHit2d::setGlobalPosition(double x, double y, double z)
{
  m_GlobalPosition[0] = x;
  m_GlobalPosition[1] = y;
  m_GlobalPosition[2] = z;
}

//! Calculate the phi-strip average
float BKLMHit2d::getPhiStripAve() const
{
  int stripMin = ((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1;
  int stripMax = ((m_ModuleID & BKLM_MAXSTRIP_MASK) >> BKLM_MAXSTRIP_BIT) + 1;
  return 0.5 * (stripMin + stripMax);
}

//! Calculate the phi-strip count
int BKLMHit2d::getPhiStripCount() const
{
  int stripMin = ((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1;
  int stripMax = ((m_ModuleID & BKLM_MAXSTRIP_MASK) >> BKLM_MAXSTRIP_BIT) + 1;
  return stripMax - stripMin + 1;
}
//! Calculate the z-strip average
float BKLMHit2d::getZStripAve() const
{
  int stripMin = ((m_ZStrips & BKLM_ZSTRIP_MASK) >> BKLM_ZSTRIP_BIT) + 1;
  int stripMax = ((m_ZStrips & BKLM_ZMAXSTRIP_MASK) >> BKLM_ZMAXSTRIP_BIT) + 1;
  return 0.5 * (stripMin + stripMax);
}

//! Calculate the z-strip count
int BKLMHit2d::getZStripCount() const
{
  int stripMin = ((m_ZStrips & BKLM_ZSTRIP_MASK) >> BKLM_ZSTRIP_BIT) + 1;
  int stripMax = ((m_ZStrips & BKLM_ZMAXSTRIP_MASK) >> BKLM_ZMAXSTRIP_BIT) + 1;
  return stripMax - stripMin + 1;
}
