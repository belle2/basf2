/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDGlobalPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDGlobalPar::read(const GearDir& content)
{
  m_electronTolerance = (float)content.getDouble("ElectronTolerance", m_electronTolerance);
  m_minimumElectrons = (float)content.getDouble("MinimumElectrons", m_minimumElectrons);
  m_activeStepSize = content.getLength("ActiveStepSize", m_activeStepSize);
  m_activeChips = content.getBool("ActiveChips", m_activeChips);
  m_seeNeutrons = content.getBool("SeeNeutrons", m_seeNeutrons);
  m_onlyPrimaryTrueHits = content.getBool("OnlyPrimaryTrueHits", m_onlyPrimaryTrueHits);
  m_onlyActiveMaterial = content.getBool("OnlyActiveMaterial", m_onlyActiveMaterial);
  m_defaultMaterial = content.getString("DefaultMaterial", "Air");
  m_distanceTolerance = (float)content.getLength("DistanceTolerance", m_distanceTolerance);

  B2INFO("Reading global parameters: DefaultMaterial " << m_defaultMaterial);
}
