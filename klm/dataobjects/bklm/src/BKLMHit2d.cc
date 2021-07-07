/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/bklm/BKLMHit2d.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMHit1d.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

// empty constructor for ROOT - do not use this
BKLMHit2d::BKLMHit2d() :
  RelationsObject(),
  m_ModuleID(0),
  m_ZStrips(0),
  m_GlobalPosition{0, 0, 0},
  m_Time(0.0),
  m_EnergyDeposit(0.0)
{
}

// Constructor with orthogonal 1D hits
BKLMHit2d::BKLMHit2d(const BKLMHit1d* hitPhi, const BKLMHit1d* hitZ, const CLHEP::Hep3Vector& globalPos, double time) :
  RelationsObject()
{
  m_ModuleID = hitPhi->getModuleID() | (hitZ->getModuleID() & BKLM_STATUS_MASK);
  m_ZStrips = ((hitZ->getStripMin() - 1) << BKLM_ZSTRIP_BIT) | ((hitZ->getStripMax() - 1) << BKLM_ZMAXSTRIP_BIT);

  if (!BKLMElementNumbers::hitsFromSameModule(m_ModuleID, hitZ->getModuleID())) {
    B2WARNING("Attempt to form a 2D hit from distinct-module 1D hits");
  }

  m_GlobalPosition[0] = globalPos.x();
  m_GlobalPosition[1] = globalPos.y();
  m_GlobalPosition[2] = globalPos.z();
  m_Time = time;
  m_EnergyDeposit = hitPhi->getEnergyDeposit() + hitZ->getEnergyDeposit();

  addRelationTo(hitPhi);
  addRelationTo(hitZ);
}

// Copy constructor
BKLMHit2d::BKLMHit2d(const BKLMHit2d& h) :
  RelationsObject(h),
  m_ModuleID(h.m_ModuleID),
  m_ZStrips(h.m_ZStrips),
  m_Time(h.m_Time),
  m_EnergyDeposit(h.m_EnergyDeposit)
{
  m_GlobalPosition[0] = h.m_GlobalPosition[0];
  m_GlobalPosition[1] = h.m_GlobalPosition[1];
  m_GlobalPosition[2] = h.m_GlobalPosition[2];
}

// Assignment operator
BKLMHit2d& BKLMHit2d::operator=(const BKLMHit2d& h)
{
  m_ModuleID = h.m_ModuleID;
  m_ZStrips = h.m_ZStrips;
  m_Time = h.m_Time;
  m_EnergyDeposit = h.m_EnergyDeposit;
  m_GlobalPosition[0] = h.m_GlobalPosition[0];
  m_GlobalPosition[1] = h.m_GlobalPosition[1];
  m_GlobalPosition[2] = h.m_GlobalPosition[2];
  return *this;
}
