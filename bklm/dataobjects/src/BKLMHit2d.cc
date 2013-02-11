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

#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(BKLMHit2d)

//! empty constructor
BKLMHit2d::BKLMHit2d() : TObject()
{
}

//! Constructor with orthogonal 1D hits
BKLMHit2d::BKLMHit2d(const BKLMDigit* hit1, const BKLMDigit* hit2) :
  TObject()
{
  // DIVOT see common/com-klm/hit/KlmHit2D initializer
  // this is a dummy line to make use of symbols hit1 and hit2
  if (hit1 == hit2) return;
}

//! Constructor with initial values
BKLMHit2d::BKLMHit2d(int status, bool isForward, int sector, int layer,
                     const double position[2], const double positionError[2],
                     double time, double energy) :
  TObject(),
  m_Status(status),
  m_IsForward(isForward),
  m_Sector(sector),
  m_Layer(layer),
  m_Time(time),
  m_Energy(energy)
{
  m_Position[0] = position[0];
  m_Position[1] = position[1];
  m_PositionError[0] = positionError[0];
  m_PositionError[1] = positionError[1];
  // DIVOT convert m_Position to 3D global coordinates in m_GlobalPosition
}

//! Copy constructor
BKLMHit2d::BKLMHit2d(const BKLMHit2d& h) :
  TObject(h),
  m_Status(h.m_Status),
  m_IsForward(h.m_IsForward),
  m_Sector(h.m_Sector),
  m_Layer(h.m_Layer),
  m_Time(h.m_Time),
  m_Energy(h.m_Energy)
{
  m_Position[0] = h.m_Position[0];
  m_Position[1] = h.m_Position[1];
  m_PositionError[0] = h.m_PositionError[0];
  m_PositionError[1] = h.m_PositionError[1];
  m_GlobalPosition = h.m_GlobalPosition;
}

void BKLMHit2d::getLocalPosition(double position[2], double positionError[2]) const
{
  position[0] = m_Position[0];
  position[1] = m_Position[1];
  positionError[0] = m_PositionError[0];
  positionError[1] = m_PositionError[1];
}

void BKLMHit2d::setLocalPosition(const double position[2], const double positionError[2])
{
  m_Position[0] = position[0];
  m_Position[1] = position[1];
  m_PositionError[0] = positionError[0];
  m_PositionError[1] = positionError[1];
  // DIVOT convert m_Position to 3D global position in m_GlobalPosition
}

