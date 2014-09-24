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
   * which is responsible for creating visualisations using the TEve classes.
   *
   * The viewers, projections and interactive elements are controlled by DisplayUI.
   *
   * Full documentation is available at https://belle2.cc.kek.jp/~twiki/bin/view/Software/EventDisplay
   */
  class DisplayModule : public Module {
  public:
    DisplayModule(); //< ctor
    ~DisplayModule(); //< dtor

    void initialize(); //< Sets up geometry if needed.
    void event(); //< Show genfit::Tracks & hits in event viewer until next event is requested.
    void terminate(); //< terminate gEve to avoid problems with root's cleanup
  private:
    /** List of drawing options, a combination of:
     *
     * - A autoscale errors - use when hits are too small to be seen (because of tiny errors)
     * - D draw detectors - draw simple detector representation (with different size) for each hit
     * - H draw track hits
     * - M draw track markers - intersections of track with detector planes (use with T)
     * - P draw detector planes
     * - S scale manually - spacepoint hits are drawn as spheres and scaled with errors
     * - T draw track (straight line between detector planes)
     *
     * Note that you can always turn off an individual detector component or track
     * interactively by removing its checkmark in the 'Eve' tab.
     *
     * @sa EVEVisualization::setOptions()
     */
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

    /** If true, fitted genfit::Tracks, GFRave Vertices and ECLGamma objects will be shown in the display. */
    bool m_showTrackLevelObjects;

    /** Wether to show genfit::TrackCands */
    bool m_showTrackCandidates;

    /** Wether to use cluster hits for track candidates */
    bool m_useClusters;

    /** Non-interactively save visualisations for each event. */
    bool m_automatic;

    /** Show full geometry instead of simplified shapes. */
    bool m_fullGeometry;

    /** objects which are to be hidden (can be manually re-enabled in tree view). Names correspond to the object names in the 'Event Scene'. */
    std::vector<std::string> m_hideObjects;

    /** pointer to actual display */
    DisplayUI* m_display;

    /** Pointer to visualizer. */
    EVEVisualization* m_visualizer;
  };
}

#endif
