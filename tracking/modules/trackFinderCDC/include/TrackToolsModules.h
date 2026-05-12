/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/TrackRejecter.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackQualityAsserter.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackOrienter.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackFlightTimeAdjuster.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackExporter.h>

#include <tracking/trackingUtilities/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackingUtilities/findlets/base/FindletModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the TrackRejecter
     */
    class TFCDC_TrackRejecterModule : public TrackingUtilities::FindletModule<TrackRejecter> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<TrackRejecter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackRejecterModule();
    };

    /**
     * Module implementation using the TrackQualityAsserter
     */
    class TFCDC_TrackQualityAsserterModule: public TrackingUtilities::FindletModule<TrackQualityAsserter> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<TrackQualityAsserter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackQualityAsserterModule();
    };

    /**
     * Module implementation using the TrackOrienter
     */
    class TFCDC_TrackOrienterModule : public TrackingUtilities::FindletModule<TrackOrienter> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<TrackOrienter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackOrienterModule();
    };

    /**
     * Module implementation using the TrackFlightTimeAdjuster
     */
    class TFCDC_TrackFlightTimeAdjusterModule : public TrackingUtilities::FindletModule<TrackFlightTimeAdjuster> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<TrackFlightTimeAdjuster>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackFlightTimeAdjusterModule();
    };

    /**
     * Module implementation using the TrackExporter
     */
    class TFCDC_TrackExporterModule : public TrackingUtilities::FindletModule<TrackExporter> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<TrackExporter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackExporterModule();
    };
  }
}
