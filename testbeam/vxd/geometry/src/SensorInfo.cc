/*
 * SensorInfo.cc
 *
 *  Created on: Jan 9, 2014
 *      Author: kvasnicka
 */

#include <testbeam/vxd/geometry/SensorInfo.h>
#include <geometry/bfieldmap/BFieldMap.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::TEL;

const TVector3 SensorInfo::getBField(const TVector3& point) const
{
  TVector3 pointGlobal = pointToGlobal(point);
  TVector3 bGlobal = BFieldMap::Instance().getBField(pointGlobal);
  TVector3 bLocal = vectorToLocal(bGlobal);
  return Unit::TinStdUnits * bLocal;
}



