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
#include <tracking/trackFindingCDC/trackFinderOutputCombining/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>

#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;
  }

  namespace TrackFindingCDC {
    /** Module for the combination of tracks and segments. Development edition.
     *
     * This module uses configurable filters to create matches between segments (from the local finder)
     * and tracks (from the global finder). It also has capabilities to filter the remaining tracks for fakes
     * and the remaining segments for new tracks/background.
     *
     * Use this module at the end of the path, after you have run the local and the global track finder.
     *
     * It uses several (filtering) steps, to do its job, namely:
     *  1. Creation of a fast segment and track lookup.
     *  2. First matching of segment and tracks that share one or more hits. \label{list-start}
     *  3. Deletion of fake segments. \label{list-fakes}
     *  4. Flagging of segments belonging to particles that probably can not be found by the global track finder (and can therefore not be matched).
     *  5. Matching of the remaining segments with the tracks or among themselves and then with the tracks (not used in the following). \label{list-second}
     *  6. Filtering of fake tracks in the made combinations.  \label{list-end}
     *  7. Cleanup of the lookup cache.
     */
    class SegmentTrackCombinerImplModule : public TrackFinderCDCFromSegmentsModule {

    public:
      /**
       * Constructor.
       */
      SegmentTrackCombinerImplModule(): TrackFinderCDCFromSegmentsModule(),
        m_combiner() { }

      /** Initialize the filter. */
      void initialize() override
      {
        TrackFinderCDCFromSegmentsModule::initialize();

        if (m_ptrSegmentTrackFilterFirstStep) {
          m_ptrSegmentTrackFilterFirstStep->initialize();
        }

        if (m_ptrBackgroundSegmentFilter) {
          m_ptrBackgroundSegmentFilter->initialize();
        }

        if (m_ptrNewSegmentFilter) {
          m_ptrNewSegmentFilter->initialize();
        }

        if (m_ptrSegmentTrackFilterSecondStep) {
          m_ptrSegmentTrackFilterSecondStep->initialize();
        }

        if (m_ptrSegmentTrainFilter) {
          m_ptrSegmentTrainFilter->initialize();
        }

        if (m_ptrSegmentInformationListTrackFilter) {
          m_ptrSegmentInformationListTrackFilter->initialize();
        }

        if (m_ptrTrackFilter) {
          m_ptrTrackFilter->initialize();
        }
      }

      /** Terminate the filter. */
      void terminate() override
      {
        TrackFinderCDCFromSegmentsModule::terminate();

        if (m_ptrSegmentTrackFilterFirstStep) {
          m_ptrSegmentTrackFilterFirstStep->terminate();
        }

        if (m_ptrBackgroundSegmentFilter) {
          m_ptrBackgroundSegmentFilter->terminate();
        }

        if (m_ptrNewSegmentFilter) {
          m_ptrNewSegmentFilter->terminate();
        }

        if (m_ptrSegmentTrackFilterSecondStep) {
          m_ptrSegmentTrackFilterSecondStep->terminate();
        }

        if (m_ptrSegmentTrainFilter) {
          m_ptrSegmentTrainFilter->terminate();
        }

        if (m_ptrSegmentInformationListTrackFilter) {
          m_ptrSegmentInformationListTrackFilter->terminate();
        }

        if (m_ptrTrackFilter) {
          m_ptrTrackFilter->terminate();
        }
      }

    private:
      /**
       * Try to combine the segments and the tracks.
       */
      void generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks) override;

    public:
      /// Getter for the current SegmentTrackChooser. The module keeps ownership of the pointer.
      BaseSegmentTrackFilter* getSegmentTrackFilterFirstStep()
      {
        return m_ptrSegmentTrackFilterFirstStep.get();
      }

      /// Setter for the SegmentTrackFilter for the first step. The module takes ownership of the pointer.
      void setSegmentTrackFilterFirstStep(std::unique_ptr<BaseSegmentTrackFilter> ptrBaseSegmentTrackFilter)
      {
        m_ptrSegmentTrackFilterFirstStep = std::move(ptrBaseSegmentTrackFilter);
      }

      /// Getter for the current SegmentTrackFilter for the second step. The module keeps ownership of the pointer.
      BaseSegmentTrackFilter* getSegmentTrackFilterSecondStep()
      {
        return m_ptrSegmentTrackFilterSecondStep.get();
      }

      /// Setter for the SegmentTrackChooser. The module takes ownership of the pointer.
      void setSegmentTrackFilterSecondStep(std::unique_ptr<BaseSegmentTrackFilter> ptrSegmentTrackFilterSecondStep)
      {
        m_ptrSegmentTrackFilterSecondStep = std::move(ptrSegmentTrackFilterSecondStep);
      }

      /// Getter for the current SegmentTrainFilter. The module keeps ownership of the pointer.
      BaseSegmentTrainFilter* getSegmentTrainFilter()
      {
        return m_ptrSegmentTrainFilter.get();
      }

      /// Setter for the SegmentTrainFilter. The module takes ownership of the pointer.
      void setSegmentTrainFilter(std::unique_ptr<BaseSegmentTrainFilter> ptrSegmentTrainFilter)
      {
        m_ptrSegmentTrainFilter = std::move(ptrSegmentTrainFilter);
      }

      /// Getter for the current SegmentInformationListTrackFilter. The module keeps ownership of the pointer.
      BaseSegmentInformationListTrackFilter* getSegmentInformationListTrackFilter()
      {
        return m_ptrSegmentInformationListTrackFilter.get();
      }

      /// Setter for the SegmentInformationListTrackFilter. The module takes ownership of the pointer.
      void setSegmentInformationListTrackFilter(std::unique_ptr<BaseSegmentInformationListTrackFilter>
                                                ptrSegmentInformationListTrackFilter)
      {
        m_ptrSegmentInformationListTrackFilter = std::move(ptrSegmentInformationListTrackFilter);
      }

      /// Getter for the current BackgroundSegmentFilter. The module keeps ownership of the pointer.
      BaseBackgroundSegmentsFilter* getBackgroundSegmentFilter()
      {
        return m_ptrBackgroundSegmentFilter.get();
      }

      /// Setter for the BackgroundSegmentFilter. The module takes ownership of the pointer.
      void setBackgroundSegmentFilter(std::unique_ptr<BaseBackgroundSegmentsFilter> ptrBackgroundSegmentFilter)
      {
        m_ptrBackgroundSegmentFilter = std::move(ptrBackgroundSegmentFilter);
      }

      /// Getter for the current NewSegmentFilter. The module keeps ownership of the pointer.
      BaseNewSegmentsFilter* getNewSegmentFilter()
      {
        return m_ptrNewSegmentFilter.get();
      }

      /// Setter for the NewSegmentFilter. The module takes ownership of the pointer.
      void setNewSegmentFilter(std::unique_ptr<BaseNewSegmentsFilter> ptrNewSegmentFilter)
      {
        m_ptrNewSegmentFilter = std::move(ptrNewSegmentFilter);
      }

      /// Getter for the current TrackFilter. The module keeps ownership of the pointer.
      BaseTrackFilter* getTrackFilter()
      {
        return m_ptrTrackFilter.get();
      }

      /// Setter for the TrackFilter. The module takes ownership of the pointer.
      void setTrackFilter(std::unique_ptr<BaseTrackFilter> ptrTrackFilter)
      {
        m_ptrTrackFilter = std::move(ptrTrackFilter);
      }

    private:
      /// Object that handles the combination.
      SegmentTrackCombiner m_combiner;

      /// Reference to the chooser to be used for matching segments and tracks in the first step.
      std::unique_ptr<BaseSegmentTrackFilter> m_ptrSegmentTrackFilterFirstStep;

      /// Reference to the background segment filter.
      std::unique_ptr<BaseBackgroundSegmentsFilter> m_ptrBackgroundSegmentFilter;

      /// Reference to the new segment filter.
      std::unique_ptr<BaseNewSegmentsFilter> m_ptrNewSegmentFilter;

      /// Reference to the chooser to be used for matching segments and tracks in the second step.
      std::unique_ptr<BaseSegmentTrackFilter> m_ptrSegmentTrackFilterSecondStep;

      /// Reference to the filter to be used for construction segment trains.
      std::unique_ptr<BaseSegmentTrainFilter> m_ptrSegmentTrainFilter;

      /// Reference to the filter to be used to do an unique segment train <-> track matching.
      std::unique_ptr<BaseSegmentInformationListTrackFilter> m_ptrSegmentInformationListTrackFilter;

      /// Reference to the filter to be used to filter out fake tracks.
      std::unique_ptr<BaseTrackFilter> m_ptrTrackFilter;
    };
  }
}
