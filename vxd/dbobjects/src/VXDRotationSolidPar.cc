/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDRotationSolidPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

// Read parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDRotationSolidPar::read(const GearDir& params)
{
  m_name = params.getString("Name", "");
  m_material = params.getString("Material", "Air");
  m_color = params.getString("Color", "");

  m_minPhi = params.getAngle("minPhi", 0);
  m_maxPhi = params.getAngle("maxPhi", 2 * M_PI);

  for (const GearDir point : params.getNodes("InnerPoints/point")) {
    pair<double, double> ZXPoint(point.getLength("z"), point.getLength("x"));
    m_innerPoints.push_back(ZXPoint);
  }
  for (const GearDir point : params.getNodes("OuterPoints/point")) {
    pair<double, double> ZXPoint(point.getLength("z"), point.getLength("x"));
    m_outerPoints.push_back(ZXPoint);
  }

  m_exists = m_outerPoints.size() > 0 && m_innerPoints.size() > 0;
}



