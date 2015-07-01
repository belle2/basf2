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
#include <tracking/trackFindingCDC/filters/segment_track_chooser/BaseSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track_chooser/SimpleSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track/BaseSegmentTrackFilter.h>
#include <tracking/trackFindingCDC/filters/segment_track/SimpleSegmentTrackFilter.h>
#include <tracking/trackFindingCDC/filters/segment_train/BaseSegmentTrainFilter.h>
#include <tracking/trackFindingCDC/filters/segment_train/SimpleSegmentTrainFilter.h>
#include <tracking/trackFindingCDC/filters/background_segment/BaseBackgroundSegmentsFilter.h>
#include <tracking/trackFindingCDC/filters/background_segment/TMVABackgroundSegmentsFilter.h>
#include <tracking/trackFindingCDC/filters/new_segment/BaseNewSegmentsFilter.h>
#include <tracking/trackFindingCDC/filters/new_segment/TMVANewSegmentsFilter.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;

    /// Forward declaration of the module implementing the segment track combiner based on various filters
    template < class SegmentTrackChooserFirstStep = TrackFindingCDC::BaseSegmentTrackChooser,
               class BackgroundSegmentFilter = TrackFindingCDC::BaseBackgroundSegmentsFilter,
               class NewSegmentFilter = TrackFindingCDC::BaseNewSegmentsFilter,
               class SegmentTrackChooserSecondStep = TrackFindingCDC::BaseSegmentTrackChooser,
               class SegmentTrainFilter = TrackFindingCDC::BaseSegmentTrainFilter,
               class SegmentTrackFilter  = TrackFindingCDC::BaseSegmentTrackFilter>
    class SegmentTrackCombinerImplModule;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  typedef TrackFindingCDC::SegmentTrackCombinerImplModule <
  TrackFindingCDC::SimpleSegmentTrackChooser,
                  TrackFindingCDC::TMVABackgroundSegmentsFilter,
                  TrackFindingCDC::TMVANewSegmentsFilter,
                  TrackFindingCDC::SimpleSegmentTrackChooser,
                  TrackFindingCDC::SimpleSegmentTrainFilter,
                  TrackFindingCDC::SimpleSegmentTrackFilter
                  > SegmentTrackCombinerModule;

  namespace TrackFindingCDC {
    /// This module matches the found segments and the found tracks with a given filter.
    template < class SegmentTrackChooserFirstStep,
               class BackgroundSegmentFilter,
               class NewSegmentFilter,
               class SegmentTrackChooserSecondStep,
               class SegmentTrainFilter,
               class SegmentTrackFilter>
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
    };

    /// Do the combination work. See the SegmentTrackCombiner for more details.
    template < class SegmentTrackChooserFirstStep,
               class BackgroundSegmentFilter,
               class NewSegmentFilter,
               class SegmentTrackChooserSecondStep,
               class SegmentTrainFilter,
               class SegmentTrackFilter>
    void SegmentTrackCombinerImplModule<SegmentTrackChooserFirstStep,
         BackgroundSegmentFilter,
         NewSegmentFilter,
         SegmentTrackChooserSecondStep,
         SegmentTrainFilter,
         SegmentTrackFilter>::generate(
           std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks)
    {
      m_combiner.fillWith(tracks, segments);
      m_combiner.match(*m_ptrSegmentTrackChooserFirstStep);
      m_combiner.filter(*m_ptrBackgroundSegmentFilter);
      m_combiner.filterOutNewSegments(*m_ptrNewSegmentFilter);
      m_combiner.combine(*m_ptrSegmentTrackChooserSecondStep, *m_ptrSegmentTrainFilter, *m_ptrSegmentTrackFilter);
      m_combiner.clear();

      /*
       * REFITTING LEADS TO STRANGE RESULTS!
       * // Refit the tracks
      const CDCSZFitter& szFitter = CDCSZFitter::getFitter();
      const CDCRiemannFitter& xyFitter = CDCRiemannFitter::getFitter();

      for (CDCTrack& track : tracks) {
        CDCTrajectory2D trajectory2D;
        CDCObservations2D observations;
        for (const CDCRecoHit3D& recoHit : track) {
          observations.append(recoHit);
        }
        xyFitter.update(trajectory2D, observations);

        CDCTrajectorySZ szTrajectory;
        szFitter.update(szTrajectory, track);

        trajectory2D.setLocalOrigin(track.front().getRecoPos2D());
        if (trajectory2D.calcPerpS(track.back().getRecoPos2D()) < 0) {
          trajectory2D.reverse();
        }

       *if(szTrajectory.isFitted()) {
          CDCTrajectory3D trajectory3D(trajectory2D, szTrajectory);
          track.setStartTrajectory3D(trajectory3D);
        }



      }*/

      /*// Add the remaining segments
      for(CDCTrack & track : tracks) {
        const CDCTrajectory2D & trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
        const CDCTrajectorySZ & trajectorySZ = track.getStartTrajectory3D().getTrajectorySZ();
        double szSlope = trajectorySZ.getSZSlope();

        bool hasZInformation = szSlope != 0;

        if(hasZInformation) {
          for(const CDCRecoSegment2D & segment : segments) {
            if(segment.getAutomatonCell().hasTakenFlag())
              continue;

            for(const CDCRecoHit2D & recoHit2D : segment) {
              Vector3D reconstructedPosition = recoHit2D.reconstruct3D(trajectory2D);
              double perpS = recoHit2D.getPerpS(trajectory2D);

              B2INFO(trajectorySZ.getZDist(perpS, reconstructedPosition.z()));
            }
          }
        }
      }*/
    }
  }
}
