/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/mclookup/ITrackType.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

#include <map>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Class providing information whether a PR CDC track is the best match or a clone
     *
     * In contrast to all other MC lookup classes, it requires the information of all tracks in an
     * event to decide which are clones. Therofore, it has to be filled once with a CDCTrack vector
     * before being used.
     *
     * The definition of which track is a better match and thus which to classify * as a clone is
     * encoded in the separate functor CompareCDCTracks.
     */
    class CDCMCCloneLookUp {

    public:
      /// Getter for the singletone instance
      static CDCMCCloneLookUp& getInstance();
      /// Singleton: Delete copy constructor and assignment operator
      CDCMCCloneLookUp(CDCMCCloneLookUp&) = delete;
      CDCMCCloneLookUp& operator=(const CDCMCCloneLookUp&) = delete;

      /// fill with all cdcTracks in an event
      void fill(std::vector<CDCTrack>& cdcTracks);

      /// Clear eventwise lookup tables
      void clear();

      /// getter for information from m_cdcTrackIsCloneMap
      bool isTrackClone(const CDCTrack& cdcTrack);

    private:
      /// Singleton: Default ctor only available to getInstance method
      CDCMCCloneLookUp() = default;

      /// Helper function which returns a map of MCTrackIDs to vectors of CDTrack pointers.
      /// Need that to find clone candidates: tracks with same MCTrackID
      std::map<const ITrackType, std::vector<CDCTrack*>> getMatchedCDCTracksByMCID(
                                                        std::vector<CDCTrack>& cdcTracks);

      /// Helper function which takes a vector of pointers to CDCTracks which are matched to the
      /// same MC particle.
      /// Returns track ptr which is assumed to be not a clone.
      CDCTrack* findBestMatchedTrack(std::vector<CDCTrack*> matchedTrackPtrs);

      /// Map of track pointers to isClone indicator from MCTruth-based assumption
      std::map<const CDCTrack*, bool> m_cdcTrackIsCloneMap;
    };

    /// Functor which which decides which of two tracks to declare as best match
    struct CompareCDCTracks {
      /// marker function for the isFunctor test
      operator FunctorTag();

      /// Constructor of the CDC track comparer to get the better match, taking references to MC
      /// lookup singletons.
      CompareCDCTracks(const CDCMCTrackLookUp& cdcMCTrackLookUp,
                       const CDCMCHitLookUp& cdcMCHitLookUp)
        : m_CDCMCTrackLookUp(cdcMCTrackLookUp)
        , m_CDCMCHitLookUp(cdcMCHitLookUp) {};


      /** Compare both CDC tracks to get the better matched one.
       *
       * Returns true if track1 has a lower loop number at the first hit than track1.
       * If both tracks have the same loop number, return true if track1 has larger number of matched hits.
       */
      bool operator()(const CDCTrack* ptrCDCTrack1, const CDCTrack* ptrCDCTrack2) const;

    private:
      /// Get number of hits in track that are correctly matched
      unsigned int getNumberOfCorrectHits(const CDCTrack* ptrCDCTrack) const;

      /// Reference to a CDCMCTrackLookUp instance, assigned in the constructor of this functor
      const CDCMCTrackLookUp& m_CDCMCTrackLookUp;
      /// Reference to a CDCMCHitLookUp instance, assigned in the constructor of this functor
      const CDCMCHitLookUp& m_CDCMCHitLookUp;
    };
  } // namespace TrackFindingCDC
} // namespace Belle2
