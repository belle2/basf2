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


# include <tracking/trackFindingVXD/segmentNetwork/ActiveSector.h>
# include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>
# include <tracking/trackFindingVXD/segmentNetwork/Segment.h>

#include <framework/core/FrameworkExceptions.h>

#include <vector> // TODO test that one
#include <utility>
#include <string>

// #ifndef __CINT__
// #include <unordered_set> // TODO test that one
// #endif

#include <framework/logging/Logger.h>


/** - under construction notes - *
 *
 * It is not yet clear, whether the SegmentNetwork is created at the end of its creating module
 * (which means that it shall only held quasi static information).
 * The alternative would be, that it will be used during the whole construction process for the network.
 * In this case some extra features have to be implemented for that.
 * These features are described below under section "other functionality" and are marked with TODO.
 *
 * Unordered_set and/or SortableVector should be used if this class shall not only held static information.
 * (SortableVector has to be copied first and needs some adaption since currently it is not possible to use custom sorting functions).
 * Update Oct 29th, 2014: since we are using pointers and new, the only relevant containers left are vector and set/map,
 *
 * TODO: find a way to freeze the network-structure after building it (to prevent breaking of pointers), if necessary
 * */
namespace Belle2 {


  /** The SegmentNetwork Class.
   *
   * It contains a Network with nodes of activated sectors "ActiveSector" which are linked to their inner friends.
   * In that network of activated sectors, another network is formed by "TrackNode"s lying on activated sectors.
   * This TrackNodes are in a geometrical view SpacePoints lying on sensors of the VXD
   * (a SpacePoint is a single (PXD) or a combination (SVD) of Clusters in an event).
   * TrackNodes are connected by "Segment"s, if certain geometrical conditions are fullfilled.
   * Alltogether they form a "SegmentNetwork" describing their relations.
   *
   */
  class SegmentNetwork {
  public:


    /** ********************************* exceptions ********************************* **/



    /** this is an exception indicating that the sectorMap and its related segment network do not contain the given sectorID */
    BELLE2_DEFINE_EXCEPTION(SectorNotFound, "the given sectorID/index number does not match any existing sector in this network!");



    /** ********************************* typedefs ********************************* **/



    /** .first is the name of the related StoreArray of associated Elements, .second is the vector of the local version of that stuff */
    template<typename T> using StoreVector = std::pair<std::string, std::vector<T*>>;


#ifndef __CINT__
    /** .first is the name of the related StoreArray of associated Elements, .second is the vector of the local version of that stuff */
    template<typename T> using StoreSet = std::pair<std::string, std::unordered_set<T*>>;
#endif



    /** ********************************* constructors/destructor ********************************* **/



    /** Constructor.
     *      //      * @param nameSectors name of associated StoreArray for Sectors.
     *       //      * @param nameSpacePoints name of associated StoreArray for SpacePoints.
     * awaits the names of the associated storeArrays of Sectors and SpacePoints. */
    SegmentNetwork(std::string nameSectors, std::string nameSpacePoints) :
      m_activatedSectors.first(nameSectors),
      m_nodes.first(nameSpacePoints),
      m_segments.first(nameSpacePoints) {}


    /** standard Constructor for Root-IO. */
    SegmentNetwork() {}


    /** destructor does the cleanup */
    ~SegmentNetwork() {
      for (auto * aSector : m_activatedSectors.second) { delete aSector; }

      for (auto * aNode : m_nodes.second) { delete aNode; }

      for (auto * aSegment : m_segments.second) { delete aSegment; }
    }



    /** ********************************* getter ********************************* **/



    /** ********************************* setter ********************************* **/



    /** adding single activeSector to the network */
    void addSector(unsigned int activeSectorIndex, FullSecID::BaseType secID) { m_activatedSectors.second.push_back(new ActiveSector(activeSectorIndex, secID)); }


    /** adding single trackNode to the network */
    void addTrackNode(SpacePoint* spIndex, ActiveSector* activatedSector, FullSecID::BaseType aFullSecID) {
      TrackNode* newNode = new TrackNode(spIndex, activatedSector, aFullSecID);
      m_nodes.second.push_back(newNode);
      activatedSector->addTrackNode(newNode);
    }


    /** adding single segment to the network.
     *
     * the outerNode is the index number of the outer TrackNode,
     * the innerNode is the index number of the inner TrackNode,
     * the outer- and innerSector-Entries carry the indices for the associated sectors, respectively
     * It therefore should only be filled, if nodes already exist
     */
    void addSegment(TrackNode* outerNode, TrackNode* innerNode, ActiveSector* outerSector, ActiveSector* innerSector) {
      Segment* newSeg = new Segment(outerNode, innerNode, outerSector, innerSector);
      m_segments.second.push_back(newSeg);
      outerNode->addInnerSegment(newSeg);
      innerNode->addOuterSegment(newSeg);
    }



    /** ********************************* other functionality ********************************* **/



    /** connecting sector of given secID with friend sector of given friendSecID */
    void connectSectors(FullSecID::BaseType secID, FullSecID::BaseType friendSecID) {
      // indices to be found for main and friend sector
      unsigned int mainIndex = std::numeric_limits<unsigned int>::max();
      unsigned int friendIndex = std::numeric_limits<unsigned int>::max();

      // check whether friend exists:
      try {
        friendIndex = findActiveSector(friendSecID);
      } catch (SectorNotFound& anException) {
        B2WARNING("SegmentNetwork::connectSectors exception thrown for friend secID " << FullSecID(friendSecID) << ", " << anException.what());
        return;
      }

      // check whether main sector exists:
      try {
        mainIndex = findActiveSector(secID);
      } catch (SectorNotFound& anException) {
        B2WARNING("SegmentNetwork::connectSectors exception thrown for main secID " << FullSecID(secID) << ", " << anException.what());
        return;
      }

      m_activatedSectors.second[mainIndex]->addActiveFriend(m_activatedSectors.second[friendIndex]);
    }


    /** connecting sectors directly */
    void connectSectors(ActiveSector* mainSector, ActiveSector* friendSector) {
      mainSector->addActiveFriend(friendSector);
    }

    /** accepts a sectorID and searches the activated sectors for given ID.
     *
     * return value is found index number if sector has been found,
     * return value is std::numeric_limits<unsigned int>::max(), if no sector was found
     * */
    unsigned int findActiveSector(FullSecID::BaseType iD) {

      for (unsigned int index = 0; index < m_activatedSectors.second.size(); index++) {
        if (m_activatedSectors.second[index]->getFullSecID() == iD) {
          return index;
        }
      }

      throw SectorNotFound();
//    return index;
    }


    /** safely remove an ActiveSector from the network.
     *
     * This also affects other elements of the network!
     * Segments, TrackNodes can be deactivated during the process if removed ActiveSector affects them.
     * */
    void deactivateSector(FullSecID::BaseType iD) {
      ActiveSector* dyingSector = m_activatedSectors(findActiveSector(iD));
      dyingSector->deactivateSector();
      /* TODO */
    }


    /** safely remove a tracknode from the network.
     *
     * This also affects other elements of the network.
     * Segments, ActiveSectors can be deactivated during the process if removed TrackNode affects them.
     * */
    void deactivateTrackNode() { /* TODO */ }


    /** safely remove a tracknode from the network.
     *
     * This also affects other elements of the network.
     * Segments, ActiveSectors can be deactivated during the process if removed Segment affects them.
     * */
    void deactivateSegment() { /* TODO */ }


    /** sorts Segments by their geometrical position in the network.
     *
     * this position is encoded in the FullSecID
    * -> this does _not_ sort the segments but only a bunch of pointers to them!
     */
    void sortSegments() { /* TODO */ }


    /** sorts ActiveSectors by their geometrical position in the network.
     *
     * this position is encoded in the FullSecID
    * -> this does _not_ sort the activeSectors but only a bunch of pointers to them!
     * */
    void sortActiveSectors() { /* TODO */ }



  protected:


    /** ********************************* members ********************************* **/



    /** stores the name of the associated storeArray of static sectors and all activated sectors connected to them in this network */
    StoreVector<ActiveSector> m_activatedSectors;


    /** stores the name of the associated storeArray of SpacePoints and all associated TrackNodes connected to them in this network */
    StoreVector<TrackNode> m_nodes;


    /** stores the name of the associated storeArray of SpacePoints and all Segments created for this network */
    StoreVector<Segment> m_segments;
  };

} //Belle2 namespace
