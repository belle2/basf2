/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "tracking/dataobjects/FilterID.h"
#include <framework/logging/Logger.h>

#include <algorithm>

using namespace std;
using namespace Belle2;

// 2hit:
const string FilterID::nameDistance3D = "distance3D";
const string FilterID::nameDistanceXY = "distanceXY";
const string FilterID::nameDistanceZ = "distanceZ";
const string FilterID::nameSlopeRZ = "slopeRZ";
const string FilterID::nameNormedDistance3D = "normedDistance3D";


//2+1hit:
const string FilterID::nameAnglesHighOccupancy3D  = "HioCangles3D"; // HioC means high occupancy
const string FilterID::nameAnglesHighOccupancyXY = "HioCanglesXY";
const string FilterID::nameAnglesHighOccupancyRZ  = "HioCanglesRZ";
const string FilterID::nameDistanceHighOccupancy2IP = "HioCdistance2IP";
const string FilterID::nameDeltaSlopeHighOccupancyRZ = "HioCdeltaSlopeRZ";
const string FilterID::namePTHighOccupancy = "HioCpT";
const string FilterID::nameHelixParameterHighOccupancyFit = "HioChelixFit";


//3hit:
const string FilterID::nameAngles3D = "angles3D";
const string FilterID::nameAnglesRZ = "anglesRZ";
const string FilterID::nameAnglesXY = "anglesXY";
const string FilterID::nameDeltaSlopeRZ = "deltaSlopeRZ";
const string FilterID::namePT = "pT";
const string FilterID::nameDistance2IP = "distance2IP";
const string FilterID::nameHelixParameterFit = "helixParameterFit";
const string FilterID::nameDeltaSOverZ = "deltaSOverZ";
const string FilterID::nameDeltaSlopeZOverS = "deltaSlopeZOverS";


//3+1hit:
const string FilterID::nameDeltapTHighOccupancy  = "HioCdeltaPt";
const string FilterID::nameDeltaDistanceHighOccupancy2IP  = "HioCdeltaDistance2IP";


//4hit:
const string FilterID::nameDeltapT = "deltaPt";
const string FilterID::nameDeltaDistance2IP = "deltaDistance2IP";


//Collector FilterIDs
const string FilterID::nameNbFinderLost  = "NbFinderLost";
const string FilterID::nameCellularAutomaton  = "CellularAutomaton";


// trackletFilters:
const string FilterID::nameZiggZaggXY = "ZiggZaggXY";
const string FilterID::nameZiggZaggXYWithSigma = "ZiggZaggXYWithSigma";
const string FilterID::nameZiggZaggRZ = "ZiggZaggRZ";
const string FilterID::nameCirclefit = "Circlefit";
const string FilterID::nameHelixfit = "Helixfit";
const string FilterID::nameSimpleLineFit3D = "SimpleLineFit3D";


//TF-steps:
const string FilterID::nameHopfield = "Hopfield";
const string FilterID::nameGreedy = "Greedy";
const string FilterID::nameTcDuel = "TcDuel";
const string FilterID::nameTcFinderCurr = "TcFinderCurr";
const string FilterID::nameCalcQIbyKalman = "CalcQIbyKalman";
const string FilterID::nameOverlapping = "Overlapping";
/// WARNING FIXME TODO some things are missing here (calcQIbyLineFit) and similar stuff


// for display: silent kills
const string FilterID::nameSilentTcc = "SilentTcc";
const string FilterID::nameSilentHitFinder = "SilentHitFinder";
const string FilterID::nameSilentSegFinder = "SilentSegFinder";


// tests which are only for debugging and validation processes
const string FilterID::nameAlwaysTrue2Hit = "alwaysTrue2Hit";
const string FilterID::nameAlwaysFalse2Hit = "alwaysFalse2Hit";
const string FilterID::nameAlwaysTrue3Hit = "alwaysTrue3Hit";
const string FilterID::nameAlwaysFalse3Hit = "alwaysFalse3Hit";
const string FilterID::nameAlwaysTrue4Hit = "alwaysTrue4Hit";
const string FilterID::nameAlwaysFalse4Hit = "alwaysFalse4Hit";
const string FilterID::nameRandom2Hit = "random2Hit";
const string FilterID::nameRandom3Hit = "random3Hit";
const string FilterID::nameRandom4Hit = "random4Hit";


// other stuff
const string FilterID::nameOverHighestAllowedLayer = "overHighestAllowedLayer";
const string FilterID::nameOutOfSectorRange = "outOfSectorRange";




const boost::array<string, FilterID::numFilters> FilterID::nameVector = { {
    // 2hit:
    nameDistance3D,
    nameDistanceXY,
    nameDistanceZ,
    nameSlopeRZ,
    nameNormedDistance3D,


    //2+1hit:
    nameAnglesHighOccupancy3D,
    nameAnglesHighOccupancyXY,
    nameAnglesHighOccupancyRZ,
    nameDistanceHighOccupancy2IP,
    nameDeltaSlopeHighOccupancyRZ,
    namePTHighOccupancy,
    nameHelixParameterHighOccupancyFit,


    //3hit:
    nameAngles3D,
    nameAnglesRZ,
    nameAnglesXY,
    nameDeltaSlopeRZ,
    namePT,
    nameDistance2IP,
    nameHelixParameterFit,
    nameDeltaSOverZ,
    nameDeltaSlopeZOverS,


    //3+1hit:
    nameDeltapTHighOccupancy,
    nameDeltaDistanceHighOccupancy2IP,


    //4hit:
    nameDeltapT,
    nameDeltaDistance2IP,


    //Collector FilterIDs
    nameNbFinderLost,
    nameCellularAutomaton,


    // trackletFilters:
    nameZiggZaggXY,
    nameZiggZaggXYWithSigma,
    nameZiggZaggRZ,
    nameCirclefit,
    nameHelixfit,
    nameSimpleLineFit3D,


    //TF-steps:
    nameHopfield,
    nameGreedy,
    nameTcDuel,
    nameTcFinderCurr,
    nameCalcQIbyKalman,
    nameOverlapping,
    /// WARNING FIXME TODO some things are missing here (calcQIbyLineFit) and similar stuff


    // for display: silent kills
    nameSilentTcc,
    nameSilentHitFinder,
    nameSilentSegFinder,


    // tests which are only for debugging and validation processes
    nameAlwaysTrue2Hit,
    nameAlwaysFalse2Hit,
    nameAlwaysTrue3Hit,
    nameAlwaysFalse3Hit,
    nameAlwaysTrue4Hit,
    nameAlwaysFalse4Hit,
    nameRandom2Hit,
    nameRandom3Hit,
    nameRandom4Hit,


    // other stuff
    nameOverHighestAllowedLayer,
    nameOutOfSectorRange
  }
};


const boost::array<FilterID::filterTypes, FilterID::numFilters> FilterID::enumVector = {
  {
    // 2hit:
    FilterID::distance3D,
    FilterID::distanceXY,
    FilterID::distanceZ,
    FilterID::slopeRZ,
    FilterID::normedDistance3D,


    //2+1hit:
    FilterID::anglesHighOccupancy3D,
    FilterID::anglesHighOccupancyXY,
    FilterID::anglesHighOccupancyRZ,
    FilterID::distanceHighOccupancy2IP,
    FilterID::deltaSlopeHighOccupancyRZ,
    FilterID::pTHighOccupancy,
    FilterID::helixParameterHighOccupancyFit,


    //3hit:
    FilterID::angles3D,
    FilterID::anglesRZ,
    FilterID::anglesXY,
    FilterID::deltaSlopeRZ,
    FilterID::pT,
    FilterID::distance2IP,
    FilterID::helixParameterFit,
    FilterID::deltaSOverZ,
    FilterID::deltaSlopeZOverS,


    //3+1hit:
    FilterID::deltapTHighOccupancy,
    FilterID::deltaDistanceHighOccupancy2IP,


    //4hit:
    FilterID::deltapT,
    FilterID::deltaDistance2IP,


    //Collector FilterIDs
    FilterID::nbFinderLost,
    FilterID::cellularAutomaton,


    // trackletFilters:
    FilterID::ziggZaggXY,
    FilterID::ziggZaggXYWithSigma,
    FilterID::ziggZaggRZ,
    FilterID::circlefit,
    FilterID::helixfit,
    FilterID::simpleLineFit3D,


    //TF-steps:
    FilterID::hopfield,
    FilterID::greedy,
    FilterID::tcDuel,
    FilterID::tcFinderCurr,
    FilterID::calcQIbyKalman,
    FilterID::overlapping,
    /// WARNING FIXME TODO some things are missing here (calcQIbyLineFit) and similar stuff


    // for display: silent kills
    FilterID::silentTcc,
    FilterID::silentHitFinder,
    FilterID::silentSegFinder,


    // tests which are only for debugging and validation processes
    FilterID::alwaysTrue2Hit,
    FilterID::alwaysFalse2Hit,
    FilterID::alwaysTrue3Hit,
    FilterID::alwaysFalse3Hit,
    FilterID::alwaysTrue4Hit,
    FilterID::alwaysFalse4Hit,
    FilterID::random2Hit,
    FilterID::random3Hit,
    FilterID::random4Hit,


    // other stuff
    FilterID::overHighestAllowedLayer,
    FilterID::outOfSectorRange
  }
};



FilterID::filterTypes FilterID::getFilterType(const std::string& filterString)
{
  // WARNING TODO FIXME: improve that function (by the way, it is not sorted like the entries from above)
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
  else if (filterString == nameZiggZaggXYWithSigma)
  {return ziggZaggXYWithSigma;}
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

  else if (filterString == nameAlwaysTrue2Hit)
  {return alwaysTrue2Hit;}
  else if (filterString == nameAlwaysFalse2Hit)
  {return alwaysFalse2Hit;}
  else if (filterString == nameAlwaysTrue3Hit)
  {return alwaysTrue3Hit;}
  else if (filterString == nameAlwaysFalse3Hit)
  {return alwaysFalse3Hit;}
  else if (filterString == nameAlwaysTrue4Hit)
  {return alwaysTrue4Hit;}
  else if (filterString == nameAlwaysFalse4Hit)
  {return alwaysFalse4Hit;}
  else if (filterString == nameRandom2Hit)
  {return random2Hit;}
  else if (filterString == nameRandom3Hit)
  {return random3Hit;}
  else if (filterString == nameRandom4Hit)
  {return random4Hit;}

  else if (filterString == nameOverHighestAllowedLayer)
  {return overHighestAllowedLayer;}
  else if (filterString == nameOutOfSectorRange)
  {return outOfSectorRange;}

  else {
    B2WARNING(" FilterID::getFilterType. Input string is out of range, please check that...");
    return numFilters;
  }
}



FilterID::filterTypes FilterID::getFilterType(int filterInt)
{
  if (filterInt >= distance3D && filterInt < numFilters) {
    return enumVector[filterInt];
  }
  B2WARNING(" FilterID::getFilterType. Input int is out of range, please check that...");
  return numFilters;
}



std::string FilterID::getFilterString(int filterType)
{
  if (filterType >= distance3D && filterType < numFilters) {
    return nameVector[filterType];
  }
  B2WARNING(" FilterID::getFilterString. Input int is out of range, please check that...");
  return "";
}


/** for given FilterID the corresponding string-name will be returned. For invalid types, UnknownType will be passed */
std::string FilterID::getTypeName(FilterID::filterTypes type)
{
  /// TODO
  auto pos = std::find(enumVector.begin(), enumVector.end(), type);
  if (pos == enumVector.end()) {
    B2WARNING("FilterID::getTypeName(): given iD " << type <<
              " is not a valid FilterID, return UnknownType!");
    return std::string("UnknownType");
  }
  return FilterID::nameVector[type];
}
