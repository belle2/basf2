/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/minimal/EPreferredDirection.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>
#include <tracking/trackFindingCDC/topology/EWirePosition.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <cdc/geometry/CDCGeometryParConstants.h>

#include <framework/database/DBObjPtr.h>
#include <cdc/dbobjects/CDClayerTimeCut.h>

#include <vector>
#include <tuple>
#include <string>
#include <memory>

namespace Belle2 {
  namespace CDC {
    class TDCCountTranslatorBase;
    class ADCCountTranslatorBase;
  }
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
      /// Default constructor
      WireHitCreator();

      /// Default destructor
      ~WireHitCreator();

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
      std::string m_param_wirePosition = "base";

      /// Parameter : Switch to deactivate the sag of the wires for the concerns of the track finders.
      bool m_param_ignoreWireSag = false;

      /// Parameter : Method for the initial time of flight estimation as string
      std::string m_param_flightTimeEstimation = "none";

      /// Parameter : Location of the flight time zero
      std::tuple<double, double, double> m_param_triggerPoint = {0.0, 0.0, 0.0};

      /// Parameter : List of super layers to be used - mostly for debugging
      std::vector<int> m_param_useSuperLayers;

      /// Parameter : List of layers to be used
      std::vector<uint> m_param_useLayers;

      /// Parameter : List of layers to be ignored in tracking e.g. for simulating too high occupancy
      std::vector<uint> m_param_ignoreLayers;

      /// Parameter : Cut for approximate drift time (super-layer dependent)
      std::vector<float> m_param_maxDriftTimes = { -1, -1, -1, -1, -1, -1, -1, -1, -1};

      /// Parameter : If true, the second hit information will be used to create Wire Hits
      bool m_param_useSecondHits = false;

      /// Parameter : If true, the hits on bad wires are not ignored.
      bool m_param_useBadWires = false;

      /// Parameter : Angular range in degrees for which hits should be unpacked
      std::tuple<double, double> m_param_useDegreeSector{INFINITY, INFINITY};

      /// Parameter : Indices of the Monte Carlo particles for which hits should be used
      std::vector<int> m_param_useMCParticleIds{};

    private: // Prepared variables
      /// Geometry set to be used.
      EWirePosition m_wirePosition = EWirePosition::c_Base;

      /// Cut for approximate drift time (super-layer dependent)
      std::array<float, ISuperLayerUtil::c_N> m_maxDriftTimes = { -1, -1, -1, -1, -1, -1, -1, -1, -1};

      /// Method for the initial time of flight estimation
      EPreferredDirection m_flightTimeEstimation = EPreferredDirection::c_None;

      /// Central location of the flight time zero position. Usually the location of the trigger.
      Vector3D m_triggerPoint = Vector3D(0.0, 0.0, 0.0);

      /// Bits for the used super layers
      std::array<bool, ISuperLayerUtil::c_N> m_useSuperLayers{};

      /// Bits for the used layers
      std::array<bool, c_maxNSenseLayers> m_useLayers{};

      /// Unit vectors denoting the sector for which hits should be created
      std::array<Vector2D, 2> m_useSector{};

    private: // Translators
      /// TDC Count translator to be used to calculate the initial dirft length estiamtes
      std::unique_ptr<CDC::TDCCountTranslatorBase> m_tdcCountTranslator;

      /// ADC Count translator to be used to calculate the charge deposit in the drift cell
      std::unique_ptr<CDC::ADCCountTranslatorBase> m_adcCountTranslator;

      /// Cut for approximate drift time (super-layer dependent)
      DBObjPtr<CDClayerTimeCut> m_DBCDClayerTimeCut;
    };
  }
}
