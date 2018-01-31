#pragma once

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/numerics/Index.h>
#include <tracking/trackFindingCDC/mclookup/ITrackType.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

#include <map>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCMCCurlerCloneLookUp {

    public:
      /// Getter for the singletone instance
      static CDCMCCurlerCloneLookUp& getInstance();
      /// Singleton: Delete copy constructor and assignment operator
      CDCMCCurlerCloneLookUp(CDCMCCurlerCloneLookUp&) = delete;
      CDCMCCurlerCloneLookUp& operator=(const CDCMCCurlerCloneLookUp&) = delete;

      /// fill with all cdcTracks in an event
      void fill(std::vector<CDCTrack>& cdcTracks);

      /// Clear eventwise lookup tables
      void clear();

      /// getter for information from m_cdcTrackIsCurlerCloneMap
      bool isTrackCurlerClone(const CDCTrack& cdcTrack);

    private:
      /// Singleton: Default ctor only available to getInstance method
      /// TODO: Manage singleton by MCManager? Then, ctor needs to be public.
      CDCMCCurlerCloneLookUp() = default;

      /// Helper function which returns a map of MCTrackIDs to vectors of CDTrack pointers.
      /// Need that to find clone candidates: tracks with same MCTrackID
      std::map<const ITrackType, std::vector<CDCTrack*>> getMatchedCDCTracksByMCID(
                                                        std::vector<CDCTrack>& cdcTracks);

      /// Helper function which takes a vector of pointers to CDCTracks which are matched to the
      /// same MC particle.
      /// Returns track ptr which is assumed to be not a clone from a curler.
      CDCTrack* findNonCurlerCloneTrack(std::vector<CDCTrack*> matchedTrackPtrs);

      /// Map of track pointers to isClone indicator from MCTruth-based assumption
      std::map<const CDCTrack*, bool> m_cdcTrackIsCurlerCloneMap;
    };

    // Compare Functor which which decides from two tracks to declare non-clone
    struct CompareCurlerTracks {
      /// marker function for the isFunctor test
      operator FunctorTag();

      CompareCurlerTracks(const CDCMCTrackLookUp& cdcMCTrackLookUp,
                          const CDCMCHitLookUp& cdcMCHitLookUp)
        : m_CDCMCTrackLookUp(cdcMCTrackLookUp)
        , m_CDCMCHitLookUp(cdcMCHitLookUp) {};

      // Return true if Track1 has a lower NLoops of first hit than Track1, in the reversed case
      // return false
      // If both tracks have the same firstNLoops, return true if Track1 has larger number hits.
      bool operator()(const CDCTrack* ptrCDCTrack1, const CDCTrack* ptrCDCTrack2) const;

    private:
      /// Get number of hits in track that are correctly matched
      /// TODO: maybe implement this method in HitCollectionLookUp?
      unsigned int getNumberOfCorrectHits(const CDCTrack* ptrCDCTrack) const;

      const CDCMCTrackLookUp& m_CDCMCTrackLookUp;
      const CDCMCHitLookUp& m_CDCMCHitLookUp;
    };
  }
}
