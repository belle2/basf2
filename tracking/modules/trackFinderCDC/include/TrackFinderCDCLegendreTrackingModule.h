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

#include <framework/core/Module.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCConformalHit;
    class CDCWireHit;
    enum class LegendreFindingPass;
  }

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
    /// Parameter
    /// Maximum Level of FastHough Algorithm.
    int m_param_maxLevel;

    /// Defines whether early track merging will be performed.
    bool m_param_doEarlyMerging;

    /// Parameter: Name of the output StoreObjPtr of the tracks generated within this module.
    std::string m_param_tracksStoreObjName = "CDCTrackVector";

  private:
    /// Worker
    /// Object for holding all found cdc tracks to be passed around to the postprocessing functions.
    std::vector<TrackFindingCDC::CDCTrack> m_tracks;

    /// List to collect all axial wire hits
    std::vector<const TrackFindingCDC::CDCWireHit*> m_allAxialWireHits;

    /// List for holding all used conformal CDC wire hits.
    std::vector<TrackFindingCDC::CDCConformalHit> m_conformalCDCWireHitList;

    /// Main method to apply the track finding.
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks);

    /// All objects in m_hitList and m_trackList are deleted and the two lists are cleared.
    void clearVectors();

    /// Do the real tree track finding.
    void findTracks();

    /// Execute one pass over a quad tree
    void applyPass(TrackFindingCDC::LegendreFindingPass pass);

    /// Startup code before the event starts.
    void startNewEvent();

    /// Write the objects to the store array.
    void outputObjects(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks);

  private: // findlets
    /// Findlet to merge the tracks after the legendre finder.
    TrackFindingCDC::AxialTrackMerger m_axialTrackMerger;
  };
}
