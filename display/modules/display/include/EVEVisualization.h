#ifndef EVEVISUALIZATION_H
#define EVEVISUALIZATION_H

#include <generators/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <bklm/dataobjects/BKLMSimHit.h>
//#include <eklm/dataobjects/EKLMSimHit.h>
#include <ecl/dataobjects/HitECL.h>

#include <GFTrack.h>

#include <TVector3.h>
#include <TString.h>

#include <string>
#include <vector>

class TEveBox;
class TEveCaloDataVec;
class TEveElementList;
class TEvePointSet;
class TEveTrack;
class TEveTrackList;
class TEveTrackPropagator;

namespace Belle2 {

  /** Produces visualisation for MCParticles, simhits, GFTracks, geometry and other things.
   *
   * Basically this creates TEve objects from the given data, and adds them to the global
   * or event scene.
   *
   *  @sa DisplayModule
   */
  class EVEVisualization {

    /** Hold MC tracks and associated visualisation objects. */
    struct MCTrack {
      TEveTrack* track; /**< the actual MC track. */
      TEvePointSet* simhits; /**< simhit positions. */
    };

  public:
    /** Constructor.
     */
    explicit EVEVisualization();
    /** Destructor. */
    ~EVEVisualization();

    /** Add TGeo geometry to Eve (only needs to be done once.) */
    void addGeometry();

    /** export full geometry to file. */
    void saveGeometry(const std::string& name);

    // === adding event data ===

    /** Add this GFTrack to event data.
     *
     *  Adapted from GenfitDisplay, originally written by Karl Bicker. */
    void addTrack(const GFTrack* gftrack, const TString& label = "");

    /** Add a CDCSimHit. */
    void addSimHit(const CDCSimHit* hit, const MCParticle* particle);

    /** Add a PXDSimHit. */
    void addSimHit(const PXDSimHit* hit, const MCParticle* particle);

    /** Add a SVDSimHit. */
    void addSimHit(const SVDSimHit* hit, const MCParticle* particle);

    /** Add a TOPSimHit. */
    void addSimHit(const TOPSimHit* hit, const MCParticle* particle);

    /** Add a BKLMSimHit. */
    void addSimHit(const BKLMSimHit* hit, const MCParticle* particle);

    /** Add a EKLMSimHit. */
    //void addSimHit(const EKLMSimHit* hit, const MCParticle* particle);

    /** Add simhit as a simple point. */
    void addSimHit(const TVector3& v, const MCParticle* particle);

    /** Add a ECL hit. */
    void addECLHit(const HitECL* hit);

    /** Return MCTrack for given particle, add it if it doesn't exist yet. */
    MCTrack& addMCParticle(const MCParticle* particle);

    /** Create visual representation of all tracks.
     *
     *  Needs to be called after all hits / tracks are added.
     */
    void makeTracks();

    /** clear event data. */
    void clearEvent();

    // === configuration options ===

    /** @brief Set the display options.
     *
     * The option string lets you steer the way the events are displayed. The following
     * options are available:\n
     * \n
     * 'A': Autoscale errors. The representation of hits are scaled with the error found in
     *      their covariance matrix. This can lead to hits not being displayed beause the
     *      errors are too small. Autoscaling ensures that the errors are scaled up
     *      sufficiently to ensure all hits are displayed. However, this can lead to unwanted
     *      results if there are only a few hits with very small errors, as all hits are scaled
     *      by the same factor to ensure consistency.\n\n
     * 'D': Draw detectors. This causes a simple representation for all detectors to be drawn. For
     *      planar detectors, this is a plane with the same position and orientation of the real
     *      detector plane, but with different size. For wires, this is a tube whose diameter
     *      is equal to the value measured by the wire. Spacepoint hits are not affected by this
     *      option.\n\n
     * 'H': Draw hits. This causes the hits to be visualized. Normally, the size of the hit
     *      representation is connected to the covariance matrix of the hit, scaled by the value
     *      set in setErrScale which is normally 1. See also option 'A' and 'S'. Normally used in
     *      connection with 'D'.\n\n
     * 'G': Draw geometry. Also draw the geometry in the gGeoManager. This feature is experimental
     *      and may lead to strange things being drawn.\n\n
     * 'M': Draw track markers. Draw the intersection points between the track and the virtual
     *      (and/or real) detector planes. Can only be used in connection with 'T'.\n\n
     * 'P': Draw detector planes. Draws the virtual (and/or real) detector planes.\n\n
     * 'S': Scale manually. This leads to the spacepoint hits (and only them up to now!) being drawn
     *      as spheres with radius 0.5 scaled with the error scale factor. Can be used if the scaling
     *      with errors leads to problems.\n\n
     * 'T': Draw Track. Draw the track as straight lines between the virtual (and/or real) detector
     *      planes.\n\n
     * 'X': Draw silent. Does not run the TApplication.
     *
     */
    void setOptions(const std::string& opts = "MHTG");

    /** @brief Set the scaling factor for the visualization of the errors.*/
    void setErrScale(double errScale = 1.);

    /** @brief Get the error scaling factor.*/
    double getErrScale() const;

    /** If true, hits created by secondary particles (e.g. delta electrons) will be assigned to the original primary particle. */
    void setAssignToPrimaries(bool on) { m_assignToPrimaries = on; }


  private:
    /** @brief Create a box around o, orientet along u and v with widths ud, vd and depth and
     *  return a pointer to the box object.
     */
    TEveBox* boxCreator(const TVector3& o, TVector3 u, TVector3 v, float ud, float vd, float depth);

    /** enable/disable rendering of the volume 'name', or only its daughters if only_daughters is set. */
    void enableVolume(const char* name, bool only_daughters = false, bool enable = true);

    /** disable rendering of the volume 'name', or only its daughters if only_daughters is set. */
    void disableVolume(const char* name, bool only_daughters = false) { enableVolume(name, only_daughters, false); }

    /** set fill color of the volume 'name' to 'col'. */
    void setVolumeColor(const char* name, Color_t col);

    /** Rescale errors with this factor to ensure visibility. */
    double m_errorScale;

    /** Option string for GFTrack visualisation.
     *
     * @sa setOptions
     */
    std::string m_options;

    /** If true, hits created by secondary particles (e.g. delta electrons) will be assigned to the original primary particle. */
    bool m_assignToPrimaries;

    /** map MCParticles to MCTrack (so hits can be added to the correct track). */
    std::map<const MCParticle*, MCTrack> m_mcparticleTracks;

    /** parent object for reconstructed tracks. */
    TEveTrackList* m_gftracklist;

    /** Track propagator for MCParticles*/
    TEveTrackPropagator* m_trackpropagator;

    /** Track propagator for GFTracks (different mainly because of drawing options) */
    TEveTrackPropagator* m_gftrackpropagator;

    /** ECL hit data. */
    TEveCaloDataVec* m_eclsimhitdata;
  };
}
#endif
