/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/bklm/BKLMHit1d.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMStatus.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

/* C++ headers. */
#include <algorithm>
#include <climits>

using namespace Belle2;

// empty constructor for ROOT - do not use this
BKLMHit1d::BKLMHit1d() :
  RelationsObject(),
  m_ModuleID(0),
  m_Time(0.0),
  m_EnergyDeposit(0.0)
{
}

// Constructor with a cluster of contiguous parallel KLMDigits
BKLMHit1d::BKLMHit1d(const std::vector<const KLMDigit*>& digits) :
  RelationsObject(),
  m_ModuleID(0),
  m_Time(0.0),
  m_EnergyDeposit(0.0)
{
  if (digits.size() == 0) {
    B2WARNING("Attempt to create a BKLMHit1d with no KLMDigits");
    return;
  }
  int stripMin = INT_MAX;
  int stripMax = INT_MIN;
  const KLMDigit* bklmDigit = digits.front();
  if (bklmDigit->getSubdetector() != KLMElementNumbers::c_BKLM)
    B2FATAL("Trying to construct a BKLMHit1d using KLMDigits from EKLM.");
  BKLMElementNumbers::setSectionInModule(m_ModuleID, bklmDigit->getSection());
  BKLMElementNumbers::setSectorInModule(m_ModuleID, bklmDigit->getSector());
  BKLMElementNumbers::setLayerInModule(m_ModuleID, bklmDigit->getLayer());
  BKLMElementNumbers::setPlaneInModule(m_ModuleID, bklmDigit->getPlane());
  BKLMElementNumbers::setStripInModule(m_ModuleID, bklmDigit->getStrip());
  for (std::vector<const KLMDigit*>::const_iterator iDigit = digits.begin(); iDigit != digits.end(); ++iDigit) {
    const KLMDigit* digit = *iDigit;
    if (!(bklmDigit->getSection() == digit->getSection() &&
          bklmDigit->getSector() == digit->getSector() &&
          bklmDigit->getLayer() == digit->getLayer() &&
          bklmDigit->getPlane() == digit->getPlane())) {
      B2WARNING("Attempt to combine non-parallel or distinct-module KLMDigits");
      continue;
    }
    m_Time += digit->getTime();
    m_EnergyDeposit += digit->getEnergyDeposit();
    int strip = digit->getStrip();
    stripMin = std::min(stripMin, strip);
    stripMax = std::max(stripMax, strip);
    addRelationTo(digit);
  }

  if (stripMax >= stripMin) {
    m_Time /= ((stripMax - stripMin) + 1.0);
    BKLMElementNumbers::setStripInModule(m_ModuleID, stripMin);
    BKLMStatus::setMaximalStrip(m_ModuleID, stripMax);
  }
}

// Copy constructor
BKLMHit1d::BKLMHit1d(const BKLMHit1d& h) :
  RelationsObject(h),
  m_ModuleID(h.m_ModuleID),
  m_Time(h.m_Time),
  m_EnergyDeposit(h.m_EnergyDeposit)
{
}

// Assignment operator
BKLMHit1d& BKLMHit1d::operator=(const BKLMHit1d& h)
{
  m_ModuleID = h.m_ModuleID;
  m_Time = h.m_Time;
  m_EnergyDeposit = h.m_EnergyDeposit;
  return *this;
}
