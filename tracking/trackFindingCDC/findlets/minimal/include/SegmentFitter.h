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



#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/EFitPos.h>
#include <tracking/trackFindingCDC/fitting/EFitVariance.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <framework/gearbox/Const.h>

#include <boost/math/special_functions/sinc.hpp>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Fits segments with the Riemann method.
    class SegmentFitter:
      public Findlet<CDCRecoSegment2D> {

    private:
      /// Type of the base class
      typedef Findlet<CDCRecoSegment2D> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Fits each segment with a selectable method";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final
      {
        moduleParamList->addParameter(prefixed(prefix, "karimakiFit"),
                                      m_param_karimakiFit,
                                      "Switch to select Karimaki method for fitting instead of Riemann fit",
                                      m_param_karimakiFit);

        moduleParamList->addParameter(prefixed(prefix, "fitPos"),
                                      m_param_fitPosString,
                                      "Positional information of the hits to be used in the fit. "
                                      "Options are 'recoPos', 'rlDriftCircle', 'wirePos'.",
                                      m_param_fitPosString);

        moduleParamList->addParameter(prefixed(prefix, "fitVariance"),
                                      m_param_fitVarianceString,
                                      "Positional information of the hits to be used in the fit. "
                                      "Options are 'unit', 'driftLength', 'pseudo', 'proper'.",
                                      m_param_fitVarianceString);

        moduleParamList->addParameter(prefixed(prefix, "updateDriftLength"),
                                      m_param_updateDriftLength,
                                      "Switch to reestimate the drift length",
                                      m_param_updateDriftLength);

        moduleParamList->addParameter(prefixed(prefix, "useAlphaInDriftLength"),
                                      m_param_useAlphaInDriftLength,
                                      "Switch to serve the alpha angle to the drift length translator",
                                      m_param_useAlphaInDriftLength);
      }

      /// Signals the beginning of the event processing
      void initialize() override
      {
        Super::initialize();
        if (m_param_fitPosString != std::string("")) {
          try {
            m_fitPos = getFitPos(m_param_fitPosString);
          } catch (std::invalid_argument& e) {
            B2ERROR("Unexpected fitPos parameter : '" << m_param_fitPosString);
          }
        }

        if (m_param_fitVarianceString != std::string("")) {
          try {
            m_fitVariance = getFitVariance(m_param_fitVarianceString);
          } catch (std::invalid_argument& e) {
            B2ERROR("Unexpected fitVariance parameter : '" << m_param_fitVarianceString);
          }
        }

      }

    public:
      /// Main algorithm applying the fit to each segment
      virtual void apply(std::vector<CDCRecoSegment2D>& outputSegments) override final
      {
        // Update the drift length
        if (m_param_updateDriftLength) {
          CDC::RealisticTDCCountTranslator tdcCountTranslator;
          for (CDCRecoSegment2D& segment : outputSegments) {
            for (CDCRecoHit2D& recoHit2D : segment) {
              Vector2D flightDirection = recoHit2D.getFlightDirection2D();
              Vector2D recoPos = recoHit2D.getRecoPos2D();
              double alpha = recoPos.angleWith(flightDirection);
              double arcLength2D = recoPos.cylindricalR() / boost::math::sinc_pi(alpha);
              double flightTimeEstimation = arcLength2D / Const::speedOfLight;

              const CDCWire& wire = recoHit2D.getWire();
              const CDCHit* hit = recoHit2D.getWireHit().getHit();
              const bool rl = recoHit2D.getRLInfo() == ERightLeft::c_Right;

              if (not m_param_useAlphaInDriftLength) {
                alpha = 0;
              }

              double driftLength =
                tdcCountTranslator.getDriftLength(hit->getTDCCount(),
                                                  wire.getWireID(),
                                                  flightTimeEstimation,
                                                  rl,
                                                  wire.getRefZ(),
                                                  alpha);

              recoHit2D.setRefDriftLength(driftLength);
              recoHit2D.snapToDriftCircle();
            }
          }
        }

        for (const CDCRecoSegment2D& segment : outputSegments) {

          CDCObservations2D observations2D(m_fitPos, m_fitVariance);
          observations2D.appendRange(segment);
          if (observations2D.size() < 4) {
            segment.getTrajectory2D().clear();
          } else {
            if (m_param_karimakiFit) {
              CDCTrajectory2D trajectory2D = m_karimakiFitter.fit(observations2D);
              segment.setTrajectory2D(trajectory2D);
            } else {
              CDCTrajectory2D trajectory2D = m_riemannFitter.fit(observations2D);
              segment.setTrajectory2D(trajectory2D);
            }
          }
        }
      }

    private:
      /// Parameter : Switch to use Karimaki fit
      bool m_param_karimakiFit = false;

      /// Parameter : Option string which positional information from the hits should be used
      std::string m_param_fitPosString = "recoPos";

      /// Parameter : Option string which variance information from the hits should be used
      std::string m_param_fitVarianceString = "proper";

      /// Parameter : Switch to reestimate the drift length
      bool m_param_updateDriftLength = false;

      /// Parameter : Switch to serve the alpha angle to the drift length translator.
      bool m_param_useAlphaInDriftLength = false;

      /// Option which positional information from the hits should be used
      EFitPos m_fitPos = EFitPos::c_RecoPos;

      /// Option which variance information from the hits should be used
      EFitVariance m_fitVariance = EFitVariance::c_Proper;

      /// Instance of the riemann fitter to be used.
      CDCRiemannFitter m_riemannFitter;

      /// Instance of the karimaki fitter to be used.
      CDCKarimakiFitter m_karimakiFitter;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2
