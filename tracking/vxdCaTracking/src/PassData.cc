/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/PassData.h"
#include <framework/logging/Logger.h> // for B2WARNING

#include "tracking/dataobjects/FullSecID.h"

using namespace std;
using namespace Belle2;


std::pair<int, int> PassData::importSectorMap(const VXDTFSecMapTypedef::SecMapCopy& rawSecMap, const  VXDTFRawSecMapTypedef::SectorDistancesMap& distancesMap, bool useDistances)
{
  this->sectorMap.clear();
  int totalFriendCounter = 0;
  int storedCutoffCtr = 0;

  double cutoffMinValue, cutoffMaxValue;
  VXDTFSecMapTypedef::CutoffValue cutoff; // will be rewritten for each cutoff to be imported

  for (const VXDTFSecMapTypedef::Sector & aSector : rawSecMap) {
    FullSecID currentSecID = aSector.first, newSecID;

    B2DEBUG(110, "PassData::importSectorMap: importing sector: " << currentSecID << " including " << aSector.second.size() << " friends. ");
    totalFriendCounter += aSector.second.size();

    VXDSector* pSector;
    if (useDistances == true) {
      auto iterator = std::find_if(distancesMap.begin(),
                                   distancesMap.end(),
                                   [&currentSecID](VXDTFRawSecMapTypedef::SectorDistance const & elem)
      { return elem.first == currentSecID; } // searching for current sector
                                  );

      if (iterator == distancesMap.end()) {
        newSecID = FullSecID(currentSecID.getVxdID(), false, currentSecID.getSecID());
        iterator = std::find_if(distancesMap.begin(),
                                distancesMap.end(),
                                [&newSecID](VXDTFRawSecMapTypedef::SectorDistance const & elem)
        { return elem.first == newSecID; } // searching for current sector
                               );
      }
      if (iterator == distancesMap.end()) { // if still not found, print warning...
        B2WARNING("neither sector " << currentSecID << " nor " << newSecID << " found, initialising using 0 for distance")
      }
      pSector = new VXDSector(currentSecID, iterator->second, useDistances);
    } else {
      pSector = new VXDSector(currentSecID);
    }

    // importing friends and filters
    for (const VXDTFSecMapTypedef::Friend & aFriend : aSector.second) {
      FullSecID currentFriendID = aFriend.first;
      B2DEBUG(120, "PassData::importSectorMap: importing friend: " << currentFriendID << " including " << aFriend.second.size() << " filters. ");

      for (const VXDTFSecMapTypedef::Cutoff & aFilter : aFriend.second) {
        B2DEBUG(130, "PassData::importSectorMap: importing filter: " << FilterID().getFilterString(aFilter.first) << " (named " << aFilter.first << " as an int) including Min/Max: " << aFilter.second.first << "/" << aFilter.second.second);
        // aFilter.first is filterID, .second is cutoff, where .second.first is min, .second.second is max
        unsigned int filterID = aFilter.first;
        bool doNotStore = false;
        cutoffMinValue = 0, cutoffMaxValue = 0;
        if (filterID >= static_cast<unsigned int>(FilterID::numFilters)) { B2FATAL("PassData::importSectorMap: Filter in XML-File does not exist! check FilterID-class!")}

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

        } else if (filterID == FilterID::helixParameterFit &&  this->helixParameterFit.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->helixParameterFit.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->helixParameterFit.second);

        } else if (filterID == FilterID::deltaSOverZ &&  this->deltaSOverZ.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->deltaSOverZ.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->deltaSOverZ.second);

        } else if (filterID == FilterID::deltaSlopeZOverS &&  this->deltaSlopeZOverS.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->deltaSlopeZOverS.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->deltaSlopeZOverS.second);

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

        } else if (filterID == FilterID::helixParameterHighOccupancyFit &&  this->helixParameterHighOccupancyFit.first == true) {
          cutoffMinValue = addExtraGain(-1, aFilter.second.first, this->helixParameterHighOccupancyFit.second);
          cutoffMaxValue = addExtraGain(+1, aFilter.second.second, this->helixParameterHighOccupancyFit.second);

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
        } else { B2DEBUG(130, "PassData::importSectorMap: filter not accepted: sector/friend/filterID: " << currentSecID << "/" << currentFriendID << "/" << FilterID().getFilterString(aFilter.first) << "!"); doNotStore = true; }

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

  linkSectorsToFriends();

  return make_pair(totalFriendCounter, storedCutoffCtr);
}



void PassData::activateAllFilters()
{
  PassDataTypedef::Filter standardValue = make_pair(true, 0); // type pair<bool isActivated, double tuningParameter>
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
  helixParameterHighOccupancyFit = standardValue;
  activatedHighOccupancySegFinderTests = 7;

  angles3D = standardValue;
  anglesXY = standardValue;
  anglesRZ = standardValue;
  deltaSlopeRZ = standardValue;
  pT = standardValue;
  distance2IP = standardValue;
  helixParameterFit = standardValue;
  deltaSOverZ = standardValue;
  deltaSlopeZOverS = standardValue;
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