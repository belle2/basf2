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

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCWireHit;

    /// Marks hits as background based on the result of a filter
    class AsicBackgroundDetector : public Findlet<CDCWireHit&> {
    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&>;

    public:
      /// Default constructor
      AsicBackgroundDetector() = default;

      /// Access database here:
      void initialize() final;

      /// Reload channel map if needed
      void beginRun() final;

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)  final;

      /// Main algorithm marking hit as background
      virtual void apply(std::vector<CDCWireHit>& wireHits) final;

    private:

      /// Algorithm marking hit as background for each CDC ASIC
      void applyAsicFilter(std::vector<CDCWireHit*>& wireHits);
      /**
       * Channel map retrieved from DB.
       */
      std::unique_ptr<DBArray<CDCChannelMap> > m_channelMapFromDB;

      /// map from ewire to board/channel ID
      std::map<int, std::pair<int, int> > m_map;

      /// min. number of hits in ASIC for background check
      size_t m_minimal_hit_number{4};

      /// distance from median TDC, to be considered as bg.
      double m_deviation_from_median{10.};

      /// max. number of signal-like hits in ASIC for background check
      size_t m_nsignal_max{4};
    };
  }
}