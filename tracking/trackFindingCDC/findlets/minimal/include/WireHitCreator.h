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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/minimal/EPreferredDirection.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/ADCCountTranslatorBase.h>
#include <cdc/dataobjects/TDCCountTranslatorBase.h>

#include <vector>
#include <tuple>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCWireHit;

    /**
     *  Combines the geometrical information and the raw hit information into wire hits,
     *  which can be used from all modules after that.
     */
    class WireHitCreator : public Findlet<CDCWireHit> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Signals the beginning of the event processing
      void initialize() final;

      /// Signal the beginning of a new run
      void beginRun() final;

      /// Main algorithm creating the wire hits
      void apply(std::vector<CDCWireHit>& outputWireHits) final;

    private:
      /// Parameter : Geometry set to be used. Either "base", "misalign" or " aligned"
      std::string m_param_wirePosSet = "base";

      /// Parameter : Switch to deactivate the sag of the wires for the concerns of the track finders.
      bool m_param_ignoreWireSag = false;

      /// Parameter : Method for the initial time of flight estimation as string
      std::string m_param_flightTimeEstimation = "none";

      /// Parameter : Location of the flight time zero
      std::tuple<double, double, double> m_param_triggerPoint = {0.0, 0.0, 0.0};

      /// Parameter : List of super layers to be used - mostly for debugging
      std::vector<int> m_param_useSuperLayers;

    private: // Prepared variables
      /// Geometry set to be used.
      CDC::CDCGeometryPar::EWirePosition m_wirePosSet = CDC::CDCGeometryPar::c_Base;

      /// Method for the initial time of flight estimation
      EPreferredDirection m_flightTimeEstimation = EPreferredDirection::c_None;

      /// Central location of the flight time zero position. Usually the location of the trigger.
      Vector3D m_triggerPoint = Vector3D(0.0, 0.0, 0.0);

      /// Bits for the used super layers
      std::array<bool, ISuperLayerUtil::c_N> m_useSuperLayers{};

    private: // Translators
      /// TDC Count translator to be used to calculate the initial dirft length estiamtes
      std::unique_ptr<CDC::TDCCountTranslatorBase> m_tdcCountTranslator = nullptr;

      /// ADC Count translator to be used to calculate the charge deposit in the drift cell
      std::unique_ptr<CDC::ADCCountTranslatorBase> m_adcCountTranslator = nullptr;
    };
  }
}
