/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/dbobjects/SVDGeometryPar.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;


bool SVDGeometryPar::getSupportRibsExist(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDSupportRibsPar>::const_iterator cached = m_supportRibs.find(layer);
  if (cached == m_supportRibs.end()) {
    return false;
  }
  return true;
}

bool SVDGeometryPar::getEndringsExist(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDEndringsPar>::const_iterator cached = m_endrings.find(layer);
  if (cached == m_endrings.end()) {
    return false;
  }
  return true;
}

bool SVDGeometryPar::getCoolingPipesExist(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDCoolingPipesPar>::const_iterator cached = m_coolingPipes.find(layer);
  if (cached == m_coolingPipes.end()) {
    return false;
  }
  return true;
}


const SVDSupportRibsPar& SVDGeometryPar::getSupportRibs(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDSupportRibsPar>::const_iterator cached = m_supportRibs.find(layer);
  if (cached == m_supportRibs.end()) {
    B2FATAL("No SupportRibs found for layer " << std::to_string(layer));
  }
  return cached->second;
}

const SVDEndringsPar& SVDGeometryPar::getEndrings(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDEndringsPar>::const_iterator cached = m_endrings.find(layer);
  if (cached == m_endrings.end()) {
    B2FATAL("No Endrings found for layer " << std::to_string(layer));
  }
  return cached->second;
}

const SVDCoolingPipesPar& SVDGeometryPar::getCoolingPipes(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDCoolingPipesPar>::const_iterator cached = m_coolingPipes.find(layer);
  if (cached == m_coolingPipes.end()) {
    B2FATAL("No CoolingPipes found for layer " << std::to_string(layer));
  }
  return cached->second;
}


