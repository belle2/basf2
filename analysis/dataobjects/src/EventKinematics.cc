/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
