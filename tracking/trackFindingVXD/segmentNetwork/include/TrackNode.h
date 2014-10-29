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

#include <tracking/trackFindingVXD/segmentNetwork/ActiveSector.h>
#include <tracking/trackFindingVXD/segmentNetwork/Segment.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <tracking/dataobjects/FullSecID.h>

#include <framework/logging/Logger.h>

#include <limits>       // std::numeric_limits
#include <vector>


namespace Belle2 {


  /** The VXD Track Finder Hit class
   * This class stores all information needed for reconstructing track candidates within the VXDonlyTrackFinder.
   * Nodes are generated individually for each pass.
   * Although this generates n times the number of hits per event (where n is the number of passes executed),
   * the total information has a low degree of redundancy since each pass has got its individual segments and TCs
   * Track candidates use the same trackNode, when they share the same VXDID and at least one of the clusterIndices.
   * This definition for the comparing operators allows pass-independent comparisons.
   */
  class TrackNode {
  public:



    /** ********************************* constructors ********************************* **/



    /** Default constructor for root compatibility */
    TrackNode():
      m_spacePoint(NULL),
      m_activatedSector(NULL),
      m_FullSecID(std::numeric_limits<FullSecID::BaseType>::max()),
      m_attachedTrackCandidates(0),
      m_collector_id(-1) {}


    /** Constructor.
    *      //      * @param spIndex index number of spacePoint associated with this trackNode.
    *      //      * @param activatedSector index number of Activated Sector containing trackNode.
    *      //      * @param aFullSecID basetype of Full sector ID of activated sector containing this trackNode.
     *      //      */
    TrackNode(SpacePoint* spIndex, ActiveSector* activatedSector, FullSecID::BaseType aFullSecID):
      m_spacePoint(spIndex),
      m_activatedSector(activatedSector),
      m_FullSecID(aFullSecID),
      m_attachedTrackCandidates(0),
      m_collector_id(-1) {}



    /** ********************************* operator overload ********************************* **/



    /** overloaded '=='-operator for sorting algorithms */
    bool operator==(const TrackNode& b) const {
      B2WARNING("somebody is using the '=='-operator of TrackNode, this should not be used at the moment!");
      return (getSpacePoint() == b.getSpacePoint());
    }


    /** overloaded '<'-operator for sorting algorithms */
    bool operator<(const TrackNode& b) const {
      B2WARNING("somebody is using the '=='-operator of TrackNode, this should not be used at the moment!");
      return (getFullSecID() < b.getFullSecID());
    }


    /** overloaded '>'-operator for sorting algorithms */
    bool operator>(const TrackNode& b) const {
      B2WARNING("somebody is using the '=='-operator of TrackNode, this should not be used at the moment!");
      return (getFullSecID() > b.getFullSecID());
    }



    /** ********************************* getter ********************************* **/



    /** returns all inner Cells attached to hit */
    inline const std::vector<Segment*>& getAttachedInnerSegments() const { return m_attachedInnerSegments; }


    /** returns all outer Cells attached to hit */
    inline const std::vector<Segment*>& getAttachedOuterSegments() const { return m_attachedOuterSegments; }


    /** returns number of segments connected to this hit (hits without attached segments are ignored during TF process) */
    inline unsigned int getNumberOfSegments() const { return int(m_attachedInnerSegments.size() + m_attachedOuterSegments.size()); }


    /** returns number of segments alive connected to this hit (hits without attached segments are ignored during TF process) */
    unsigned int getNumberOfSegmentsAlive() const {
      unsigned int foundAlive = 0;
      for (auto * seg : m_attachedInnerSegments) {
        foundAlive += seg->getState() ? 1 : 0;
      }
      for (auto * seg : m_attachedOuterSegments) {
        foundAlive += seg->getState() ? 1 : 0;
      }
      return foundAlive;
    }


    /** returns number of TCs using this hit */
    inline unsigned int getNumberOfTrackCandidates() const { return m_attachedTrackCandidates; }


    /** returns VxdID of sensor carrying current sector */
    inline FullSecID::BaseType getFullSecID() const { return m_FullSecID; }


    /** returns pointer to associated spacePoint */
    inline SpacePoint* getSpacePoint() const {return m_spacePoint; }


    /** returns pointer to associated ActiveSector */
    inline ActiveSector* getActiveSector() const { return m_activatedSector; }


    /** returns the clusterID in the collectorTFinfo-class */
    inline int getCollectorID() { return m_collector_id; }



    /** ********************************* setter ********************************* **/



    /** adds new Segment to vector of inner Cells attached to current hit */
    inline void addInnerSegment(Segment* newCell) { m_attachedInnerSegments.push_back(newCell); }


    /** adds new Segment to vector of outer Cells attached to current hit */
    inline void addOuterSegment(Segment* newCell) { m_attachedOuterSegments.push_back(newCell); }


    /** counting them is enough to check their occupancy */
    inline void addTrackCandidate() { m_attachedTrackCandidates++; }


    /** decrease number of TCs using this hit */
    inline void removeTrackCandidate() {
      if (m_attachedTrackCandidates == 0) {
        B2ERROR("TrackNode::removeTrackCandidate: can not have less than 0 track candidates! Value stays at 0...")
      } else {
        m_attachedTrackCandidates--;
      }
    }


    /** sets the clusterID for the collectorTFinfo-class */
    inline void setCollectorID(int value) { m_collector_id = value; }


  protected:


    /** ********************************* members ********************************* **/



    /** stores the index of associated SpacePoint */
    SpacePoint* m_spacePoint;


    /** index of sector containing current trackNode */
    ActiveSector* m_activatedSector;


    /** FullSecID of sector containing hit - WARNING redundant information, is this really needed? */
    FullSecID::BaseType m_FullSecID;


    /** contains indices to segments using this hit as outer end  */
    std::vector<Segment*> m_attachedInnerSegments;


    /** contains indices to segments using this hit as inner end  */
    std::vector<Segment*> m_attachedOuterSegments;


    /** number of trackCandidates using this hit */
    unsigned int m_attachedTrackCandidates;


    /** ID of the hit in the Collector */
    int m_collector_id;
  };
} //Belle2 namespace
