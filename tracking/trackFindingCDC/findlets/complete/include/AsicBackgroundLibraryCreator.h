/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <vector>
#include <string>
#include <map>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/database/DBArray.h>
#include <cdc/dbobjects/CDCChannelMap.h>
#include <tracking/trackFindingCDC/mva/Recorder.h>
#include <cdc/dbobjects/CDCCrossTalkClasses.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCWireHit;
    class CDCTrack;

    /// Finds ASICs with single signal hit, records info to the library
    class AsicBackgroundLibraryCreator : public Findlet<const CDCWireHit, const CDCTrack> {
    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit, const CDCTrack >;

    public:
      /// Default constructor
      AsicBackgroundLibraryCreator() = default;

      /// Access database here, open library for writing:
      void initialize() final;

      /// Reload channel map if needed
      void beginRun() final;

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)  final;

      /// Main algorithm marking hit as background
      void apply(const std::vector<CDCWireHit>& wireHits, const std::vector<CDCTrack>& tracks) final;

      /// write out the library
      void terminate() final;
    private:

      /// Algorithm to select suitable ASIC for library creation
      void selectAsic(const std::vector<const CDCWireHit*>& wireHits, const std::vector<CDCTrack>& tracks);
      /**
       * Channel map retrieved from DB.
       */
      std::unique_ptr<DBArray<CDCChannelMap> > m_channelMapFromDB;

      /// map from ewire to board/channel ID
      std::map<int, std::pair<int, int> > m_map;

      /// output library name
      std::string m_library_name{"CDCAsicLibrary.root"};

      /// min. number of hits in ASIC for background check
      size_t m_minimal_hit_number{1};

      /// maximal distance from track to signal hit
      double m_distance_signal_max{0.25};

      /// minimal distance from track to background hit
      double m_distance_background_min{0.5};

      /// use axial layers only
      bool m_use_axial_hits_only{false};

      /// extra vars to the library
      bool m_write_extra_vars{false};


      /// min. number of hits on the track
      size_t m_minimal_hits_on_track{40};

      /// Recorder for the root output
      std::unique_ptr<Recorder> m_recorder;

      // Variables to be stored:

      /// all 8 channels
      asicChannel m_asic_info[8];

      /// signal boardID
      UChar_t m_board{0};

      /// signal channelID
      UChar_t m_channel{0};

      /// For debuging, store also number of channels with hits
      UChar_t m_n_hit{0};

      // Store also distance to the signal as well as minimal distance to bg

      /// Distance to signal hit
      float m_dist_signal{0};

      /// min. distance to non-linked hits
      float m_dist_bg{0};

      /// ADC of the signal
      Short_t m_adc_sig{0};

      /// Max. ADC background
      Short_t m_adc_max_bg{0};

      /// Number of hits on the track
      UShort_t m_n_hit_track{0};

    };
  }
}
