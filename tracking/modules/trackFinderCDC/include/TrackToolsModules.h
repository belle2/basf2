/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/TrackRejecter.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackQualityAsserter.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackOrienter.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackFlightTimeAdjuster.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackExporter.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the TrackRejecter
     */
    class TFCDC_TrackRejecterModule : public FindletModule<TrackRejecter> {

    private:
      /// Type of the base class
      using Super = FindletModule<TrackRejecter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackRejecterModule();
    };

    /**
     * Module implementation using the TrackQualityAsserter
     */
    class TFCDC_TrackQualityAsserterModule: public FindletModule<TrackQualityAsserter> {

    private:
      /// Type of the base class
      using Super = FindletModule<TrackQualityAsserter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackQualityAsserterModule();
    };

    /**
     * Module implementation using the TrackOrienter
     */
    class TFCDC_TrackOrienterModule : public FindletModule<TrackOrienter> {

    private:
      /// Type of the base class
      using Super = FindletModule<TrackOrienter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackOrienterModule();
    };

    /**
     * Module implementation using the TrackFlightTimeAdjuster
     */
    class TFCDC_TrackFlightTimeAdjusterModule : public FindletModule<TrackFlightTimeAdjuster> {

    private:
      /// Type of the base class
      using Super = FindletModule<TrackFlightTimeAdjuster>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackFlightTimeAdjusterModule();
    };

    /**
     * Module implementation using the TrackExporter
     */
    class TFCDC_TrackExporterModule : public FindletModule<TrackExporter> {

    private:
      /// Type of the base class
      using Super = FindletModule<TrackExporter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackExporterModule();
    };
  }
}
