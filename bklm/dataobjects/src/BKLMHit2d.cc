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
  m_ModuleID = hitPhi->getModuleID() & ~MODULE_PLANE_MASK;
  if (m_ModuleID != (hitZ->getModuleID() & ~MODULE_PLANE_MASK)) {
    B2WARNING("BKLMHit2d:  Attempt to form a 2D hit from distinct-module 1D hits")
  }
  if (hitPhi->isPhiReadout() == hitZ->isPhiReadout()) {
    B2WARNING("BKLMHit2d:  Attempt to form a 2D hit from parallel 1D hits")
  } else if (hitZ->isPhiReadout()) {
    hitPhi = hit2;
    hitZ   = hit1;
  }
  m_Status = hitPhi->getStatus() | hitZ->getStatus();
  m_IsForward = hitPhi->isForward();
  m_Sector = hitPhi->getSector();
  m_Layer = hitPhi->getLayer();
  m_PhiStripMin = hitPhi->getStripMin();
  m_PhiStripMax = hitPhi->getStripMax();
  m_PhiStripCount = hitPhi->getStripCount();
  m_PhiStripAve = hitPhi->getStripAve();
  m_PhiStripErr = hitPhi->getStripErr();
  m_ZStripMin = hitZ->getStripMin();
  m_ZStripMax = hitZ->getStripMax();
  m_ZStripCount = hitZ->getStripCount();
  m_ZStripAve = hitZ->getStripAve();
  m_ZStripErr = hitZ->getStripErr();
  m_GlobalPosition = TVector3(0.0, 0.0, 0.0);
  m_LocalPosition = TVector3(0.0, 0.0, 0.0);
  m_LocalVariance.ResizeTo(2, 2);
  m_LocalVariance.Clear();
  m_Time = 0.5 * (hitPhi->getTime() + hitZ->getTime());
  m_EDep = hitPhi->getEDep() + hitZ->getEDep();
}

//! Copy constructor
BKLMHit2d::BKLMHit2d(const BKLMHit2d& h) :
  RelationsObject(h),
  m_Status(h.m_Status),
  m_IsForward(h.m_IsForward),
  m_Sector(h.m_Sector),
  m_Layer(h.m_Layer),
  m_ModuleID(h.m_ModuleID),
  m_PhiStripMin(h.m_PhiStripMin),
  m_PhiStripMax(h.m_PhiStripMax),
  m_PhiStripCount(h.m_PhiStripCount),
  m_PhiStripAve(h.m_PhiStripAve),
  m_PhiStripErr(h.m_PhiStripErr),
  m_ZStripMin(h.m_ZStripMin),
  m_ZStripMax(h.m_ZStripMax),
  m_ZStripCount(h.m_ZStripCount),
  m_ZStripAve(h.m_ZStripAve),
  m_ZStripErr(h.m_ZStripErr),
  m_GlobalPosition(h.m_GlobalPosition),
  m_LocalPosition(h.m_LocalPosition),
  m_LocalVariance(h.m_LocalVariance),
  m_Time(h.m_Time),
  m_EDep(h.m_EDep)
{
}

void BKLMHit2d::getLocalPosition(double position[2], double error[2]) const
{
//*  position[0] = m_LocalPosition.Y();
//*  position[1] = m_LocalPosition.Z();
//*  error[0] = sqrt(m_LocalVariance[0][0]);
//*  error[1] = sqrt(m_LocalVariance[1][1]);
  position[0] = m_PhiStripAve;
  position[1] = m_ZStripAve;
  error[0] = m_PhiStripErr;
  error[1] = m_ZStripErr;
}

// DIVOT this function is not used
//void BKLMHit2d::getLocalVariance(double variance[2]) const
//{
//  variance[0] = m_LocalVariance[0][0];
//  variance[1] = m_LocalVariance[1][1];
//}

void BKLMHit2d::setLocalPosition(double x, double y, double z)
{
  m_LocalPosition.SetX(x);
  m_LocalPosition.SetY(y);
  m_LocalPosition.SetZ(z);
}

/* unused function
void BKLMHit2d::setLocalVariance(double yy, double yz, double zy, double zz)
{
  m_LocalVariance[0][0] = yy;
  m_LocalVariance[0][1] = yz;
  m_LocalVariance[1][0] = zy;
  m_LocalVariance[1][1] = zz;
}
*/

void BKLMHit2d::setGlobalPosition(double x, double y, double z)
{
  m_GlobalPosition.SetX(x);
  m_GlobalPosition.SetY(y);
  m_GlobalPosition.SetZ(z);
}
