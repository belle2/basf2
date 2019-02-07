#ifndef EVEVISUALIZATION_H
#define EVEVISUALIZATION_H

#include <display/ObjectInfo.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <simulation/dataobjects/MCParticleTrajectory.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <bklm/dataobjects/BKLMSimHit.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <arich/dataobjects/ARICHHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <vxd/geometry/GeoCache.h>
#include <tracking/dataobjects/ROIid.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <genfit/GFRaveVertex.h>

#include <TEveStraightLineSet.h>
#include <TVector3.h>
#include <TString.h>
#include <TEveTrack.h>

#include <string>


class TEveBox;
class TEveCaloDataVec;
class TEveCalo3D;
class TEveElementList;
class TEvePointSet;
class TEveTrackList;
class TEveTrackPropagator;

namespace Belle2 {
  class DisplayData;
  class VisualRepMap;
  class EveVisBField;

  /** Produces visualisation for MCParticles, simhits, genfit::Tracks, geometry and other things.
   *
   * Creates TEve objects from the given data, and adds them to the global
   * or event scene.
   *
   *  @sa DisplayModule
   */
  class EVEVisualization {
    /** Fitter type to be used for addTrack(). */
    enum eFitterType {
      SimpleKalman,
      RefKalman,
      DafSimple,
      DafRef,
      Gbl
    };


    /** Hold MC tracks and associated visualisation objects. */
    struct MCTrack {
      TEveTrack* track; /**< the actual MC track. */
      TEvePointSet* simhits; /**< simhit positions. */
      const MCParticle* parentParticle; /**< parent particle, or nullptr. */
    };

    /** Group of TEveElements, remembers wether user wants it visible or not. */
    struct ElementGroup {
      ElementGroup(): group(nullptr), visible(true) { }
      TEveElementList* group; /**< Contains elements of this group. Set to nullptr after event. */
      bool visible; /**< Stores wether this group was visible in last event. */
    };

    /** Color for reco hits. */
    const static int c_recoHitColor;
    /** Color for TrackCandidates. */
    const static int c_recoTrackColor;
    /** Color for tracks. */
    const static int c_trackColor;
    /** Color for track markers. */
    const static int c_trackMarkerColor;
    /** Color for unassigned (reco)hits. */
    const static int c_unassignedHitColor;
    /** Color for KLMCluster objects. */
    const static int c_klmClusterColor;

  public:
    /** Constructor.
     */
    explicit EVEVisualization();

    /** disabled. */
    EVEVisualization(const EVEVisualization&) = delete;
    /** Destructor. */
    ~EVEVisualization();

    // === adding event data ===

    /** Add this genfit::Track to event data.
     *
     *  Adapted from GenfitDisplay, originally written by Karl Bicker. */
    void addTrack(const Belle2::Track* belle2Track);

    /** Add a RecoTrack, to evaluate track finding. */
    void addTrackCandidate(const std::string& collectionName,
                           const RecoTrack& recoTrack);

    /** Add a CDCTriggerTrack. */
    void addCDCTriggerTrack(const std::string& collectionName,
                            const CDCTriggerTrack& track);

    /** Add all entries in the given 'hits' array (and the corresponding MCParticles) to the event scene. */
    template <class T> void addSimHits(const StoreArray<T>& hits)
    {
      const int numHits = hits.getEntries();
      for (int i = 0; i < numHits; i++) {
        const RelationsObject* rel = hits[i];
        const MCParticle* mcpart = rel->getRelatedFrom<MCParticle>();

        addSimHit(hits[i], mcpart);
      }
    }

    /** Add a CDCSimHit. */
    void addSimHit(const CDCSimHit* hit, const MCParticle* particle);

    /** Add a PXDSimHit. */
    void addSimHit(const PXDSimHit* hit, const MCParticle* particle);

    /** Add a SVDSimHit. */
    void addSimHit(const SVDSimHit* hit, const MCParticle* particle);

    /** Add a BKLMSimHit. */
    void addSimHit(const BKLMSimHit* hit, const MCParticle* particle);

    /** Add a EKLMSimHit. */
    void addSimHit(const EKLMSimHit* hit, const MCParticle* particle);

    /** Add simhit as a simple point. */
    void addSimHit(const TVector3& v, const MCParticle* particle);

    /** Return MCTrack for given particle, add it if it doesn't exist yet.
     *
     * If particle is NULL, a dummy MCTrack (with track=0) is created which can accept otherwise unassigned hits.
     * Returns NULL if this particle and its hits shouldn't be shown.
     **/
    MCTrack* addMCParticle(const MCParticle* particle);

    /** Add a vertex point and its covariance matrix. */
    void addVertex(const genfit::GFRaveVertex* vertex);

    /** Add a reconstructed cluster in the ECL. */
    void addECLCluster(const ECLCluster* cluster);

    /** Add a reconstructed cluster in the KLM. */
    void addKLMCluster(const KLMCluster* cluster);

    /** Add a reconstructed 2d hit in the BKLM. */
    void addBKLMHit2d(const BKLMHit2d* bklm2dhit);

    /** Add a reconstructed 2d hit in the EKLM. */
    void addEKLMHit2d(const EKLMHit2d* bklm2dhit);

    /** Add recontructed hit in ARICH */
    void addARICHHit(const ARICHHit* hit);

    /** Add a Region Of Interest, computed by the PXDDataReduction module */
    void addROI(const ROIid* roi);

    /** After adding recohits for tracks/candidates, this function adds the remaining hits in a global collection. */
    template <class T> void addUnassignedRecoHits(const StoreArray<T>& hits)
    {
      if (hits.getEntries() == 0)
        return;
      if (!m_unassignedRecoHits) {
        m_unassignedRecoHits = new TEveStraightLineSet("Unassigned RecoHits");
        m_unassignedRecoHits->SetTitle("Unassigned RecoHits");
        m_unassignedRecoHits->SetMainColor(c_unassignedHitColor);
        m_unassignedRecoHits->SetMarkerColor(c_unassignedHitColor);
        m_unassignedRecoHits->SetMarkerStyle(6);
        //m_unassignedRecoHits->SetMainTransparency(60);
      }
      for (const T& hit : hits) {
        if (m_shownRecohits.count(&hit) == 0) {
          addRecoHit(&hit, m_unassignedRecoHits);
        }
      }
    }

    /** show CDCHits directly. */
    void addCDCHit(const CDCHit* hit, bool showTriggerHits = false);

    /** show outline of track segments. */
    void addCDCTriggerSegmentHit(const CDCTriggerSegmentHit* hit);

    /** Add TOPDigits (shown aggregated per module). */
    void addTOPDigits(const StoreArray<TOPDigit>& digits);

    /** Generic function to keep track of which objects have which visual representation.
     *
     * Should be called by functions adding TEveElements to the event scene
     * (Hits are currently excluded).
     */
    void addObject(const TObject* dataStoreObject, TEveElement* visualRepresentation);


    /** Add user-defined data (labels, points, etc.) */
    void showUserData(const DisplayData& displayData);


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
     * 'D': Draw detectors. This causes a simple representation for all detectors to be drawn. For
     *      planar detectors, this is a plane with the same position and orientation of the real
     *      detector plane, but with different size. For wires, this is a tube whose diameter
     *      is equal to the value measured by the wire. Spacepoint hits are not affected by this
     *      option.\n\n
     * 'H': Draw hits. This causes the hits to be visualized. Normally, the size of the hit
     *      representation is connected to the covariance matrix of the hit, scaled by the value
     *      set in setErrScale which is normally 1. Normally used in connection with 'D'.\n\n
     * 'M': Draw track markers. Draw the intersection points between the track and the virtual
     *      (and/or real) detector planes.
     * 'P': Draw detector planes. Draws the virtual (and/or real) detector planes.\n\n
     */
    void setOptions(const std::string& opts);

    /** @brief Set the scaling factor for the visualization of track hit errors.
     *
     * (only affects PXD and SpacePoint hits, which are somewhat small
     */
    void setErrScale(double errScale = 1.);

    /** If true, hits created by secondary particles (e.g. delta electrons) will be assigned to the original primary particle. */
    void setAssignToPrimaries(bool on) { m_assignToPrimaries = on; }

    /** If true, secondary MCParticles (and hits created by them) will not be shown. */
    void setHideSecondaries(bool on) { m_hideSecondaries = on; }


  private:
    /** Object for the energy bar visualisation. */
    TEveCalo3D* m_calo3d;

    /** @brief Create a box around o, oriented along u and v with widths ud, vd and depth and
     *  return a pointer to the box object.
     */
    TEveBox* boxCreator(const TVector3& o, TVector3 u, TVector3 v, float ud, float vd, float depth);

    /** Create hit visualisation for the given options, and add them to 'eveTrack'. */
    void makeLines(TEveTrack* eveTrack, const genfit::StateOnPlane* prevState, const genfit::StateOnPlane* state,
                   const genfit::AbsTrackRep* rep,
                   TEvePathMark::EType_e markType, bool drawErrors, int markerPos = 1);

    /** adds given VXD hit to lines. */
    template <class SomeVXDHit> void addRecoHit(const SomeVXDHit* hit, TEveStraightLineSet* lines)
    {
      static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

      const TVector3 local_pos(hit->getU(), hit->getV(), 0.0); //z-component is height over the center of the detector plane
      const VXD::SensorInfoBase& sensor = geo.get(hit->getSensorID());
      const TVector3 global_pos = sensor.pointToGlobal(local_pos);
      lines->AddMarker(global_pos.x(), global_pos.y(), global_pos.z());

      m_shownRecohits.insert(hit);
    }

    /** specialisation for SVDCluster */
    void addRecoHit(const SVDCluster* hit, TEveStraightLineSet* lines);

    /** specialisation for CDCHit. */
    void addRecoHit(const CDCHit* hit, TEveStraightLineSet* lines);

    /** Add 'elem' to the element group 'name' (created if necessary).
     *
     * name can also be a path, e.g. MyOwnStuff/SpecialObject A, which will automatically create sub-groups.
     *
     * slashes at beginning and end of name are ignored.
     */
    void addToGroup(const std::string& name, TEveElement* elem);

    /** Rescale PXD/SVD errors with this factor to ensure visibility. */
    double m_errorScale;

    /** Option string for genfit::Track visualisation.
     *
     * @sa setOptions
     */
    std::string m_options;

    /** If true, hits created by secondary particles (e.g. delta electrons) will be assigned to the original primary particle. */
    bool m_assignToPrimaries;

    /** If true, secondary MCParticles (and hits created by them) will not be shown. */
    bool m_hideSecondaries;

    /** map MCParticles to MCTrack (so hits can be added to the correct track). */
    std::map<const MCParticle*, MCTrack> m_mcparticleTracks;

    /** name -> grouping element. */
    std::map<std::string, ElementGroup> m_groups;

    /** parent object for MC tracks. */
    TEveTrackList* m_tracklist;

    /** Track propagator for MCParticles*/
    TEveTrackPropagator* m_trackpropagator;

    /** Track propagator for genfit::Tracks (different mainly because of drawing options) */
    TEveTrackPropagator* m_gftrackpropagator;

    /** Track propagator for CDCTriggerTracks (uses constant B field)*/
    TEveTrackPropagator* m_consttrackpropagator;

    /** ECL cluster data. */
    TEveCaloDataVec* m_eclData;

    /** The global magnetic field. */
    EveVisBField* m_bfield;

    /** List of shown recohits (PXDCluster, SVDCluster, CDCHit). */
    std::set<const TObject*> m_shownRecohits;

    /** Unassigned recohits. */
    TEveStraightLineSet* m_unassignedRecoHits = nullptr;
    /** is m_unassignedRecoHits visible? */
    bool m_unassignedRecoHitsVisibility = true;

    /** don't show MCParticles with momentum below this cutoff. */
    static constexpr double c_minPCut = 0.00;

    /** Draw cardinal representation in addTrack */
    bool m_drawCardinalRep = true;

    /** Draw errors in addTrack */
    bool m_drawErrors = false;

    /** Draw reference track in addTrack */
    bool m_drawRefTrack = false;

    /** draw forward in addTrack */
    bool m_drawForward = false;

    /** draw backward in addTrack */
    bool m_drawBackward = false;
  };
}
#endif
