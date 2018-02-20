/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DISPLAYMODULE_H
#define DISPLAYMODULE_H

#include <framework/core/Module.h>

#include <string>


namespace Belle2 {
  class DisplayUI;
  class EVEVisualization;

  /** The event display module.
   *
   * The module will show MCParticles and fitted Tracks for each event, allowing the
   * user to continue with next event by pressing the '->' button. When events
   * are read using RootInput, loading previous events is also possible.
   *
   * The DisplayModule itself is responsible for fetching data on hits, tracks and
   * relations from the data store, and passes them along to the EVEVisualization class,
   * which is responsible for creating visualizations using the TEve classes.
   *
   * The viewers, projections and interactive elements are controlled by DisplayUI.
   *
   * Full documentation is available at https://confluence.desy.de/display/BI/Software+EventDisplay
   */
  class DisplayModule : public Module {
  public:
    /// Constructor. Sets all the module parameters.
    DisplayModule();

    /// Sets up geometry if needed.
    void initialize();

    /// Show various reconstructed or simulated objects in the event viewer until the next event is requested.
    void event();

    /// Terminate gEve to avoid problems with root's cleanup.
    void terminate();

  private:
    /** List of drawing options, see EVEVisualization::setOptions() */
    std::string m_options;

    /** Show Monte Carlo information (MCParticles, SimHits). */
    bool m_showMCInfo;

    /** If true, hits created by secondary particles (e.g. delta electrons) will be assigned to the original primary particle. */
    bool m_assignToPrimaries;

    /** If true, all primary MCParticles will be shown, regardless of wether hits are produced. */
    bool m_showAllPrimaries;

    /** If true, secondary MCParticles (and hits created by them) will not be shown. */
    bool m_hideSecondaries;

    /** If true, all charged primary and secondary MCParticles will be shown, regardless of wether hits are produced. */
    bool m_showCharged;

    /** If true, all neutral primary and secondary MCParticles will be shown, regardless of wether hits are produced. */
    bool m_showNeutrals;

    /** If true, fitted RecoTracks, GFRave Vertices and ECLGamma objects will be shown in the display. */
    bool m_showTrackLevelObjects;

    /** Whether to show RecoTracks */
    bool m_showRecoTracks;

    /** If true, CDCHit objects will be shown as drift cylinders (shortened, z position set to zero). */
    bool m_showCDCHits;

    /** If true, CDCHit objects will be assigned to trigger segments and trigger tracks will be shown. */
    bool m_showTriggerObjects;

    /** If true, BKLMHit objects will be shown in the display. */
    bool m_showBKLM2dHits;

    /** If true, ARICHHit objects will be shown as squares, corresponding to channel pixels */
    bool m_showARICHHits;

    /** Non-interactively save visualizations for each event. */
    bool m_automatic;

    /** Show full geometry instead of simplified shapes. */
    bool m_fullGeometry;

    /** objects which are to be hidden (can be manually re-enabled in tree view). Names correspond to the object names in the 'Event Scene'. */
    std::vector<std::string> m_hideObjects;

    /** Path to custom file with geometry extract */
    std::string m_customGeometryExtractPath;

    /** Path to custom file with geometry extract with corrected placement of TOP bars */
    std::string m_customGeometryExtractPathTop;

    /** List of volumes to be hidden (can be re-enabled in Eve panel / Geometry scene. The volume and all its daughters will be hidden. */
    std::vector<std::string> m_hideVolumes;

    /** List of volumes to be deleted. The volume and all its daughters will be removed completely. */
    std::vector<std::string> m_deleteVolumes;

    /** pointer to actual display */
    DisplayUI* m_display;

    /** Pointer to visualizer. */
    EVEVisualization* m_visualizer;
  };
}

#endif
