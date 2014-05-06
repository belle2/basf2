/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/PassData.h"
#include <framework/logging/Logger.h> // for B2WARNING

#include "tracking/dataobjects/FullSecID.h"

using namespace std;
using namespace Belle2;


std::pair<int, int> Belle2::PassData::importSectorMap(const Belle2::SecMapCopy& rawSecMap)
{
  this->sectorMap.clear();
  int totalFriendCounter = 0;
  int storedCutoffCtr = 0;

  double cutoffMinValue, cutoffMaxValue;
  pair<double, double> cutoff; // will be rewritten for each cutoff to be imported

  for (const SectorPack & aSector : rawSecMap) {
    B2DEBUG(110, "PassData::importSectorMap: importing sector: " << FullSecID(aSector.first) << " (named " << aSector.first << " as an int) including " << aSector.second.size() << " friends. ");
    totalFriendCounter += aSector.second.size();
    VXDSector* pSector = new VXDSector(aSector.first);

    for (const FriendPack & aFriend : aSector.second) { // Friend.first is friendID
      B2DEBUG(120, "PassData::importSectorMap: importing friend: " << FullSecID(aFriend.first) << " (named " << aFriend.first << " as an int) including " << aFriend.second.size() << " filters. ");

      for (const CutoffPack & aFilter : aFriend.second) {
        B2DEBUG(130, "PassData::importSectorMap: importing filter: " << FilterID().getFilterString(aFilter.first) << " (named " << aFilter.first << " as an int) including Min/Max: " << aFilter.second.first << "/" << aFilter.second.second);
        // aFilter.first is filterID, .second is cutoff, where .second.first is min, .second.second is max
        unsigned int filterID = aFilter.first;
        bool doNotStore = false;
        cutoffMinValue = 0, cutoffMaxValue = 0;
        if (filterID >= int(FilterID::numFilters)) { B2FATAL("PassData::importSectorMap: Filter in XML-File does not exist! check FilterID-class!")}
        // now, for each filter will be checked, whether it shall be stored or not and whether the cutoffs shall be modified:
        if (filterID == FilterID::distance3D && this->distance3D.first == true) {   // first: activateDistance3D, second: tuneDistance3D
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->distance3D.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->distance3D.second);

        } else if (filterID == FilterID::distanceXY &&  this->distanceXY.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->distanceXY.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->distanceXY.second);

        } else if (filterID == FilterID::distanceZ &&  this->distanceZ.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->distanceZ.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->distanceZ.second);

        } else if (filterID == FilterID::slopeRZ &&  this->slopeRZ.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->slopeRZ.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->slopeRZ.second);

        } else if (filterID == FilterID::normedDistance3D &&  this->normedDistance3D.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->normedDistance3D.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->normedDistance3D.second);

        } else if (filterID == FilterID::angles3D &&  this->angles3D.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->angles3D.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->angles3D.second);

        } else if (filterID == FilterID::anglesXY &&  this->anglesXY.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->anglesXY.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->anglesXY.second);

        } else if (filterID == FilterID::anglesRZ &&  this->anglesRZ.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->anglesRZ.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->anglesRZ.second);

        } else if (filterID == FilterID::pT &&  this->pT.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->pT.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->pT.second);

        } else if (filterID == FilterID::helixParameterFit &&  this->helixFit.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->helixFit.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->helixFit.second);

        } else if (filterID == FilterID::distance2IP &&  this->distance2IP.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->distance2IP.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->distance2IP.second);

        } else if (filterID == FilterID::deltaSlopeRZ &&  this->deltaSlopeRZ.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->deltaSlopeRZ.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->deltaSlopeRZ.second);

        } else if (filterID == FilterID::deltapT &&  this->deltaPt.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->deltaPt.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->deltaPt.second);

        } else if (filterID == FilterID::deltaDistance2IP &&  this->deltaDistance2IP.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->deltaDistance2IP.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->deltaDistance2IP.second);

        } else if (filterID == FilterID::anglesHighOccupancy3D &&  this->anglesHighOccupancy3D.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->anglesHighOccupancy3D.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->anglesHighOccupancy3D.second);

        } else if (filterID == FilterID::anglesHighOccupancyXY &&  this->anglesHighOccupancyXY.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->anglesHighOccupancyXY.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->anglesHighOccupancyXY.second);

        } else if (filterID == FilterID::anglesHighOccupancyRZ &&  this->anglesHighOccupancyRZ.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->anglesHighOccupancyRZ.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->anglesHighOccupancyRZ.second);

        } else if (filterID == FilterID::pTHighOccupancy &&  this->pTHighOccupancy.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->pTHighOccupancy.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->pTHighOccupancy.second);

        } else if (filterID == FilterID::helixHighOccupancyFit &&  this->helixHighOccupancyFit.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->helixHighOccupancyFit.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->helixHighOccupancyFit.second);

        } else if (filterID == FilterID::distanceHighOccupancy2IP &&  this->distanceHighOccupancy2IP.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->distanceHighOccupancy2IP.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->distanceHighOccupancy2IP.second);

        } else if (filterID == FilterID::deltaSlopeHighOccupancyRZ &&  this->deltaSlopeHighOccupancyRZ.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->deltaSlopeHighOccupancyRZ.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->deltaSlopeHighOccupancyRZ.second);

        } else if (filterID == FilterID::deltapTHighOccupancy &&  this->deltaPtHighOccupancy.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->deltaPtHighOccupancy.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->deltaPtHighOccupancy.second);

        } else if (filterID == FilterID::deltaDistanceHighOccupancy2IP &&  this->deltaDistanceHighOccupancy2IP.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->deltaDistanceHighOccupancy2IP.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->deltaDistanceHighOccupancy2IP.second);
        } else { B2DEBUG(130, "PassData::importSectorMap: filter not accepted: sector/friend/filterID: " << FullSecID(aSector.first).getFullSecString() << "/" << FullSecID(aFriend.first).getFullSecString() << "/" << FilterID().getFilterString(aFilter.first) << "!"); doNotStore = true; }

        if (doNotStore == false) {
          cutoff = make_pair(cutoffMinValue, cutoffMaxValue);
          pSector->addCutoff(filterID, aFriend.first, cutoff); // adds friendSector if it does not exist yet
          ++storedCutoffCtr;
        }
      }
    }
    this->sectorMap.insert(make_pair(aSector.first, pSector));
  }


  //generating virtual sector (represents the region of the primary vertex)
  unsigned int centerSecID = FullSecID().getFullSecID(); // automatically produces secID of centerSector
  VXDSector* pCenterSector = new VXDSector(centerSecID);
  this->sectorMap.insert(make_pair(centerSecID, pCenterSector));
  B2DEBUG(10, "PassData::importSectorMap: adding virtual centerSector with " << this->sectorMap.find(centerSecID)->second->getFriends().size() << " friends. SecMap got " << this->sectorMap.size() << " entries now");

  return make_pair(totalFriendCounter, storedCutoffCtr);
}



void PassData::activateAllFilters()
{
  Filter standardValue = make_pair(true, 0); // type pair<bool isActivated, double tuningParameter>
  distance3D = standardValue;
  distanceXY = standardValue;
  distanceZ = standardValue;
  slopeRZ = standardValue;
  normedDistance3D = standardValue;
  activatedSegFinderTests = 5;

  anglesHighOccupancy3D = standardValue;
  anglesHighOccupancyXY = standardValue;
  anglesHighOccupancyRZ = standardValue;
  deltaSlopeHighOccupancyRZ = standardValue;
  pTHighOccupancy = standardValue;
  distanceHighOccupancy2IP = standardValue;
  helixHighOccupancyFit = standardValue;
  activatedHighOccupancySegFinderTests = 7;

  angles3D = standardValue;
  anglesXY = standardValue;
  anglesRZ = standardValue;
  deltaSlopeRZ = standardValue;
  pT = standardValue;
  distance2IP = standardValue;
  helixFit = standardValue;
  activatedNbFinderTests = 7;

  deltaPtHighOccupancy = standardValue;
  deltaDistanceHighOccupancy2IP = standardValue;
  activatedHighOccupancyNbFinderTests = 2;

  zigzagXY = standardValue;
  zigzagRZ = standardValue;
  deltaPt = standardValue;
  deltaDistance2IP = standardValue;
  circleFit = standardValue;
  activatedTccFilterTests = 5;
}