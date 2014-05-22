/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/dataobjects/FilterID.h"
#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;

const string FilterID::nameAngles3D = "angles3D";
const string FilterID::nameAnglesRZ = "anglesRZ";
const string FilterID::nameAnglesXY = "anglesXY";
const string FilterID::nameDistance3D = "distance3D";
const string FilterID::nameDistanceXY = "distanceXY";
const string FilterID::nameDistanceZ = "distanceZ";
const string FilterID::nameHelixParameterFit = "helixParameterFit";
const string FilterID::nameDeltaSOverZ = "deltaSOverZ";
const string FilterID::nameDeltaSlopeZOverS = "deltaSlopeZOverS";
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
const string FilterID::nameHelixParameterHighOccupancyFit = "HioChelixFit";
const string FilterID::nameDeltapTHighOccupancy  = "HioCdeltaPt";
const string FilterID::nameDeltaDistanceHighOccupancy2IP  = "HioCdeltaDist2IP";

const string FilterID::nameNbFinderLost  = "NbFinderLost";
const string FilterID::nameCellularAutomaton  = "CellularAutomaton";


const string FilterID::nameHopfield = "Hopfield";
const string FilterID::nameGreedy = "Greedy";
const string FilterID::nameTcDuel = "TcDuel";
const string FilterID::nameTcFinderCurr = "TcFinderCurr";
const string FilterID::nameZiggZaggXY = "ZiggZaggXY";
const string FilterID::nameZiggZaggRZ = "ZiggZaggRZ";
const string FilterID::nameCalcQIbyKalman = "CalcQIbyKalman";
const string FilterID::nameOverlapping = "Overlapping";
const string FilterID::nameCirclefit = "Circlefit";
const string FilterID::nameSimpleLineFit3D = "SimpleLineFit3D";
const string FilterID::nameHelixfit = "Helixfit";


const string FilterID::nameSilentTcc = "SilentTcc";
const string FilterID::nameSilentHitFinder = "SilentHitFinder";
const string FilterID::nameSilentSegFinder = "SilentSegFinder";

const string FilterID::nameAlwaysTrue = "alwaysTrue";
const string FilterID::nameAlwaysFalse = "alwaysFalse";
const string FilterID::nameRandomTrue = "randomTrue";

const string FilterID::nameOverHighestAllowedLayer = "overHighestAllowedLayer";
const string FilterID::nameOutOfSectorRange = "outOfSectorRange";



const boost::array<string, FilterID::numFilters> FilterID::nameVector = { {
    nameAngles3D,
    nameAnglesRZ,
    nameAnglesXY,
    nameDistance3D,
    nameDistanceXY,
    nameDistanceZ,
    nameHelixParameterFit,
    nameDeltaSOverZ,
    nameDeltaSlopeZOverS,
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
    nameHelixParameterHighOccupancyFit,
    nameDeltapTHighOccupancy,
    nameDeltaDistanceHighOccupancy2IP,
    nameNbFinderLost,
    nameCellularAutomaton,

    nameHopfield,
    nameGreedy,
    nameTcDuel,
    nameTcFinderCurr,
    nameZiggZaggXY,
    nameZiggZaggRZ,
    nameCalcQIbyKalman,
    nameOverlapping,
    nameCirclefit,
    nameHelixfit,
    nameSimpleLineFit3D,
    nameSilentTcc,
    nameSilentHitFinder,
    nameSilentSegFinder,

    nameAlwaysTrue,
    nameAlwaysFalse,
    nameRandomTrue,

    nameOverHighestAllowedLayer,
    nameOutOfSectorRange
  }
};


const boost::array<FilterID::filterTypes, FilterID::numFilters> FilterID::enumVector = { {
    FilterID::angles3D,
    FilterID::anglesRZ,
    FilterID::anglesXY,
    FilterID::distance3D,
    FilterID::distanceXY,
    FilterID::distanceZ,
    FilterID::helixParameterFit,
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
    FilterID::helixParameterHighOccupancyFit,
    FilterID::deltapTHighOccupancy,
    FilterID::deltaDistanceHighOccupancy2IP,
    FilterID::nbFinderLost,
    FilterID::cellularAutomaton,

    FilterID::hopfield,
    FilterID::greedy,
    FilterID::tcDuel,
    FilterID::tcFinderCurr,
    FilterID::ziggZaggXY,
    FilterID::ziggZaggRZ,
    FilterID::calcQIbyKalman,
    FilterID::overlapping,
    FilterID::circlefit,
    FilterID::helixfit,
    FilterID::simpleLineFit3D,

    FilterID::silentTcc,
    FilterID::silentHitFinder,
    FilterID::silentSegFinder,

    FilterID::alwaysTrue,
    FilterID::alwaysFalse,
    FilterID::randomTrue,

    FilterID::deltaSOverZ,
    FilterID::deltaSlopeZOverS,

    FilterID::overHighestAllowedLayer,
    FilterID::outOfSectorRange

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
  else if (filterString == nameHelixParameterFit)
  {return helixParameterFit;}
  else if (filterString == nameDeltaSOverZ)
  {return deltaSOverZ;}
  else if (filterString == nameDeltaSlopeZOverS)
  {return deltaSlopeZOverS;}
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
  else if (filterString == nameHelixParameterHighOccupancyFit)
  {return helixParameterHighOccupancyFit;}
  else if (filterString == nameDeltapTHighOccupancy)
  {return deltapTHighOccupancy;}
  else if (filterString == nameDeltaDistanceHighOccupancy2IP)
  {return deltaDistanceHighOccupancy2IP;}
  else if (filterString == nameNbFinderLost)
  {return nbFinderLost;}
  else if (filterString == nameCellularAutomaton)
  {return cellularAutomaton;}

  else if (filterString == nameHopfield)
  {return hopfield;}
  else if (filterString == nameGreedy)
  {return greedy;}

  else if (filterString == nameTcDuel)
  {return tcDuel;}
  else if (filterString == nameTcFinderCurr)
  {return tcFinderCurr;}

  else if (filterString == nameZiggZaggXY)
  {return ziggZaggXY;}
  else if (filterString == nameZiggZaggRZ)
  {return ziggZaggRZ;}

  else if (filterString == nameCalcQIbyKalman)
  {return calcQIbyKalman;}
  else if (filterString == nameOverlapping)
  {return overlapping;}
  else if (filterString == nameCirclefit)
  {return circlefit;}
  else if (filterString == nameHelixfit)
  {return helixfit;}
  else if (filterString == nameSimpleLineFit3D)
  {return simpleLineFit3D;}

  else if (filterString == nameSilentTcc)
  {return silentTcc;}
  else if (filterString == nameSilentHitFinder)
  {return silentHitFinder;}
  else if (filterString == nameSilentSegFinder)
  {return silentSegFinder;}

  else if (filterString == nameAlwaysTrue)
  {return alwaysTrue;}
  else if (filterString == nameAlwaysFalse)
  {return alwaysFalse;}
  else if (filterString == nameRandomTrue)
  {return randomTrue;}

  else if (filterString == nameOverHighestAllowedLayer)
  {return overHighestAllowedLayer;}
  else if (filterString == nameOutOfSectorRange)
  {return outOfSectorRange;}

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


std::string FilterID::getFilterString(int filterType)
{
  if (filterType >= angles3D && filterType < numFilters) {
    return nameVector[filterType];
  }
  B2DEBUG(1, " FilterID::getFilterString. Input int is out of range, please check that...")
  return "";
}
