/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/FilterID.h"
#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;

const string FilterID::nameAngles3D = "angles3D";
const string FilterID::nameAnglesRZ = "anglesRZ";
const string FilterID::nameAnglesXY = "anglesXY";
const string FilterID::nameDistance3D = "distance3D";
const string FilterID::nameDistanceXY = "distanceXY";
const string FilterID::nameDistanceZ = "distanceZ";
const string FilterID::nameHelixFit = "helixFit";
const string FilterID::nameSlopeRZ = "slopeRZ";
const string FilterID::nameDeltaSlopeRZ = "deltaSlopeRZ";
const string FilterID::namePT = "pT";
const string FilterID::nameDeltapT = "deltaPt";
const string FilterID::nameNormedDistance3D = "normedDistance3D";
const string FilterID::nameDistance2IP = "dist2IP";
const string FilterID::nameDeltaDistance2IP = "deltaDist2IP";

const string FilterID::nameAnglesHighOccupancy3D  = "HioCangles3D"; // HioC means high occupancy
const string FilterID::nameAnglesHighOccupancyXY = "HioCanglesXY";
const string FilterID::nameAnglesHighOccupancyRZ  = "HioCanglesRZ";
const string FilterID::nameDistanceHighOccupancy2IP = "HioCdist2IP";
const string FilterID::nameDeltaSlopeHighOccupancyRZ = "HioCdeltaSlopeRZ";
const string FilterID::namePTHighOccupancy = "HioCpT";
const string FilterID::nameHelixHighOccupancyFit = "HioChelixFit";
const string FilterID::nameDeltapTHighOccupancy  = "HioCdeltaPt";
const string FilterID::nameDeltaDistanceHighOccupancy2IP  = "HioCdeltaDist2IP";


const boost::array<string, FilterID::numFilters> FilterID::nameVector = { {
    nameAngles3D,
    nameAnglesRZ,
    nameAnglesXY,
    nameDistance3D,
    nameDistanceXY,
    nameDistanceZ,
    nameHelixFit,
    nameSlopeRZ,
    nameDeltaSlopeRZ,
    namePT,
    nameDeltapT,
    nameNormedDistance3D,
    nameDistance2IP,
    nameDeltaDistance2IP,
    nameAnglesHighOccupancy3D,
    nameAnglesHighOccupancyXY,
    nameAnglesHighOccupancyRZ,
    nameDistanceHighOccupancy2IP,
    nameDeltaSlopeHighOccupancyRZ,
    namePTHighOccupancy,
    nameHelixHighOccupancyFit,
    nameDeltapTHighOccupancy,
    nameDeltaDistanceHighOccupancy2IP
  }
};


const boost::array<FilterID::filterTypes, FilterID::numFilters> FilterID::enumVector = { {
    FilterID::angles3D,
    FilterID::anglesRZ,
    FilterID::anglesXY,
    FilterID::distance3D,
    FilterID::distanceXY,
    FilterID::distanceZ,
    FilterID::helixFit,
    FilterID::slopeRZ,
    FilterID::deltaSlopeRZ,
    FilterID::pT,
    FilterID::deltapT,
    FilterID::normedDistance3D,
    FilterID::distance2IP,
    FilterID::deltaDistance2IP,
    FilterID::anglesHighOccupancy3D,
    FilterID::anglesHighOccupancyXY,
    FilterID::anglesHighOccupancyRZ,
    FilterID::distanceHighOccupancy2IP,
    FilterID::deltaSlopeHighOccupancyRZ,
    FilterID::pTHighOccupancy,
    FilterID::helixHighOccupancyFit,
    FilterID::deltapTHighOccupancy,
    FilterID::deltaDistanceHighOccupancy2IP
  }
};


FilterID::filterTypes FilterID::getFilterType(std::string filterString)
{
  if (filterString == nameAngles3D)
  { return angles3D; }
  else if (filterString == nameAnglesRZ)
  {return anglesRZ;}
  else if (filterString == nameAnglesXY)
  {return anglesXY;}
  else if (filterString == nameDistance3D)
  {return distance3D;}
  else if (filterString == nameDistanceXY)
  {return distanceXY;}
  else if (filterString == nameDistanceZ)
  {return distanceZ;}
  else if (filterString == nameHelixFit)
  {return helixFit;}
  else if (filterString == nameSlopeRZ)
  {return slopeRZ;}
  else if (filterString == nameDeltaSlopeRZ)
  {return deltaSlopeRZ;}
  else if (filterString == namePT)
  {return pT;}
  else if (filterString == nameDeltapT)
  {return deltapT;}
  else if (filterString == nameNormedDistance3D)
  {return normedDistance3D;}
  else if (filterString == nameDistance2IP)
  {return distance2IP;}
  else if (filterString == nameDeltaDistance2IP)
  {return deltaDistance2IP;}
  else if (filterString == nameAnglesHighOccupancy3D)
  {return anglesHighOccupancy3D;}
  else if (filterString == nameAnglesHighOccupancyXY)
  {return anglesHighOccupancyXY;}
  else if (filterString == nameAnglesHighOccupancyRZ)
  {return anglesHighOccupancyRZ;}
  else if (filterString == nameDistanceHighOccupancy2IP)
  {return distanceHighOccupancy2IP;}
  else if (filterString == nameDeltaSlopeHighOccupancyRZ)
  {return deltaSlopeHighOccupancyRZ;}
  else if (filterString == namePTHighOccupancy)
  {return pTHighOccupancy;}
  else if (filterString == nameHelixHighOccupancyFit)
  {return helixHighOccupancyFit;}
  else if (filterString == nameDeltapTHighOccupancy)
  {return deltapTHighOccupancy;}
  else if (filterString == nameDeltaDistanceHighOccupancy2IP)
  {return deltaDistanceHighOccupancy2IP;}
  else {
    B2DEBUG(1, " FilterID::getFilterType. Input string is out of range, please check that...")
    return numFilters;
  }
}

FilterID::filterTypes FilterID::getFilterType(int filterInt)
{
  if (filterInt >= angles3D && filterInt < numFilters) {
    return enumVector[filterInt];
  }
  B2DEBUG(1, " FilterID::getFilterType. Input int is out of range, please check that...")
  return numFilters;
}

std::string FilterID::getFilterString(FilterID::filterTypes filterType)
{
  return nameVector[filterType];
//  switch (filterType) {
//    case angles3D:
//      return GlobalNames::nameAngles3D;
//    case anglesRZ:
//      return GlobalNames::nameAnglesRZ;
//    case anglesXY:
//      return GlobalNames::nameAnglesXY;
//    case distance3D:
//      return GlobalNames::nameDistance3D;
//    case distanceXY:
//      return GlobalNames::nameDistanceXY;
//    case distanceZ:
//      return GlobalNames::nameDistanceZ;
//    case helixFit:
//      return GlobalNames::nameHelixFit;
//    case slopeRZ:
//      return GlobalNames::nameSlopeRZ;
//    case deltaSlopeRZ:
//      return GlobalNames::nameDeltaSlopeRZ;
//    case pT:
//      return GlobalNames::namePT;
//    case deltapT:
//      return GlobalNames::nameDeltapT;
//    case normedDistance3D:
//      return GlobalNames::nameNormedDistance3D;
//    case distance2IP:
//      return GlobalNames::nameDistance2IP;
//    case deltaDistance2IP:
//      return GlobalNames::nameDeltaDistance2IP;
//    case anglesHighOccupancy3D:
//      return GlobalNames::nameAnglesHighOccupancy3D;
//    case anglesHighOccupancyXY:
//      return GlobalNames::nameAnglesHighOccupancyXY;
//    case anglesHighOccupancyRZ:
//      return GlobalNames::nameAnglesHighOccupancyRZ;
//    case distanceHighOccupancy2IP:
//      return GlobalNames::nameDistanceHighOccupancy2IP;
//    case deltaSlopeHighOccupancyRZ:
//      return GlobalNames::nameDeltaSlopeHighOccupancyRZ;
//    case pTHighOccupancy:
//      return GlobalNames::namePTHighOccupancy;
//    case helixHighOccupancyFit:
//      return GlobalNames::nameHelixHighOccupancyFit;
//    case deltapTHighOccupancy:
//      return GlobalNames::nameDeltapTHighOccupancy;
//    case deltaDistanceHighOccupancy2IP:
//      return GlobalNames::nameDeltaDistanceHighOccupancy2IP;
//    default:
//      return "";
//  }
}

std::string FilterID::getFilterString(int filterType)
{
  if (filterType >= angles3D && filterType < numFilters) {
    return nameVector[filterType];
  }
  B2DEBUG(1, " FilterID::getFilterString. Input int is out of range, please check that...")
  return "";
}