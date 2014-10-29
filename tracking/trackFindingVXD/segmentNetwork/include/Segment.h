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
#include <list>



namespace Belle2 {


  /** The VXD SegmentCell class
   * This class represents segments of track candidates needed for the Cellular automaton (CA) implemented in the VXDTF.
   */
  class Segment {
  public:



    /** ********************************* constructors ********************************* **/



    /** Default constructor for the ROOT IO. */
    Segment():
      m_outerNode(std::numeric_limits<unsigned int>::max()),
      m_innerNode(std::numeric_limits<unsigned int>::max()),
      m_outerSector(std::numeric_limits<unsigned int>::max()),
      m_innerSector(std::numeric_limits<unsigned int>::max()),
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
    Segment(unsigned int outerNode, unsigned int innerNode, unsigned int outerSector, unsigned int innerSector):
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
    int getState() const { return m_state; }


    /** returns whether Cell is allowed to be a seed for TCs */
    bool isSeed() const { return m_seed; }


    /** returns activationState (CA-feature) */
    bool isActivated() const { return m_activated; }


    /** returns info whether stateIncrease is allowed or not (CA-feature) */
    bool isUpgradeAllowed() const { return m_stateUpgrade; }


    /** returns inner hit of current Cell */
    unsigned int getInnerNode() const { return m_innerNode; }


    /** returns outer hit of current Cell */
    unsigned int getOuterNode() const { return m_outerNode; }


    /** returns list of inner Neighbours (CA-feature and needed by TC-Collector), does deliver different results depending on when you call that function */
    std::list<unsigned int>* getInnerNeighbours() { return &m_innerNeighbours; }


    /** returns list of all inner neighbours (does not change during event) */
    const std::list<unsigned int>* getAllInnerNeighbours() const { return &m_allInnerNeighbours; }


    /** returns the clusterID in the collectorTFinfo-class */
    int getCollectorID() { return m_collector_id; }


    /** sets the clusterID for the collectorTFinfo-class */
    void setCollectorID(int value) { m_collector_id = value; }


    /** returns current number of inner neighbours */
    int sizeOfInnerNeighbours() const { return m_innerNeighbours.size(); }


    /** returns total number of inner neighbours (including those which already got killed)*/
    int sizeOfAllInnerNeighbours() const { return m_allInnerNeighbours.size(); }


    /** returns total number of outer neighbours (including those which already got killed)*/
    int sizeOfOuterNeighbours() const { return m_outerNeighbours.size(); }



    /** ********************************* setter ********************************* **/



    /** makes a copy of m_innerNeighbours (to be used before CA!) */
    void copyNeighbourList() { m_allInnerNeighbours = m_innerNeighbours; }


    /** increases state during CA update step */
    void increaseState() { m_state++; }


    /** sets flag whether Cell is allowed to increase state during update step within CA */
    void allowStateUpgrade(bool upgrade) { m_stateUpgrade = upgrade; }


    /** sets flag whether Cell is allowed to be the seed of a new track candidate or not */
    void setSeed(bool seedValue) { m_seed = seedValue; }


    /** sets flag whether Cell is active (takes part during current CA iteration) or inactive (does not take part, it is 'dead') */
    void setActivationState(bool activationState) { m_activated = activationState; }


    /** adds an inner neighbour-segment */
    void addInnerNeighbour(unsigned int aSegment) { m_innerNeighbours.push_back(aSegment); }


    /**< adds an outer neighbour-segment */
    void addOuterNeighbour(unsigned int aSegment) { m_outerNeighbours.push_back(aSegment); }



    /** ********************************* other functionality ********************************* **/



    /** incompatible neighbours get kicked when new information about the situation recommends that step */
    std::list<unsigned int>::iterator eraseInnerNeighbour(std::list<unsigned int>::iterator it) {
      it = m_innerNeighbours.erase(it);
      return it;
    } //items.erase(i++);  or  i = items.erase(i);


    /** checks whether the segment has got any neighbours. If not, it dies (ActivationState = false)*/
    bool dieIfNoNeighbours() {
      /** ATTENTION sep19th, 2014:
       *
       * at the moment it is not clear, which of the following if-cases shall be used.
       * the CA kills them by itself, so final output seems to be the same, but detailed measurements recommended
       * */
//    if (sizeOfInnerNeighbours()) { setActivationState(false); }
      //    if (sizeOfInnerNeighbours() == 0 and sizeOfAllInnerNeighbours() == 0) { setActivationState(false);  B2WARNING("in dieIfNoNeighbours() sizeOfInnerNeighbours() == 0 and sizeOfOuterNeighbours() == 0 ") }
      if (sizeOfInnerNeighbours() == 0 and sizeOfOuterNeighbours() == 0) { setActivationState(false); }

      return m_activated;
    }


  protected:


    /** ********************************* members ********************************* **/



    /** index of trackNode forming the outer end of the Segment. */
    unsigned int m_outerNode;


    /** index of trackNode forming the inner end of the Segment. */
    unsigned int m_innerNode;


    /** index of sector carrying outer trackNode */
    unsigned int m_outerSector;


    /** index of sector carrying inner trackNode */
    unsigned int m_innerSector;


    /** state of Cell during CA process, begins with 0 */
    int m_state;


    /** activation state. Living Cells (active) are allowed to evolve in the CA, dead ones (inactive) are not allowed */
    bool m_activated;


    /** sets flag whether Cell is allowed to increase state during update step within CA */
    bool m_stateUpgrade;


    /** sets flag whether Cell is allowed to be the seed of a new track candidate or not */
    bool m_seed;


    /** segments attached at the inner end of current segment. Since this list gets reduced during CA process, a copy is made before that step. If you want to see all neighbours, use getAllInnerNeighbours */
    std::list<unsigned int> m_innerNeighbours;


    /** carries full list of all inner neighbour-Cells. */
    std::list<unsigned int> m_allInnerNeighbours;


    /** carries list of outer neighbour-Cells */
    std::list<unsigned int> m_outerNeighbours;


    /** ID of the Cell in the Collector */
    int m_collector_id;
  };

} //Belle2 namespace
