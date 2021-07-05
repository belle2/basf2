/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vxd/dbobjects/VXDGeometryPar.h>
#include <framework/logging/Logger.h>
#include <boost/format.hpp>

using namespace Belle2;
using namespace std;


const VXDGeoComponentPar& VXDGeometryPar::getComponent(std::string name) const
{
  //Check if component already exists
  std::map<string, VXDGeoComponentPar>::const_iterator cached = m_componentCache.find(name);
  if (cached == m_componentCache.end()) {
    B2FATAL("Could not find component " << name);
  }
  return cached->second;
}



int  VXDGeometryPar::getSensitiveChipID(std::string name) const
{
  //Check if sensorType already exists
  std::map<string, int>::const_iterator cached = m_sensitiveIDCache.find(name);
  if (cached == m_sensitiveIDCache.end()) {
    return -1;
  }
  return cached->second;
}



const VXDGeoSensorPar& VXDGeometryPar::getSensor(string sensorTypeID) const
{
  //Check if sensorType already exists
  std::map<string, VXDGeoSensorPar>::const_iterator cached = m_sensorMap.find(sensorTypeID);
  if (cached == m_sensorMap.end()) {
    B2FATAL("Invalid SensorTypeID " << sensorTypeID);
  }
  return cached->second;
}

const VXDGeoLadderPar& VXDGeometryPar::getLadder(int layer) const
{
  //Check if component already exists
  map<int, VXDGeoLadderPar>::const_iterator cached = m_ladders.find(layer);
  if (cached == m_ladders.end()) {
    B2FATAL("Could not find ladder for layer " << (boost::format("%1%") % layer).str());
  }
  return cached->second;
}


VXDAlignmentPar VXDGeometryPar::getAlignment(std::string name) const
{
  //Check if component already exists
  map<string, VXDAlignmentPar>::const_iterator cached = m_alignment.find(name);
  if (cached == m_alignment.end()) {
    B2FATAL("Could not find alignment parameters for component " << name);
  }
  return cached->second;
}





