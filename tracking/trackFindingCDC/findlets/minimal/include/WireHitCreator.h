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

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/minimal/EPreferredDirection.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/CDCHit.h>

#include <framework/datastore/StoreArray.h>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Combines the geometrical information and the raw hit information into wire hits,
     *  which can be used from all modules after that.
     */
    class WireHitCreator : public Findlet<CDCWireHit> {
    private:
      /// Type of the base class
      typedef Findlet<CDCWireHit> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Combines the geometrical information and the raw hit information into wire hits, "
               "which can be used from all modules after that";
      }

      /// Add the parameters to the surrounding module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final
      {
        moduleParamList->addParameter(prefixed(prefix, "flightTimeEstimation"),
                                      m_param_flightTimeEstimation,
                                      "Option which flight direction should be assumed for "
                                      "an initial time of flight estimation. Options are: "
                                      "'none' (no TOF correction), "
                                      "'outwards', "
                                      "'downwards'.",
                                      m_param_flightTimeEstimation);

        // moduleParamList->addParameter(prefixed(prefix, "triggerPoint"),
        //            m_param_triggerPoint,
        //            "Point relative to which the flight times of tracks should be adjusted",
        //            m_param_triggerPoint);
      }

      /// Signals the start of the event processing
      virtual void initialize() override final
      {
        StoreArray<CDCHit> hits;
        hits.isRequired();

        // Preload geometry during initialization
        // Marked as unused intentionally to avoid a compile warning
        CDC::CDCGeometryPar& cdcGeo __attribute__((unused)) = CDC::CDCGeometryPar::Instance();
        CDCWireTopology& wireTopology  __attribute__((unused)) = CDCWireTopology::getInstance();

        m_tdcCountTranslator.reset(new CDC::RealisticTDCCountTranslator);
        m_adcCountTranslator.reset(new CDC::LinearGlobalADCCountTranslator);

        if (m_param_flightTimeEstimation != std::string("")) {
          try {
            m_flightTimeEstimation = getPreferredDirection(m_param_flightTimeEstimation);
          } catch (std::invalid_argument& e) {
            B2ERROR("Unexpected 'flightTimeEstimation' parameter : '" << m_param_flightTimeEstimation);
          }
        }

        m_triggerPoint = Vector3D(std::get<0>(m_param_triggerPoint),
                                  std::get<1>(m_param_triggerPoint),
                                  std::get<2>(m_param_triggerPoint));

        if (m_flightTimeEstimation == EPreferredDirection::c_Symmetric) {
          B2ERROR("Unexpected 'flightTimeEstimation' parameter : '" << m_param_flightTimeEstimation);
        } else if (m_flightTimeEstimation == EPreferredDirection::c_Downwards) {
          FlightTimeEstimator::instance(std::unique_ptr<FlightTimeEstimator>(new CosmicRayFlightTimeEstimator(m_triggerPoint)));
        } else if (m_flightTimeEstimation == EPreferredDirection::c_Outwards) {
          FlightTimeEstimator::instance(std::unique_ptr<FlightTimeEstimator>(new BeamEventFlightTimeEstimator));
        }

        Super::initialize();
      }

      /// Main algorithm creating the wire hits
      virtual void apply(std::vector<CDCWireHit>& outputWireHits) override final
      {
        if (not outputWireHits.empty()) return;

        StoreArray<CDCHit> hits;

        // Create the wirehits into a vector
        Index useNHits = hits.getEntries();
        outputWireHits.reserve(useNHits);

        const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
        CDC::TDCCountTranslatorBase& tdcCountTranslator = *m_tdcCountTranslator;
        CDC::ADCCountTranslatorBase& adcCountTranslator = *m_adcCountTranslator;

        for (const CDCHit& hit : hits) {
          if (not wireTopology.isValidWireID(WireID(hit.getID()))) {
            B2WARNING("Skip invalid wire id " << hit.getID());
            continue;
          }

          const CDCWire* wire = CDCWire::getInstance(hit);
          const Vector2D& pos2D = wire->getRefPos2D();
          double alpha = 0;
          if (m_flightTimeEstimation == EPreferredDirection::c_Downwards and pos2D.y() > 0) {
            // Fix the flight direction to downwards.
            alpha = M_PI;
          }
          const double beta = 1;
          double flightTimeEstimate = FlightTimeEstimator::instance().getFlightTime2D(pos2D, alpha, beta);

          double refDriftLengthRight =
            tdcCountTranslator.getDriftLength(hit.getTDCCount(),
                                              wire->getWireID(),
                                              flightTimeEstimate,
                                              false, //bool leftRight
                                              wire->getRefZ(),
                                              alpha);

          double refDriftLengthLeft =
            tdcCountTranslator.getDriftLength(hit.getTDCCount(),
                                              wire->getWireID(),
                                              flightTimeEstimate,
                                              true, //bool leftRight
                                              wire->getRefZ(),
                                              alpha);

          double refDriftLength = (refDriftLengthLeft + refDriftLengthRight) / 2.0;

          double refDriftLengthVariance =
            tdcCountTranslator.getDriftLengthResolution(refDriftLength,
                                                        wire->getWireID(),
                                                        false, //bool leftRight ?
                                                        wire->getRefZ());

          double refChargeDeposit =
            adcCountTranslator.getCharge(hit.getADCCount(),
                                         wire->getWireID(),
                                         false, //bool leftRight
                                         wire->getRefZ(),
                                         0); //theta

          outputWireHits.push_back(CDCWireHit(&hit,
                                              refDriftLength,
                                              refDriftLengthVariance,
                                              refChargeDeposit));

        }

        if (useNHits == 0) {
          B2WARNING("Event with no hits");
        }

        std::sort(outputWireHits.begin(), outputWireHits.end());
      }

    private:
      /// Parameter : Method for the initial time of flight estimation as string
      std::string m_param_flightTimeEstimation = "none";

      /// Method for the initial time of flight estimation
      EPreferredDirection m_flightTimeEstimation = EPreferredDirection::c_None;

      /// Parameter : Location of the flight time zero
      std::tuple<double, double, double> m_param_triggerPoint{0.0, 0.0, 0.0};

      /// Central location of the flight time zero position usually the location of the trigger.
      Vector3D m_triggerPoint =  Vector3D(0, 0, 0);

      /// TDC Count translator to be used to calculate the initial dirft length estiamtes
      std::unique_ptr<CDC::TDCCountTranslatorBase> m_tdcCountTranslator{nullptr};

      /// ADC Count translator to be used to calculate the charge deposit in the drift cell
      std::unique_ptr<CDC::ADCCountTranslatorBase> m_adcCountTranslator{nullptr};

      // Note we can only create it on initialisation because the gearbox has to be connected.

    }; // end class WireHitCreator

  } // end namespace TrackFindingCDC
} // end namespace Belle2
