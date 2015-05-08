/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>

#include <framework/core/Module.h>
#include <tracking/modules/trackFinderCDC/TrackFinderCDCFromSegmentsModule.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;
  }

  class SegmentTrackCombinerModule : public TrackFinderCDCFromSegmentsModule {

    class SegmentInformation;
    typedef std::vector<SegmentInformation*> ListOfSegmentPointer;

    class TrackInformation {
    public:
      TrackInformation(genfit::TrackCand* trackCand) :
        m_trackCand(trackCand), m_trajectory(), m_perpSList(), m_minPerpS(0), m_maxPerpS(0),
        m_matchingSegments()
      {
        m_matchingSegments.reserve(5);
      }

      double getMaxPerpS() const
      {
        return m_maxPerpS;
      }

      double getMinPerpS() const
      {
        return m_minPerpS;
      }

      std::vector<double>& getPerpSList()
      {
        return m_perpSList;
      }

      const std::vector<double>& getPerpSList() const
      {
        return m_perpSList;
      }

      genfit::TrackCand* getTrackCand() const
      {
        return m_trackCand;
      }

      const TrackFindingCDC::CDCTrajectory2D& getTrajectory() const
      {
        return m_trajectory;
      }

      void setTrajectory(const TrackFindingCDC::CDCTrajectory2D& trajectory)
      {
        m_trajectory = trajectory;
      }

      void calcPerpS()
      {
        std::sort(m_perpSList.begin(), m_perpSList.end());
        m_minPerpS = m_perpSList.front();
        m_maxPerpS = m_perpSList.back();
      }

      ListOfSegmentPointer& getMatchingSegments()
      {
        return m_matchingSegments;
      }

      void clearMatchingSegments()
      {
        m_matchingSegments.clear();
      }

    private:
      genfit::TrackCand* m_trackCand;
      TrackFindingCDC::CDCTrajectory2D m_trajectory;
      std::vector<double> m_perpSList;
      double m_minPerpS;
      double m_maxPerpS;
      ListOfSegmentPointer m_matchingSegments;
    };

    class SegmentInformation {
    public:
      SegmentInformation(TrackFindingCDC::CDCRecoSegment2D* segment) :
        m_segment(segment), m_matchingTracks(), m_usedInTrack(false)
      {
        m_matchingTracks.reserve(5);
      }

      TrackFindingCDC::CDCRecoSegment2D* getSegment() const
      {
        return m_segment;
      }

      std::vector<TrackInformation*>& getMatchingTracks()
      {
        return m_matchingTracks;
      }

      void clearMatchingTracks()
      {
        m_matchingTracks.clear();
      }

      bool isUsedInTrack() const
      {
        return m_usedInTrack;
      }

      void setUsedInTrack(bool usedInTrack = true)
      {
        m_usedInTrack = usedInTrack;
      }

    private:
      TrackFindingCDC::CDCRecoSegment2D* m_segment;
      std::vector<TrackInformation*> m_matchingTracks;
      bool m_usedInTrack;
    };


  public:

    /**
     * Constructor.
     */
    SegmentTrackCombinerModule();

    /**
     * Check for the given StoreArray also
     */
    void initialize() override;

    /**
     * Used the QuadTree to generate tracks from segments.
     */
    void generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks) override;


  private:
    std::string m_param_legendreTrackCandsStoreArrayName;
    const float m_param_percentageForPerpSMeasurements = 0.05;
    const float m_param_minimalFitProbability = 0.5;

    // Object pools
    typedef std::vector<SegmentInformation> SegmentsList;
    std::vector<SegmentsList> m_segmentLookUp;
    std::vector<TrackInformation> m_trackLookUp;

    StoreArray<genfit::TrackCand> m_legendreTrackCands;
    StoreArray<CDCHit> m_cdcHits;

    bool segmentMatchesToTrack(const SegmentInformation& segmentInformation, const TrackInformation& trackInformation);
    void makeAllCombinations(std::list<ListOfSegmentPointer>& trainsOfSegments, const ListOfSegmentPointer& matchedSegments,
                             const TrackInformation& trackInformation);
    bool doesFitTogether(const ListOfSegmentPointer& list, const TrackInformation& trackInformation);
    void combineSegmentTrainAndTrack(const ListOfSegmentPointer& trainOfSegments);
    void calculateSegmentLookUp(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments);
    void calculateTrackLookup();
    void createTracksForOutput(std::vector<TrackFindingCDC::CDCTrack>& tracks);
    void matchTracksToSegments(unsigned int superLayerCounter, SegmentInformation& segmentInformation);
    void createTrainsOfSegments(std::list<ListOfSegmentPointer>& trainsOfSegments, TrackInformation& trackInformation);
    void addSegmentToTrack(SegmentInformation* segmentInformation, TrackInformation* matchingTracks);
    double testFitSegmentToTrack(SegmentInformation* segmentInformation, TrackInformation* trackInformation);
  };
}
