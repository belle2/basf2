#include <reconstruction/modules/dedxPID/DedxPIDModule.h>
#include <reconstruction/modules/dedxPID/HelixHelper.h>

#include <reconstruction/dataobjects/DedxTrack.h>
#include <reconstruction/dataobjects/DedxLikelihood.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Const.h>
#include <framework/utilities/FileSystem.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <vxd/geometry/GeoCache.h>
#include <geometry/GeometryManager.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include <genfit/TrackCand.h>
#include <genfit/Track.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/Exception.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/StateOnPlane.h>

#include <TFile.h>
#include <TGeoManager.h>
#include <TH2F.h>
#include <TMath.h>

#include <memory>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <utility>

using namespace Belle2;
using namespace CDC;
using namespace Dedx;

REG_MODULE(DedxPID)

//if defined, use path length along a helical track instead of step-by-step geometrical correction
//in theory this should be more accurate, unsure why it doesn't work
//#define USE_HELIX_PATH_LENGTH



DedxPIDModule::DedxPIDModule() : Module(),
  m_pdfs()
{
  setPropertyFlags(c_ParallelProcessingCertified);

  //Set module properties
  setDescription("Extract dE/dx and corresponding log-likelihood from fitted tracks and hits in the CDC, SVD and PXD.");

  //Parameter definitions
  addParam("useIndividualHits", m_useIndividualHits, "Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used.", true);
  addParam("removeLowest", m_removeLowest, "portion of events with low dE/dx that should be discarded if UseIndividualHits is false", double(0.0));
  addParam("removeHighest", m_removeHighest, "portion of events with high dE/dx that should be discarded if UseIndividualHits is false", double(0.8));

  addParam("onlyPrimaryParticles", m_onlyPrimaryParticles, "Only save data for primary particles (as determined by MC truth)", false);
  addParam("usePXD", m_usePXD, "Use PXDClusters for dE/dx calculation", false);
  addParam("useSVD", m_useSVD, "Use SVDClusters for dE/dx calculation", true);
  addParam("useCDC", m_useCDC, "Use CDCHits for dE/dx calculation", true);

  addParam("trackDistanceThreshold", m_trackDistanceThreshhold, "Use a faster helix parametrisation, with corrections as soon as the approximation is more than ... cm off.", double(4.0));
  addParam("enableDebugOutput", m_enableDebugOutput, "Wether to save information on tracks and associated hits and dE/dx values in DedxTrack objects.", false);

  addParam("pdfFile", m_pdfFile, "The dE/dx:momentum PDF file to use. Use an empty string to disable classification.", std::string("/data/reconstruction/dedxPID_PDFs_r8682_200k_events_upper_80perc_trunc.root"));
  addParam("ignoreMissingParticles", m_ignoreMissingParticles, "Ignore particles for which no PDFs are found", false);

  m_eventID = -1;
  m_trackID = 0;
  m_numExtrapolations = 0;
}


DedxPIDModule::~DedxPIDModule() { }


void DedxPIDModule::initialize()
{
  if (!m_pdfFile.empty()) {

    std::string fullPath = FileSystem::findFile(m_pdfFile);
    if (fullPath.empty()) {
      B2ERROR("PDF file " << m_pdfFile << " not found!");
    }
    m_pdfFile = fullPath;
  }
  if (!m_enableDebugOutput and m_pdfFile.empty()) {
    B2ERROR("No PDFFile given and debug output disabled. DedxPID module will produce no output!");
  }

  //required inputs
  StoreArray<genfit::Track>::required();
  StoreArray<Track>::required();
  StoreArray<TrackFitResult>::required();
  StoreArray<genfit::TrackCand>::required();

  //optional inputs
  StoreArray<MCParticle>::optional();
  if (m_useCDC)
    StoreArray<CDCHit>::required();
  else
    StoreArray<CDCHit>::optional();
  if (m_useSVD)
    StoreArray<SVDCluster>::required();
  else
    StoreArray<SVDCluster>::optional();
  if (m_usePXD)
    StoreArray<PXDCluster>::required();
  else
    StoreArray<PXDCluster>::optional();

  //register outputs (if needed)
  if (m_enableDebugOutput) {
    StoreArray<DedxTrack>::registerPersistent();
    RelationArray::registerPersistent<Track, DedxTrack>();
  }

  if (!m_pdfFile.empty()) {
    StoreArray<DedxLikelihood>::registerPersistent();
    RelationArray::registerPersistent<Track, DedxLikelihood>();

    //load pdfs
    TFile* pdf_file = new TFile(m_pdfFile.c_str(), "READ");
    if (!pdf_file->IsOpen())
      B2FATAL("Couldn't open pdf file: " << m_pdfFile);

    //load dedx:momentum PDFs
    const char* suffix = (!m_useIndividualHits) ? "_trunc" : "";
    for (int detector = 0; detector < c_num_detectors; detector++) {
      int nBinsX, nBinsY;
      double xMin, xMax, yMin, yMax;
      nBinsX = nBinsY = -1;
      xMin = xMax = yMin = yMax = 0.0;
      for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
        const int pdg_code = Const::chargedStableSet.at(iPart).getPDGCode();
        m_pdfs[detector][iPart] =
          dynamic_cast<TH2F*>(pdf_file->Get(TString::Format("hist_d%i_%i%s", detector, pdg_code, suffix)));

        if (!m_pdfs[detector][iPart]) {
          if (m_ignoreMissingParticles)
            continue;
          B2FATAL("Couldn't find PDF for PDG " << pdg_code << ", detector " << detector << suffix);
        }

        //check that PDFs have the same dimensions and same binning
        const double eps_factor = 1e-5;
        if (nBinsX == -1 and nBinsY == -1) {
          nBinsX = m_pdfs[detector][iPart]->GetNbinsX();
          nBinsY = m_pdfs[detector][iPart]->GetNbinsY();
          xMin = m_pdfs[detector][iPart]->GetXaxis()->GetXmin();
          xMax = m_pdfs[detector][iPart]->GetXaxis()->GetXmax();
          yMin = m_pdfs[detector][iPart]->GetYaxis()->GetXmin();
          yMax = m_pdfs[detector][iPart]->GetYaxis()->GetXmax();
        } else if (nBinsX != m_pdfs[detector][iPart]->GetNbinsX()
                   or nBinsY != m_pdfs[detector][iPart]->GetNbinsY()
                   or fabs(xMin - m_pdfs[detector][iPart]->GetXaxis()->GetXmin()) > eps_factor * xMax
                   or fabs(xMax - m_pdfs[detector][iPart]->GetXaxis()->GetXmax()) > eps_factor * xMax
                   or fabs(yMin - m_pdfs[detector][iPart]->GetYaxis()->GetXmin()) > eps_factor * yMax
                   or fabs(yMax - m_pdfs[detector][iPart]->GetYaxis()->GetXmax()) > eps_factor * yMax) {
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

    //initialize some things for genfit
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());
  }
}


void DedxPIDModule::event()
{
  //go through Tracks
  //get fitresult and gftrack and do extrapolations, save corresponding dE/dx and likelihood values
  //  !check if i've already done extrapolations for the given result (i.e. I got less than 5 fits)
  //   get genfit::TrackCand through genfit::Track::getCand()
  //   get hit indices through genfit::TrackCand::getHit(...)
  //   create one DedxTrack per fitresult/gftrack
  //create one DedkLikelihood per Track (plus rel)
  m_eventID++;

  StoreArray<Track> tracks;
  StoreArray<MCParticle> mcparticles;
  const int num_mcparticles = mcparticles.getEntries();

  StoreArray<CDCHit> cdcHits;
  StoreArray<PXDCluster> pxdClusters;
  StoreArray<SVDCluster> svdClusters;

  //outputs
  StoreArray<DedxLikelihood> likelihood_array;
  StoreArray<DedxTrack> dedx_array;

  //loop over all tracks
  for (int iTrack = 0; iTrack < tracks.getEntries(); iTrack++) {
    const Track* track = tracks[iTrack];
    m_trackID++;

    //get pion fit hypothesis
    //should be ok in most cases, for MC fitting this will return the fit with the true PDG value
    //at some point, it might be worthwhile to look into using a different fit if the differences are large
    const TrackFitResult* fitResult = track->getTrackFitResult(Const::pion);
    if (!fitResult) {
      B2ERROR("No TrackFitResult, skipping Track " << iTrack << ".");
      continue;
    }

    std::shared_ptr<DedxTrack> dedxTrack = std::make_shared<DedxTrack>();

    if (m_enableDebugOutput or m_onlyPrimaryParticles) {
      if (num_mcparticles != 0) {
        //find MCParticle corresponding to this track
        const MCParticle* mcpart = track->getRelatedTo<MCParticle>();

        if (mcpart) {
          if (m_onlyPrimaryParticles && !mcpart->hasStatus(MCParticle::c_PrimaryParticle)) {
            continue; //not a primary particle, ignore
          }

          //add some MC truths to DedxTrack object
          dedxTrack->m_pdg = mcpart->getPDG();
          const MCParticle* mother = mcpart->getMother();
          dedxTrack->m_mother_pdg = mother ? mother->getPDG() : 0;

          //find slow pions (i.e. D* daughter pions)
          dedxTrack->m_slow_pion = (TMath::Abs(dedxTrack->m_pdg) == 211 and TMath::Abs(dedxTrack->m_mother_pdg) == 413);

          const TVector3 true_momentum = mcpart->getMomentum();
          dedxTrack->m_p_true = true_momentum.Mag();
        } else {
          B2WARNING("No MCParticle found for current track!");
        }
      }
    }

    //calculate dE/dx values using associated genfit::Track
    const genfit::Track* gftrack = fitResult->getRelatedFrom<genfit::Track>();
    genfit::TrackCand gftrackcand = *fitResult->getRelatedFrom<genfit::TrackCand>();
    genfit::AbsTrackRep* trackrep = gftrack->getCardinalRep();

    //get momentum (at origin) from fit result
    const TVector3& poca = fitResult->getPosition();
    const TVector3& poca_momentum = fitResult->getMomentum();
    const genfit::AbsFitterInfo* fitterInfo = gftrack->getPointWithMeasurement(0)->getFitterInfo(trackrep);
    if (!fitterInfo) {
      B2ERROR("No fitterInfo found, skipping track.")
      continue;
    }
    genfit::StateOnPlane pocaState = fitterInfo->getFittedState(true);

    dedxTrack->m_pdg_hyp = fitResult->getParticleType().getPDGCode();
    dedxTrack->m_chi2 = gftrack->getFitStatus(trackrep)->getChi2();
    dedxTrack->m_charge = (short)((gftrack->getFitStatus(trackrep)->getCharge() >= 0) ? 1 : -1);
    dedxTrack->m_p_vec = poca_momentum;
    dedxTrack->m_p = poca_momentum.Mag();

    //used for PXD/SVD hits
    const HelixHelper helix_at_origin(poca, poca_momentum, dedxTrack->m_charge);

    //sort hits in the order they were created
    //this is required if I want to use the helix path length
    //genfit::TrackCand gftrackcand = gftrack->getCand();
    gftrackcand.sortHits();
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
      const std::vector<int>& cdc_hit_ids = gftrackcand.getHitIDs(Const::CDC);
      const int num_cdc_hits = cdc_hit_ids.size();
      for (int iCDC = 0; iCDC < num_cdc_hits; iCDC++) {
        const int cdc_idx = cdc_hit_ids[iCDC];

        //add data
        const int layer = cdcHits[cdc_idx]->getILayer();
        const int superlayer = cdcHits[cdc_idx]->getISuperLayer();
        const int current_layer = (superlayer == 0) ? layer : (8 + (superlayer - 1) * 6 + layer); //this is essentially the layer ID you'd get from a CDCRecoHit
        const int wire = cdcHits[cdc_idx]->getIWire();

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
        const double tolerance = 0.1; //some deviation is ok
        if (where < 0.0 - tolerance or where > 1.0 + tolerance) {
          B2WARNING("Event " << m_eventID << ", Track: " << iTrack << ": helix extrapolation: Hit outside drift chamber! where: " << where);
          where = (where < 0.0 - tolerance) ? 0.0 - tolerance : 1.0 + tolerance;
        }
        TVector3 hit_pos = wire_pos_f + where * (wire_pos_b - wire_pos_f);

        //was the helix position close enough to the hit?
        const bool helix_accurate = (hit_pos - hit_pos_helix).Perp() <= m_trackDistanceThreshhold;

        if (!track_extrapolation_failed && !helix_accurate) {
          try {
            TVector3 wire_dir = (wire_pos_b - wire_pos_f).Unit();
            trackrep->extrapolateToLine(pocaState, wire_pos_f, wire_dir);
            TVector3 poca = trackrep->getPos(pocaState);
            TVector3 pocaMom = trackrep->getMom(pocaState);

            //update cdchelix
            cdchelix = HelixHelper(poca, pocaMom, dedxTrack->m_charge);

            TVector3 poca_on_wire = wire_pos_f + (poca - wire_pos_f).Dot(wire_dir) * wire_dir;


            local_momentum = pocaMom;
            hit_pos_helix = poca;
            hit_pos = poca_on_wire;
          } catch (genfit::Exception) {
            B2WARNING("Event " << m_eventID << ", Track: " << iTrack << ": genfit::Track extrapolation failed (in CDC), further hits will be less accurate");

            //if extrapolation fails once, it's unlikely to work again
            track_extrapolation_failed = true;
          }
          m_numExtrapolations++;
        }

        const float phi = hit_pos.Phi() - local_momentum.Phi();

        const float hit_charge = cdcHits[cdc_idx]->getADCCount();

        if (m_enableDebugOutput)
          dedxTrack->addHit(hit_pos, current_layer, wire, (hit_pos - hit_pos_helix).Perp(), hit_charge, track_extrapolation_failed, cdcHits[cdc_idx]->getTDCCount());

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
            total_distance = getTraversedLengthCDC(current_layer, dedxTrack->m_p_vec.Theta(), phi_average);
            total_distance = TMath::Min(direct_distance, total_distance); //total_distance diverges for phi ~ 0
          }
#else
          float total_distance = getTraversedLengthCDC(current_layer, dedxTrack->m_p_vec.Theta(), phi_average);
          total_distance = TMath::Min(direct_distance, total_distance); //total_distance diverges for phi ~ 0
#endif

          const float layer_dedx = layer_charge / total_distance;

          if (layer_dedx > 0) {
            //store data, if there's energy loss in last layer
            dedxTrack->m_dedx_avg[c_CDC] += layer_dedx;
            dedxTrack->addDedx(current_layer, total_distance, layer_dedx);
            if (!m_pdfFile.empty() and m_useIndividualHits) {
              saveLogLikelihood(dedxTrack->m_cdcLogl, dedxTrack->m_p, layer_dedx, m_pdfs[c_CDC]);
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
        //dedxTrack.dedx only contains CDC hits right now
        calculateMeans(
          &(dedxTrack->m_dedx_avg[c_CDC]),
          &(dedxTrack->m_dedx_avg_truncated[c_CDC]),
          &(dedxTrack->m_dedx_avg_truncated_err[c_CDC]),
          dedxTrack->dedx);
      }
    }

    if (m_usePXD) {
      const std::vector<int>& pxdClusterIDs = gftrackcand.getHitIDs(Const::PXD);
      saveSiHits(dedxTrack.get(), helix_at_origin, pxdClusters, pxdClusterIDs);
    }

    if (m_useSVD) {
      const std::vector<int>& svdClusterIDs = gftrackcand.getHitIDs(Const::SVD);
      saveSiHits(dedxTrack.get(), helix_at_origin, svdClusters, svdClusterIDs);
    }


    if (dedxTrack->dedx.empty()) {
      B2WARNING("Found track with no hits, ignoring.");
      continue;
    }

    if (!m_useIndividualHits) {
      //calculate likelihoods for truncated mean
      for (int detector = 0; detector < c_num_detectors; detector++) {
        if (!detectorEnabled(static_cast<Detector>(detector)))
          continue; //unwanted detector

        saveLogLikelihood((detector == c_CDC) ? dedxTrack->m_cdcLogl : dedxTrack->m_svdLogl,
                          dedxTrack->m_p, dedxTrack->m_dedx_avg_truncated[detector], m_pdfs[detector]);
      }
    }

    //save this track's data to StoreArray
    if (m_enableDebugOutput) {
      dedxTrack->m_event_id = m_eventID;
      dedxTrack->m_track_id = iTrack;

      DedxTrack* new_dedxTrack = dedx_array.appendNew(*dedxTrack);
      track->addRelationTo(new_dedxTrack);
    }

    //save DedxLikelihood
    if (!m_pdfFile.empty()) {
      DedxLikelihood* likelihoodObj = likelihood_array.appendNew(dedxTrack->m_cdcLogl, dedxTrack->m_svdLogl);
      track->addRelationTo(likelihoodObj);
    }
  } //end loop over tracks
}


void DedxPIDModule::terminate()
{
  B2INFO("DedxPIDModule exiting after processing " << m_trackID << " tracks in " << m_eventID + 1 << " events.");
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


float DedxPIDModule::getTraversedLengthCDC(int layerid, float theta, float phi)
{
  static CDCGeometryPar& geo = CDCGeometryPar::Instance();

  //distance d between layers (smaller in inner superlayers)
  const double d = geo.outerRadiusWireLayer()[layerid] - geo.innerRadiusWireLayer()[layerid];

  return TMath::Abs(d / (sin(theta) * cos(phi)));
}

double DedxPIDModule::getTraversedLength(const PXDCluster* hit, const HelixHelper* helix)
{
  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensor = geo.get(hit->getSensorID());

  const TVector3 local_pos(hit->getU(), hit->getV(), 0.0); //z-component is height over the center of the detector plane
  const TVector3& global_pos = sensor.pointToGlobal(local_pos);
  const TVector3& local_momentum = helix->momentum(helix->pathLengthToPoint(global_pos));

  const TVector3& sensor_normal = sensor.vectorToGlobal(TVector3(0.0, 0.0, 1.0));
  const double angle = sensor_normal.Angle(local_momentum); //includes theta and phi components

  //I'm assuming there's only one hit per sensor, there are _very_ rare exceptions to that (most likely curlers)
  return TMath::Min(sensor.getWidth(), sensor.getThickness() / fabs(cos(angle)));
}

double DedxPIDModule::getTraversedLength(const SVDCluster* hit, const HelixHelper* helix)
{
  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensor = geo.get(hit->getSensorID());

  TVector3 a, b;
  if (hit->isUCluster()) {
    const float u = hit->getPosition();
    a = sensor.pointToGlobal(TVector3(sensor.getBackwardWidth() / sensor.getWidth(0) * u, -0.5 * sensor.getLength(), 0.0));
    b = sensor.pointToGlobal(TVector3(sensor.getForwardWidth() / sensor.getWidth(0) * u, +0.5 * sensor.getLength(), 0.0));
  } else {
    const float v = hit->getPosition();
    a = sensor.pointToGlobal(TVector3(-0.5 * sensor.getWidth(v), v, 0.0));
    b = sensor.pointToGlobal(TVector3(+0.5 * sensor.getWidth(v), v, 0.0));
  }
  const double path_length = helix->pathLengthToLine(a, b);
  const TVector3& local_momentum = helix->momentum(path_length);

  const TVector3& sensor_normal = sensor.vectorToGlobal(TVector3(0.0, 0.0, 1.0));
  const double angle = sensor_normal.Angle(local_momentum); //includes theta and phi components

  //I'm assuming there's only one hit per sensor, there are _very_ rare exceptions to that (most likely curlers)
  return TMath::Min(sensor.getWidth(), sensor.getThickness() / fabs(cos(angle)));
}

template <class HitClass> void DedxPIDModule::saveSiHits(DedxTrack* track, const HelixHelper& helix, const StoreArray<HitClass>& hits, const std::vector<int>& hit_indices) const
{
  const int num_hits = hit_indices.size();
  if (num_hits == 0)
    return;

  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //figure out which detector to assign hits to
  const int current_detector = geo.get(hits[hit_indices.at(0)]->getSensorID()).getType();
  assert(current_detector == VXD::SensorInfoBase::PXD or current_detector == VXD::SensorInfoBase::SVD);
  assert(current_detector <= 1); //used as array index

  std::vector<float> silicon_dedx; //used for averages
  silicon_dedx.reserve(num_hits);

  for (int i = 0; i < num_hits; i++) {
    const HitClass* hit = hits[hit_indices.at(i)];
    if (!hit) {
      B2ERROR(hits.getName() << " index out of bounds!");
      continue;
    }
    int layer = -1;
    if (m_enableDebugOutput) {
      layer = -hit->getSensorID().getLayerNumber();
      assert(layer >= -6 && layer < 0);
    }

    //active medium traversed, in cm (can traverse one sensor at most)
    //assumption: Si detectors are close enough to the origin that helix is still accurate
    const double total_distance = getTraversedLength(hit, &helix);
    const float charge = hit->getCharge();
    const float dedx = charge / total_distance;
    if (dedx > 0) {
      //store data, if there's energy loss in last layer
      silicon_dedx.push_back(dedx);
      track->m_dedx_avg[current_detector] += dedx;
      track->addDedx(layer, total_distance, dedx);
      if (!m_pdfFile.empty() and m_useIndividualHits) {
        saveLogLikelihood(track->m_svdLogl, track->m_p, dedx, m_pdfs[current_detector]);
      }
    } else {
      B2WARNING("dE/dx is zero in layer " << layer);
    }

    if (m_enableDebugOutput) {
      track->addHit(TVector3(0, 0, 0), layer, hit->getSensorID(), 0, charge, false);
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


void DedxPIDModule::saveLogLikelihood(float(&logl)[Const::ChargedStable::c_SetSize], float p, float dedx, TH2F* const* pdf) const
{
  //all pdfs have the same dimensions
  const Int_t bin_x = pdf[0]->GetXaxis()->FindFixBin(p);
  const Int_t bin_y = pdf[0]->GetYaxis()->FindFixBin(dedx);

  for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
    if (!pdf[iPart]) //might be NULL if m_ignoreMissingParticles is set
      continue;
    double probability = 0.0;

    //check if this is still in the histogram, take overflow bin otherwise
    if (bin_x < 1 or bin_x > pdf[iPart]->GetNbinsX()
        or bin_y < 1 or bin_y > pdf[iPart]->GetNbinsY()) {
      probability = pdf[iPart]->GetBinContent(bin_x, bin_y);
    } else {
      //in normal histogram range
      probability = pdf[iPart]->Interpolate(p, dedx);
    }

    if (probability != probability)
      B2ERROR("probability NAN for a track with p=" << p << " and dedx=" << dedx);

    //my pdfs aren't perfect...
    if (probability == 0.0)
      probability = m_useIndividualHits ? (1e-5) : (1e-3); //likelihoods for truncated mean are much higher

    logl[iPart] += log(probability);
  }
}
