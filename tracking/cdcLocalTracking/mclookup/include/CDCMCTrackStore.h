/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCMCTRACKSTORE_H
#define CDCMCTRACKSTORE_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

#include <cdc/dataobjects/CDCHit.h>
#include <map>
#include <vector>

namespace Belle2 {
  namespace CDCLocalTracking {

    ///Class to organize and present the monte carlo hit information
    class CDCMCTrackStore : public UsedTObject {

    public:
      /// Type for an ordered sequence of pointers to the CDCHit
      typedef std::vector<const CDCHit*> CDCHitVector;

    public:
      /// Default constructor
      CDCMCTrackStore();

      /// Empty destructor
      ~CDCMCTrackStore();

    public:
      /// Getter for the singletone instance
      static CDCMCTrackStore& getInstance();

    public:
      /// Clear all Monte Carlo hits
      void clear();

      /** Fill the store with the tracks from Monte Carlo information.
       *  It uses the data store store to construct the Monte Carlo tracks.
       */
      void fill();

    public:
      /// Getter for the stored Monte Carlo tracks ordered by their Monte Carlo Id
      const std::map<int, Belle2::CDCLocalTracking::CDCMCTrackStore::CDCHitVector>& getMCTracksByMCParticleIdx() const
      { return  m_mcTracksByMCParticleIdx;}

      /// Getter for the stored Monte Carlo segments ordered by their Monte Carlo Id
      const std::map<int, std::vector<Belle2::CDCLocalTracking::CDCMCTrackStore::CDCHitVector> >& getMCSegmentsByMCParticleIdx() const
      { return  m_mcSegmentsByMCParticleIdx;}

    private:
      void fillMCTracks();
      void fillMCSegments();

      void arrangeMCTrack(CDCHitVector& mcTrack) const;

      void fillInTrackId();
      void fillInTrackSegmentId();
      void fillNPassedSuperLayers();

    public:
      int getInTrackId(const CDCHit* hit) const;
      int getInTrackSegmentId(const CDCHit* hit) const;
      int getNPassedSuperLayers(const CDCHit* hit) const;

    private:
      std::map<int, CDCHitVector> m_mcTracksByMCParticleIdx;
      std::map<int, std::vector<CDCHitVector>> m_mcSegmentsByMCParticleIdx;

      std::map<const CDCHit*, int> m_inTrackIds;
      std::map<const CDCHit*, int> m_inTrackSegmentIds;
      std::map<const CDCHit*, int> m_nPassedSuperLayers;


      /** ROOT Macro to make CDCMCTrackStore a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCMCTrackStore, 1);


    }; //class CDCMCTrackStore
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCMCTRACKSTORE
