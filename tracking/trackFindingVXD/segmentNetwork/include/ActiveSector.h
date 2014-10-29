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

#include <vector>

namespace Belle2 {


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
      m_index(std::numeric_limits<unsigned int>::max()),
      m_FullSecID(std::numeric_limits<unsigned int>::max()) {}


    /** Constructor.
     *      //      * @param activeSectorIndex index number of Activated Sector associated with this one.
     *      //      * @param FullSecID basetype of Full sector ID of activated sector associated with this one.
     *      //      */
    ActiveSector(unsigned int  activeSectorIndex, FullSecID::BaseType secID):
      m_index(activeSectorIndex),
      m_FullSecID(secID) {}



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
    const std::vector<unsigned int>& getTrackNodes() const { return m_nodes; }


    /** returns all indices of attached ActiveSectors (e.g. "inner friends") */
    const std::vector<unsigned int>& getActiveFriends() const { return m_activatedFriends; }


    /** returns index number of associated ActiveSector */
    unsigned int getActiveSectorIndex() const { return m_index; }


    /** returns VxdID of sensor carrying current sector */
    FullSecID::BaseType getFullSecID() const { return m_FullSecID; }



    /** ********************************* setter ********************************* **/



    /** adds new Segment to vector of inner Cells attached to current hit */
    void addTrackNode(unsigned int newNode) { m_nodes.push_back(newNode); }


    /** adds index number of ActiveSector which is a Friend of this one*/
    void addActiveFriend(unsigned int newSector) { m_activatedFriends.push_back(newSector); }


  protected:


    /** ********************************* members ********************************* **/



    /** FullSecID of sector containing hit - WARNING redundant information, is this really needed? */
    FullSecID::BaseType m_FullSecID;


    /** index number of associated static sector */
    unsigned int m_index;


    /** stores indices of all associated TrackNodes */
    std::vector<unsigned int> m_nodes;


    /** stores all active Friend sectors */
    std::vector< unsigned int> m_activatedFriends;
  };

} //Belle2 namespace