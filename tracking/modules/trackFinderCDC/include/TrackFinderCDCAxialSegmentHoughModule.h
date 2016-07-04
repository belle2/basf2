/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorSegmentHough.h>
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Deprecated : Axial track finder from segments for mainly for cosmics.
    class TrackFinderCDCAxialSegmentHoughModule: public TrackFinderCDCFromSegmentsModule {

    public:
      /// Default constructor initialising the filters with the default settings
      TrackFinderCDCAxialSegmentHoughModule()
      {
        setDescription("Generates axial tracks from segments using a hough space over phi0 impact and curvature for the spares case.");
        ModuleParamList moduleParamList = this->getParamList();
        m_axialTrackCreatorSegmentHough.exposeParameters(&moduleParamList);
        this->setParamList(moduleParamList);
      }

      ///  Initialize the Module before event processing
      virtual void initialize() override
      {
        TrackFinderCDCFromSegmentsModule::initialize();
        m_axialTrackCreatorSegmentHough.initialize();
      }

      /// Generates the tracks from the given segments into the output argument.
      virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                            std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override final
      {
        m_axialTrackCreatorSegmentHough.apply(segments, tracks);
      }

      virtual void terminate() override
      {
        m_axialTrackCreatorSegmentHough.terminate();
        TrackFinderCDCFromSegmentsModule::terminate();
      }

    private:
      /// Findlet implementing the hough finder.
      AxialTrackCreatorSegmentHough m_axialTrackCreatorSegmentHough;
    }; // end class
  }

  using TrackFinderCDCAxialSegmentHoughModule = TrackFindingCDC::TrackFinderCDCAxialSegmentHoughModule;

} //end namespace Belle2
