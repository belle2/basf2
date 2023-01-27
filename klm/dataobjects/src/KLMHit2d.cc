/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMHit2d.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMHit2d::KLMHit2d() :
  m_ChiSq(-1)
{
}

KLMHit2d::KLMHit2d(KLMDigit* digit) :
  m_ChiSq(-1)
{
  if (digit->getSubdetector() != KLMElementNumbers::c_EKLM)
    B2FATAL("Trying to construct a KLMHit2d using KLMDigit from BKLM.");
  m_Subdetector = KLMElementNumbers::c_EKLM;
  setSection(digit->getSection());
  setLayer(digit->getLayer());
  setSector(digit->getSector());
}

// Constructor with orthogonal 1D hits
KLMHit2d::KLMHit2d(const BKLMHit1d* hitPhi, const BKLMHit1d* hitZ, const CLHEP::Hep3Vector& globalPos, double time) :
  RelationsObject()
{
  if (!BKLMElementNumbers::hitsFromSameModule(hitPhi->getModuleID(),
                                              hitZ->getModuleID()))
    B2WARNING("Attempt to form a 2D hit from distinct-module 1D hits");
  m_Subdetector = KLMElementNumbers::c_BKLM;
  m_Section = hitPhi->getSection();
  m_Sector = hitPhi->getSector();
  m_Layer = hitPhi->getLayer();
  m_Strip[BKLMElementNumbers::c_ZPlane] = hitZ->getStripMin();
  m_LastStrip[BKLMElementNumbers::c_ZPlane] = hitZ->getStripMax();
  m_Strip[BKLMElementNumbers::c_PhiPlane] = hitPhi->getStripMin();
  m_LastStrip[BKLMElementNumbers::c_PhiPlane] = hitPhi->getStripMax();
  m_GlobalX = globalPos.x();
  m_GlobalY = globalPos.y();
  m_GlobalZ = globalPos.z();
  m_Time = time;
  m_EnergyDeposit = hitPhi->getEnergyDeposit() + hitZ->getEnergyDeposit();
  addRelationTo(hitPhi);
  addRelationTo(hitZ);
}
