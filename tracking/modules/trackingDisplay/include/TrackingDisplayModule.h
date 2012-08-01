#ifndef TRACKINGDISPLAYMODULE_H
#define TRACKINGDISPLAYMODULE_H

#include <framework/core/Module.h>

#include <string>


namespace Belle2 {
  class GenfitDisplay;

  /** Visualize GFTrack objects and PXD/SVD/CDCSimHits (plus geometry).
   *
   * The module will show GFTracks and SimHits for each event, allowing the
   * user to continue with next event by pressing the '->' button. When events
   * are read using SimpleInput, loading previous events is also possible.
   *
   * Most of the work is done by Genfit's GenfitDisplay class.
   * Please send suggestions and patches to <cpulvermacher@ekp.uni-karlsruhe.de>
   */
  class TrackingDisplayModule : public Module {
  public:
    TrackingDisplayModule(); //< ctor
    ~TrackingDisplayModule(); //< dtor

    void initialize(); //< Sets up geometry if needed.
    void event(); //< Show GFTracks & hits in event viewer until next event is requested.
  private:
    /** List of drawing options, a combination of:
     *
     * - A autoscale errors - use when hits are too small to be seen (because of tiny errors)
     * - D draw detectors - draw simple detector representation (with different size) for each hit
     * - H draw track hits
     * - R draw raw simhits (not associated with a track) - drawn as points only
     * - G draw geometry (whole detector)
     * - M draw track markers - intersections of track with detector planes (use with T)
     * - P draw detector planes
     * - S scale manually - spacepoint hits are drawn as spheres and scaled with errors
     * - T draw track (straight line between detector planes)
     * - X silent - open TEve, but don't actually draw anything
     *
     * Note that you can always turn off an individual detector component or track
     * interactively by removing its checkmark in the 'Eve' tab.
     *
     * @sa GenfitDisplay::setOptions()
     */
    std::string m_options;

    /** wether to add PXDSimHits */
    bool m_addPXDHits;

    /** wether to add SVDSimHits */
    bool m_addSVDHits;

    /** wether to add CDCSimHits */
    bool m_addCDCHits;

    /** pointer to actual display */
    GenfitDisplay* m_display;
  };
}

#endif
