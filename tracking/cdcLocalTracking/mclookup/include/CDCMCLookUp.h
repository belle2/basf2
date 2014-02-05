/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCMCLOOKUP_H
#define CDCMCLOOKUP_H

#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>

#include <framework/gearbox/Unit.h>

#include <map>
#include <list>
#include <vector>

namespace Belle2 {
  namespace CDCLocalTracking {

    ///Class to organize and present the monte carlo hit information
    class CDCMCLookUp : public UsedTObject {

    public:
      typedef std::map<Belle2::CDCLocalTracking::ITrackType, size_t> TrackIdCountMap;
      typedef std::map<float, Belle2::CDCLocalTracking::ITrackType>  EfficiencyTrackIdMap;
      typedef std::pair<float, Belle2::CDCLocalTracking::ITrackType>  EfficiencyTrackIdPair;

      //typedef Index IndexInTrack;

    public:

      /** Constructor. */
      /** This is as well the parameter free I/O constructor.
       */
      CDCMCLookUp();

      /** Destructor. */
      ~CDCMCLookUp();

    public:
      static CDCMCLookUp& Instance();
    private:
      static CDCMCLookUp* m_instance;

    public:

      static bool checkComposition(const StoreArray<Belle2::CDCHit>& storedHits,
                                   const StoreArray<Belle2::CDCSimHit>& storedSimhits,
                                   const StoreArray<Belle2::MCParticle>& storedMCParticles);

      void clear();

      void addAllSimHits(const CDCWireHitCollection& wirehits,
                         const Belle2::StoreArray<Belle2::CDCHit>& storedHits,
                         const Belle2::StoreArray<Belle2::CDCSimHit>& storedSimhits);

      void addAllMCParticle(const CDCWireHitCollection& wirehits,
                            const Belle2::StoreArray<Belle2::CDCHit>& storedHits,
                            const Belle2::StoreArray<Belle2::MCParticle>& storedMCParticles);


      /** Adding a wirehit <-> simhit pair for later look up. This correspondence should be 1 to 1 */
      void addSimHit(const CDCWireHit* wirehit, const Belle2::CDCSimHit* simhit, size_t iSimhit) {
        size_t iStoredHit = wirehit->getStoreIHit();
        m_iWirehitToSimHitMap[iStoredHit] = simhit;
        m_iWirehitToISimHitMap[iStoredHit] = iSimhit;
      }

      /** Adding a wirehit <-> mcparticle pair for later look up. This correspondence should be n to 1 */
      void addMCParticle(const CDCWireHit* wirehit, const Belle2::MCParticle* mcparticle) {
        size_t iStoredHit = wirehit->getStoreIHit();
        m_iWirehitToMCParticleMap[iStoredHit] = mcparticle;
      }

      void registerMajorMCParticle(const Belle2::MCParticle* mcpart, const Belle2::MCParticle* mcmajorpart)
      { m_mcParticleToMajorParticleMap[mcpart] = mcmajorpart; }


      void addSegments(const std::vector< Belle2::CDCLocalTracking::CDCRecoSegment2D >& segments);


      const Belle2::CDCSimHit* getSimHit(const CDCWireHit* wireHit) const;
      const Belle2::CDCSimHit* getSimHit(const CDCRecoHit2D& recoHit) const {
        const CDCWireHit& wireHit = recoHit.getWireHit();
        return getSimHit(&wireHit);
      }


      const Belle2::MCParticle* getMCParticle(const CDCWireHit* wireHit) const;
      const Belle2::MCParticle* getMCParticle(const CDCRecoHit2D& recoHit) const {
        const CDCWireHit& wireHit = recoHit.getWireHit();
        return getMCParticle(&wireHit);
      }
      const Belle2::MCParticle* getMCParticle(const CDCRecoHit3D& recoHit) const {
        const CDCWireHit& wireHit = recoHit.getWireHit();
        return getMCParticle(&wireHit);
      }

    private:
      const Belle2::MCParticle* getMajorMCParticle(const Belle2::MCParticle* mcpart) const {
        return m_mcParticleToMajorParticleMap.count(mcpart) == 0 ?
               nullptr :
               m_mcParticleToMajorParticleMap.find(mcpart)->second;
      }

    public:

      const Belle2::MCParticle* getMajorMCParticle(const CDCWireHit* wirehit) const;

      const Belle2::MCParticle* getMCParticle(const CDCRecoSegment2D& segment) const;

      CDCRecoHit3D getMCHit3D(const CDCWireHit* wirehit) const;
      CDCRecoHit2D getMCHit2D(const CDCWireHit* wirehit) const;

      ITrackType getMCTrackId(const CDCWireHit* wirehit) const;
      ITrackType getMajorMCTrackId(const CDCWireHit* wirehit) const;
      ITrackType getMajorMCTrackId(const CDCWireHit& wirehit) const { return getMajorMCTrackId(&wirehit);}
      ITrackType getSimTrackId(const CDCWireHit* wirehit) const;

      void checkSimToMCTrackIdEquivalence(const CDCWireHitCollection& wirehits) const;

      Index getIndexInTrack(const CDCWireHit* wirehit) const {
        Index iStoredIHit = wirehit->getStoreIHit();
        IWireHitToIndexInTrackMap::const_iterator itIndexInTrackPair =
          m_iWireHitToIndexInTrackMap.find(iStoredIHit);
        return itIndexInTrackPair == m_iWireHitToIndexInTrackMap.end() ?
               999 : itIndexInTrackPair->second;
      }

      ITrackType getMCTrackId(const CDCRecoHit2D& recoHit) const {
        const CDCWireHit& wireHit = recoHit.getWireHit();
        return getMCTrackId(&wireHit);
      }
      ITrackType getMCTrackId(const CDCRecoHit3D& recoHit) const {
        const CDCWireHit& wireHit = recoHit.getWireHit();
        return getMCTrackId(&wireHit);
      }


      ITrackType getMajorMCTrackId(const CDCWireHit* one,
                                   const CDCWireHit* two,
                                   const CDCWireHit* three) const;

      bool isTOFAligned(const CDCWireHit* one,
                        const CDCWireHit* two,
                        const CDCWireHit* three) const;

      bool isAlignedInMCTrack(const CDCWireHit* one,
                              const CDCWireHit* two,
                              const CDCWireHit* three) const;

    private:

      template<class HitIterator>
      void addMajorMCTrackIdCountGeneric(const HitIterator& begin, const HitIterator& end, TrackIdCountMap& trackIdCount) const;

    public:
      void addMajorMCTrackIdCount(const CDCRecoSegment2D& segment, TrackIdCountMap& trackIdCount) const;
      TrackIdCountMap getMajorMCTrackIdCount(const CDCRecoSegment2D& segment) const;

      void fillEfficiencies(const TrackIdCountMap& trackIdCount,
                            size_t nHits,
                            EfficiencyTrackIdMap& efficiencies) const;

      void fillEfficiencies(const TrackIdCountMap& trackIdCount,
                            EfficiencyTrackIdMap& efficiencies) const;

      void fillEfficiencies(const CDCRecoSegment2D& segment,
                            EfficiencyTrackIdMap& efficiencies) const;

      EfficiencyTrackIdMap getEfficiencies(const CDCRecoSegment2D& segment) const;
      EfficiencyTrackIdPair getHighestEfficieny(const CDCRecoSegment2D& segment) const;

      const CDCSimHit* getFirstSimHit(const CDCRecoSegment2D& segment, ITrackType trackId = INVALID_ITRACK) const;
      const CDCSimHit* getLastSimHit(const CDCRecoSegment2D& segment, ITrackType trackId = INVALID_ITRACK) const;

      ForwardBackwardInfo isForwardOrBackward(const CDCRecoSegment2D& segment,
                                              ITrackType trackId) const;

      Index getIndexInTrack(const CDCRecoSegment2D& segment) const {
        SegmentToIndexInTrackMap::const_iterator itIndexInTrackPair =
          m_segmentToIndexInTrackMap.find(&segment);
        return itIndexInTrackPair == m_segmentToIndexInTrackMap.end() ?
               999 : itIndexInTrackPair->second;
      }

      int getNSuperLayersTraversed(const CDCRecoSegment2D& from,
                                   const CDCRecoSegment2D& to) const;


      //same for tracks
      void addMajorMCTrackIdCount(const CDCTrack& track, TrackIdCountMap& trackIdCount) const;
      TrackIdCountMap getMajorMCTrackIdCount(const CDCTrack& track) const;

      void fillEfficiencies(const CDCTrack& track,
                            EfficiencyTrackIdMap& efficiencies) const;

      EfficiencyTrackIdMap getEfficiencies(const CDCTrack& track) const;
      EfficiencyTrackIdPair getHighestEfficieny(const CDCTrack& track) const;

      //double getPurity(const CDCRecoHitCollection & recohits) const;
      //double getPurity(const CDCWireHitCollection & wirehits) const;

      //void collectMCSegmentsForCluster(std::vector<Belle2::CDCRecoSegment> & segments, const CDCCluster & cluster) const;
      //void collectMCTracks(std::vector<Belle2::CDCTrackCand> & tracks) const;

    private:

      //typedef std::map<const Belle2::CDCLocalTracking::CDCWireHit *,const Belle2::CDCSimHit *> WireHitToSimHitMap;
      typedef std::map<Index, const Belle2::CDCSimHit*> IWireHitToSimHitMap;
      IWireHitToSimHitMap m_iWirehitToSimHitMap;

      typedef std::map<Index, Index> IWireHitToISimHitMap;
      IWireHitToISimHitMap m_iWirehitToISimHitMap;

      typedef double FlightTime;
      typedef std::pair<FlightTime, Index> FlightTimeAndIndex;
      //take the index in the simhit store array in addition to the flightime to determine which hit came first
      typedef std::map<FlightTimeAndIndex, const CDCWireHit*> TimeSortedWireHits;
      TimeSortedWireHits m_timeSortedWireHits;

      typedef std::map<Index, const Belle2::MCParticle*> IWireHitToMCParticleMap;
      IWireHitToMCParticleMap m_iWirehitToMCParticleMap;

      typedef std::map<const Belle2::MCParticle*, const Belle2::MCParticle*> MCParticleToMajorParticleMap;
      MCParticleToMajorParticleMap m_mcParticleToMajorParticleMap;


      typedef std::vector<const CDCWireHit*> MCTrack;
      typedef std::map<ITrackType, MCTrack> ITrackToMCTrackMap;
      ITrackToMCTrackMap m_iTrackToMCTrackMap;


      typedef std::map<Index, Index> IWireHitToIndexInTrackMap;
      IWireHitToIndexInTrackMap m_iWireHitToIndexInTrackMap;


      /* segments */
      typedef std::multimap<FlightTime, const CDCRecoSegment2D*> TimeSortedSegments;
      TimeSortedSegments m_timeSortedSegments;

      typedef std::list<const CDCRecoSegment2D*> MCSegmentTrack;
      typedef std::pair<ITrackType, ForwardBackwardInfo> ITrackAndFBInfo;
      typedef std::map<ITrackAndFBInfo, MCSegmentTrack> ITrackToMCSegmentTrackMap;
      ITrackToMCSegmentTrackMap m_iTrackToMCSegmentTrackMap;

      typedef std::map<const CDCRecoSegment2D*, Index> SegmentToIndexInTrackMap;
      SegmentToIndexInTrackMap m_segmentToIndexInTrackMap;

      struct SimHitTrackIdAndFlightTimeCompare {
        bool operator()(Belle2::CDCSimHit* const& lhs,  Belle2::CDCSimHit* const& rhs) const {
          if (lhs == NULL) return true; //NULL is always the smallest
          else if (rhs == NULL) return false;
          else return lhs->getTrackId() <  rhs->getTrackId() || (
                          lhs->getTrackId() == rhs->getTrackId() && lhs->getFlightTime() < rhs->getFlightTime());
        }
      };


      /** ROOT Macro to make CDCMCLookUp a ROOT class.*/
      //ClassDefInCDCLocalTracking(CDCMCLookUp, 1);

    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCMCLOOKUP
