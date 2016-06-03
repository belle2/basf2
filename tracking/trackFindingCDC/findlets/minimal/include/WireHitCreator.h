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
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/minimal/EPreferredDirection.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/CDCHit.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

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
                                      "an initial time of flight estimation."
                                      "'none' (no TOF correction), "
                                      "'outwards', "
                                      "'downwards'.",
                                      std::string(m_param_flightTimeEstimation));
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

        if (m_flightTimeEstimation == EPreferredDirection::c_Symmetric) {
          B2ERROR("Unexpected 'flightTimeEstimation' parameter : '" << m_param_flightTimeEstimation);
        }

        Super::initialize();
      }

      /// Main algorithm creating the wire hits
      virtual void apply(std::vector<CDCWireHit>& outputWireHits) override final
      {
        if (not outputWireHits.empty()) return;

        StoreArray<CDCHit> hits;
        hits.isRequired();

        // Create the wirehits into a vector
        Index useNHits = hits.getEntries();
        outputWireHits.reserve(useNHits);

        for (CDCHit& hit : hits) {
          CDC::TDCCountTranslatorBase& tdcCountTranslator = *m_tdcCountTranslator;
          CDC::ADCCountTranslatorBase& adcCountTranslator = *m_adcCountTranslator;

          const CDCWire* wire = CDCWire::getInstance(hit);

          double initialTOFEstimate = 0;
          if (m_flightTimeEstimation == EPreferredDirection::c_None) {
            initialTOFEstimate = 0;
          } else if (m_flightTimeEstimation == EPreferredDirection::c_Outwards) {
            initialTOFEstimate = wire->getRefCylindricalR() / Const::speedOfLight;
          } else if (m_flightTimeEstimation == EPreferredDirection::c_Downwards) {
            initialTOFEstimate = -wire->getRefPos2D().y() / Const::speedOfLight;
          }

          double refDriftLengthRight =
            tdcCountTranslator.getDriftLength(hit.getTDCCount(),
                                              wire->getWireID(),
                                              initialTOFEstimate,
                                              false, //bool leftRight
                                              wire->getRefZ());

          double refDriftLengthLeft =
            tdcCountTranslator.getDriftLength(hit.getTDCCount(),
                                              wire->getWireID(),
                                              initialTOFEstimate,
                                              true, //bool leftRight
                                              wire->getRefZ());

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

        // Some safety checks from funky times - did not trigger for ages.
        // cppcheck-suppress syntaxError
        for (int iHit = 0; iHit < useNHits; ++iHit) {
          CDCHit* ptrHit = hits[iHit];
          CDCHit& hit = *ptrHit;
          const WireID wireID(hit.getID());
          const CDCWireHit& wireHit = outputWireHits[iHit];

          if (iHit != hit.getArrayIndex()) {
            B2ERROR("CDCHit.getArrayIndex() produced wrong result. Expected : " << iHit << " Actual : " << hit.getArrayIndex());
          }

          if (wireID.getEWire() != hit.getID()) {
            B2ERROR("WireID.getEWire() differs from CDCHit.getID()");
          }

          if (hit.getID() != wireHit.getWire().getEWire()) {
            B2ERROR("CDCHit.getID() differs from CDCWireHit.getWire().getEWire()");
          }
          if (hit.getArrayIndex() != wireHit.getStoreIHit()) {
            B2ERROR("CDCHit.getArrayIndex() differs from CDCWireHit.getStoreIHit");
          }
        }

        B2DEBUG(100, "  Created number of CDCWireHits == " << useNHits);
        B2DEBUG(100, "  Number of usable CDCWireHits == " << useNHits);

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

      /// TDC Count translator to be used to calculate the initial dirft length estiamtes
      std::unique_ptr<CDC::TDCCountTranslatorBase> m_tdcCountTranslator{nullptr};

      /// ADC Count translator to be used to calculate the charge deposit in the drift cell
      std::unique_ptr<CDC::ADCCountTranslatorBase> m_adcCountTranslator{nullptr};

      // Note we can only create it on initialisation because the gearbox has to be connected.

    }; // end class WireHitCreator

  } // end namespace TrackFindingCDC
} // end namespace Belle2
