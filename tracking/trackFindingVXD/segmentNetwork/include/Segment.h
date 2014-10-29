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

#include <framework/logging/Logger.h>

// C++-std:
#include <limits>       // std::numeric_limits
#include <vector>



namespace Belle2 {

  /** predeclaration, real class can be found in tracking/trackFindingVXD/segmentNetwork/TrackNode.h */
  class TrackNode;


  /** predeclaration, real class can be found in tracking/trackFindingVXD/segmentNetwork/ActiveSector.h */
  class ActiveSector;


  /** The VXD SegmentCell class
   * This class represents segments of track candidates needed for the Cellular automaton (CA) implemented in the VXDTF.
   */
  class Segment {
  public:



    /** ********************************* constructors ********************************* **/



    /** Default constructor for the ROOT IO. */
    Segment():
      m_outerNode(NULL),
      m_innerNode(NULL),
      m_outerSector(NULL),
      m_innerSector(NULL),
      m_state(0),
      m_activated(true),
      m_stateUpgrade(false),
      m_seed(true),
      m_collector_id(-1) {}



    /** Constructor.
     *      //      * @param outerNode index number of outer TrackNode associated with this segment.
     *      //      * @param innerNode index number of inner TrackNode associated with this segment.
     *      //      * @param outerSector index number of outer ActiveSector associated with this segment.
     *      //      * @param innerSector index number of inner ActiveSector associated with this segment.
      *      //      */
    Segment(TrackNode* outerNode, TrackNode* innerNode, ActiveSector* outerSector, ActiveSector* innerSector):
      m_outerNode(outerNode),
      m_innerNode(innerNode),
      m_outerSector(outerSector),
      m_innerSector(innerSector),
      m_state(0),
      m_activated(true),
      m_stateUpgrade(false),
      m_seed(true),
      m_collector_id(-1) {}



    /** ********************************* getter ********************************* **/



    /** returns state of Cell (CA-feature) */
    inline int getState() const { return m_state; }


    /** returns whether Cell is allowed to be a seed for TCs */
    inline bool isSeed() const { return m_seed; }


    /** returns activationState (CA-feature) */
    inline bool isActivated() const { return m_activated; }


    /** returns info whether stateIncrease is allowed or not (CA-feature) */
    inline bool isUpgradeAllowed() const { return m_stateUpgrade; }


    /** returns inner hit of current Cell */
    inline const TrackNode* getInnerNode() const { return m_innerNode; }


    /** returns outer hit of current Cell */
    inline const TrackNode* getOuterNode() const { return m_outerNode; }


    /** returns list of inner Neighbours (CA-feature and needed by TC-Collector), does deliver different results depending on when you call that function */
    inline std::vector<Segment*>* getInnerNeighbours() { return &m_innerNeighbours; }


    /** returns the clusterID in the collectorTFinfo-class */
    inline int getCollectorID() { return m_collector_id; }


    /** sets the clusterID for the collectorTFinfo-class */
    inline void setCollectorID(int value) { m_collector_id = value; }


    /** returns number of inner neighbours which are currently alive */
    unsigned int sizeOfInnerNeighbours() const {
      unsigned int nbsAlive = 0;
      for (auto * nb : m_innerNeighbours) {
        nbsAlive += nb->getState() ? 1 : 0;
      }
      return nbsAlive;
    }


    /** returns total number of inner neighbours (including those which already got killed)*/
    inline unsigned int sizeOfAllInnerNeighbours() const { return m_innerNeighbours.size(); }


    /** returns number of outer neighbours which are currently alive */
    unsigned int sizeOfOuterNeighbours() const {
      unsigned int nbsAlive = 0;
      for (auto * nb : m_outerNeighbours) {
        nbsAlive += nb->getState() ? 1 : 0;
      }
      return nbsAlive;
    }


    /** returns total number of outer neighbours (including those which already got killed)*/
    inline unsigned int sizeOfAllOuterNeighbours() const { return m_outerNeighbours.size(); }



    /** ********************************* setter ********************************* **/



    /** increases state during CA update step */
    inline void increaseState() { m_state++; }


    /** sets flag whether Cell is allowed to increase state during update step within CA */
    inline void allowStateUpgrade(bool upgrade) { m_stateUpgrade = upgrade; }


    /** sets flag whether Cell is allowed to be the seed of a new track candidate or not */
    inline void setSeed(bool seedValue) { m_seed = seedValue; }


    /** sets flag whether Cell is active (takes part during current CA iteration) or inactive (does not take part, it is 'dead') */
    inline void setActivationState(bool activationState) { m_activated = activationState; }


    /** adds an inner neighbour-segment */
    inline void addInnerNeighbour(Segment* aSegment) { m_innerNeighbours.push_back(aSegment); }


    /**< adds an outer neighbour-segment */
    inline void addOuterNeighbour(Segment* aSegment) { m_outerNeighbours.push_back(aSegment); }



    /** ********************************* other functionality ********************************* **/



    /** checks whether the segment has got any neighbours. If not, it dies (ActivationState = false)*/
    bool dieIfNoNeighbours() {
      if (sizeOfInnerNeighbours() == 0 and sizeOfOuterNeighbours() == 0) { setActivationState(false); }

      return m_activated;
    }


  protected:


    /** ********************************* members ********************************* **/



    /** index of trackNode forming the outer end of the Segment. */
    TrackNode* m_outerNode;


    /** index of trackNode forming the inner end of the Segment. */
    TrackNode* m_innerNode;


    /** index of sector carrying outer trackNode */
    ActiveSector* m_outerSector;


    /** index of sector carrying inner trackNode */
    ActiveSector* m_innerSector;


    /** state of Cell during CA process, begins with 0 */
    int m_state;


    /** activation state. Living Cells (active) are allowed to evolve in the CA, dead ones (inactive) are not allowed */
    bool m_activated;


    /** sets flag whether Cell is allowed to increase state during update step within CA */
    bool m_stateUpgrade;


    /** sets flag whether Cell is allowed to be the seed of a new track candidate or not */
    bool m_seed;


    /** segments attached at the inner end of current segment. Segments are not to be deleted (just deactivated)! */
    std::vector<Segment*> m_innerNeighbours;


    /** carries list of outer neighbour-Cells */
    std::vector<Segment*> m_outerNeighbours;


    /** ID of the Cell in the Collector */
    int m_collector_id;
  };

} //Belle2 namespace
