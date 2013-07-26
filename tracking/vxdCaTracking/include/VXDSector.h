/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/// NOTE:These Classes are needed by dataobjects, but shall not be compatible to the datastore for performance reasons, therefore only implemented as .h-file (compare to VXDID)
#ifndef VXDSECTOR_H
#define VXDSECTOR_H

#include <map>
#include <algorithm>
#include <string>
#include <vector>

#include "SectorFriends.h"

namespace Belle2 {
  namespace Tracking {

    class Cutoff;
    //  class SectorFriends;
    class VXDTFHit;
    class VXDSegmentCell;

    /**VXDSector
    * defined as a pass-dependent (the VXDTF allows several reconstruction passes per event) sub-unit of sensors. Needed for the VXDTFModule. Allows to define sublayers which consider "inner" hits on the same layer.
    * knows which sectors are allowed to be combined with current one for track finding (pass dependent).
    **/
    class VXDSector { /// depends of VXTFHit, SectorFriends, Cutoff
    public:
      typedef std::map<unsigned int, SectorFriends> FriendMap; //key is Name of friedSector
      //     typedef std::map<std::string, Cutoff*> ScopeMap; // key is scope of cutoff
      //     typedef std::map<std::string, ScopeMap*> CutoffTypeMap; // key is type of cutoff

      /** minimalconstructor (produces useless sectors)*/
      VXDSector(): m_sectorID(0) { m_friends.clear(); }

      /** useful constructor */
      VXDSector(unsigned int secID): m_sectorID(secID) { m_friends.clear(); } // default values

      bool operator<(const VXDSector& b)  const { return getSecID() < b.getSecID(); } /**< overloaded '<'-operator for sorting algorithms */
      bool operator==(const VXDSector& b) const { return getSecID() == b.getSecID(); } /**< overloaded '=='-operator for sorting algorithms */
      bool operator>(const VXDSector& b)  const { return getSecID() > b.getSecID(); } /**< overloaded '>'-operator for sorting algorithms */

      /** setter - addHit adds hits to current sector */
      void addHit(VXDTFHit* newSpacePoint) { m_hits.push_back(newSpacePoint); }

      /** setter - addFriend adds friends to current sector. Friends are compatible sectors defined by the currently loaded sectorMap */
      void addFriend(int newSector);

      /** setter - addCutoff adds a pair of Min and Max-values to current sector. Values depends on sectorMap and can be identified by cutoffType and friendName */
      void addCutoff(int cutOffType, unsigned int friendName, std::pair<double, double> values);

      /** setter - addInnerSegmentCell adds a VXDSegmentCell to member vector carrying inner Cells. 'inner' means, that these Cells are connected with sectors of inner layers. */
      void addInnerSegmentCell(VXDSegmentCell* newSegment) { m_innerSegmentCells.push_back(newSegment); }

      /** setter - addOuterSegmentCell adds a VXDSegmentCell to member vector carrying inner Cells. 'outer' means, that these Cells are connected with sectors of outer layers. */
      void addOuterSegmentCell(VXDSegmentCell* newSegment) { m_outerSegmentCells.push_back(newSegment); }

      /** getter - getSecID returns the ID of the sector (for definition of secID, see m_sectorID). */
      unsigned int getSecID() const { return m_sectorID; }

      /** getter - returns vector of Hits lying at current sector */
      const std::vector<VXDTFHit*>& getHits() const { return m_hits; }

      /** getter - returns vector of addresses of friends stored in friendMap */
      const std::vector<unsigned int>& getFriends() const { return m_friends; } // TODO long term: replace by vector of iterators for that map (faster)

      /** getter - get vector of inner segmentCells */
      const std::vector<VXDSegmentCell*>& getInnerSegmentCells() const { return m_innerSegmentCells; }

      /** getter - get vector of outer segmentCells */
      const std::vector<VXDSegmentCell*>& getOuterSegmentCells() { return m_outerSegmentCells; }

      /** getter - returns vector of Filter-Cutoff-types currently stored in the sector */
      const std::vector<int> getSupportedCutoffs(unsigned int aFriend);

      /** getter - return the cutoff. To get it, you have to know which friendSector and which CutoffType you want to have */
      Cutoff* getCutoff(int cutOffType, unsigned int aFriend) {
        FriendMap::iterator mapIter = m_friendMap.find(aFriend);
        if (mapIter == m_friendMap.end()) { // not found
          return NULL;
        }
        return mapIter->second.getCutOff(cutOffType);
      }

      /** resetSector allows to delete all event-wise information (e.g. hits and segmentCells), but keeps longterm information like friends and cutoffs */
      void resetSector() { m_hits.clear(); m_innerSegmentCells.clear(); m_outerSegmentCells.clear(); } // should be called at the end of each event.

    protected:
      unsigned int m_sectorID; /**< secID allows identification of sector. Current definition ABCD, A: layerNumber(1-6), B: subLayerNumber(0,1)-defines whether sector has friends on same layer (=1) or not (=0), C:uniID, D: sectorID on sensor (0-X), whole info stored in an int, can be converted to human readable code by using FullSecID-class */

      std::vector<unsigned int> m_friends; /**< vector of addresses of compatible sectors (where neighbouring hits can be retrieved) */
      std::vector<VXDTFHit*> m_hits; /**< vector of hits lying on current sector area */
      std::vector<VXDSegmentCell*> m_innerSegmentCells; /**< vector of VXDSegmentCells connected to this sector pointing toward the IP */
      std::vector<VXDSegmentCell*> m_outerSegmentCells; /**< vector of VXDSegmentCells connected to this sector pointing away from the IP */

      FriendMap m_friendMap; /**< map of compatible sectors (where neighbouring hits can be retrieved) */
    };

    /** @}*/
  } // Tracking namespace
} //Belle2 namespace
#endif
