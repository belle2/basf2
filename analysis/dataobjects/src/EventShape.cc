/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michel Hernandez Villanueva, Ami Rostomyan               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/EventShape.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;

void EventShape::addThrustAxis(TVector3 thrustAxis)
{
  m_thrustAxis = thrustAxis;
}

void EventShape::addMissingMomentum(TVector3 missingMomentum)
{
  m_missingMomentum = missingMomentum;
}

void EventShape::addMissingMomentumCMS(TVector3 missingMomentumCMS)
{
  m_missingMomentumCMS = missingMomentumCMS;
}

void EventShape::addMissingEnergyCMS(float missingEnergyCMS)
{
  m_missingEnergyCMS = missingEnergyCMS;
}

void EventShape::addMissingMass2(float missingMass2)
{
  m_missingMass2 = missingMass2;
}

void EventShape::addVisibleEnergy(float visibleEnergy)
{
  m_visibleEnergy = visibleEnergy;
}
