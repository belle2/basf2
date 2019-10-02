/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michel Hernandez Villanueva, Ami Rostomyan               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/EventKinematics.h>

using namespace Belle2;

void EventKinematics::addMissingMomentum(const TVector3& missingMomentum)
{
  m_missingMomentum = missingMomentum;
}

void EventKinematics::addMissingMomentumCMS(const TVector3& missingMomentumCMS)
{
  m_missingMomentumCMS = missingMomentumCMS;
}

void EventKinematics::addMissingEnergyCMS(float missingEnergyCMS)
{
  m_missingEnergyCMS = missingEnergyCMS;
}

void EventKinematics::addMissingMass2(float missingMass2)
{
  m_missingMass2 = missingMass2;
}

void EventKinematics::addVisibleEnergyCMS(float visibleEnergyCMS)
{
  m_visibleEnergyCMS = visibleEnergyCMS;
}

void EventKinematics::addTotalPhotonsEnergy(float totalPhotonsEnergy)
{
  m_photonsEnergy = totalPhotonsEnergy;
}
