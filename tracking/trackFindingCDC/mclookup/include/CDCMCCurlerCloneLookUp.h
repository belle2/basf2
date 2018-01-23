#pragma once

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/numerics/Index.h>
#include <tracking/trackFindingCDC/mclookup/ITrackType.h>

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

      std::map<const ITrackType, std::vector<CDCTrack*>> getMapMCIDToCDCTracks(std::vector<CDCTrack>& cdcTracks);

      /// Takes a vector of pointers to CDCTracks which are matched to the same MC particle
      /// Returns track ptr which is assumed to be not a clone from a curler.
      CDCTrack* findNonCurlerCloneTrack(std::vector<CDCTrack*> matchedTrackPtrs);

      /// Map of track pointers to isClone indicator from MCTruth-based assumption
      std::map<const CDCTrack*, bool> m_cdcTrackIsCurlerCloneMap;
    };
    //
    // Compare Functor with decision logic, which from multiple curler tracks to use
    struct CompareCurlerTracks {
      /// marker function for the isFunctor test
      operator FunctorTag();

      CompareCurlerTracks() = default;

      bool operator()(const CDCTrack* ptrTrack1,  const CDCTrack* ptrTrack2) const;
    };
  }
}
