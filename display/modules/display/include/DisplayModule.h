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
   * are read using SimpleInput, loading previous events is also possible.
   *
   * The DisplayModule itself is responsible for fetching data on hits, tracks and
   * relations from the data store, and passes them along to the EVEVisualization class,
   * which is responsible for creating visualisations using the TEve classes.
   *
   * The viewers, projections and interactive elements are controlled by DisplayUI.
   *
   *
   * Please send suggestions and patches to <cpulvermacher@ekp.uni-karlsruhe.de>
   */
  class DisplayModule : public Module {
  public:
    DisplayModule(); //< ctor
    ~DisplayModule(); //< dtor

    void initialize(); //< Sets up geometry if needed.
    void event(); //< Show GFTracks & hits in event viewer until next event is requested.
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

    /** If true, hits created by secondary particles (e.g. delta electrons) will be assigned to the original primary particle. */
    bool m_assignToPrimaries;

    /** If true, all primary MCParticles will be shown, regardless of wether hits are produced. */
    bool m_showAllPrimaries;

    /** If true, all charged primary and secondary MCParticles will be shown, regardless of wether hits are produced. */
    bool m_showCharged;

    /** If true, all neutral primary and secondary MCParticles will be shown, regardless of wether hits are produced. */
    bool m_showNeutrals;

    /** Wether to show GFTracks */
    bool m_showGFTracks;

    /** pointer to actual display */
    DisplayUI* m_display;

    /** Pointer to visualizer. */
    EVEVisualization* m_visualizer;
  };
}

#endif
