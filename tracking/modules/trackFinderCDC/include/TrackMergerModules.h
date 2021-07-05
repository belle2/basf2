/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/TrackLinker.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCombiner.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

/******* Minimal Findlets **********/

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the TrackLinker
     */
    class TFCDC_TrackLinkerModule : public FindletModule<TrackLinker> {

      /// Type of the base class
      using Super = FindletModule<TrackLinker>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackLinkerModule();
    };

    /**
     * Module implementation using the TrackCombiner
     */
    class TFCDC_TrackCombinerModule : public FindletModule<TrackCombiner> {

      /// Type of the base class
      using Super = FindletModule<TrackCombiner>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackCombinerModule();
    };
  }
}
