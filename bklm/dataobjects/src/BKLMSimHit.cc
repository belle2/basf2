/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dataobjects/BKLMSimHit.h>

using namespace Belle2;

ClassImp(BKLMSimHit)

//! empty constructor for ROOT
BKLMSimHit::BKLMSimHit() :
  SimHitBase(),
  m_Status(0),
  m_PDG(0),
  m_TrackID(0),
  m_ParentID(0),
  m_IsForward(false),
  m_Sector(0),
  m_Layer(0),
  m_IsPhiReadout(false),
  m_StripMin(0),
  m_StripMax(0),
  m_ModuleID(0),
  m_GlobalPosition(TVector3(0.0, 0.0, 0.0)),
  m_LocalPosition(TVector3(0.0, 0.0, 0.0)),
  m_Time(0.0),
  m_EDep(0.0),
  m_Momentum(TVector3(0.0, 0.0, 0.0)),
  m_Energy(0.0),
  m_KineticEnergy(0.0)
{
}

//! Constructor with initial values
BKLMSimHit::BKLMSimHit(unsigned int status, int pdg, int trackID, int parentID,
                       bool isForward, int sector, int layer, bool isPhiReadout, int stripMin, int stripMax,
                       const TVector3& globalPosition, const TVector3& localPosition,
                       double time, double eDep,
                       const TVector3& momentum, double energy, double kineticEnergy) :
  SimHitBase(),
  m_Status(status),
  m_PDG(pdg),
  m_TrackID(trackID),
  m_ParentID(parentID),
  m_IsForward(isForward),
  m_Sector(sector),
  m_Layer(layer),
  m_IsPhiReadout(isPhiReadout),
  m_StripMin(stripMin),
  m_StripMax(stripMax),
  m_GlobalPosition(globalPosition),
  m_LocalPosition(localPosition),
  m_Time(time),
  m_EDep(eDep),
  m_Momentum(momentum),
  m_Energy(energy),
  m_KineticEnergy(kineticEnergy)
{
  m_ModuleID = (m_IsForward ? 0 : MODULE_END_MASK)
               + ((m_Sector - 1) << MODULE_SECTOR_BIT)
               + ((m_Layer - 1) << MODULE_LAYER_BIT)
               + (m_IsPhiReadout ? 0 : MODULE_PLANE_MASK);
}

//! Copy constructor
BKLMSimHit::BKLMSimHit(const BKLMSimHit& hit) :
  SimHitBase(hit),
  m_Status(hit.m_Status),
  m_PDG(hit.m_PDG),
  m_TrackID(hit.m_TrackID),
  m_ParentID(hit.m_ParentID),
  m_IsForward(hit.m_IsForward),
  m_Sector(hit.m_Sector),
  m_Layer(hit.m_Layer),
  m_IsPhiReadout(hit.m_IsPhiReadout),
  m_StripMin(hit.m_StripMin),
  m_StripMax(hit.m_StripMax),
  m_ModuleID(hit.m_ModuleID),
  m_GlobalPosition(hit.m_GlobalPosition),
  m_LocalPosition(hit.m_LocalPosition),
  m_Time(hit.m_Time),
  m_EDep(hit.m_EDep),
  m_Momentum(hit.m_Momentum),
  m_Energy(hit.m_Energy),
  m_KineticEnergy(hit.m_KineticEnergy)
{
}
