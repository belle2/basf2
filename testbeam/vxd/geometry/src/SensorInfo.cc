/*
 * SensorInfo.cc
 *
 *  Created on: Jan 9, 2014
 *      Author: kvasnicka
 */

#include <framework/gearbox/Unit.h>
#include <testbeam/vxd/geometry/SensorInfo.h>
#include <framework/geometry/BFieldManager.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::TEL;

const TVector3 SensorInfo::getBField(const TVector3& point) const
{
  TVector3 pointGlobal = pointToGlobal(point);
  TVector3 bGlobal = BFieldManager::getField(pointGlobal);
  TVector3 bLocal = vectorToLocal(bGlobal);
  return bLocal;
}



