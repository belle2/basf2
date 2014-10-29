/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/FullSecID.h>

// C++-std:
#include <limits>       // std::numeric_limits
#include <vector>

namespace Belle2 {

  /** predeclaration, real class can be found in tracking/trackFindingVXD/segmentNetwork/TrackNode.h */
  class TrackNode;


  /** The ActiveSector Class.
   *
   * associated with static sector to be able to access filter cutoffs.
   * It contains a vector of trackNodes associated with it and allows to determine the related ActiveSectors.
   *
   */
  class ActiveSector {
  public:



    /** ********************************* constructors ********************************* **/



    /** Default constructor for root compatibility */
    ActiveSector():
      m_FullSecID(std::numeric_limits<unsigned int>::max()),
      m_index(std::numeric_limits<unsigned int>::max()) {}


    /** Constructor.
     *      //      * @param activeSectorIndex index number of Activated Sector associated with this one.
     *      //      * @param FullSecID basetype of Full sector ID of activated sector associated with this one.
     *      //      */
    ActiveSector(unsigned int  activeSectorIndex, FullSecID::BaseType secID):
      m_FullSecID(secID),
      m_index(activeSectorIndex) {}



    /** ********************************* operator overload ********************************* **/



    /** overloaded '=='-operator for sorting algorithms */
    bool operator==(const ActiveSector& b) const {
      return (getFullSecID() == b.getFullSecID());
    }


    /** overloaded '<'-operator for sorting algorithms */
    bool operator<(const ActiveSector& b) const {
      return (getFullSecID() < b.getFullSecID());
    }


    /** overloaded '>'-operator for sorting algorithms */
    bool operator>(const ActiveSector& b) const {
      return (getFullSecID() > b.getFullSecID());
    }



    /** ********************************* getter ********************************* **/



    /** returns all indices of attached TrackNodes */
    inline const std::vector<TrackNode*>& getTrackNodes() const { return m_nodes; }


    /** returns all indices of attached ActiveSectors (e.g. "inner friends") */
    inline const std::vector<ActiveSector*>& getActiveFriends() const { return m_activatedFriends; }


    /** returns index number of associated ActiveSector in StoreArray TODO replace by pointer! */
    inline unsigned int getActiveSectorIndex() const { return m_index; }


    /** returns VxdID of sensor carrying current sector */
    inline FullSecID::BaseType getFullSecID() const { return m_FullSecID; }



    /** ********************************* setter ********************************* **/



    /** adds new Segment to vector of inner Cells attached to current hit */
    inline void addTrackNode(TrackNode* newNode) { m_nodes.push_back(newNode); }


    /** adds index number of ActiveSector which is a Friend of this one*/
    inline void addActiveFriend(ActiveSector* newSector) { m_activatedFriends.push_back(newSector); }


    /** deactivate this sector all connected Nodes */
    void deactivateSector() { /* TODO */ }

  protected:


    /** ********************************* members ********************************* **/



    /** FullSecID of sector containing hit - WARNING redundant information, is this really needed? */
    FullSecID::BaseType m_FullSecID;


    /** index number of associated static sector TODO replace by Pointer to real sector after design of SectorMap */
    unsigned int m_index;


    /** stores indices of all associated TrackNodes */
    std::vector<TrackNode*> m_nodes;


    /** stores all active Friend sectors */
    std::vector<ActiveSector*> m_activatedFriends;
  };

} //Belle2 namespace