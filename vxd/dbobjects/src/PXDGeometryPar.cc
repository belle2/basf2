/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <vxd/dbobjects/PXDGeometryPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;


PXDGeometryPar::~PXDGeometryPar()
{
  // FIXME: delete sensorInfo causes run time crashes, outcomment it for the moment
  //for (PXDSensorInfoPar* sensorInfo : m_SensorInfo) delete sensorInfo;
  //m_SensorInfo.clear();
}

void PXDGeometryPar::createHalfShellSupport(GearDir support)
{
  if (!support) return;

  for (const GearDir& endflange : support.getNodes("Endflange")) {
    m_endflanges.push_back(VXDPolyConePar(endflange));
  }

  // Cout outs for endflanges
  m_nCutouts = support.getInt("Cutout/count");
  m_cutOutWidth = support.getLength("Cutout/width");
  m_cutOutHeight = support.getLength("Cutout/height");
  m_cutOutShifz = support.getLength("Cutout/shift");
  m_cutOutRphi = support.getLength("Cutout/rphi");
  m_cutOutStartPhi = support.getAngle("Cutout/startPhi");
  m_cutOutDeltaPhi = support.getAngle("Cutout/deltaPhi");

  //Create Carbon cooling tubes
  m_nTubes = support.getInt("CarbonTubes/count");
  m_tubesMinZ = support.getLength("CarbonTubes/minZ");
  m_tubesMaxZ = support.getLength("CarbonTubes/maxZ");
  m_tubesMinR = support.getLength("CarbonTubes/innerRadius");
  m_tubesMaxR = support.getLength("CarbonTubes/outerRadius");
  m_tubesRPhi = support.getLength("CarbonTubes/rphi");
  m_tubesStartPhi = support.getAngle("CarbonTubes/startPhi");
  m_tubesDeltaPhi = support.getAngle("CarbonTubes/deltaPhi");
  m_tubesMaterial = support.getString("CarbonTubes/Material", "Carbon");

  return;

}


void PXDGeometryPar::createLayerSupport(int layer, GearDir support)
{

}


void PXDGeometryPar::createLadderSupport(int layer, GearDir support)
{

}

VXDSensorInfoBasePar* PXDGeometryPar::createSensorInfo(const GearDir& sensor)
{
  PXDSensorInfoPar* info = new PXDSensorInfoPar(
    VxdID(0, 0, 0),
    sensor.getLength("width"),
    sensor.getLength("length"),
    sensor.getLength("height"),
    sensor.getInt("pixelsU"),
    sensor.getInt("pixelsV[1]"),
    sensor.getLength("splitLength", 0),
    sensor.getInt("pixelsV[2]", 0)
  );

  info->setDEPFETParams(
    sensor.getDouble("BulkDoping") / (Unit::um * Unit::um * Unit::um),
    sensor.getWithUnit("BackVoltage"),
    sensor.getWithUnit("TopVoltage"),
    sensor.getLength("SourceBorderSmallPixel"),
    sensor.getLength("ClearBorderSmallPixel"),
    sensor.getLength("DrainBorderSmallPixel"),
    sensor.getLength("SourceBorderLargePixel"),
    sensor.getLength("ClearBorderLargePixel"),
    sensor.getLength("DrainBorderLargePixel"),
    sensor.getLength("GateDepth"),
    sensor.getBool("DoublePixel"),
    sensor.getDouble("ChargeThreshold"),
    sensor.getDouble("NoiseFraction")
  );
  info->setIntegrationWindow(
    sensor.getTime("IntegrationStart"),
    sensor.getTime("IntegrationEnd")
  );
  m_SensorInfo.push_back(info);
  return info;
}


