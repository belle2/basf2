/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackMerger.h>
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackHitMigrator.h>

#include <framework/core/Module.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCWireHit;
    enum class LegendreFindingPass;

    /**
     * CDC tracking module, using Legendre transformation of the drift time circles.
     * This is a module, performing tracking in the CDC. It is based on the paper
     * "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers"
     * by T. Alexopoulus, et al. NIM A592 456-462 (2008).
     */
    class TrackFinderCDCLegendreTrackingModule: public Module {

    private:
      /// Type of the base class
      using Super = Module;

    public:
      /// Create and allocate memory for variables here and add the module parameters in this method.
      TrackFinderCDCLegendreTrackingModule();

      /// Initialisation before the event processing starts
      void initialize() override;

      /// Processes the event and generates track candidates
      void event() override;

    private:
      /// Main method to apply the track finding.
      void apply(const std::vector<CDCWireHit>& wireHits, std::vector<CDCTrack>& tracks);

      /// Execute one pass over a quad tree
      void applyPass(LegendreFindingPass pass,
                     const std::vector<const CDCWireHit*>& axialWireHits,
                     std::vector<CDCTrack>& tracks);

    private:
      /// Parameter
      /// Maximum Level of FastHough Algorithm.
      int m_param_maxLevel;

      /// Parameter: Name of the output StoreObjPtr of the tracks generated within this module.
      std::string m_param_tracksStoreObjName = "CDCTrackVector";

    private: // findlets
      /// Findlet to exchange hits between tracks based on their proximity to the respective trajectory
      TrackFindingCDC::AxialTrackHitMigrator m_axialTrackHitMigrator;

      /// Findlet to merge the tracks after the legendre finder.
      TrackFindingCDC::AxialTrackMerger m_axialTrackMerger;
    };
  }
}
