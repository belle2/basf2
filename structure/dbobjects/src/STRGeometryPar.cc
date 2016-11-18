/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexandre BEAULIEU                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <structure/dbobjects/STRGeometryPar.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

STRGeometryPar::STRGeometryPar()
{
  // Set geometry version
  m_Version = 0;

  // Initialize values (to fix cppcheck warnings)
  for (int i = 0; i < NECLSHIELDS; i++) {
    m_NLayers[i] = 0;
  }
  for (int i = 0; i < NPOLEPIECES; i++) {
    m_PoleNPlanes[i] = 0;
  }


}

STRGeometryPar::~STRGeometryPar()
{
}



