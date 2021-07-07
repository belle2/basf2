/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <structure/dbobjects/STRGeometryPar.h>

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



