/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackMerger.h>
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackHitMigrator.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCWireHit;
    enum class LegendreFindingPass;

    /**
     * Generates axial tracks from hit using special leaf postprocessing.
     *
     * Implements Legendre transformation of the drift time circles.
     * This is a module, performing tracking in the CDC. It is based on the paper
     * "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers"
     * by T. Alexopoulus, et al. NIM A592 456-462 (2008).
     */
    class AxialTrackFinderLegendre : public Findlet<const CDCWireHit, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit, CDCTrack>;

    public:
      /// Constructor
      AxialTrackFinderLegendre();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialisation before the event processing starts
      void initialize() final;

      /// Main method to apply the track finding.
      void apply(const std::vector<CDCWireHit>& wireHits, std::vector<CDCTrack>& tracks);

    private:
      /// Execute one pass over a quad tree
      void applyPass(LegendreFindingPass pass,
                     const std::vector<const CDCWireHit*>& axialWireHits,
                     std::vector<CDCTrack>& tracks);

    private: // Parameters
      /// Maximum Level of FastHough - fixed as the behaviour for other values is not well defined
      static const int m_param_maxLevel = 12;

    private: // findlets
      /// Findlet to exchange hits between tracks based on their proximity to the respective trajectory
      AxialTrackHitMigrator m_axialTrackHitMigrator;

      /// Findlet to merge the tracks after the legendre finder.
      AxialTrackMerger m_axialTrackMerger;
    };
  }
}
