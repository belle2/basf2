/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCMCLookUp.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

//ClassImpInCDCLocalTracking(CDCMCLookUp)


CDCMCLookUp::CDCMCLookUp()
{
}


/** Destructor. */
CDCMCLookUp::~CDCMCLookUp()
{
}

CDCMCLookUp& CDCMCLookUp::Instance()
{

  if (m_instance == nullptr) m_instance = new CDCMCLookUp();
  return *m_instance;

}
CDCMCLookUp* CDCMCLookUp::m_instance = nullptr;


bool CDCMCLookUp::checkComposition(const StoreArray<CDCHit>& storedHits,
                                   const StoreArray<CDCSimHit>& storedSimhits,
                                   const StoreArray<MCParticle>& storedMCParticles)
{

  RelationArray simhitsToHitsRelation(storedSimhits, storedHits);
  RelationArray mcParticlesToHitsRelation(storedMCParticles, storedHits);
  RelationArray mcParticlesToSimhitsRelation(storedMCParticles, storedSimhits);

  int nHits = storedHits.getEntries();
  int nSimHits = storedSimhits.getEntries();
  //int nMCParts = storedMCParticles.getEntries();

  int nSimHitsToHits = simhitsToHitsRelation.getEntries();
  int nMCPartsToHits = mcParticlesToHitsRelation.getEntries();
  int nMCPartsToSimHits = mcParticlesToSimhitsRelation.getEntries();


  // check if simhits to cdchits is 1 to 0..1
  B2DEBUG(200, "Check if CDCSimHit -> CDCHit Relation is 1 -> 0..1");
  for (int iHit = 0; iHit < nHits; ++iHit) {
    size_t nFound = 0;
    for (int iRelation = 0; iRelation < nSimHitsToHits; ++iRelation) {

      const RelationElement& relationElement = simhitsToHitsRelation[iRelation];
      const std::vector< RelationElement::index_type >& toIndices = relationElement.getToIndices();

      for (std::vector< RelationElement::index_type >::const_iterator itToIndex = toIndices.begin();
           itToIndex != toIndices.end(); ++itToIndex) {

        RelationElement::index_type toIndex = *itToIndex;
        //RelationElement::index_type fromIndex = relationElement.getFromIndex();

        if (int(toIndex) == iHit) ++nFound;

      }
    }

    //there must be exactly one entry for every hit
    if (nFound != 1) B2WARNING("iHit == " << iHit << " has " << nFound << " entries in the CDCSimHit to CDCHit relation. Expected 1");

  }


  for (int iSimHit = 0; iSimHit < nSimHits ; ++iSimHit) {
    size_t nFound = 0;
    for (int iRelation = 0; iRelation < nSimHitsToHits; ++iRelation) {

      const RelationElement& relationElement = simhitsToHitsRelation[iRelation];
      const std::vector< RelationElement::index_type >& toIndices = relationElement.getToIndices();

      for (std::vector< RelationElement::index_type >::const_iterator itToIndex = toIndices.begin();
           itToIndex != toIndices.end(); ++itToIndex) {

        //RelationElement::index_type toIndex = *itToIndex;
        RelationElement::index_type fromIndex = relationElement.getFromIndex();

        if (int(fromIndex) == iSimHit) ++nFound;

      }
    }

    //there must be one or zero entries for each simhit
    if (nFound != 1 and nFound != 0) B2WARNING("iSimHit == " << iSimHit << " has " << nFound << " entries in the CDCSimHit to CDCHit relation. Expected 0 or 1");

  }


  // check if mcpart to cdchits is 1 to 0..n
  B2DEBUG(200, "Check if MCParticles -> CDCHit Relation is 1 -> 0..n");
  for (int iHit = 0; iHit < nHits ; ++iHit) {
    size_t nFound = 0;
    for (int iRelation = 0; iRelation < nMCPartsToHits; ++iRelation) {

      const RelationElement& relationElement = mcParticlesToHitsRelation[iRelation];
      const std::vector< RelationElement::index_type >& toIndices = relationElement.getToIndices();

      for (std::vector< RelationElement::index_type >::const_iterator itToIndex = toIndices.begin();
           itToIndex != toIndices.end(); ++itToIndex) {

        RelationElement::index_type toIndex = *itToIndex;
        //RelationElement::index_type fromIndex = relationElement.getFromIndex();

        if (int(toIndex) == iHit) {
          ++nFound;
          //if (iHit == 1236) B2DEBUG(200,"Found MCParticle " << fromIndex << " for iHit " << iHit); //example
        }
      }
    }

    //there must be exactly one entry for every hit
    if (nFound != 1) B2WARNING("iHit == " << iHit << " has " << nFound << " entries in the MCParticle to CDCHit relation. Expected 1");

  }


  // check if mcpart to simhits is 1 to 0..n
  B2DEBUG(200, "Check if MCParticles -> CDCSimHit Relation is 1 -> 0..n");
  for (int iSimHit = 0; iSimHit < nSimHits ; ++iSimHit) {
    size_t nFound = 0;
    for (int iRelation = 0; iRelation < nMCPartsToSimHits; ++iRelation) {

      const RelationElement& relationElement = mcParticlesToSimhitsRelation[iRelation];
      const std::vector< RelationElement::index_type >& toIndices = relationElement.getToIndices();

      for (std::vector< RelationElement::index_type >::const_iterator itToIndex = toIndices.begin();
           itToIndex != toIndices.end(); ++itToIndex) {

        RelationElement::index_type toIndex = *itToIndex;
        //RelationElement::index_type fromIndex = relationElement.getFromIndex();

        if (int(toIndex) == iSimHit) ++nFound;

      }
    }

    //there must be exactly one entry for every simhit
    if (nFound != 1) B2WARNING("iSimHit == " << iSimHit << " has " << nFound << " entries in the MCParticle to CDCHit relation. Expected 1");

  }

  //the inverse relation  does not yield any information

  return true;
}


void CDCMCLookUp::clear()
{

  m_iWirehitToSimHitMap.clear();
  m_iWirehitToISimHitMap.clear();
  m_timeSortedWireHits.clear();

  m_iWirehitToMCParticleMap.clear();
  m_mcParticleToMajorParticleMap.clear();

  m_iTrackToMCTrackMap.clear();
  m_iWireHitToIndexInTrackMap.clear();

  m_timeSortedSegments.clear();
  m_iTrackToMCSegmentTrackMap.clear();
  m_segmentToIndexInTrackMap.clear();

}





void CDCMCLookUp::addSegments(const std::vector< Belle2::CDCLocalTracking::CDCRecoSegment2D >& segments)
{

  //estimate which segment is followed by which other

  for (vector< CDCRecoSegment2D >::const_iterator itSegment = segments.begin();
       itSegment != segments.end(); ++itSegment) {

    const CDCRecoSegment2D& segment = *itSegment;

    EfficiencyTrackIdPair effTrackIdPair = getHighestEfficieny(*itSegment);

    //FloatType efficiency = effTrackIdPair.first;
    ITrackType iTrack = effTrackIdPair.second;

    size_t nHits = 0;
    FlightTime cumulatedFlightTime = 0;
    for (CDCRecoSegment2D::const_iterator itRecoHit2D = segment.begin();
         itRecoHit2D != segment.end(); ++itRecoHit2D) {

      const CDCRecoHit2D& recohit = *itRecoHit2D;
      const CDCWireHit& wirehit = recohit.getWireHit();
      if (getMCTrackId(&wirehit) == iTrack) {
        //consider this hit for the alignement only if it has the same as the highest efficiency track id
        const CDCSimHit* simhit = getSimHit(&wirehit);

        cumulatedFlightTime += simhit->getFlightTime();
        ++nHits;

      }
    }
    FlightTime averageTOF = float(cumulatedFlightTime) / nHits;

    TimeSortedSegments::value_type newItem(averageTOF, &segment);
    m_timeSortedSegments.insert(newItem);
  }

  //B2DEBUG(200," Assigned " << m_timeSortedSegments.size() << " Segments to m_timeSortedSegments" );

  //build order tracks with the segments
  //two versions for on track id can be there one forward one backward
  for (TimeSortedSegments::const_iterator itFlightTimeSegmentPair =  m_timeSortedSegments.begin();
       itFlightTimeSegmentPair != m_timeSortedSegments.end(); ++itFlightTimeSegmentPair) {

    //const FlightTime & averageTOF = itFlightTimeSegmentPair->first;
    const CDCRecoSegment2D* segment = itFlightTimeSegmentPair->second;

    EfficiencyTrackIdPair effTrackIdPair = getHighestEfficieny(*segment);

    //FloatType efficiency = effTrackIdPair.first;
    ITrackType iTrack = effTrackIdPair.second;

    ForwardBackwardInfo fbInfo = isForwardOrBackward(*segment, iTrack);

    ITrackAndFBInfo iTrackAndFBInfo(iTrack, fbInfo);

    if (fbInfo == FORWARD) {
      //time is used in ascending order
      m_iTrackToMCSegmentTrackMap[iTrackAndFBInfo].push_back(segment);

    } else if (fbInfo == BACKWARD) {
      //time is used in reverse order
      m_iTrackToMCSegmentTrackMap[iTrackAndFBInfo].push_front(segment);


    } else {
      //keep the unknow alignement as dummy tracks just for homogenity
      m_iTrackToMCSegmentTrackMap[iTrackAndFBInfo].push_back(segment);
      B2WARNING("Segment with no proper forward backward information found");
    }


  }
  //B2DEBUG(200," Created " << m_iTrackToMCSegmentTrackMap.size() << " MCSegmentTracks" );


  //no build the indexInTrack map for the segments
  for (ITrackToMCSegmentTrackMap::const_iterator itMCSegmentTrack = m_iTrackToMCSegmentTrackMap.begin();
       itMCSegmentTrack != m_iTrackToMCSegmentTrackMap.end(); ++itMCSegmentTrack) {

    //const ITrackAndFBInfo & iTrackAndFBInfo = itMCSegmentTrack->first;
    const MCSegmentTrack& mcSegmentTrack   = itMCSegmentTrack->second;

    //B2DEBUG(200," iTrack " << iTrackAndFBInfo.first <<
    //            " FBInfo " << iTrackAndFBInfo.second <<
    //            " size " << mcSegmentTrack.size() );


    Index indexInTrack = 0;
    for (MCSegmentTrack::const_iterator itSegment = mcSegmentTrack.begin();
         itSegment != mcSegmentTrack.end(); ++itSegment , ++indexInTrack) {

      const CDCRecoSegment2D* segment = *itSegment;
      m_segmentToIndexInTrackMap[segment] = indexInTrack;

    }

  }
}

const CDCSimHit* CDCMCLookUp::getSimHit(const CDCWireHit* wirehit) const
{
  size_t iStoredHit = wirehit->getStoreIHit();
  IWireHitToSimHitMap::const_iterator itSimHit = m_iWirehitToSimHitMap.find(iStoredHit);
  return (itSimHit ==  m_iWirehitToSimHitMap.end()) ? nullptr : itSimHit->second ;

}

const MCParticle* CDCMCLookUp::getMCParticle(const CDCWireHit* wirehit) const
{

  size_t iStoredHit = wirehit->getStoreIHit();
  IWireHitToMCParticleMap::const_iterator itMCParticle = m_iWirehitToMCParticleMap.find(iStoredHit);
  return (itMCParticle == m_iWirehitToMCParticleMap.end()) ? nullptr : itMCParticle->second ;

}

const Belle2::MCParticle* CDCMCLookUp::getMajorMCParticle(const CDCWireHit* wirehit) const
{
  const MCParticle* mcPart = getMCParticle(wirehit);
  return getMajorMCParticle(mcPart);
}

const Belle2::MCParticle* CDCMCLookUp::getMCParticle(const CDCRecoSegment2D& segment) const
{

  EfficiencyTrackIdPair efficiencyTrackIdPair = getHighestEfficieny(segment);
  ITrackType iTrack = efficiencyTrackIdPair.second;

  if (iTrack ==  INVALID_ITRACK) return nullptr;

  for (CDCRecoSegment2D::const_iterator itRecoHit = segment.begin();
       itRecoHit != segment.end(); ++itRecoHit) {

    const CDCWireHit& wirehit = itRecoHit->getWireHit();
    if (getMCTrackId(&wirehit) == iTrack) return getMCParticle(&wirehit);

  }
  return nullptr;
}


CDCRecoHit3D CDCMCLookUp::getMCHit3D(const CDCWireHit* wirehit) const
{

  const CDCSimHit* simhit = getSimHit(wirehit);
  return simhit == nullptr ? CDCRecoHit3D() : CDCRecoHit3D::fromSimHit(wirehit, *simhit);

}

CDCRecoHit2D CDCMCLookUp::getMCHit2D(const CDCWireHit* wirehit) const
{

  const CDCSimHit* simhit = getSimHit(wirehit);
  return simhit == nullptr ? CDCRecoHit2D() : CDCRecoHit2D::fromSimHit(wirehit, *simhit);

}

ITrackType CDCMCLookUp::getMCTrackId(const CDCWireHit* wirehit) const
{

  const MCParticle* mcpart = getMCParticle(wirehit);

  ITrackType iTrackDummy = mcpart == nullptr ? INVALID_ITRACK : mcpart->getIndex();

  if (iTrackDummy == INVALID_ITRACK and mcpart != nullptr) {
    B2WARNING("mcpart->getIndex is INVALID_ITRACK." <<
              " Change INVALID_ITRACK to something more obscure than " << INVALID_ITRACK);
  }
  return iTrackDummy;

}

ITrackType CDCMCLookUp::getMajorMCTrackId(const CDCWireHit* wirehit) const
{
  //decide whether this hit has the trackid of its major particle based on how far the simhit is from the production vertex of the last descendant before the major particle
  //const double deltaTCut =  4.0; //ns
  const double distanceCut =  4.0; //<- this is the more important variable since the time is rather extended in the cdc and can very much for particles of different velocities


  const MCParticle* mcpart = getMCParticle(wirehit);
  if (mcpart == nullptr) {
    B2WARNING("No MCParticle for wirehit " << wirehit);
    return INVALID_ITRACK;
  }
  const MCParticle* majorMCPart = getMajorMCParticle(mcpart);

  if (mcpart == majorMCPart) return getMCTrackId(wirehit);
  else if (majorMCPart == nullptr) {
    //B2DEBUG(100,"No Major particle registered");
  } else {
    //B2DEBUG(100,"Try to resolve TrackId ");
    //find daughter of the major particle that points to mc particle of the wirehit
    const MCParticle* daughterParticle = mcpart;
    while (daughterParticle != nullptr and daughterParticle->getMother() != majorMCPart) {
      daughterParticle = daughterParticle->getMother();
    }
    if (daughterParticle == nullptr) return INVALID_ITRACK;

    TVector3 productionVertexFromMajor = daughterParticle->getVertex();
    //float productionTime = daughterParticle->getProductionTime();
    //compare the vertex with the hit position and the productionTime with the hit time
    const CDCSimHit* simhit  = getSimHit(wirehit);
    if (simhit == nullptr) {
      B2WARNING("No CDCSimHit for wirehit " << wirehit);
      return INVALID_ITRACK;
    }

    TVector3 hitPosition = simhit->getPosTrack();
    //float hitTime = simhit->getFlightTime();

    hitPosition -= productionVertexFromMajor;
    float distanceFromVertex = hitPosition.Mag();
    //float deltaT = hitTime - productionTime;

    // deltaT < deltaTCut or
    if (distanceFromVertex < distanceCut) {
      //B2DEBUG(100,"Resolve TrackId " << getMCTrackId(wirehit) << " -> " << majorMCPart->getIndex() << " for Wirehit " << wirehit);

      return majorMCPart->getIndex();

    } else {
      //get unresolved trackid
      //B2DEBUG(100,"Unresolve TrackId " << getMCTrackId(wirehit) << " -> " << majorMCPart->getIndex() << " for Wirehit " << wirehit);
      //B2DEBUG(100,"deltaT == " << deltaT << " distanceFromVertex == " << distanceFromVertex);
      return getMCTrackId(wirehit);
    }

  }
  return INVALID_ITRACK;

}

ITrackType CDCMCLookUp::getSimTrackId(const CDCWireHit* wirehit) const
{

  const CDCSimHit* simhit = getSimHit(wirehit);

  return simhit == nullptr ? INVALID_ITRACK : simhit->getTrackId();

}

/*
void CDCMCLookUp::checkSimToMCTrackIdEquivalence(const CDCWireHitCollection& wirehits) const
{

  //trackids differ but same tracks emerge from the different trackids
  TrackIdCountMap mcTrackIdCount;
  TrackIdCountMap simTrackIdCount;

  for (CDCWireHitCollection::const_iterator itWireHit = wirehits.begin();
       itWireHit != wirehits.end(); ++itWireHit) {

    const CDCWireHit& wirehit = *itWireHit;

    ITrackType mcTrackId = getMCTrackId(&wirehit);
    ITrackType simTrackId = getSimTrackId(&wirehit);

    if (mcTrackIdCount.count(mcTrackId) == 0) {
      mcTrackIdCount[mcTrackId] = 0;
    }
    ++(mcTrackIdCount[mcTrackId]);


    if (simTrackIdCount.count(simTrackId) == 0) {
      simTrackIdCount[simTrackId] = 0;
    }
    ++(simTrackIdCount[simTrackId]);


    if (not(simTrackId == mcTrackId)) {
      B2INFO("mcTrackId " << simTrackId <<
             " != simTrackId " << mcTrackId);
    }

  }


  for (TrackIdCountMap::iterator itTrackIdCount = mcTrackIdCount.begin();
       itTrackIdCount != mcTrackIdCount.end(); ++itTrackIdCount) {
    B2INFO("mcTrackId " << itTrackIdCount->first <<
           " : count " << itTrackIdCount->second);

  }

  for (TrackIdCountMap::iterator itTrackIdCount = simTrackIdCount.begin();
       itTrackIdCount != simTrackIdCount.end(); ++itTrackIdCount) {
    B2INFO("simTrackId " << itTrackIdCount->first <<
           " : count " << itTrackIdCount->second);

  }

  double d;
  cin >> d;

  }*/

ITrackType CDCMCLookUp::getMajorMCTrackId(const CDCWireHit* one,
                                          const CDCWireHit* two,
                                          const CDCWireHit* three) const
{

  ITrackType iOne   =  getMajorMCTrackId(one);
  ITrackType iTwo   =  getMajorMCTrackId(two);
  ITrackType iThree =  getMajorMCTrackId(three);

  if (iOne == iTwo and iTwo == iThree) return iOne;
  else return INVALID_ITRACK;

}


bool CDCMCLookUp::isTOFAligned(const CDCWireHit* one,
                               const CDCWireHit* two,
                               const CDCWireHit* three) const
{

  //use the normal track id to check if the time of flight is the correct measure
  //if the hits are not from the same particle the secondary particles over took the
  //primary screws the comparability of the time of flight

  ITrackType iOne   =  getMCTrackId(one);
  ITrackType iTwo   =  getMCTrackId(two);
  ITrackType iThree =  getMCTrackId(three);

  const Belle2::CDCSimHit*   simhitOne = getSimHit(one);
  const Belle2::CDCSimHit*   simhitTwo = getSimHit(two);
  const Belle2::CDCSimHit*   simhitThree = getSimHit(three);

  Vector3D directionOfFlight;

  if (iOne == iTwo and iTwo == iThree) {
    if ((simhitOne ->getFlightTime() <  simhitTwo   ->getFlightTime()  and
         simhitTwo ->getFlightTime() <= simhitThree ->getFlightTime()) or
        (simhitOne ->getFlightTime() <= simhitTwo   ->getFlightTime()  and
         simhitTwo ->getFlightTime() <  simhitThree ->getFlightTime())) {

      return true;

    } else if (simhitOne ->getFlightTime() == simhitTwo   ->getFlightTime() and
               simhitTwo ->getFlightTime() == simhitThree ->getFlightTime()) {

      //in case you wonder this actually occures in the simulation
      directionOfFlight = Vector3D::average(simhitOne->getMomentum(),
                                            simhitTwo->getMomentum(),
                                            simhitThree->getMomentum());

    } else if (simhitOne ->getFlightTime() > simhitTwo   ->getFlightTime() or
               simhitTwo ->getFlightTime() > simhitThree ->getFlightTime()) {

      return false;

    }
  } else if (iOne == iTwo) {

    if (simhitOne ->getFlightTime() > simhitTwo   ->getFlightTime()) return false;
    directionOfFlight = Vector3D::average(simhitOne->getMomentum(),
                                          simhitTwo->getMomentum());


  } else if (iOne == iThree) {

    if (simhitOne ->getFlightTime() > simhitThree   ->getFlightTime()) return false;
    directionOfFlight = Vector3D::average(simhitOne->getMomentum(),
                                          simhitThree->getMomentum());
  } else if (iTwo == iThree) {

    if (simhitTwo ->getFlightTime() > simhitThree   ->getFlightTime()) return false;
    directionOfFlight = Vector3D::average(simhitTwo->getMomentum(),
                                          simhitThree->getMomentum());
  } else {
    //maybe refine that that at the major track is checked
    directionOfFlight = Vector3D(0.0, 0.0, 0.0);

  }

  {
    // if the simhits are not from the same particle the time of flight is not a good indicator
    // since secondary slow particles might overtake the electron avalanche in a though large time periode
    // it still belongs to the same track but the timing will indicate that the hit accured much later
    // so we hope that if the hit position of the second hit is in between the other two the hits are still
    // aligned corretly

    Vector3D posOne   = simhitOne->getPosTrack();
    Vector3D posTwo   = simhitTwo->getPosTrack();
    Vector3D posThree = simhitThree->getPosTrack();

    FloatType parallelOne   = posOne.parallelComp(directionOfFlight);
    FloatType parallelTwo   = posTwo.parallelComp(directionOfFlight);
    FloatType parallelThree = posThree.parallelComp(directionOfFlight);

    return (parallelOne <= parallelTwo and parallelTwo <  parallelThree) or
           (parallelOne <  parallelTwo and parallelTwo <= parallelThree);


    /*
    Vector3D vecOneToThree(posThree);
    vecOneToThree.subtract(posOne);

    Vector3D vecOneToTwo(posTwo);
    vecOneToTwo.subtract(posOne);

    FloatType orthogonalTwo = vecOneToTwo.orthogonalComp(vecOneToThree);

    if(  0 < parallelTwo and parallelTwo < vecOneToThree.norm() and
         orthogonalTwo < vecOneToThree.norm() / 2.0
        ){
      return true;

    } else {
      return false;
      B2DEBUG(100, "    Start : "   << one <<
                   " Middle : " << two <<
                   " End : "    << three )   ;



      B2DEBUG(100, "    Start : "   << posOne <<
                   " Middle : " << posTwo <<
                   " End : "    << posThree )   ;


      B2DEBUG(100, "    vecOneToThree : "   << vecOneToThree );
      B2DEBUG(100, "    0 < " << parallelTwo << " and " << parallelTwo<< " < " << vecOneToThree.norm() );
      cin >> parallelTwo;

    }*/


  }
  return false;
}



bool CDCMCLookUp::isAlignedInMCTrack(const CDCWireHit* one,
                                     const CDCWireHit* two,
                                     const CDCWireHit* three) const
{


  //accept only wire hits if they are directly behind each other in the according mc track

  ITrackType iOne   =  getMCTrackId(one);
  ITrackType iTwo   =  getMCTrackId(two);
  ITrackType iThree =  getMCTrackId(three);

  Index iInOne   = getIndexInTrack(one);
  Index iInTwo   = getIndexInTrack(two);
  Index iInThree = getIndexInTrack(three);

  const Index tolerance = 2;

  if (iOne == iTwo and iTwo == iThree) {

    return 0 < iInTwo   - iInOne and iInTwo   - iInOne <= tolerance and
           0 < iInThree - iInTwo and iInThree - iInTwo <= tolerance;

  } else if (iOne == iTwo) {

    return 0 < iInTwo - iInOne and iInTwo - iInOne <= tolerance;

  } else if (iOne == iThree) {
    //assume that in this case the intermediate hit has some other track id, hence does not appear in the same mctrack anyways
    //so we assume the other two behind each other anyways

    return 0 < iInThree - iInOne and iInThree - iInOne <= tolerance;

  } else if (iTwo == iThree) {

    return 0 < iInThree - iInTwo and iInThree - iInTwo <= tolerance;

  } else {
    return true;
    //can not say anything about the alignement of the hits
  }
}



template<class HitIterator>
void CDCMCLookUp::addMajorMCTrackIdCountGeneric(const HitIterator& begin, const HitIterator& end, TrackIdCountMap& trackIdCount) const
{

  for (HitIterator itRecoHit = begin; itRecoHit != end; ++itRecoHit) {

    const auto& wireHit = (*itRecoHit)->getWireHit();

    ITrackType trackIdOfHit = getMajorMCTrackId(wireHit);
    if (trackIdCount.count(trackIdOfHit) == 0) {
      trackIdCount[trackIdOfHit] = 0;
    }
    ++(trackIdCount[trackIdOfHit]);
  }

}



void CDCMCLookUp::addMajorMCTrackIdCount(const CDCRecoSegment2D& segment, TrackIdCountMap& trackIdCount) const
{

  addMajorMCTrackIdCountGeneric(segment.begin(), segment.end(), trackIdCount);

}

CDCMCLookUp::TrackIdCountMap CDCMCLookUp::getMajorMCTrackIdCount(const CDCRecoSegment2D& segment) const
{

  TrackIdCountMap result;
  addMajorMCTrackIdCount(segment, result);
  return result;

}

void CDCMCLookUp::fillEfficiencies(const TrackIdCountMap& trackIdCount, size_t nHits , EfficiencyTrackIdMap& efficiencies) const
{

  float nHitsFloat = nHits;

  for (TrackIdCountMap::const_iterator itTrackIdCountPair = trackIdCount.begin();
       itTrackIdCountPair != trackIdCount.end(); ++itTrackIdCountPair) {

    float efficiency = (itTrackIdCountPair->second) / nHitsFloat;
    efficiencies[efficiency] = itTrackIdCountPair->first;
  }

}

void CDCMCLookUp::fillEfficiencies(const TrackIdCountMap& trackIdCount, EfficiencyTrackIdMap& efficiencies) const
{

  size_t nHits = 0;
  for (TrackIdCountMap::const_iterator itTrackIdCountPair = trackIdCount.begin();
       itTrackIdCountPair != trackIdCount.end(); ++itTrackIdCountPair) {

    nHits += itTrackIdCountPair->first;
  }
  fillEfficiencies(trackIdCount, nHits, efficiencies);
}

void CDCMCLookUp::fillEfficiencies(const CDCRecoSegment2D& segment, EfficiencyTrackIdMap& efficiencies) const
{

  TrackIdCountMap trackIdCount;
  addMajorMCTrackIdCount(segment, trackIdCount);

  size_t nHits = segment.size();

  fillEfficiencies(trackIdCount, nHits, efficiencies);

}


CDCMCLookUp::EfficiencyTrackIdMap CDCMCLookUp::getEfficiencies(const CDCRecoSegment2D& segment) const
{

  EfficiencyTrackIdMap efficiencies;
  fillEfficiencies(segment, efficiencies);
  return efficiencies;

}
CDCMCLookUp::EfficiencyTrackIdPair CDCMCLookUp::getHighestEfficieny(const CDCRecoSegment2D& segment) const
{

  EfficiencyTrackIdMap efficiencies;
  fillEfficiencies(segment, efficiencies);

  EfficiencyTrackIdMap::reverse_iterator itLast = efficiencies.rbegin();
  while (itLast != efficiencies.rend() and itLast->second == INVALID_ITRACK) ++itLast;

  return itLast == efficiencies.rend() ? EfficiencyTrackIdPair(0.0f, INVALID_ITRACK) : EfficiencyTrackIdPair(*itLast);

}

const CDCSimHit* CDCMCLookUp::getFirstSimHit(const CDCRecoSegment2D& segment, ITrackType trackId) const
{

  CDCRecoSegment2D::const_iterator itRecoHit = segment.begin();

  //advance as long the end is not reached
  //stops if both the simhit and the current recohit is not nullptr
  //and the current trackId is the one asked

  // look for the mcTrackId instead of the major track id to have a hit really belonging to that mc particle

  // in case the given trackId is INVALID_ITRACK it ignores the trackId and gives the first simhit that is not nullptr
  while (itRecoHit != segment.end() and
         (getSimHit(&(itRecoHit->getWireHit())) == nullptr or
          (trackId != INVALID_ITRACK and getMCTrackId(&(itRecoHit->getWireHit())) != trackId))) ++itRecoHit;

  return itRecoHit == segment.end() ? nullptr : getSimHit(&(itRecoHit->getWireHit())) ;


}
const CDCSimHit* CDCMCLookUp::getLastSimHit(const CDCRecoSegment2D& segment, ITrackType trackId) const
{

  CDCRecoSegment2D::const_reverse_iterator itRecoHit = segment.rbegin();

  //advance from the back as long the begin is not reached
  //stops if both the simhit and the current recohit is not nullptr
  //and the current trackId is the one asked

// look for the mcTrackId instead of the major track id to have a hit really belonging to that mc particle

  // in case the given trackId is INVALID_ITRACK it ignores the trackId and gives the first simhit that is not nullptr
  while (itRecoHit != segment.rend() and
         (getSimHit(&(itRecoHit->getWireHit())) == nullptr or
          (trackId != INVALID_ITRACK and getMCTrackId(&(itRecoHit->getWireHit())) != trackId))) ++itRecoHit;

  return itRecoHit == segment.rend() ? nullptr : getSimHit(&(itRecoHit->getWireHit())) ;

}

ForwardBackwardInfo CDCMCLookUp::isForwardOrBackward(const CDCRecoSegment2D& segment,
                                                     ITrackType iTrack) const
{

  //replace with more stable version ?

  const CDCSimHit* firstSimHit = getFirstSimHit(segment, iTrack);
  const CDCSimHit* lastSimHit  = getLastSimHit(segment, iTrack);

  if (firstSimHit == nullptr or lastSimHit == nullptr) {
    return UNKNOWN;
  } else {

    FlightTime firstTOF = firstSimHit->getFlightTime();
    FlightTime lastTOF  = lastSimHit->getFlightTime();


    return
      firstTOF <  lastTOF ? FORWARD :
      firstTOF >  lastTOF ? BACKWARD :
      UNKNOWN;
  }

  return UNKNOWN;

}

/* Returns the number a of superlayer changes when advancing from _from_ to _to_.
Note that it intentionally does not count the actual number of superlayers passed by the track but only the ones where there is actually a segment of this track found */
int CDCMCLookUp::getNSuperLayersTraversed(const CDCRecoSegment2D& from,
                                          const CDCRecoSegment2D& to) const
{

  EfficiencyTrackIdPair effTrackIdPairFrom = getHighestEfficieny(from);

  //FloatType efficiency = effTrackIdPair.first;
  ITrackType iTrackFrom = effTrackIdPairFrom.second;

  EfficiencyTrackIdPair effTrackIdPairTo = getHighestEfficieny(to);

  //FloatType efficiency = effTrackIdPair.first;
  ITrackType iTrackTo = effTrackIdPairTo.second;

  if (iTrackFrom != iTrackTo or iTrackFrom == INVALID_ITRACK or iTrackTo == INVALID_ITRACK) return -999;
  //now iTrackFrom == iTrackTo

  ForwardBackwardInfo fromIsFB = isForwardOrBackward(from, iTrackFrom);
  ForwardBackwardInfo toIsFB   = isForwardOrBackward(to, iTrackTo);

  if (fromIsFB != toIsFB or fromIsFB == UNKNOWN or toIsFB == UNKNOWN) return -999;
  //now fromIsFB == toIsFB

  // now we know that both segments belong to the same mc track
  // get this track
  ITrackAndFBInfo iTrackAndFBInfo(iTrackFrom, fromIsFB);

  ITrackToMCSegmentTrackMap::const_iterator itITrackMCSegmentPair =
    m_iTrackToMCSegmentTrackMap.find(iTrackAndFBInfo);

  if (itITrackMCSegmentPair ==  m_iTrackToMCSegmentTrackMap.end()) {
    B2WARNING("No MCSegmentTrack for iTrack " << iTrackFrom << " and FBInfo " << fromIsFB);
    return -999;
  }
  const MCSegmentTrack& mcSegmentTrack = itITrackMCSegmentPair->second;

  Index fromIndex = getIndexInTrack(from);
  Index toIndex   = getIndexInTrack(to);

  //set the start point and the end point of a forward iteration in the segment track
  MCSegmentTrack::const_iterator begin = mcSegmentTrack.begin();
  advance(begin, std::min(fromIndex, toIndex));

  MCSegmentTrack::const_iterator end   =  mcSegmentTrack.begin();
  advance(end, std::max(fromIndex, toIndex) + 1);

  //now count the changes of iSuperlayer during the advance to the last segment
  //note that correctly does not count the actual number of superlayers passed but only the ones where there is actually a segment of this track found
  ILayerType currentISuperLayer = (*begin)->getISuperLayer();
  int nSuperlayers = 0;
  for (MCSegmentTrack::const_iterator itSegment = begin; itSegment != end; ++itSegment) {

    if (currentISuperLayer != (*itSegment)->getISuperLayer()) {
      ++nSuperlayers;
      currentISuperLayer = (*itSegment)->getISuperLayer();
    }

  }

  //if the from index is larger than the to index the return value is positive
  //else it is marked negativ

  return fromIndex < toIndex ? nSuperlayers : -nSuperlayers;

}


void CDCMCLookUp::addMajorMCTrackIdCount(const CDCTrack& track, TrackIdCountMap& trackIdCount) const
{

  addMajorMCTrackIdCountGeneric(track.begin(), track.end(), trackIdCount);

}

CDCMCLookUp::TrackIdCountMap CDCMCLookUp::getMajorMCTrackIdCount(const CDCTrack& track) const
{

  TrackIdCountMap result;
  addMajorMCTrackIdCount(track, result);
  return result;

}



void CDCMCLookUp::fillEfficiencies(const CDCTrack& track,
                                   EfficiencyTrackIdMap& efficiencies) const
{

  TrackIdCountMap trackIdCount;
  addMajorMCTrackIdCount(track, trackIdCount);

  size_t nHits = track.size();

  fillEfficiencies(trackIdCount, nHits, efficiencies);

}


CDCMCLookUp::EfficiencyTrackIdMap CDCMCLookUp::getEfficiencies(const CDCTrack& track) const
{

  EfficiencyTrackIdMap efficiencies;
  fillEfficiencies(track, efficiencies);
  return efficiencies;

}

CDCMCLookUp::EfficiencyTrackIdPair CDCMCLookUp::getHighestEfficieny(const CDCTrack& track) const
{

  EfficiencyTrackIdMap efficiencies;
  fillEfficiencies(track, efficiencies);

  EfficiencyTrackIdMap::reverse_iterator itLast = efficiencies.rbegin();
  while (itLast != efficiencies.rend() and itLast->second == INVALID_ITRACK) ++itLast;

  return itLast == efficiencies.rend() ? EfficiencyTrackIdPair(0.0f, INVALID_ITRACK) : EfficiencyTrackIdPair(*itLast);

}




/*
void CDCMCLookUp::collectMCSegmentsForCluster(std::vector<Belle2::CDCRecoSegment> & segments, const CDCCluster & cluster) const{

  typedef multimap<CDCSimHit*,CDCWireHit *,SimHitTrackIdAndFlightTimeCompare> SimHitToWireHitMap;
  //typedef pair<MCParticlesToWireHitMap::iterator,MCParticlesToWireHitMap::iterator> MCParticlesToWireHitRange;
  SimHitToWireHitMap simHitToWireHitMap;

  //group the hits together by the same track id
  //sort them by the time of flight as a second parameter
  for( CDCCluster::iterator itWirehit = cluster.begin();
       itWirehit != cluster.end(); ++itWirehit){

    CDCWireHit * wirehit = *itWirehit;
    CDCSimHit * simhit = getSimHit(wirehit);

    simHitToWireHitMap.insert(make_pair(simhit,wirehit));

  }

  SimHitToWireHitMap::iterator itPair = simHitToWireHitMap.begin();

  while (itPair != simHitToWireHitMap.end()){

    CDCSimHit * simhit = itPair -> first;
    if ( simhit == NULL ) continue; //should be not nessecary to check that since all wire hits are mapped to concrete simhits on creation
    const int & currentTrackId = simhit->getTrackId();

    segments.push_back( CDCRecoSegment() );
    CDCRecoSegment & segment = segments.back();

    while( itPair != simHitToWireHitMap.end() && itPair -> first->getTrackId() == currentTrackId ){

      CDCWireHit * wirehit = itPair->second;

      segment.expand(getMCHit(wirehit));

      ++itPair;
    }

  }

}
*/
// Implement all functions

