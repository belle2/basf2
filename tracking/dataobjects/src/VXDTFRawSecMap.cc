/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/VXDTFRawSecMap.h>
#include <framework/logging/Logger.h>

#include <algorithm> // std::sort

#include <string> // 
#include <iostream> // stringstream
#include <math.h>       // round
#include "tracking/dataobjects/FullSecID.h"


using namespace std;
using namespace Belle2;



ClassImp(VXDTFRawSecMap);


void VXDTFRawSecMap::addSectorMap(StrippedRawSecMap& newMap)
{
  // first: deal with trivial cases:
  if (int(newMap.size()) == 0) {
    B2DEBUG(1, "addSectorMap: incoming map was empty");
    return;
  }

  sortMap(newMap);

  if (int(m_sectorMap.size()) == 0) {
    m_sectorMap = newMap;
    B2DEBUG(1, "addSectorMap: old map was empty, but new one is not (sectors/friends/values: " << getNumOfSectors() << "/" << getNumOfFriends() << "/" << getNumOfValues() << ") - replacing old map");
    return;
  } // easy case, fully copy the sectorMap after sort

  /// what happens now:
  /*if sector-friend-combination already exists and current filterType too, then all values of added map will be moved to the existing one.
   * If the sector, or the sector-friend-combination or the filterType in that combi does not exist yet, then the data gets copied, not moved
   * -> imported map is not empty in that case!
   * */

  int addedSectorCtr = 0, addedFriendCtr = 0, addedCutoffCtr = 0, addedValuesCtr = 0, // counters for mentioned types
      internalBefore = 0, importedBefore = 0; // store info of values before import to be able to compare it with the values coming afterwards

  B2DEBUG(5, "addSectorMap: internal map before merging size: sectors/friends/values: " << getNumOfSectors() << "/" << getNumOfFriends() << "/" << getNumOfValues());
  B2DEBUG(5, "addSectorMap: imported map before merging size: sectors/friends/values: " << newMap.size() << "/" << getNumOfFriends(newMap) << "/" << getNumOfValues(newMap));

  internalBefore = getNumOfValues();
  importedBefore = getNumOfValues(newMap);

  for (SectorPack & newSector : newMap) {
    bool foundSector = false;
    for (SectorPack & oldSector : m_sectorMap) {
      if (oldSector.first != newSector.first) { continue; }// compares ID of sectors
      foundSector = true;

      for (FriendPack & newFriend : newSector.second) {
        bool foundFriend = false;
        for (FriendPack & oldFriend : oldSector.second) {
          if (oldFriend.first != newFriend.first) { continue; } // compares ID of sectorfriends
          foundFriend = true;

          for (CutoffPack & newCutoffType : newFriend.second) {
            bool foundCutoff = false;
            for (CutoffPack & oldCutoffType : oldFriend.second) {
              if (oldCutoffType.first != newCutoffType.first) { continue; } // compares ID of cutoffType
              foundCutoff = true;
              addedValuesCtr += newCutoffType.second.size();
              oldCutoffType.second.merge(newCutoffType.second);

              break; // we have already found the partner
            }
            if (foundCutoff == false) { oldFriend.second.push_back(newCutoffType); addedCutoffCtr++; }
          }
          break; // we have already found the partner
        }
        if (foundFriend == false) { oldSector.second.push_back(newFriend); addedFriendCtr++; }
      }
      break; // we have already found the partner
    }
    if (foundSector == false) { m_sectorMap.push_back(newSector); addedSectorCtr++; }
  }
  B2DEBUG(5, "addSectorMap: internal map after merging size: sectors/friends/values: " << getNumOfSectors() << "/" << getNumOfFriends() << "/" << getNumOfValues());
  B2DEBUG(5, "addSectorMap: imported map after merging size: sectors/friends/values: " << newMap.size() << "/" << getNumOfFriends(newMap) << "/" << getNumOfValues(newMap));
  B2DEBUG(10, "addSectorMap: if 'values' are not 0 in imported map, then the full friend or sector has been copied which does not delete values from the imported secMap -> 'sectors' or 'friends' != 0!")

  if ((importedBefore + internalBefore) != getNumOfValues()) {
    B2ERROR(" values before (" << importedBefore + internalBefore << ") do not match with values after: " << getNumOfValues())
  }


  B2DEBUG(1, "addSectorMap: added " << addedSectorCtr << " sectors, " << addedFriendCtr << " friends, " << addedCutoffCtr << " cutoffTypes and " << addedValuesCtr << " new values directly ( it was not counted to friends, if new sector was added. Same for friends <->cutoffTypes and cutoffTypes<->new values)");
}


void VXDTFRawSecMap::sortMap(StrippedRawSecMap& newMap)
{
  std::sort(newMap.begin(), newMap.end()); // sorts sectors by value
  for (SectorPack & newSector : newMap) {
    std::sort(newSector.second.begin(), newSector.second.end()); // sorts friends by value
    for (FriendPack & newFriend : newSector.second) {
      std::sort(newFriend.second.begin(), newFriend.second.end()); // sorts filterType by value
      for (CutoffPack & newCutoffType : newFriend.second) {
        newCutoffType.second.sort(); // sorts total cutoff values by value
      }
    }
  }
}



int VXDTFRawSecMap::getNumOfValues(StrippedRawSecMap& secMap)
{
  int result = 0;
  for (SectorPack & aSector : secMap) { // looping over sectors (StrippedRawSecMap)
    for (FriendPack & afriend : aSector.second) { // looping over friends
      for (CutoffPack & aCutoffType : afriend.second) { // looping over CutoffTypes
        result += aCutoffType.second.size();
      }
    }
  }
  return result;
}

std::pair<double, double> VXDTFRawSecMap::calcCutoff(CutoffValues& sample, std::pair<double, double>& quantiles, double stretchFactor)
{
  double lowerCutoff = 0, upperCutoff = 0;
  int sampleSize = sample.size();
  int sampleSizeMinusOne = sampleSize - 1;
  int reverseIndex = -1;
  if (sampleSizeMinusOne < 0) { sampleSizeMinusOne = 0; }
  double sampleSizeD = double(sampleSize);
  pair<int, int> quantileIndex = make_pair(-1, -1); // stores indexPosition of chosen quantile. first is lower cutoff, .second is higher one

  CutoffValues::iterator firstIndex = sample.begin(); // points at first entry
  CutoffValues::reverse_iterator secondIndex = sample.rbegin(); //points at last entry

  // determine which entries we want to export: calculates index position using sampleSize and does boundary-checks
  if (quantiles.first != 0) {
    quantileIndex.first = static_cast<int>(sampleSizeD * quantiles.first + 0.5);
//     quantileIndex.first = int(round(sampleSizeD * quantiles.first));
    checkBiggerThan(quantileIndex.first, sampleSizeMinusOne);
    checkSmallerThan(quantileIndex.first, 0);

    for (int i = 0; i < quantileIndex.first; ++i) { ++firstIndex; } // move iterator to position of the first cutoff
  }

  if (quantiles.second != 1) {
    // maxIndex is sampleSize - 1, therefore -0.5:
    quantileIndex.second = static_cast<int>(sampleSizeD * quantiles.second - 0.5);
//     quantileIndex.second = int(round(sampleSizeD * quantiles.second));
    checkBiggerThan(quantileIndex.second, sampleSizeMinusOne);
    checkSmallerThan(quantileIndex.second, 0);

    reverseIndex = sampleSizeMinusOne - quantileIndex.second; // tells the reverse_iterator how many slots back shall be moved within the list

    // we use a reverse_iterator to move backwards through the list, therefore do a boundary check beforehand:
    checkBiggerThan(reverseIndex, sampleSizeMinusOne);
    checkSmallerThan(reverseIndex, 0);
    for (int i = 0; i < reverseIndex; ++i) { ++secondIndex; } // move iterator to position
  }

  B2DEBUG(50, "calcCutoff: quantiles.first: " << quantiles.first << ", quantiles.second: " << quantiles.second << ", sampleSize: " << sampleSize << ", quantileIndex.first/index: " << quantileIndex.first << ", quantileIndex.second: " << quantileIndex.second << ", reverseIndex: " << reverseIndex << " (all indizes are only set if quantiles.first != 0 and quantiles.second != 1, standardValue = -1)")

  if ((*firstIndex) < (*secondIndex)) {  // give the lower value to lowerCutoff and higher value to upperCutoff
    lowerCutoff = (*firstIndex);
    upperCutoff = (*secondIndex);
  } else {
    lowerCutoff = (*secondIndex);
    upperCutoff = (*firstIndex);
  }
  B2DEBUG(100, "calcCutoff: lowerCutoff: " << lowerCutoff << ", upperCutoff: " << upperCutoff)

  if (stretchFactor != 0) {
    lowerCutoff = addExtraGain(lowerCutoff, (-1.) * stretchFactor); // we want to lower that value, therefore -1
    upperCutoff = addExtraGain(upperCutoff, stretchFactor);
  }

  B2DEBUG(100, "calcCutoff: after adding gain (of " << stretchFactor << "): lowerCutoff: " << lowerCutoff << ", upperCutoff: " << upperCutoff)

  return make_pair(lowerCutoff, upperCutoff); // normal case
}



/** checks sector-combination whether it is allowed or not (checks: isSameSensor, isUncommonCombination, isTooManyLayersAway), return value is true, if sectorCombi is valid */
bool VXDTFRawSecMap::filterBadSectorCombis(SectorPack& aSector)
{
  if (int(aSector.second.size()) == 0) {
    B2DEBUG(20, "filterBadSectorCombis: sector " << FullSecID(aSector.first) << " has no friends, validation failed")
    return false;
  }
  int layerCorrectionFactor = 0; // is 0 for PXD and VXD, but 2 for SVD only
  if (m_detectorType == "SVD") { layerCorrectionFactor = 2; }  // takes care of the case when there is no layer 2 and 1 WARNING hardcoded number of layers!
  int centerSectorID = FullSecID().getFullSecID();
  FullSecID mainSector = FullSecID(aSector.first);
  FullSecID friendSector;
  IDVector badIDs; // collecting bad friends here to be killed afterwards
  bool upgradeSecID = false; // collecting secIDs which get higher subLayerID
  int nFriends = aSector.second.size(); // number of friendsectors before doing anything
  double rarenessThreshold = 0; // if secCombi is occuring even more seldom than this threshold, it will be filtered (value will be set later)
  int nTotalValues = 0; // counts total number of occurrences of all sector-combinations of current sector
  for (FriendPack & aCombination : aSector.second) {
    for (CutoffPack & aCutoff : aCombination.second) {
      nTotalValues += aCutoff.second.size();
    }
  }

  if (m_rareSectorCombinations.second > 0) { rarenessThreshold = double(nTotalValues) / m_rareSectorCombinations.second; }
  // calculated once (even if it is not needed), since the result can be used several times

  B2DEBUG(20, "filterBadSectorCombis: sector " << mainSector << " got total number of values: " << nTotalValues << " and calculated rarenessThreshold of " << rarenessThreshold)

  for (FriendPack & aCombination : aSector.second) {
    friendSector = FullSecID(aCombination.first);

    /// now let's do some security checks;
    // first: if friendLayer > sectorLayer -> kick (WARNING if sectormap shall contain curler-compatibility, then this part should be overhauled!)
    if (mainSector.getLayerNumber() < friendSector.getLayerNumber()) {
      badIDs.push_back(aCombination.first);
      B2DEBUG(30, "filterBadSectorCombis: friend " << friendSector << " of sector " << mainSector << " got kicked because of bad layer number (sector/friend): " << mainSector.getLayerNumber() << "/" << friendSector.getLayerNumber())
      continue;
    }
    if (mainSector.getVxdID() == friendSector.getVxdID()) {
      badIDs.push_back(aCombination.first);
      B2DEBUG(30, "filterBadSectorCombis: friend " << friendSector << " of sector " << mainSector << " got kicked because of being on the same sensor (sector/friend): " << mainSector.getLayerNumber() << "/" << friendSector.getLayerNumber())
      continue;
    }

    // next: check for layerJumps
    if (m_maxLayerLevelDifference.first == true) {
      if ((friendSector.getLayerNumber() + m_maxLayerLevelDifference.second) < mainSector.getLayerNumber()) {

        // catch case of SVD only
        if ((friendSector.getFullSecID() == centerSectorID) and ((friendSector.getLayerNumber() + m_maxLayerLevelDifference.second + layerCorrectionFactor) >= mainSector.getLayerNumber())) {
          // everything is fine -> do nothing...
        } else {
          badIDs.push_back(aCombination.first);
          B2DEBUG(30, "filterBadSectorCombis: friend " << friendSector << " of sector " << mainSector << " got kicked because of bad layer number (sector/friend): " << mainSector.getLayerNumber() << "/" << friendSector.getLayerNumber() << " (layer difference of " << m_maxLayerLevelDifference.second << " allowed)")
          continue;
        }
      }
    }

    // next: check for rare sectorCombinations
    if (m_rareSectorCombinations.first == true) {
      int nFriendValues = 0;
      for (CutoffPack & aCutoff : aCombination.second) {
        nFriendValues += aCutoff.second.size();
      }
      if (rarenessThreshold > double(nFriendValues)) {
        badIDs.push_back(aCombination.first);
        B2DEBUG(30, "filterBadSectorCombis: friend " << friendSector << " of sector " << mainSector << " got kicked because of rare sector combination (nFriendValues/nTotalValues/rarenessThreshold): " << nFriendValues << "/" << nTotalValues << "/" << rarenessThreshold)
        continue;
      }
    }

    // last: if on same layer: kick if on same sensor, upgrade if not
    if (mainSector.getLayerNumber() == friendSector.getLayerNumber()) {
      if (mainSector.getVxdID() == friendSector.getVxdID()) {
        badIDs.push_back(aCombination.first);
        B2DEBUG(30, "filterBadSectorCombis: friend " << friendSector << " of sector " << mainSector << " got kicked because of bad layer number (sector/friend): " << mainSector.getLayerNumber() << "/" << friendSector.getLayerNumber() << " and bad VxdID: (sector/friend) " << mainSector.getVxdID() << "/" << friendSector.getVxdID())
        continue;
      } else {
        upgradeSecID = true; // that's why this check is done last
        B2DEBUG(30, "filterBadSectorCombis: friend " << friendSector << " of sector " << mainSector << " has marked sector for subLayerID-upgrade")
      }
    }
  }

  B2DEBUG(30, "filterBadSectorCombis: friend " << friendSector << " of sector " << mainSector << " has got " << badIDs.size() << " badIDs after safety checks")
  if (int(badIDs.size()) == 0) { return upgradeSecID; }

  searchForBadEntries(aSector.second, badIDs);        /// kicking friend having no values stored

  if ((nFriends - int(badIDs.size())) != int(aSector.second.size())) { B2FATAL("filterBadSectorCombis: deleting bad friends went wrong! (nBadfriends/nFriends@begin/nFriendsNow): " << badIDs.size() << "/" << nFriends << "/" << aSector.second.size()) }

  if (int(badIDs.size()) != 0) {
    stringstream badFriends;
    for (int iD : badIDs) {
      badFriends << iD << "/" << FullSecID(iD) << " ";
    }
    B2DEBUG(20, "filterBadSectorCombis: sector (id/name) " << aSector.first << "/" << mainSector << " had the following sectors as bad friends: " << badFriends.str())
  }

  return upgradeSecID;
}



void VXDTFRawSecMap::repairSecMap()
{
  // WARNING: destroys order of map (because of changed subLayerIDs), if sorted map is essential, insert sort at the end!

  B2DEBUG(1, "repairSecMap: before starting process: there are " << getNumOfSectors() << "/" << getNumOfFriends() << "/" << getNumOfValues() << " sectors/friends/values in map")
  vector< pair<unsigned int, unsigned int> > updatedSubLayerIDs; // .first is old ID, .second is new one
  IDVector killedSectors; // collects names of killed sectors
  unsigned int centerSectorID = FullSecID().getFullSecID();

  // first: check all sectors for security issues, collect subLayerIDs of sectors/friends to be renamed (renames sectors directly, friends later), kills empty sectors
  StrippedRawSecMap::iterator sectorIt = m_sectorMap.begin();
  while (sectorIt != m_sectorMap.end()) {
    FullSecID currentSecID = FullSecID(sectorIt->first);
    if (sectorIt->first == centerSectorID) {
      B2DEBUG(1, "repairSecMap: centerSector found. skipping it...")
      ++sectorIt;
      continue;
    }

    int nFriendsBefore = sectorIt->second.size();
    bool upgradeMe = filterBadSectorCombis((*sectorIt));                  /// filterBadSectorCombis
    int nFriendsAfter = sectorIt->second.size();
    B2DEBUG(25, "repairSecMap: at sector (id/name): " << sectorIt->first << "/" << currentSecID << ", had friends before/after: " << nFriendsBefore << "/" << nFriendsAfter)

    if (nFriendsAfter == 0) {
      killedSectors.push_back(sectorIt->first);
      B2DEBUG(15, "repairSecMap: at sector (id/name): " << sectorIt->first << "/" << currentSecID << ", got killed because of no friends")
      sectorIt = m_sectorMap.erase(sectorIt);
      continue; // no need for iter++, since it already points at new sector
    }

    if (upgradeMe == true) {
      unsigned int oldID = sectorIt->first;
      unsigned int newID = FullSecID(currentSecID.getLayerID(), true, currentSecID.getVxdID(), currentSecID.getSecID()).getFullSecID();
      updatedSubLayerIDs.push_back(make_pair(oldID, newID));
      B2DEBUG(20, "repairSecMap: sector with old name (id/name): " << oldID << "/" << currentSecID << ", got upgrade to (id/name): " << newID << "/" << FullSecID(newID))
      sectorIt->first = newID; // replacing old sectorID with new one
    }

    ++sectorIt;
  }

  // now doing second run where friendIDs are upgraded too TODO: maybe better to kick dead sectors as friends too, since they are a dead end anyway -> problem, in this case we have to redo these steps until no sector/friend has died anymore.
  for (SectorPack & aSector : m_sectorMap) {
    for (FriendPack & aFriend : aSector.second) {
      for (auto newIDPack : updatedSubLayerIDs) {
        if (newIDPack.first == aFriend.first) {
          aFriend.first = newIDPack.second; // getting updated ID
          break;
        }
      }
    }
  }

  if ((m_removeDeadSectorChains == true) and (int(killedSectors.size()) != 0)) {
    B2DEBUG(15, " removeDeadSectorChains is true and there were " << killedSectors.size() << " sectors killed, starting to clean friends too...")
    int oldNSectors = 0;
    int oldNFriends = 0;
    int newNSectors = getNumOfSectors();
    int newNFriends = getNumOfFriends();
    int nTimesRepeated = 0;
    while ((oldNSectors not_eq newNSectors) or (oldNFriends not_eq newNFriends)) {  // should restart if at least one of the conditions are fullfilled
      ++nTimesRepeated;
      int nNewSectorsKilled = 0; // number of new sectors killed
      for (SectorPack & aSector : m_sectorMap) { // kill dead friends
        if (aSector.first == centerSectorID) {
          B2DEBUG(1, "repairSecMap @ removeDeadSectorChains: centerSector found. skipping it...")
          continue;
        }
        int nFriendsBefore = aSector.second.size();
        searchForBadEntries(aSector.second, killedSectors);       /// remove all friend which are already dead sectors

        int nFriendsAfter = aSector.second.size();
        B2DEBUG(20, "repairSecMap: after killing dead friends of sector " << FullSecID(aSector.first) << " there are " << nFriendsAfter << " friends left (there were " << nFriendsBefore << " friend before)")
        if (nFriendsAfter == 0) {
          killedSectors.push_back(aSector.first);  // marked for killing later (can't be done now since all friends have to be checked again anyway) -> leads to some redundancy because of using the killedSectors-container for it, but the information of killed sectors is needed anyway and speed should not be an issue for calculating sectormaps
          ++nNewSectorsKilled;
        }
      }

      if (nNewSectorsKilled != 0) {  // kill sectors now
        searchForBadEntries(m_sectorMap, killedSectors);        /// remove all sectors which were found on blacklist
      }

      oldNSectors = newNSectors;
      oldNFriends = newNFriends;
      newNSectors = getNumOfSectors();
      newNFriends = getNumOfFriends();
      B2DEBUG(5, "repairSecMap: after killing dead sector chains " << nTimesRepeated << " times: there were " << oldNSectors << "/" << oldNFriends << " sectors/friends before and now there are " << newNSectors << "/" << newNFriends << " sectors/friends in map")
    }
  }


  B2DEBUG(1, "repairSecMap: before finishing process: there are " << getNumOfSectors() << "/" << getNumOfFriends() << "/" << getNumOfValues() << " sectors/friends/values in map")
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 20, PACKAGENAME()) == true) {
    stringstream badSectors;
    for (int iD : killedSectors) {
      badSectors << iD << "/" << FullSecID(iD) << " ";
    }
    B2DEBUG(15, "repairSecMap: secMap " << getMapName() << " had the following bad sectors (id/name): " << badSectors.str())
  }
}



void VXDTFRawSecMap::addDistances(SectorDistancesMap& aMap)
{
  if (int(aMap.size()) == 0) {
    B2DEBUG(1, "addDistances: incoming map was empty");
    return;
  }

  int oldSize = m_dist2OriginMap.size();
  if (oldSize == 0) {
//    for (SectorDistance aMapEntry: aMap) {
//      B2WARNING(" aMapEntry f/s: " << aMapEntry.first <<"/" << aMapEntry.second)
//    }
    m_dist2OriginMap = aMap;
    B2DEBUG(1, "addDistances: old map was empty, but new one is not (sectors: " << m_dist2OriginMap.size() << ") - replacing old map");
//    for (SectorDistance aMapEntry: m_dist2OriginMap) {
//      B2WARNING(" m_dist2OriginMap.Entry f/s: " << aMapEntry.first <<"/" << aMapEntry.second)
//    }
    return;
  } // easy case, fully copy the new map
  B2DEBUG(1, "addDistances: existing map had " << oldSize << " entries, importedMap has " << aMap.size() << " entries ")

  // check old map (m_dist2OriginMap) for each entry of the new one whether there is already an entry. If there is not, add new entry
  bool entryFound = false;
  for (SectorDistance aMapEntry : aMap) {
    for (SectorDistance oldMapEntry : m_dist2OriginMap) {
//       B2WARNING(" aMapEntry f/s: " << aMapEntry.first << "/" << aMapEntry.second << ", oldMapEntry f/s: " << oldMapEntry.first << "/" << oldMapEntry.second)
      if (aMapEntry == oldMapEntry) { entryFound = true; break; }
    }

    if (entryFound == false) { m_dist2OriginMap.push_back(aMapEntry); }
    entryFound = false;
  }

  B2DEBUG(1, "addDistances: " << m_dist2OriginMap.size() - oldSize << " entries were imported")
//   for (SectorDistance aMapEntry : m_dist2OriginMap) {
//     B2WARNING("after import: m_dist2OriginMap.Entry f/s: " << aMapEntry.first << "/" << aMapEntry.second)
//   }
}