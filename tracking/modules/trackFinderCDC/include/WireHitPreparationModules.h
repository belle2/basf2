/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/WireHitPreparer.h>

#include <tracking/trackFindingCDC/findlets/minimal/WireHitCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/WireHitBackgroundDetector.h>
#include <tracking/trackFindingCDC/findlets/minimal/HitReclaimer.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Module to set up the cdcwire hits used in all CDC track finding algorithms (except Trasan).
     * This module combines the geometrical information and the raw hit information into CDC Wire Hits, which can be used from all modules after that.
     * The wire hits with all their flags (like taken or background) are saved in a object on the store vector object that is constructed in this module.
     * You can access the wire hits from the 'CDCWireHitVector' object on the DataStore.
     *
     * Please keep in mind that the taken flag is propagated to all modules. So if you set/unset the taken flag of one hit, this will be used by *all*
     * following modules. Also, in your own module, check for the taken flag/background flag and do not use already taken hits twice.
     */
    class TFCDC_WireHitPreparerModule : public FindletModule<WireHitPreparer> {

      /// Type of the base class
      using Super = FindletModule<WireHitPreparer>;

    public:
      /// Constructor
      TFCDC_WireHitPreparerModule();
    };

    /********** Minimal modules **********/
    /**
     * Module implementation using the WireHitCreator
     */
    class TFCDC_WireHitCreatorModule : public FindletModule<WireHitCreator> {

      /// Type of the base class
      using Super = FindletModule<WireHitCreator>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_WireHitCreatorModule();
    };

    /**
     * Module to reclaim CDC hits
     */
    class TFCDC_HitReclaimerModule : public FindletModule<HitReclaimer> {

      /// Type of the base class
      using Super = FindletModule<HitReclaimer>;

    public:
      /// Constructor
      TFCDC_HitReclaimerModule();
    };


    /**
     * Module counterpart to the WireHitBackgroundDetector findlet,
     * which marks CDCWireHits as background based on the result of a filter.
     * FindletModule is used as an adaptor findlet->module.
     */
    class TFCDC_WireHitBackgroundDetectorModule : public FindletModule<WireHitBackgroundDetector> {

      /// Type of the base class
      using Super = FindletModule<WireHitBackgroundDetector>;

    public:
      /// Constructor
      TFCDC_WireHitBackgroundDetectorModule();
    };
  }
}
