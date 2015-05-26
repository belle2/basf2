/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// in fw:
#include <framework/logging/Logger.h>

// stl:
#include <vector>
#include <algorithm>    // std::find



namespace Belle2 {

  /** the node-class.
   *
   * carries an instance of a class which shall be woven into a network.
   *
   * prerequesites for entryClass:
   * - has to have an == operator defined
   */
  template<class EntryType>
  class DirectedNode {

    /** only the node network can create DirectedNodes and link them */
    friend class DirectedNodeNetwork;


  protected:
    /** ************************* DATA MEMBERS ************************* */

    /** entry can be of any type, DirectedNode is just the carrier */
    EntryType& m_entry;


    /** carries all links to inner nodes */
    std::vector<DirectedNode<EntryType>*> m_innerNodes;


    /** carries all links to outer nodes */
    std::vector<DirectedNode<EntryType>*> m_outerNodes;


    /** is the index position of this node in the network */
    unsigned int m_index;


    /** ************************* CONSTRUCTORS ************************* */

    /** protected constructor. accepts an entry which can not be changed any more */
    DirectedNode(EntryType& entry, unsigned int index) : m_entry(entry), m_index(index) {}


    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */

    /** adds new links to the inward direction */
    void addInnerNode(DirectedNode<EntryType>* newNode) { m_innerNodes.push_back(newNode); }


    /** adds new links to the outward direction */
    void addOuterNode(DirectedNode<EntryType>* newNode) { m_outerNodes.push_back(newNode); }


    /** returns the index position of this node in the network */
    unsigned int getIndex() const { return m_index; }


  public:

    /** ************************* OPERATORS ************************* */

    /** == -operator - compares if two nodes are identical */
    inline bool operator == (const DirectedNode& b) const { return (this->getEntry() == b.getEntry()); }


    /** != -operator - compares if two nodes are not identical */
    inline bool operator != (const DirectedNode& b) const { return !(this->getEntry() == b.getEntry()); }


    /** == -operator - compares if the entry passed is identical with the one linked in this node */
    inline bool operator == (const EntryType& b) const { return (this->getEntry() == b); }


    /** == -operator - compares if the entry passed is not identical with the one linked in this node */
    inline bool operator != (const EntryType& b) const { return !(this->getEntry() == b); }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */

    /** returns links to all inner nodes attached to this one */
    std::vector<DirectedNode<EntryType>*>& getInnerNodes() { return m_innerNodes; }


    /** returns links to all outer nodes attached to this one */
    std::vector<DirectedNode<EntryType>*>& getOuterNodes() { return m_outerNodes; }


    /** allows access to stored entry */
    EntryType& getEntry() { return m_entry; }


    /** allows access to stored entry */
    EntryType* getEntry() { return &m_entry; }
  };



  /** A network container where the nodes can carry any entryType.
   *
   * Features:
   * - All nodes store the same entryType and links can not carry any extra information.
   * - nodes can not be deactivated afterwards and links not deleted.
   * - can be walked through in a directed way (each node knows its inner and outer partners)
   * - inner and outer end-points of the networks are always automatically updated when filling the network
   * - one can not add single nodes, one always has to add pairs of nodes (this defines the link between them and which of them is the outer one).
   */
  template<class EntryType>
  class DirectedNodeNetwork {
  protected:

    /** ************************* DATA MEMBERS ************************* */

    /** carries all nodes */
    std::vector<DirectedNode<EntryType> > m_nodes;


    /** keeps track of current outerEnds (nodes which have no outerNodes) - entries are the index numbers of the nodes which currently form an outermost node */
    std::vector<unsigned int> m_outerEnds;


    /** keeps track of current innerEnds (nodes which have no innerNodes) - entries are the index numbers of the nodes which currently form an innermost node */
    std::vector<unsigned int> m_innerEnds;


    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */

    /** internal function for adding Nodes */
    DirectedNode<EntryType>& addNode(EntryType& entry)
    {
      unsigned int index = m_nodes.size();
      m_nodes.push_back(DirectedNode<EntryType>(entry, index));
      return m_nodes[index];
    }

    /** checks whether given entry is already in the vector.
     * returns iterator of entry if found, returns givenVector.end() if not
     */
    template<class T>
    std::vector<T>::iterator isInVector(const T& entry, const std::vector<T>& givenVector) const
    {
      return std::find(givenVector.begin(), givenVector.end(), entry);
    }

    /** links nodes with each other. returns true if everything went well, returns false, if not  */
    void linkNodes(DirectedNode<EntryType>& outerNode, DirectedNode<EntryType>& innerNode)
    {
      auto outerPos = std::find(outerNode.getInnerNodes().begin(), outerNode.getInnerNodes().end(), &innerNode);
      auto innerPos = std::find(innerNode.getOuterNodes().begin(), innerNode.getOuterNodes().end(), &outerNode);

      if (outerPos == outerNode.getInnerNodes().end() or innerPos == innerNode.getOuterNodes().end()) { return false; }

      outerNode.addInnerNode(&innerNode);
      innerNode.addOuterNode(&outerNode);
      return true;
    }

    /** adds newNode as a new end to the endVector and replaces old one if necessary */
    bool updateNetworkEnd(DirectedNode<EntryType>& oldNode, DirectedNode<EntryType>& newNode, std::vector<unsigned int> endVector)
    {
      auto iter = isInVector(oldNode.getIndex(), endVector);
      if (iter != endVector.end()) {
        *iter = newNode.getIndex();
        return true;
      }
      endVector.push_back(newNode.getIndex());
      return false;
    }
  public:

    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */

    /** takes two entries and weaves them into the network */
    void linkTheseEntries(EntryType& outerEntry, EntryType& innerEntry)
    {
      // check if entries are already in network.
      auto outerNodeIter = isInVector(outerEntry, m_nodes);
      auto innerNodeIter = isInVector(outerEntry, m_nodes);

      /* case: none of the entries are added yet:
       *  create nodes for both and link with each other, where outerEntry will be carried by outer node and inner entry by inner node
       *  outerNode will be added to outerEnds, and innerNode will be added to innerEnds.
       * */
      if (outerNodeIter == m_nodes.end() and innerNodeIter == m_nodes.end()) {
        DirectedNode<EntryType>& newOuterNode = addNode(outerEntry);
        DirectedNode<EntryType>& newInnerNode = addNode(innerEntry);

        linkNodes(newOuterNode, newInnerNode);

        m_outerEnds.push_back(newOuterNode.getIndex());
        m_innerEnds.push_back(newInnerNode.getIndex());
        return;
      }

      /* case: the outerEntry was already in the network, but not innerEntry:
       *  add new node(innerEntry) to network
       *  add innerNode to existing outerNode
       *  add innerNode to innerEnds, if outerNode was in innerEnds before, replace old one with new innerNode
       * */
      if (outerNodeIter != m_nodes.end() and innerNodeIter == m_nodes.end()) {
        DirectedNode<EntryType>& outerNode = *outerNodeIter;
        DirectedNode<EntryType>& newInnerNode = addNode(innerEntry);

        linkNodes(outerNode, newInnerNode);

        bool outerNodeReplaced = updateNetworkEnd(outerNode, newInnerNode, m_innerEnds);

        B2DEBUG(100, "DirectedNodeNetwork::linkTheseEntries(): outerNode was already there with index " << outerNode.getIndex() <<
                ", innerNode is new with index: " << newInnerNode.getIndex() <<
                ", outerNode was innerNode before: " << outerNodeReplaced ? "true" : "false")
        return;
      }

      /* case: outerNode was not there yet, but innerEntry was:
       *  add new node(outerEntry) to network
       *  add outerNode to existing innerNode
       *  add outerNode to outerEnds, if innerNode was in outerEnds before, replace old one with new outerNode
       * */
      if (outerNodeIter == m_nodes.end() and innerNodeIter != m_nodes.end()) {
        DirectedNode<EntryType>& newOuterNode = addNode(outerEntry);
        DirectedNode<EntryType>& innerNode = *innerNodeIter;

        linkNodes(newOuterNode, innerNode);

        bool innerNodeReplaced = updateNetworkEnd(innerNode, newOuterNode, m_outerEnds);

        B2DEBUG(100, "DirectedNodeNetwork::linkTheseEntries(): outerNode is new with index " << newOuterNode.getIndex() <<
                ", innerNode was already there with index: " << innerNode.getIndex() <<
                ", outerNode was innerNode before: " << innerNodeReplaced ? "true" : "false")
        return;
      }

      // case: both are already in the network ...
      DirectedNode<EntryType>& outerNode = *outerNodeIter;
      DirectedNode<EntryType>& innerNode = *innerNodeIter;

      bool wasSuccessful = linkNodes(outerNode, innerNode);

      /* ... but were not linked to each other yet:
       *  add innerNode to existing outerNode
       *  add outerNode to existing innerNode
       *  if innerNode was in outerEnds before, replace old one with new outerNode
       *  if outerNode was in innerEnds before, replace old one with new innerNode
       * */
      if (wasSuccessful) {
        bool outerNodeReplaced = updateNetworkEnd(outerNode, newInnerNode, m_innerEnds);
        bool innerNodeReplaced = updateNetworkEnd(innerNode, newOuterNode, m_outerEnds);

        B2DEBUG(100, "DirectedNodeNetwork::linkTheseEntries(): outerNode already there with index " << newOuterNode.getIndex() <<
                ", innerNode was already there with index: " << innerNode.getIndex() <<
                ", outerNode was innerNode before: " << innerNodeReplaced ? "true" : "false" <<
                ", innerNode was outerNode before: " << outerNodeReplaced ? "true" : "false")
        return;
      }

      // case: both are already there and already linked:
      B2WARNING("DirectedNodeNetwork::linkTheseEntries(): outerEntry and innerEntry were already in the network and were already connected. This is a sign for unintended behavior!")
    }


    /** returns all nodes which have no outer nodes (but inner ones) and therefore are outer ends of the network */
    std::vector<DirectedNode<EntryType>*> getOuterEnds()
    {
      std::vector<DirectedNode<EntryType>*> outerEnds;
      for (unsigned int index : m_outerEnds) {
        outerEnds.push_back(&m_nodes[index]);
      }
      return outerEnds;
    }


    /** returns all nodes which have no inner nodes (but outer ones) and therefore are inner ends of the network */
    std::vector<DirectedNode<EntryType>*> getInnerEnds()
    {
      std::vector<DirectedNode<EntryType>*> innerEnds;
      for (unsigned int index : m_innerEnds) {
        innerEnds.push_back(&m_nodes[index]);
      }
      return innerEnds;
    }


    /** returns all nodes of the network */
    std::vector<DirectedNode<EntryType> >& getNodes() { return m_nodes; }
  };
}



//////////////////////////
//SegmentNetworkProducer:
//////////////////////////

/** Input:
 * SpacePoints, SectorMap
 * */

/** Internal classes used
 *
 * ActiveSector, stores link to sector in secMap and will be the interface to everything which is secMap-related
 * TrackNode, stores link to SpacePoint and to the connected ActiveSector
 * Segment, stores linkt to the trackNodes used and has extra data members: bool isAlive, unsigned int state, bool isSeed
 * DirectedNodeNetwork (defined in tracking/trackFindingVXD/segmentNetwork/include/DirectedNodeNetwork.h)
 * */

/** Hauptschritte/Funktionen:
 * The SegmentNetworkProducer has the following main steps:
 *
 * - matchSpacePointToSectors (for each SpacePoint given, find according sector and store them in a fast and intermediate way
 * - buildActiveSectorNetwork (build a Network, where all ActiveSectors are stored which have SpacePoints _and_ compatible inner- or outer neighbours)
 * - segFinder/buildTrackNodeNetwork ( use SpacePoints stored in ActiveSectors to build trackNodes which will stored and linked in a trackNodeNetwork)
 * - nbFinder/buildSegmentNetwork (use connected TrackNodes to form segments which will stored and linked in a segmentNetwork)
 * */

/** Output:
 * A StoreArray-container (StoreObjPtr), which contains the networks mentioned above
 * */
/*
//////////////////////////////////////////////////////////////////////////
/// Section: matchSpacePointToSectors:
// returns:
std::vector< std::pair < secID, std::vector<SpacePoint*> > collectedData;

// match all SpacePoints with the sectors:
find Sectors for each SpacePoint;
  if sector for SpacePoint exists:
  if secID not in collectedData:
    collectedData.push_back( { secID , { SpacePoint*} } );
  else:
    collectedData[positionOfSecID].second.push_back(SpacePoint*);
  else:
  WARNING, no sector found, SpacePoint discarded!

return collectedData;
//////////////////////////////////////////////////////////////////////////
/// now we have got all secIDs, which are relevant for this secMap and event and for each of them, their SpacePoints are collected.


//////////////////////////////////////////////////////////////////////////
/// Section: buildActiveSectorNetwork: -> building our first network:
// returns:
Belle2::DirectedNodeNetwork<ActiveSector> activeSectorNetwork;

for outerSecID in collectedData:
  new outerSector = ActiveSector(outerSecID, &secMap[outerSecID]);
  outerSector.addHits(collectedData[outerSecID].second); // add all SpacePoint of this sector to ActiveSector
  innerSecIDs = outerSector.getInnerSecIDs()

  for innerSecID in innerSecIDs:
  if innerSecID found in collectedData:
    new innerSector = ActiveSector(innerSecID, link zu secMap[innerSecID]);
    innerSector.addHits(collectedData[innerSecID].second); // add all SpacePoint of this sector to ActiveSector

    activeSectorNetwork.linkTheseEntries(outerSector, innerSector);
  else:
    discard innerSecID

return activeSectorNetwork;
//////////////////////////////////////////////////////////////////////////
/// now we have got a network, with all ActiveSectors which actually had any compatible ActiveSectors
/// this network has no ActiveSectors, if the sector had no compatible sectors with SpacePoints on it



//////////////////////////////////////////////////////////////////////////
/// Section: segFinder: -> building our second network:
// returns:
Belle2::DirectedNodeNetwork<TrackNode> trackNodeNetwork;

/// there are two ways to build this one :
// - directed ( sectors = activeSectorNetwork.getOuterEnds()
// - undirected ( sectors = activeSectorNetwtake
// undirected is easier and therefore used in the sketch down below

for outerSector in sectors:
  if outerSector == innerEnd: continue; // go to next sector

  outerHits = outerSector.getHits();
  innerSectors = outerSector.getInnerNodes();

  for innerSector in innerSectors:
  innerHits == innerSector.getHits();

  for outerHit, innerHit in outerHits, innerHits;
    bool isAccepted = secMap[outerSecID, innerSecID].accept(outerHit, innerHit); // wenn kein Filter aktiviert, ist das immer true, Details hängen von Eugenios Implementierung ab

    if isAccepted:
    trackNodeNetwork.linkTheseEntries(outerHit, innerHit);

return trackNodeNetwork;
//////////////////////////////////////////////////////////////////////////
*/
/// now we got a network, where TrackNodes carry a SpacePoint each and the links between the nodes are implicitly the segments.
/// therefore the segments are there, but not visible (no extra Class for them), since their only job at the moment is to link TrackNodes.
/// that's why the old name segFinder (although the same filters (translated into new design) are used) is maybe a bit misleading. Proposal for the new one: buildTrackNodeNetwork


//////////////////////////////////////////////////////////////////////////
/// Section: nbFinder: -> building our third network:
/// TODO optimize: the way this pseudo code is written, there is a high probability that Segments are recreated (and detected by the network) several times.
/// since the network can handle them, it is not utterly wrong (because the results are fine and no clones will be stored) but it definitely costs some time.
// returns:
// Belle2::DirectedNodeNetwork<Segment> segmentNetwork;
//
// for outerHit in trackNodeNetwork.getNodes():
//   if outerHit == innerEnd: continue; // go to next hit
//
//   centerHits = outerHit.getInnerNodes();
//
//   for centerHit in centerHits:
//  if centerHit == innerEnd: continue; // go to next hit
//
//  new outerSegment = Segment((outerSecID, centerSecID, link zu outerHit und centerHit); // now the segments are actually build. building them earlier would be less efficient
//
//  innerHits = centerHit.getInnerNodes();
//
//  for innerHit in innerHits:
//
//    new innerSegment = Segment((centerSecID, innerSecID, link zu centerHit und innerHit);
//
//    /// WARNING: nbFinderFilters are not ported to a redesigned version yet, this has to be considered for time planning!
//    bool isAccepted = secMap[outerSecID, centerSecID, innerSecID].accept(outerSegment, innerSegment); // if there is not filter activated, this is always true, details depending on the implementation of the secMap interfaces.
//
//    if isAccepted:
//    segmentNetwork.linkTheseEntries(outerSegment, innerSegment);
//
// return segmentNetwork;
//////////////////////////////////////////////////////////////////////////
/// now we got a network, where segments form the nodes and the links between them are implicitly that what we called "neighbours" so far.
/// If we had more time, one could use the last Section as a blueprint for easily writing a recursive Network-algorithm, which simply continues sticking node-pairs of the input network into a single node of the output network (e.g. the next step would take neighbouring segments and would combine them to 3-hit-segments, which would form the nodes of the new network).
/// for 4-hit filters the current implementations already have some useful filters, but for 5- and more-hit-filters, this approach would simply say alwaysTrue, but would deliver you automatically the longest chains (one only has to search for multi-hit-segments which are inner-and outer end at the same time and collect them before starting the next iteration - et voilà, another tracking algorithm done.
/// since we haven't got time for this, we have to neglect that one anyway, but it would have been a nice (and cheap) idea...

/// which has not been finally decided is, whether only the segmentNetwork shall be stored in the StoreObjPtr or the activeSectorNetwork and the TrackNodeNetwork too.
/// if they shall be stored, it is easier lateron to analyze the results and the Segment-class does not have to store all the information already stored in the previos iterations (e.g. TrackNodes). This is also relevant since the 4-hit filters mentioned above can profit from the links the ActiveSectors already found to the secMap, so usage would be faster.
/// Downside of storing all of them is of course memory consumption, but at the moment I would prefer storing all of them and for the worst case, find a memory-economic way to keep the information in future redesigns.

/// that's actually it. The output lies in the DataStore and can be used for various purposes. To get a better feeling for the whole thing, I will now list 3 examples of usage.


/// CA:
/*
take the segmentNetwork, loop over all nodes (which are the segments) and asks the inner nodes (which are the neighbours) if they are compatible, in short: the normal CA-algorithm, which I do not fully describe again here.
after that, each segment has got its final state.

the second step in the module then will be to call a findSeeds(segmentNetwork)-function.
This function marks all segments as seeds, whose state is "high enough".
All these seeds are then allowed to be seed for a tc-collector (e.g the path finder-algorithm from Rudi) to start collecting.
The resulting bunch of TrackCands are then stored as SpacePointTrackCands in a storeArray. Whether there shall be a simple QI-calculator to be included, we have to decide lateron. Alternatively all of them are set onto a value of 0.5 (slightly smeared to keep Hopfield Network going).



/// CKF:

since it would take to long to fully implement an fully-grown CKF, this is a draft for a slower - but from the result point-of-view aequivalent - approach.

Short reminder: the CKF starts from a seed and goes in the direction defined by the seed and collects the x (<= 1) best next inner hits for extending the TC.
each of these valid inner hits are then added to a copy of the current TC and then independently followed on until end of road or a chi-square-threshold has been surpassed. In the end, for each seed, one keeps the y (<= 1) best TCs for further studies.

Since genFit is not really designed to allow using their algorithms in such a freely manner, the implementation would take a while.

Therefore here a sufficently good approximation to the intended behavior:

start again with the findSeeds(segmentNetwork)-function.
This one marks all segments as seeds, which are outerEnds of the network. (this would be the strictest definition. Alternatively one could mark all segments as seeds which have a hit at layer 5 or higher, but in the end the following steps are the same, no matter how the seeds were chosen.

again a tc-collector-algorithm will be called for each seed and the resulting paths then converted to genFit::Tracks to be able to use the normal KF-interface used in the old design. This would then for each seed give a bunch of TrackCandidates, where the y best ones are kept and exported to the storeArray<SpacePointTrackCand>.


/// Thomas (to get the training data he needs for his neural networks):

he simply takes the segmentNetwork and executes another custom findSeeds-function:
This one simply marks all segments as seeds, which are not innerEnd of the network.
Then he can simply loop over the network and collect each seed and its inner neighbours -> this produces 3-hit-combinations (with the seed being the outer end of them) he needs for his training.*/
