/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/Lookups.h>

#include <tracking/trackFindingCDC/filters/segment_train/SegmentTrainFilter.h>
#include <tracking/trackFindingCDC/filters/segment_train/SimpleSegmentTrainFilter.h>
#include <tracking/trackFindingCDC/filters/segment_information_list_track/SegmentInformationListTrackFilter.h>
#include <tracking/trackFindingCDC/filters/segment_information_list_track/SimpleSegmentInformationListTrackFilter.h>
#include <tracking/trackFindingCDC/filters/background_segment/BackgroundSegmentsFilter.h>
#include <tracking/trackFindingCDC/filters/new_segment/NewSegmentsFilter.h>
#include <tracking/trackFindingCDC/filters/segment_track/SegmentTrackFilter.h>
#include <tracking/trackFindingCDC/filters/segment_track/SimpleSegmentTrackFilter.h>
#include <tracking/trackFindingCDC/filters/track/TrackFilter.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;

    /// Forward declaration of the module implementing the segment track combiner based on various filters
    template < class SegmentTrackChooserFirstStep = TrackFindingCDC::BaseSegmentTrackFilter,
               class BackgroundSegmentFilter = TrackFindingCDC::BaseBackgroundSegmentsFilter,
               class NewSegmentFilter = TrackFindingCDC::BaseNewSegmentsFilter,
               class SegmentTrackChooserSecondStep = TrackFindingCDC::BaseSegmentTrackFilter,
               class SegmentTrainFilter = TrackFindingCDC::BaseSegmentTrainFilter,
               class SegmentTrackFilter  = TrackFindingCDC::BaseSegmentInformationListTrackFilter,
               class TrackFilter  = TrackFindingCDC::BaseTrackFilter>
    class SegmentTrackCombinerImplModule;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  typedef TrackFindingCDC::SegmentTrackCombinerImplModule <
  TrackFindingCDC::TMVASegmentTrackFilter,
                  TrackFindingCDC::TMVABackgroundSegmentsFilter,
                  TrackFindingCDC::TMVANewSegmentsFilter,
                  TrackFindingCDC::BaseSegmentTrackFilter, // = No
                  TrackFindingCDC::BaseSegmentTrainFilter, // = No
                  TrackFindingCDC::BaseSegmentInformationListTrackFilter, // = No
                  TrackFindingCDC::TMVATrackFilter
                  > SegmentTrackCombinerModule;

  namespace TrackFindingCDC {
    /// This module matches the found segments and the found tracks with a given filter.
    template < class SegmentTrackChooserFirstStep,
               class BackgroundSegmentFilter,
               class NewSegmentFilter,
               class SegmentTrackChooserSecondStep,
               class SegmentTrainFilter,
               class SegmentTrackFilter,
               class TrackFilter>
    class SegmentTrackCombinerImplModule : public TrackFinderCDCFromSegmentsModule {

    public:
      /**
       * Constructor.
       */
      SegmentTrackCombinerImplModule(): TrackFinderCDCFromSegmentsModule(),
        m_combiner(),
        m_ptrSegmentTrackChooserFirstStep(new SegmentTrackChooserFirstStep),
        m_ptrBackgroundSegmentFilter(new BackgroundSegmentFilter),
        m_ptrNewSegmentFilter(new NewSegmentFilter),
        m_ptrSegmentTrackChooserSecondStep(new SegmentTrackChooserSecondStep),
        m_ptrSegmentTrainFilter(new SegmentTrainFilter),
        m_ptrSegmentTrackFilter(new SegmentTrackFilter) { }

      /** Initialize the filter */
      void initialize() override
      {
        TrackFinderCDCFromSegmentsModule::initialize();

        if (m_ptrSegmentTrackChooserFirstStep) {
          m_ptrSegmentTrackChooserFirstStep->initialize();
        }

        if (m_ptrBackgroundSegmentFilter) {
          m_ptrBackgroundSegmentFilter->initialize();
        }

        if (m_ptrNewSegmentFilter) {
          m_ptrNewSegmentFilter->initialize();
        }

        if (m_ptrSegmentTrackChooserSecondStep) {
          m_ptrSegmentTrackChooserSecondStep->initialize();
        }

        if (m_ptrSegmentTrainFilter) {
          m_ptrSegmentTrainFilter->initialize();
        }

        if (m_ptrSegmentTrackFilter) {
          m_ptrSegmentTrackFilter->initialize();
        }

        if (m_ptrTrackFilter) {
          m_ptrTrackFilter->initialize();
        }
      }

      /** Terminate the filter */
      void terminate() override
      {
        TrackFinderCDCFromSegmentsModule::terminate();

        if (m_ptrSegmentTrackChooserFirstStep) {
          m_ptrSegmentTrackChooserFirstStep->terminate();
        }

        if (m_ptrBackgroundSegmentFilter) {
          m_ptrBackgroundSegmentFilter->terminate();
        }

        if (m_ptrNewSegmentFilter) {
          m_ptrNewSegmentFilter->terminate();
        }

        if (m_ptrSegmentTrackChooserSecondStep) {
          m_ptrSegmentTrackChooserSecondStep->terminate();
        }

        if (m_ptrSegmentTrainFilter) {
          m_ptrSegmentTrainFilter->terminate();
        }

        if (m_ptrSegmentTrackFilter) {
          m_ptrSegmentTrackFilter->terminate();
        }

        if (m_ptrTrackFilter) {
          m_ptrTrackFilter->terminate();
        }
      }

    private:
      /**
       * Try to combine the segments and the tracks
       */
      void generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks) override;

    public:
      /// Getter for the current SegmentTrackChooser. The module keeps ownership of the pointer.
      SegmentTrackChooserFirstStep* getSegmentTrackChooserFirstStep()
      {
        return m_ptrSegmentTrackChooserFirstStep.get();
      }

      /// Setter for the SegmentTrackChooser. The module takes ownership of the pointer.
      void setSegmentTrackChooserFirstStep(std::unique_ptr<SegmentTrackChooserFirstStep> ptrSegmentTrackChooserFirstStep)
      {
        m_ptrSegmentTrackChooserFirstStep = std::move(ptrSegmentTrackChooserFirstStep);
      }

      /// Getter for the current SegmentTrackChooser. The module keeps ownership of the pointer.
      SegmentTrackChooserSecondStep* getSegmentTrackChooserSecondStep()
      {
        return m_ptrSegmentTrackChooserSecondStep.get();
      }

      /// Setter for the SegmentTrackChooser. The module takes ownership of the pointer.
      void setSegmentTrackChooserSecondStep(std::unique_ptr<SegmentTrackChooserSecondStep> ptrSegmentTrackChooserSecondStep)
      {
        m_ptrSegmentTrackChooserSecondStep = std::move(ptrSegmentTrackChooserSecondStep);
      }

      /// Getter for the current SegmentTrainFilter. The module keeps ownership of the pointer.
      SegmentTrainFilter* getSegmentTrainFilter()
      {
        return m_ptrSegmentTrainFilter.get();
      }

      /// Setter for the SegmentTrainFilter. The module takes ownership of the pointer.
      void setSegmentTrainFilter(std::unique_ptr<SegmentTrainFilter> ptrFacetFilter)
      {
        m_ptrSegmentTrainFilter = std::move(ptrFacetFilter);
      }

      /// Getter for the current SegmentTrackFilter. The module keeps ownership of the pointer.
      SegmentTrackFilter* getSegmentTrackFilter()
      {
        return m_ptrSegmentTrackFilter.get();
      }

      /// Setter for the SegmentTrackFilter. The module takes ownership of the pointer.
      void setSegmentTrackFilter(std::unique_ptr<SegmentTrackFilter> ptrFacetNeighborChooser)
      {
        m_ptrSegmentTrackFilter = std::move(ptrFacetNeighborChooser);
      }

      /// Getter for the current BackgroundSegmentFilter. The module keeps ownership of the pointer.
      BackgroundSegmentFilter* getBackgroundSegmentFilter()
      {
        return m_ptrBackgroundSegmentFilter.get();
      }

      /// Setter for the BackgroundSegmentFilter. The module takes ownership of the pointer.
      void setBackgroundSegmentFilter(std::unique_ptr<BackgroundSegmentFilter> ptrBackgroundSegmentFilter)
      {
        m_ptrBackgroundSegmentFilter = std::move(ptrBackgroundSegmentFilter);
      }

      /// Getter for the current NewSegmentFilter. The module keeps ownership of the pointer.
      NewSegmentFilter* getNewSegmentFilter()
      {
        return m_ptrNewSegmentFilter.get();
      }

      /// Setter for the NewSegmentFilter. The module takes ownership of the pointer.
      void setNewSegmentFilter(std::unique_ptr<NewSegmentFilter> ptrNewSegmentFilter)
      {
        m_ptrNewSegmentFilter = std::move(ptrNewSegmentFilter);
      }

      /// Getter for the current TrackFilter. The module keeps ownership of the pointer.
      TrackFilter* getTrackFilter()
      {
        return m_ptrTrackFilter.get();
      }

      /// Setter for the TrackFilter. The module takes ownership of the pointer.
      void setTrackFilter(std::unique_ptr<TrackFilter> ptrTrackFilter)
      {
        m_ptrTrackFilter = std::move(ptrTrackFilter);
      }

    private:
      /// Object that handles the combination
      SegmentTrackCombiner m_combiner;

      /// Reference to the chooser to be used for matching segments and tracks in the first step
      std::unique_ptr<SegmentTrackChooserFirstStep> m_ptrSegmentTrackChooserFirstStep;

      /// Reference to the background segment filter
      std::unique_ptr<BackgroundSegmentFilter> m_ptrBackgroundSegmentFilter;

      /// Reference to the new segment filter
      std::unique_ptr<NewSegmentFilter> m_ptrNewSegmentFilter;

      /// Reference to the chooser to be used for matching segments and tracks in the second step
      std::unique_ptr<SegmentTrackChooserSecondStep> m_ptrSegmentTrackChooserSecondStep;

      /// Reference to the filter to be used for construction segment trains
      std::unique_ptr<SegmentTrainFilter> m_ptrSegmentTrainFilter;

      /// Reference to the filter to be used to do an uniqe segment train <-> track matching
      std::unique_ptr<SegmentTrackFilter> m_ptrSegmentTrackFilter;

      /// Reference to the filter to be used to filter out fake tracks
      std::unique_ptr<TrackFilter> m_ptrTrackFilter;
    };

    /// Do the combination work. See the SegmentTrackCombiner for more details.
    template < class SegmentTrackChooserFirstStep,
               class BackgroundSegmentFilter,
               class NewSegmentFilter,
               class SegmentTrackChooserSecondStep,
               class SegmentTrainFilter,
               class SegmentTrackFilter,
               class TrackFilter>
    void SegmentTrackCombinerImplModule<SegmentTrackChooserFirstStep,
         BackgroundSegmentFilter,
         NewSegmentFilter,
         SegmentTrackChooserSecondStep,
         SegmentTrainFilter,
         SegmentTrackFilter,
         TrackFilter>::generate(
           std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks)
    {
      m_combiner.fillWith(tracks, segments);
      m_combiner.match(*m_ptrSegmentTrackChooserFirstStep);
      m_combiner.filter(*m_ptrBackgroundSegmentFilter);
      m_combiner.filterOutNewSegments(*m_ptrNewSegmentFilter);
      m_combiner.combine(*m_ptrSegmentTrackChooserSecondStep, *m_ptrSegmentTrainFilter, *m_ptrSegmentTrackFilter);

      m_combiner.clear();

      for (CDCTrack& track : tracks) {
        const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
        const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
        const CDCTrajectorySZ& trajectorySZ = trajectory3D.getTrajectorySZ();
        const double radius = fabs(trajectory2D.getGlobalCircle().radius());

        // The first hit has - per definition of the trajectory2D - a perpS of 0. We want every other hit to have a perpS greater than 0,
        // especially for curlers. For this, we go through all hits and look for negative perpS. If we have found one, we shift it to positive values
        for (CDCRecoHit3D& recoHit : track) {
          double currentPerpS = recoHit.getPerpS();
          if (currentPerpS < 0) {
            recoHit.setPerpS(2 * TMath::Pi() * radius + currentPerpS);
          }
        }

        track.sortByPerpS();
      }

      // Filter out bad tracks
      tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [this](const CDCTrack & track) -> bool {
        double filterResult = m_ptrTrackFilter->operator()(track);
        return isNotACell(filterResult);
      }));

      // Recover the new segments
      for (CDCRecoSegment2D& segment : segments) {
        if (segment.getAutomatonCell().hasAssignedFlag()) {
          segment.getAutomatonCell().unsetTakenFlag();
        }
      }

    }
  }
}
