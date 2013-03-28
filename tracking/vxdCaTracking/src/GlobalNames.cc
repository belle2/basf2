/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/GlobalNames.h"


using namespace std;
using namespace Belle2;


GlobalNames::GlobalNames()
{
  nameAngles3D = "angles3D";
  nameAnglesRZ = "anglesRZ";
  nameAnglesXY = "anglesXY";
  nameDistance3D = "distance3D";
  nameDistanceXY = "distanceXY";
  nameDistanceZ = "distanceZ";
  nameHelixFit = "helixFit";
  nameSlopeRZ = "slopeRZ";
  nameDeltaSlopeRZ = "deltaSlopeRZ";
  namePT = "pT";
  nameDeltapT = "deltaPt";
  nameNormedDistance3D = "normedDistance3D";
  nameDistance2IP = "dist2IP";
  nameDeltaDistance2IP = "deltaDist2IP";

  nameAnglesHighOccupancy3D  = "HioCangles3D"; // HioC means high occupancy
  nameAnglesHighOccupancyXY = "HioCanglesXY";
  nameAnglesHighOccupancyRZ  = "HioCanglesRZ";
  nameDistanceHighOccupancy2IP = "HioCdist2IP";
  nameDeltaSlopeHighOccupancyRZ = "HioCdeltaSlopeRZ";
  namePTHighOccupancy = "HioCpT";
  nameHelixHighOccupancyFit = "HioChelixFit";
  nameDeltapTHighOccupancy  = "HioCdeltaPt";
  nameDeltaDistanceHighOccupancy2IP  = "HioCdeltaDist2IP";
}





