#include <analysis/modules/dedxPID/DedxPIDModule.h>
#include <analysis/modules/dedxPID/HelixHelper.h>
#include <analysis/modules/dedxPID/DedxConstants.h>

#include <analysis/dataobjects/TrackDedx.h>
#include <analysis/dataobjects/DedxLikelihood.h>

#include <framework/core/Environment.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

#include <tracking/dataobjects/Track.h>
#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <vxd/geometry/GeoCache.h>
#include <geometry/GeometryManager.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include "GFTrack.h"
#include "GFAbsTrackRep.h"
#include "GFFieldManager.h"
#include "TGeoManager.h"

#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TMath.h>

#include <cassert>
#include <cmath>
#include <algorithm>
#include <utility>

using namespace Belle2;
using namespace CDC;

REG_MODULE(DedxPID)

//if defined, use path length along a helical track instead of step-by-step geometrical correction
//in theory this should be more accurate, unsure why it doesn't work
//#define USE_HELIX_PATH_LENGTH


template <class HitClass> float get_edep(const HitClass* hit) { return hit->getEnergyDep(); }

//specialization for PXDCluster, has getChange() instead of getEnergyDep()
float get_edep(const PXDCluster* hit) { return hit->getCharge(); }


DedxPIDModule::DedxPIDModule() : Module()
{
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //Set module properties
  setDescription("Extract dE/dx (and some other things) from Tracks&GFTrackCandidates and PXDClusters, SVDTrueHits (not digitized) and CDCHits.");
  addParam("GFTracksColName", m_gftracks_name, "Name for GFTracks array. Leave empty for default ('GFTracks')", std::string(""));

  //Parameter definitions
  addParam("UseIndividualHits", m_useIndividualHits, "Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used.", true);
  addParam("RemoveLowest", m_removeLowest, "portion of events with low dE/dx that should be discarded if UseIndividualHits is false", double(0.15));
  addParam("RemoveHighest", m_removeHighest, "portion of events with high dE/dx that should be discarded if UseIndividualHits is false", double(0.15));

  addParam("OnlyPrimaryParticles", m_onlyPrimaryParticles, "Only save data for primary particles (as determined by MC truth)", false);
  addParam("UsePXD", m_usePXD, "Use PXD hits for dE/dx calculation", false);
  addParam("UseSVD", m_useSVD, "Use SVD hits for dE/dx calculation", true);
  addParam("UseCDC", m_useCDC, "Use CDC hits for dE/dx calculation", true);

  addParam("TrackDistanceThreshold", m_trackDistanceThreshhold, "Use a faster helix parametrisation, with corrections as soon as the approximation is more than ... cm off.", double(4.0));
  addParam("EnableDebugOutput", m_enableDebugOutput, "Wether to save information on tracks and associated hits and dE/dx values in TrackDedx objects.", false);

  addParam("PDFFile", m_pdfFilename, "The dE/dx:momentum PDF file to use. Use an empty string to disable classification.", Environment::Instance().getDataSearchPath() + std::string("/analysis/dedxPID_PDFs_r3178.root"));
  addParam("IgnoreMissingParticles", m_ignoreMissingParticles, "Ignore particles for which no PDFs are found", false);
}


DedxPIDModule::~DedxPIDModule() { }


void DedxPIDModule::initialize()
{
  m_trackID = m_eventID = 0;
  m_numExtrapolations = 0;

  if (!m_enableDebugOutput and m_pdfFilename.empty()) {
    B2ERROR("No PDFFile given and debug output disabled. DedxPID module will produce no output!");
  }

  //register outputs (if needed)
  if (m_enableDebugOutput)
    StoreArray<TrackDedx>::registerPersistent("TrackDedx", DataStore::c_Event);

  if (!m_pdfFilename.empty()) {
    StoreArray<DedxLikelihood>::registerPersistent();
    RelationArray::registerPersistent<GFTrack, DedxLikelihood>();

    //load pdfs
    TFile* pdf_file = new TFile(m_pdfFilename.c_str(), "READ");
    if (!pdf_file->IsOpen())
      B2FATAL("Couldn't open pdf file: " << m_pdfFilename);

    //load dedx:momentum PDFs
    const char* suffix = (!m_useIndividualHits) ? "_trunc" : "";
    for (int detector = 0; detector < c_Dedx_num_detectors; detector++) {
      int nBinsX, nBinsY;
      double xMin, xMax, yMin, yMax;
      nBinsX = nBinsY = -1;
      xMin = xMax = yMin = yMax = 0.0;
      for (int particle = 0; particle < c_Dedx_num_particles; particle++) {
        const int pdg_code = c_Dedx_pdg_codes[particle];
        m_pdfs[detector][particle] =
          dynamic_cast<TH2F*>(pdf_file->Get(TString::Format("hist_d%i_%i%s", detector, pdg_code, suffix)));

        if (!m_pdfs[detector][particle]) {
          if (m_ignoreMissingParticles)
            continue;
          B2FATAL("Couldn't find PDF for PDG " << pdg_code << ", detector " << detector << suffix);
        }

        //check that PDFs have the same dimensions and same binning
        const double eps_factor = 1e-5;
        if (nBinsX == -1 and nBinsY == -1) {
          nBinsX = m_pdfs[detector][particle]->GetNbinsX();
          nBinsY = m_pdfs[detector][particle]->GetNbinsY();
          xMin = m_pdfs[detector][particle]->GetXaxis()->GetXmin();
          xMax = m_pdfs[detector][particle]->GetXaxis()->GetXmax();
          yMin = m_pdfs[detector][particle]->GetYaxis()->GetXmin();
          yMax = m_pdfs[detector][particle]->GetYaxis()->GetXmax();
        } else if (nBinsX != m_pdfs[detector][particle]->GetNbinsX()
                   or nBinsY != m_pdfs[detector][particle]->GetNbinsY()
                   or fabs(xMin - m_pdfs[detector][particle]->GetXaxis()->GetXmin()) > eps_factor * xMax
                   or fabs(xMax - m_pdfs[detector][particle]->GetXaxis()->GetXmax()) > eps_factor * xMax
                   or fabs(yMin - m_pdfs[detector][particle]->GetYaxis()->GetXmin()) > eps_factor * yMax
                   or fabs(yMax - m_pdfs[detector][particle]->GetYaxis()->GetXmax()) > eps_factor * yMax) {
          B2FATAL("PDF for PDG " << pdg_code << ", detector " << detector << suffix << " has binning/dimensions differing from previous PDF.")
        }
      }
    }

    //leaking pdf_file so I can access the histograms
  }

  //create instances here to not confuse profiling
  CDCGeometryPar::Instance();
  VXD::GeoCache::getInstance();

  if (!gGeoManager) { //TGeo geometry not initialized, do it ourselves
    //convert geant4 geometry to TGeo geometry
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();
  }

  //initialize magnetic field for genfit
  GFFieldManager::getInstance()->init(new GFGeant4Field());
}


void DedxPIDModule::event()
{
  //go through all (GF)Tracks (both have same index)
  //find correct MCParticle through Track::getMCId()
  //get GFTrackCand through GFTrack::getCand()
  //get hit indices through GFTrackCand::getHit(...)
  m_eventID++;
  StoreArray<Track> tracks;
  if (tracks.getEntries() == 0) {
    return; //probably nothing fitted
  }

  StoreArray<GFTrack> gftracks(m_gftracks_name);
  if (tracks.getEntries() != gftracks.getEntries()) {
    //assuming same indices for tracks and gftracks
    //TODO replace with Tracks<->GFTracks relation when available
    B2FATAL("Tracks and GFTracks have different lengths?");
    return;
  }

  StoreArray<MCParticle> mcparticles;
  const int num_mcparticles = mcparticles.getEntries();

  StoreArray<CDCHit> cdcHits;
  StoreArray<SVDTrueHit> svdTrueHits;
  StoreArray<PXDTrueHit> pxdTrueHits;
  StoreArray<PXDCluster> pxdClusters; //no 1:1 correspondence with PXDTrueHits & PXDRecoHits!

  RelationIndex<PXDCluster, PXDTrueHit> *pxdClustersToTrueHitsIndex = 0;
  if (m_usePXD)
    pxdClustersToTrueHitsIndex = new RelationIndex<PXDCluster, PXDTrueHit> (pxdClusters, pxdTrueHits);

  //output
  StoreArray<DedxLikelihood> *likelihood_array = 0;
  StoreArray<TrackDedx> *dedx_array = 0;
  RelationArray* tracks_to_likelihoods = 0;
  if (!m_pdfFilename.empty()) {
    likelihood_array = new StoreArray<DedxLikelihood>;
    likelihood_array->create();
    tracks_to_likelihoods = new RelationArray(gftracks, *likelihood_array);
    tracks_to_likelihoods->create(gftracks, *likelihood_array); //why specify them again here?
  }
  if (m_enableDebugOutput) {
    dedx_array = new StoreArray<TrackDedx>("TrackDedx", DataStore::c_Event);
    dedx_array->create();
  }


  //loop over all tracks
  for (int iGFTrack = 0; iGFTrack < tracks.getEntries(); iGFTrack++) {
    const int iTrack = iGFTrack;
    TrackDedx track; //temporary storage for track data

    if (num_mcparticles > 0) { //only do this if we actually know the mcparticles
      //find MCParticle corresponding to this track
      const int mcparticle_idx = tracks[iTrack]->getMCId();

      if (mcparticle_idx >= 0 && mcparticles[mcparticle_idx]) { //mc idx is -1 for tracks found in background noise
        if (m_onlyPrimaryParticles && !mcparticles[mcparticle_idx]->hasStatus(MCParticle::c_PrimaryParticle))
          continue; //not a primary particle, ignore

        //add some MC truths to TrackDedx object
        track.m_pdg = mcparticles[mcparticle_idx]->getPDG();
        const MCParticle* mother = mcparticles[mcparticle_idx]->getMother();
        track.m_mother_pdg = mother ? mother->getPDG() : 0;

        //find slow pions (i.e. D* daughter pions)
        track.m_slow_pion = (TMath::Abs(track.m_pdg) == 211 and TMath::Abs(track.m_mother_pdg) == 413);

        const TVector3 true_momentum = mcparticles[mcparticle_idx]->getMomentum();
        track.m_p_true = true_momentum.Mag();
      } else {
        B2INFO("no MCParticle found for current track!");
      }
    }

    m_trackID++;

    GFAbsTrackRep* trackrep = gftracks[iGFTrack]->getCardinalRep();

    //get momentum (at origin) from fitted track
    TVector3 poca, dir_in_poca;
    try {
      trackrep->extrapolateToPoint(TVector3(0, 0, 0), poca, dir_in_poca);

      //Now choose a correct reference plane to get the momentum
      const GFDetPlane plane(poca, dir_in_poca);

      //getMom returns wrong direction, only use magnitude
      const double momentum_mag = trackrep->getMom(plane).Mag();
      dir_in_poca.SetMag(momentum_mag);

      track.m_p_vec = dir_in_poca;
      track.m_p = momentum_mag;
    } catch (...) {
      B2WARNING(m_trackID - 1 << ": Track extrapolation failed (at origin!), skipping particle");
      continue; //next particle
    }

    track.m_chi2 = tracks[iTrack]->getChi2();
    track.m_charge = (short)((tracks[iTrack]->getOmega() >= 0) ? 1 : -1);

    //used for PXD/SVD hits
    const HelixHelper helix_at_origin(poca, dir_in_poca, track.m_charge);

    //sort hits in the order they were created
    //this is required if I want to use the helix path length
    const_cast<GFTrackCand& >(gftracks[iGFTrack]->getCand()).sortHits();
    const GFTrackCand& gftrackcand = gftracks[iGFTrack]->getCand();
    const int num_hits = gftrackcand.getNHits();
    if (num_hits == 0) {
      B2WARNING("Track has no associated hits, skipping");
      continue;
    }

    if (m_useCDC) {
      int num_hits_in_current_layer = 0;
      float layer_charge = 0.0; //total charge in current layer
      float phi_sum = 0.0; //sum of phi from CDC hits, for averaging
      TVector3 last_hit_pos;
      float direct_distance = 0; //sum of distances between wires
#ifdef USE_HELIX_PATH_LENGTH
      float last_path_length = 0; //sum of distances between wires
#endif


      HelixHelper cdchelix = helix_at_origin;
      bool track_extrapolation_failed = false;

      //Loop over all CDC hits from this track
      const std::vector<unsigned int>& cdc_hit_ids = gftrackcand.GetHitIDs(c_CDC);
      const int num_cdc_hits = cdc_hit_ids.size();
      for (int iCDC = 0; iCDC < num_cdc_hits; iCDC++) {
        const int cdc_idx = cdc_hit_ids[iCDC];

        //add data
        const int layer = cdcHits[cdc_idx]->getILayer();
        const int superlayer = cdcHits[cdc_idx]->getISuperLayer();
        const int current_layer = (superlayer == 0) ? layer : (8 + (superlayer - 1) * 6 + layer); //this is essentially the layer ID you'd get from a CDCRecoHit
        const int wire = cdcHits[cdc_idx]->getIWire();
        //B2INFO(m_trackID-1 << ": cdc layer: " << layer);

        static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
        const TVector3& wire_pos_f = cdcgeo.wireForwardPosition(current_layer, wire);
        const TVector3& wire_pos_b = cdcgeo.wireBackwardPosition(current_layer, wire);

        const double path_length = cdchelix.pathLengthToLine(wire_pos_f, wire_pos_b);
        TVector3 hit_pos_helix = cdchelix.position(path_length);
        TVector3 local_momentum = cdchelix.momentum(path_length);
#ifdef USE_HELIX_PATH_LENGTH
        if (last_path_length == 0.0)
          last_path_length = path_length;
#endif

        //get actual hit coordinates by taking z from the helix and x/y from wire (with stereo angle correction)
        double where = (hit_pos_helix.z() - wire_pos_f.z()) / (wire_pos_b.z() - wire_pos_f.z());
        if (where < 0.0 or where > 1.0) {
          B2WARNING("helix extrapolation: Hit outside drift chamber! track_id: " << m_trackID - 1 << ", where: " << where);
          where = (where < 0.0) ? 0.0 : 1.0;
        }
        TVector3 hit_pos = wire_pos_f + where * (wire_pos_b - wire_pos_f);

        //was the helix position close enough to the hit?
        const bool helix_accurate = (hit_pos - hit_pos_helix).Perp() <= m_trackDistanceThreshhold;

        if (!track_extrapolation_failed && !helix_accurate) {
          TVector3 poca_on_wire;
          try {
            trackrep->extrapolateToLine(wire_pos_f, wire_pos_b,
                                        poca, dir_in_poca, poca_on_wire);

            //Now choose a correct reference plane to get the momentum
            GFDetPlane plane(poca, dir_in_poca);

            //update cdchelix
            const double momentum_mag = trackrep->getMom(plane).Mag();
            dir_in_poca.SetMag(momentum_mag);
            cdchelix = HelixHelper(poca, dir_in_poca, track.m_charge);

            local_momentum = dir_in_poca;
            hit_pos_helix = poca;
            hit_pos = poca_on_wire;
            if (hit_pos.Perp() > 111.14 or hit_pos.z() < -83.12 or hit_pos.z() > 158.57) {
              B2WARNING("Genfit extrapolation: Hit outside drift chamber! track_id: " << m_trackID - 1 << ", pos: " << hit_pos.x() << ", " << hit_pos.y() << ", " << hit_pos.z() << ", dist to helix: " << (hit_pos - hit_pos_helix).Mag());
            }
          } catch (...) {
            B2WARNING(m_eventID - 1 << ":" << m_trackID - 1 << ": Track extrapolation failed (in CDC), further hits will be less accurate");

            //if extrapolation fails once, it's unlikely to work again
            track_extrapolation_failed = true;
          }
          m_numExtrapolations++;
        }

        const float phi = hit_pos.Phi() - local_momentum.Phi();

        const float hit_charge = cdcHits[cdc_idx]->getADCCount();

        if (m_enableDebugOutput)
          track.addHit(hit_pos, current_layer, wire, (hit_pos - hit_pos_helix).Perp(), hit_charge, cdcHits[cdc_idx]->getDriftTime());

        num_hits_in_current_layer++;
        layer_charge += hit_charge;
        phi_sum += phi;
        direct_distance += (hit_pos - last_hit_pos).Mag();
        last_hit_pos = hit_pos;


        const bool last_hit = (iCDC + 1 >= num_cdc_hits);
        bool last_hit_in_current_layer = last_hit;
        if (!last_hit) {
          //peek at next hit
          const int next_cdc_idx = cdc_hit_ids[iCDC + 1];
          const int next_layer = cdcHits[next_cdc_idx]->getILayer();
          const int next_superlayer = cdcHits[next_cdc_idx]->getISuperLayer();
          const int next_layer_full = (next_superlayer == 0) ? next_layer : (8 + (next_superlayer - 1) * 6 + next_layer);
          last_hit_in_current_layer = (next_layer_full != current_layer);
        }
        if (last_hit_in_current_layer) {
          //calculate a dE/dx value using all hits in this layer
          const float phi_average = phi_sum / num_hits_in_current_layer;

#ifdef USE_HELIX_PATH_LENGTH
          float total_distance = path_length - last_path_length;
          if (!helix_accurate or total_distance == 0.0) { //path length identical to last layer??
            B2INFO("Path length zero, using simple geometrical correction");
            total_distance = getFlownDistanceCDC(current_layer, track.m_p_vec.Theta(), phi_average);
            total_distance = TMath::Min(direct_distance, total_distance); //total_distance diverges for phi ~ 0
          }
#else
          float total_distance = getFlownDistanceCDC(current_layer, track.m_p_vec.Theta(), phi_average);
          total_distance = TMath::Min(direct_distance, total_distance); //total_distance diverges for phi ~ 0
#endif

          const float layer_dedx = layer_charge / total_distance;

          if (layer_dedx > 0) {
            //store data, if there's energy loss in last layer
            track.m_dedx_avg[c_CDC] += layer_dedx;
            track.addDedx(current_layer, total_distance, layer_dedx);
            if (!m_pdfFilename.empty() and m_useIndividualHits and detectorEnabled(c_CDC)) {
              saveLogLikelihood(track.m_logl, track.m_p, layer_dedx, m_pdfs[c_CDC]);
            }
          }

          //reset layer variables
          num_hits_in_current_layer = 0;
          layer_charge = 0.0; //total charge in current layer
          phi_sum = 0.0; //sum of phi from CDC hits, for averaging
          direct_distance = 0.0;
#ifdef USE_HELIX_PATH_LENGTH
          last_path_length = path_length;
#endif
        }
      }
      //all cdc hits for this track read

      if (!m_useIndividualHits or m_enableDebugOutput) {
        //track.dedx only contains CDC hits right now
        calculateMeans(
          &(track.m_dedx_avg[c_CDC]),
          &(track.m_dedx_avg_truncated[c_CDC]),
          &(track.m_dedx_avg_truncated_err[c_CDC]),
          track.dedx);
      }
    }

    if (m_usePXD) {
      //get indices of PXDTrueHits
      const std::vector<unsigned int>& pxdHitIDs = gftrackcand.GetHitIDs(c_PXD);

      //and construct a list of associated PXDCluster indices
      std::vector<unsigned int> pxdClusterIDs;
      for (unsigned int iHit = 0; iHit < pxdHitIDs.size(); iHit++) {
        typedef RelationIndex<PXDCluster, PXDTrueHit>::Element relElement_t;
        unsigned int hitID = pxdHitIDs[iHit];
        //get all relations that point to hitID
        BOOST_FOREACH(const relElement_t & rel, pxdClustersToTrueHitsIndex->getElementsTo(pxdTrueHits[hitID])) {
          pxdClusterIDs.push_back(rel.indexFrom);
        }
      }
      sort(pxdClusterIDs.begin(), pxdClusterIDs.end());
      saveSiHits(&track, helix_at_origin, pxdClusters, pxdClusterIDs);
    }

    if (m_useSVD) {
      //no way to access digitized SVD hits, so we'll just use the SVDTrueHits directly
      const std::vector<unsigned int>& svd_hit_ids = gftrackcand.GetHitIDs(c_SVD);
      saveSiHits(&track, helix_at_origin, svdTrueHits, svd_hit_ids);
    }


    if (track.dedx.empty()) {
      B2WARNING("Found track with no hits, ignoring.");
      continue;
    }

    if (!m_useIndividualHits) {
      //calculate likelihoods for truncated mean
      for (int detector = 0; detector < c_Dedx_num_detectors; detector++) {
        if (!detectorEnabled(static_cast<DedxDetector>(detector)))
          continue; //unwanted detector

        saveLogLikelihood(track.m_logl, track.m_p, track.m_dedx_avg_truncated[detector], m_pdfs[detector]);
      }
    }

    //save this track's data to StoreArray
    if (m_enableDebugOutput) {
      track.m_event_id = m_eventID - 1;
      track.m_track_id = m_trackID - 1;

      dedx_array->appendNew(track);
    }
    if (likelihood_array) {
      //save likelihoods
      const int dedxLikelihoodIdx = likelihood_array->getEntries();
      new(likelihood_array->nextFreeAddress()) DedxLikelihood(track.m_logl, track.m_p);
      tracks_to_likelihoods->add(iGFTrack, dedxLikelihoodIdx);
    }
  } //end loop over tracks

  delete dedx_array;
  delete tracks_to_likelihoods;
  delete likelihood_array;
  delete pxdClustersToTrueHitsIndex;
}


void DedxPIDModule::terminate()
{
  B2INFO("DedxPIDModule exiting after processing " << m_trackID << " tracks in " << m_eventID << " events.");
  B2INFO("reevaluated helix " << m_numExtrapolations << " times.");
}


void DedxPIDModule::calculateMeans(float* mean, float* truncatedMean, float* truncatedMeanErr, const std::vector<float>& dedx) const
{
  //build truncated average by skipping over lowest & highest events in dEdX_sorted
  std::vector<float> sortedDedx = dedx;
  std::sort(sortedDedx.begin(), sortedDedx.end());

  double truncatedMeanTmp = 0.0;
  double meanTmp = 0.0;
  double sum_of_squares = 0.0;
  int numValuesTrunc = 0;
  const int numDedx = sortedDedx.size();
  const int lowEdgeTrunc = int(numDedx * m_removeLowest);
  const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest));
  for (int i = 0; i < numDedx; i++) {
    meanTmp += sortedDedx[i];
    if (i >= lowEdgeTrunc and i < highEdgeTrunc) {
      truncatedMeanTmp += sortedDedx[i];
      sum_of_squares += sortedDedx[i] * sortedDedx[i];
      numValuesTrunc++;
    }
  }

  if (numDedx != 0) {
    meanTmp /= numDedx;
  }
  if (numValuesTrunc != 0) {
    truncatedMeanTmp /= numValuesTrunc;
  } else {
    truncatedMeanTmp = meanTmp;
  }

  *mean = meanTmp;
  *truncatedMean = truncatedMeanTmp;

  if (numValuesTrunc > 1) {
    *truncatedMeanErr = sqrt(sum_of_squares / double(numValuesTrunc) - truncatedMeanTmp * truncatedMeanTmp) / double(numValuesTrunc - 1);
  } else {
    *truncatedMeanErr = 0;
  }
}


float DedxPIDModule::getFlownDistanceCDC(int layerid, float theta, float phi)
{
  static CDCGeometryPar& geo = CDCGeometryPar::Instance();

  //distance d between layers (smaller in inner superlayers)
  const double d = geo.outerRadiusWireLayer()[layerid] - geo.innerRadiusWireLayer()[layerid];

  return TMath::Abs(d / (sin(theta) * cos(phi)));
}

//assume HitClass provides getU/V(), getSensorID(), getEnergyDep()
//true for SVDRecoHit2D, PXDRecoHit, and associated TrueHits
template <class HitClass> void DedxPIDModule::saveSiHits(TrackDedx* track, const HelixHelper& helix, const StoreArray<HitClass> &hits, const std::vector<unsigned int> &hit_indices) const
{
  const int num_hits = hit_indices.size();
  if (num_hits == 0)
    return;

  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //figure out which detector to assign hits to
  const int current_detector = geo.get(hits[hit_indices.at(0)]->getSensorID()).getType();
  assert(current_detector == c_PXD or current_detector == c_SVD);

  std::vector<float> silicon_dedx(num_hits); //used for averages

  for (int i = 0; i < num_hits; i++) {
    const HitClass* hit = hits[hit_indices.at(i)];
    if (!hit) {
      B2ERROR("Silicon hit index out of bounds!");
      continue;
    }

    const TVector3 local_pos(hit->getU(), hit->getV(), 0.0); //z-component is height over the center of the detector plane
    const VXD::SensorInfoBase& sensor = geo.get(hit->getSensorID());
    const TVector3& global_pos = sensor.pointToGlobal(local_pos);

    //assumption: Si detectors are close enough to the origin that this is still accurate
    const TVector3& local_momentum = helix.momentum(helix.pathLengthToPoint(global_pos));

    const TVector3& sensor_normal = sensor.vectorToGlobal(TVector3(0.0, 0.0, 1.0));
    const double angle = sensor_normal.Angle(local_momentum); //includes theta and phi components
    const int layer = -hit->getSensorID().getLayerNumber();
    assert(layer >= -6 && layer < 0);
    const float hit_edep = get_edep(hit); //not a charge, but at least it's uncorrected

    if (m_enableDebugOutput)
      track->addHit(global_pos, layer, hit->getSensorID(), angle, hit_edep);


    //active medium traversed, in cm (can traverse one sensor at most)
    //I'm assuming there's only one hit per sensor, there are _very_ rare exceptions to that (most likely curlers)
    const double total_distance = TMath::Min(sensor.getWidth(), sensor.getThickness() / fabs(cos(angle)));
    const float dedx = hit_edep / total_distance;
    if (dedx > 0) {
      //store data, if there's energy loss in last layer
      silicon_dedx.push_back(dedx);
      track->m_dedx_avg[current_detector] += dedx;
      track->addDedx(layer, total_distance, dedx);
      if (!m_pdfFilename.empty() and m_useIndividualHits and detectorEnabled(static_cast<DedxDetector>(current_detector))) {
        saveLogLikelihood(track->m_logl, track->m_p, dedx, m_pdfs[current_detector]);
      }
    } else {
      B2WARNING("dE/dx is zero in layer " << layer);
    }
  }

  //save averages averages
  if (!m_useIndividualHits or m_enableDebugOutput) {
    calculateMeans(
      &(track->m_dedx_avg[current_detector]),
      &(track->m_dedx_avg_truncated[current_detector]),
      &(track->m_dedx_avg_truncated_err[current_detector]),
      silicon_dedx);
  }
}


void DedxPIDModule::saveLogLikelihood(float(&logl)[c_Dedx_num_particles], float p, float dedx, TH2F* const* pdf) const
{
  //all pdfs have the same dimensions
  const Int_t bin_x = pdf[0]->GetXaxis()->FindFixBin(p);
  const Int_t bin_y = pdf[0]->GetYaxis()->FindFixBin(dedx);

  for (int iParticle = 0; iParticle < c_Dedx_num_particles; iParticle++) {
    if (!pdf[iParticle])
      continue;
    double probability = 0.0;

    //check if this is still in the histogram, take overflow bin otherwise
    if (bin_x < 1 or bin_x > pdf[iParticle]->GetNbinsX()
        or bin_y < 1 or bin_y > pdf[iParticle]->GetNbinsY()) {
      probability = pdf[iParticle]->GetBinContent(bin_x, bin_y);
    } else {
      //in normal histogram range
      probability = pdf[iParticle]->Interpolate(p, dedx);
    }

    if (probability != probability)
      B2ERROR("probability NAN for a track with p=" << p << " and dedx=" << dedx);

    //my pdfs aren't perfect...
    if (probability == 0.0)
      probability = m_useIndividualHits ? (1e-5) : (1e-3); //likelihoods for truncated mean are much higher

    logl[iParticle] += log(probability);
  }
}
