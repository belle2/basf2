#pragma once

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/numerics/Index.h>
#include <tracking/trackFindingCDC/mclookup/ITrackType.h>

#include <map>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCMCTrackCurlerCloneLookUp {

    public:
      /// Getter for the singletone instance
      static CDCMCTrackCurlerCloneLookUp& getInstance();

      /// Singleton: Delete copy constructor and assignment operator
      CDCMCTrackCurlerCloneLookUp(CDCMCTrackCurlerCloneLookUp&) = delete;
      CDCMCTrackCurlerCloneLookUp& operator=(const CDCMCTrackCurlerCloneLookUp&) = delete;

      void fillMCIDToCDCTracksMap(std::vector<CDCTrack>& cdcTracks);

      /// Takes a vector of pointers to CDCTracks which are matched to the same MC particle
      /// Returns track which is assumed to be not a clone.
      /// Assumes clones are from curlers, so here this means first curler arm.
      CDCTrack* findNonCurlerCloneCDCTrack(std::vector<CDCTrack*> matchedTrackPtrs);

      void fill(std::vector<CDCTrack>& cdcTracks);

      bool isTrackCurlerClone(const CDCTrack& cdcTrack);

    private:
      /// Singleton: Default ctor only available to getInstance method
      /// TODO: Manage singleton by MCManager? Then, ctor needs to be public.
      CDCMCTrackCurlerCloneLookUp() = default;

      /// Map of MCTrackIds to CDCTrack pointers
      /// Use that to find all tracks matched to the same MCTrack and decide which are clones.
      /// TODO: is purity threshold for "matched" definition okay?
      std::map<const ITrackType, std::vector<CDCTrack*>> m_mcTrackIDToCDCTracksMap;


      /// Map of track pointers to isClone indicator from MCTruth-based assumption
      std::map<const CDCTrack*, bool> m_cdcTrackIsCurlerCloneMap;

    };
  }
}
