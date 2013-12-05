/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/VXDTFSecMap.h>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;


ClassImp(VXDTFSecMap)


void VXDTFSecMap::importRawSectorMap(Belle2::VXDTFRawSecMap& rawMap)
{
  if (rawMap.size() == 0) { B2WARNING("importRawSectorMap: rawMap with name '" << rawMap.getMapName() << "' is empty -> check input data! \n cancelling import..."); return; }
  SectorValue vectorOfFriends;
  FriendValue vectorOfFilterTypes;
  CutoffValue pairOfCutoffs;
  int acceptedFriends = 0, acceptedCutoffs = 0, rejectedSectors = 0, rejectedFriends = 0, rejectedCutoffs = 0, countNormalSamples = 0, countSmallSamples = 0, sampleThreshold = rawMap.getSampleThreshold();

  for (auto aSector : rawMap.getSectorMap()) {
    vectorOfFriends.clear();

    for (auto aFriend : aSector.second) {
      vectorOfFilterTypes.clear();

      for (auto aFilterType : aFriend.second) {

        pairOfCutoffs = rawMap.calcCutoffs(aFilterType.second); // is <0, 0> if it failed

        if (pairOfCutoffs.first == 0 and pairOfCutoffs.second == 0) { rejectedCutoffs++; continue; } // catch empty cutoffs
        acceptedCutoffs++;
        if (sampleThreshold > int(aFilterType.second.size())) { countNormalSamples++; } else { countSmallSamples++; }

        vectorOfFilterTypes.push_back(make_pair(aFilterType.first, pairOfCutoffs));
      }

      if (int(vectorOfFilterTypes.size()) == 0) { rejectedFriends++; continue; } // catch empty set of cutoffs (-> no filter accepted)
      acceptedFriends++;

      vectorOfFriends.push_back(make_pair(aFriend.first, vectorOfFilterTypes));
    }

    if (int(vectorOfFriends.size()) == 0) { rejectedSectors++; continue; } // catch empty set of friends (-> no friend accepted)

    m_sectorMap.push_back(make_pair(aSector.first, vectorOfFriends));
  }

  B2DEBUG(1, "importRawSectorMap: imported sectors/friends/cutoffs: " << size() << "/" << acceptedFriends << "/" << acceptedCutoffs << ", rejected sectors/friends/cutoffs: " << rejectedSectors << "/" << rejectedFriends << "/" << rejectedCutoffs);
  B2DEBUG(1, "importRawSectorMap: with small sample threshold of " << sampleThreshold << ", " << countSmallSamples << "/" << countNormalSamples << " small/normal samples were accepted");

  setMapName(rawMap.getMapName());

  setDetectorType(rawMap.getDetectorType());

  setSectorConfigU(rawMap.getSectorConfigU());

  setSectorConfigV(rawMap.getSectorConfigV());

  setOrigin(rawMap.getOrigin());

  setMagneticFieldStrength(rawMap.getMagneticFieldStrength());

  stringstream originPos;
  originPos << " x: " << getOrigin()[0] << ", y: " << getOrigin()[1] << ", z: " << getOrigin()[2];
  B2DEBUG(1, " now the following parameters are set: mapName " << getMapName() << ", detectorType " << getDetectorType() << ", magneticFieldStrength " << getMagneticFieldStrength() << ", origin " << originPos.str())

  stringstream newInfo;
  newInfo << " lower pT threshold: " << rawMap.getLowerMomentumThreshold() << ", upper pT threshold: " << rawMap.getHigherMomentumThreshold() ;
  setAdditionalInfo(newInfo.str());


  if (size() == 0) { B2WARNING("importRawSectorMap: after importing, internal map with name '" << getMapName() << "' still empty -> check input data!"); /*return;*/ }
}



int VXDTFSecMap::getNumOfValues(SecMapCopy& secMap)
{
  int result = 0;
  for (Sector & aSector : secMap) { // looping over sectors (StrippedRawSecMap)
    for (Friend & afriend : aSector.second) { // looping over friends
      result += afriend.second.size() * 2; // there is only a pair of quantiles (min and max) stored -> size = 2
//       for (Cutoff & aCutoffType : afriend.second) { // looping over CutoffTypes
//         result += 2; // there is only a pair of quantiles (min and max) stored -> size = 2
//       }
    }
  }
  return result;
}