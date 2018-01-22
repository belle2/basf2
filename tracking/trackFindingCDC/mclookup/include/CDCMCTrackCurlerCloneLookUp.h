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

      /// fill with all cdcTracks in an event
      void fill(std::vector<CDCTrack>& cdcTracks);

      /// Clear eventwise lookup tables
      void clear();

      /// getter for information from m_cdcTrackIsCurlerCloneMap
      bool isTrackCurlerClone(const CDCTrack& cdcTrack);

    private:
      /// Singleton: Default ctor only available to getInstance method
      /// TODO: Manage singleton by MCManager? Then, ctor needs to be public.
      CDCMCTrackCurlerCloneLookUp() = default;

      std::map<const ITrackType, std::vector<CDCTrack*>> getMapMCIDToCDCTracks(std::vector<CDCTrack>& cdcTracks);

      /// Takes a vector of pointers to CDCTracks which are matched to the same MC particle
      /// Returns track which is assumed to be not a clone.
      CDCTrack* findNonCurlerCloneCDCTrack(std::vector<CDCTrack*> matchedTrackPtrs);

      /// Map of track pointers to isClone indicator from MCTruth-based assumption
      std::map<const CDCTrack*, bool> m_cdcTrackIsCurlerCloneMap;
    };
  }
}
