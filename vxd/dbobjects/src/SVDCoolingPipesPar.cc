/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/SVDCoolingPipesPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void SVDCoolingPipesLayerPar::read(const GearDir& pipes)
{
  m_nPipes = pipes.getInt("nPipes");
  m_startPhi = pipes.getAngle("startPhi");
  m_deltaPhi = pipes.getAngle("deltaPhi");
  m_radius = pipes.getLength("radius");
  m_zstart = pipes.getLength("zstart");
  m_zend = pipes.getLength("zend");
}

// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void SVDCoolingPipesPar::read(const GearDir& support)
{
  m_material = support.getString("CoolingPipes/Material");
  m_outerDiameter = support.getLength("CoolingPipes/outerDiameter");
  m_wallThickness = support.getLength("CoolingPipes/wallThickness");

  for (const GearDir& layer : support.getNodes("CoolingPipes/Layer")) {
    m_layers.push_back(SVDCoolingPipesLayerPar(layer));
  }
}




