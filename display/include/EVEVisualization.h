#ifndef EVEVISUALIZATION_H
#define EVEVISUALIZATION_H

#include <geometry/bfieldmap/BFieldMap.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <simulation/dataobjects/MCParticleTrajectory.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <bklm/dataobjects/BKLMSimHit.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <vxd/geometry/GeoCache.h>
#include <tracking/dataobjects/ROIid.h>

#include <tracking/trackFindingVXD/displayInterfaceTF/TrackCandidateTFInfo.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/CellTFInfo.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/SectorTFInfo.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/GFRaveVertex.h>

#include <TEveStraightLineSet.h>
#include <TEveTrackPropagator.h>
#include <TVector3.h>
#include <TString.h>
#include <TEveTrack.h>
#include <TEveManager.h>
#include <TEveCalo.h>

#include <string>
#include <vector>


class TEveBox;
class TEveCaloDataVec;
class TEveElementList;
class TEvePointSet;
class TEveTrackList;

namespace Belle2 {
  class DisplayData;
  class VisualRepMap;

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
      DafRef
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


    /** Provide magnetic field values for TEveTrackPropagator. */
    class EveVisBField : public TEveMagField {
    public:
      EveVisBField(): TEveMagField() { }
      virtual ~EveVisBField() { }

      /** return field strength at given coordinates, using Eve conventions. */
      virtual TEveVector GetField(Float_t x, Float_t y, Float_t z) const override {
        TEveVector v;

        v.Set(BFieldMap::Instance().getBField(TVector3(x, y, z)));
        v.fZ *= -1; //Eve has inverted B field convention
        v.fZ -= 1e-6; //field must not be zero!

        return v;
      }
      /** maximal field strength (is this correct?) */
      virtual Float_t GetMaxFieldMag() const override { return 1.5; }
    };


    /** Color for reco hits. */
    const static int c_recoHitColor = kOrange;
    /** Color for TrackCandidates. */
    const static int c_trackCandColor = kAzure - 2;
    /** Color for tracks. */
    const static int c_trackColor = kAzure;
    /** Color for track markers. */
    const static int c_trackMarkerColor = kSpring;
    /** Color for unassigned (reco)hits. */
    const static int c_unassignedHitColor = kViolet - 5;

  public:
    /** Constructor.
     */
    explicit EVEVisualization();
    /** Destructor. */
    ~EVEVisualization();

    /** Add TGeo geometry to Eve (only needs to be done once.) */
    void addGeometry();

    // === adding event data ===

    /** Add this genfit::Track to event data.
     *
     *  Adapted from GenfitDisplay, originally written by Karl Bicker. */
    void addTrack(const Belle2::Track* belle2Track);

    /** Add a genfit::TrackCand, to evaluate track finding. */
    template<class PXDType, class SVDType> void addTrackCandidate(const std::string& collectionName, const genfit::TrackCand* trackCand, const TString& label,
        const StoreArray<PXDType>& pxdhits, const StoreArray<SVDType>& svdhits, const StoreArray<CDCHit>& cdchits);

    /** Add VXDTF track candidate. */
    void addTrackCandidateTFInfo(TrackCandidateTFInfo* info);
    /** Add VXDTF cell. */
    void addCellTFInfo(CellTFInfo* info);
    /** Add VXDTF sector. */
    void addSectorTFInfo(SectorTFInfo* info);

    /** Add all entries in the given 'hits' array (and the corresponding MCParticles) to the event scene. */
    template <class T> void addSimHits(const StoreArray<T>& hits) {
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
    void addVertex(const genfit::GFRaveVertex* vertex, const TString& name);

    /** Add a reconstructed cluster in the ECL. */
    void addECLCluster(const ECLCluster* cluster);

    /** Add a Region Of Interest, computed by the PXDDataReduction module */
    void addROI(const ROIid* roi, const TString& name);

    /** After adding recohits for tracks/candidates, this function adds the remaining hits in a global collection. */
    template <class T> void addUnassignedRecoHits(const StoreArray<T>& hits) {
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
      for (const T & hit : hits) {
        if (m_shownRecohits.count(&hit) == 0) {
          addRecoHit(&hit, m_unassignedRecoHits);
        }
      }
    }

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
     *      set in setErrScale which is normally 1. See also option 'S'. Normally used in
     *      connection with 'D'.\n\n
     * 'M': Draw track markers. Draw the intersection points between the track and the virtual
     *      (and/or real) detector planes.
     * 'P': Draw detector planes. Draws the virtual (and/or real) detector planes.\n\n
     * 'S': Scale manually. This leads to the spacepoint hits (and only them up to now!) being drawn
     *      as spheres with radius 0.5 scaled with the error scale factor. Can be used if the scaling
     *      with errors leads to problems.\n\n
     */
    void setOptions(const std::string& opts);

    /** @brief Set the scaling factor for the visualization of the errors.*/
    void setErrScale(double errScale = 1.);

    /** If true, hits created by secondary particles (e.g. delta electrons) will be assigned to the original primary particle. */
    void setAssignToPrimaries(bool on) { m_assignToPrimaries = on; }

    /** If true, secondary MCParticles (and hits created by them) will not be shown. */
    void setHideSecondaries(bool on) { m_hideSecondaries = on; }

    /** Show full geometry instead of simplified shapes. */
    void showFullGeo(bool on) { m_fullgeo = on; }

    /** Get TObject <-> TEveElement mapping. */
    const VisualRepMap* getVisualRepMap() const { return m_visualRepMap; }


  private:
    /** Object for the energy bar visualisation. */
    TEveCalo3D* m_calo3d;

    /** @brief Create a box around o, oriented along u and v with widths ud, vd and depth and
     *  return a pointer to the box object.
     */
    TEveBox* boxCreator(const TVector3& o, TVector3 u, TVector3 v, float ud, float vd, float depth);

    /** Create hit visualisation for the given options, and add them to 'eveTrack'. */
    void makeLines(TEveTrack* eveTrack, const genfit::StateOnPlane* prevState, const genfit::StateOnPlane* state, const genfit::AbsTrackRep* rep,
                   TEvePathMark::EType_e markType, bool drawErrors, int markerPos = 1);

    /** enable/disable rendering of the volume 'name', or only its daughters if only_daughters is set. */
    void enableVolume(const char* name, bool only_daughters = false, bool enable = true);

    /** disable rendering of the volume 'name', or only its daughters if only_daughters is set. */
    void disableVolume(const char* name, bool only_daughters = false) { enableVolume(name, only_daughters, false); }

    /** set fill color of the volume 'name' to 'col'. */
    void setVolumeColor(const char* name, Color_t col);

    /** adds given VXD hit to lines. */
    template <class SomeVXDHit> void addRecoHit(const SomeVXDHit* hit, TEveStraightLineSet* lines) {
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

    /** Show full geometry instead of simplified shapes. */
    bool m_fullgeo;

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

    /** ECL cluster data. */
    TEveCaloDataVec* m_eclData;

    /** The global magnetic field. */
    EveVisBField m_bfield;

    /** List of shown recohits (PXDCluster, SVDCluster, CDCHit). */
    std::set<const TObject*> m_shownRecohits;

    /** Unassigned recohits. */
    TEveStraightLineSet* m_unassignedRecoHits;

    /** Map TObject <-> TEveElement, shared with UI */
    VisualRepMap* m_visualRepMap;

    /** don't show MCParticles with momentum below this cutoff. */
    static constexpr double c_minPCut = 0.00;
  };

  template<class PXDType, class SVDType> void EVEVisualization::addTrackCandidate(const std::string& collectionName, const genfit::TrackCand* trackCand, const TString& label,
      const StoreArray<PXDType>& pxdhits, const StoreArray<SVDType>& svdhits, const StoreArray<CDCHit>& cdchits)
  {
    // parse the option string ------------------------------------------------------------------------
    bool drawHits = false;

    if (m_options != "") {
      for (size_t i = 0; i < m_options.length(); i++) {
        if (m_options.at(i) == 'H') drawHits = true;
      }
    }
    // finished parsing the option string -------------------------------------------------------------


    //track seeds
    TVector3 track_pos = trackCand->getPosSeed();
    TVector3 track_mom = trackCand->getMomSeed();

    TEveStraightLineSet* lines = new TEveStraightLineSet(TString::Format("RecoHits for %s", label.Data()));
    lines->SetMainColor(c_trackCandColor);
    lines->SetMarkerColor(c_trackCandColor);
    lines->SetMarkerStyle(6);
    lines->SetMainTransparency(60);

    const unsigned int numhits = trackCand->getNHits();
    if (drawHits) {
      for (unsigned int iHit = 0; iHit < numhits; iHit++) { // loop over all hits in the track

        //get hit and detector indices from candidate
        int detId;
        int hitId;
        trackCand->getHit(iHit, detId, hitId);

        //get actual hit data from associated StoreArray
        if (detId == Const::PXD) {
          if (!pxdhits)
            continue;
          const PXDType* hit = pxdhits[hitId];
          if (!hit) {
            B2ERROR("No " << PXDType::Class_Name() << " found at index " << hitId << "!");
            continue;
          }
          addRecoHit(hit, lines);
        } else if (detId == Const::SVD) {
          if (!svdhits)
            continue;
          const SVDType* hit = svdhits[hitId];
          if (!hit) {
            B2ERROR("No " << SVDType::Class_Name() << " found at index " << hitId << "!");
            continue;
          }
          addRecoHit(hit, lines);
        } else if (detId == Const::CDC) {
          if (!cdchits)
            continue;
          const CDCHit* hit = cdchits[hitId];
          if (!hit) {
            B2ERROR("No " << CDCHit::Class_Name() << " found at index " << hitId << "!");
            continue;
          }
          addRecoHit(hit, lines);
        } else {
          B2ERROR("Got invalid detector ID from track candidate, skipping hit!")
          continue;
        }
      }
    }

    TEveRecTrack rectrack;
    rectrack.fP.Set(track_mom);
    rectrack.fV.Set(track_pos);

    TEveTrack* track_lines = new TEveTrack(&rectrack, m_gftrackpropagator);
    track_lines->SetName(label); //popup label set at end of function
    track_lines->SetPropagator(m_gftrackpropagator);
    track_lines->SetLineColor(c_trackCandColor);
    track_lines->SetLineWidth(1);
    track_lines->SetTitle(TString::Format("%s\n"
                                          "#hits: %u\n"
                                          "pT=%.3f, pZ=%.3f",
                                          label.Data(), numhits,
                                          track_mom.Pt(), track_mom.Pz()));

    track_lines->SetCharge((int)trackCand->getChargeSeed());


    track_lines->AddElement(lines);
    addToGroup(collectionName, track_lines);
    addObject(trackCand, track_lines);
  }
}
#endif
