//For GFTrack visualisation:
/* Copyright 2011, Technische Universitaet Muenchen,
   Author: Karl Bicker

   This file is part of GENFIT.

   GENFIT is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   GENFIT is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GENFIT.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <display/EVEVisualization.h>

#include <display/dataobjects/DisplayData.h>
#include <display/VisualRepMap.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <vxd/geometry/GeoCache.h>
#include <bklm/dataobjects/BKLMSimHitPosition.h>

#include <svd/reconstruction/SVDRecoHit.h>
#include <genfit/AbsMeasurement.h>
#include <genfit/PlanarMeasurement.h>
#include <genfit/SpacepointMeasurement.h>
#include <genfit/WireMeasurement.h>
#include <genfit/WirePointMeasurement.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/DetPlane.h>
#include <genfit/Exception.h>
#include <genfit/KalmanFitterInfo.h>

#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitter.h>
#include <genfit/DAF.h>
#include <genfit/KalmanFitterRefTrack.h>

#include <TEveArrow.h>
#include <TEveBox.h>
#include <TEveCalo.h>
#include <TEveManager.h>
#include <TEveGeoNode.h>
#include <TEveGeoShape.h>
#include <TEveGeoShapeExtract.h>
#include <TEveLine.h>
#include <TEvePointSet.h>
#include <TEveSelection.h>
#include <TEveStraightLineSet.h>
#include <TEveTriangleSet.h>
#include <TEveText.h>
#include <TEveTrack.h>
#include <TEveTrackPropagator.h>
#include <TFile.h>
#include <TGeoEltu.h>
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TGeoNode.h>
#include <TGeoSphere.h>
#include <TGeoTube.h>
#include <TGLLogicalShape.h>
#include <TParticle.h>
#include <TMath.h>
#include <TMatrixT.h>
#include <TMatrixDEigen.h>

#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <assert.h>
#include <cmath>


using namespace Belle2;

const double MIN_P_CUT = 0.00;

EVEVisualization::EVEVisualization():
  m_fullgeo(false),
  m_assignToPrimaries(false),
  m_trackcandlist(0),
  m_eclData(0),
  m_unassignedRecoHits(0),
  m_visualRepMap(new VisualRepMap())
{
  setErrScale();

  TGLLogicalShape::SetIgnoreSizeForCameraInterest(kTRUE);     // Allows the visualization of the "small" error ellipsoid.

  //create new containers
  m_trackpropagator = new TEveTrackPropagator();
  m_trackpropagator->IncDenyDestroy();
  m_trackpropagator->SetMagFieldObj(&m_bfield, false);
  m_trackpropagator->SetFitDaughters(false); //most secondaries are no longer immediate daughters since we might discard those!

  //find a point that is inside the top node
  TGeoVolume* top_node = gGeoManager->GetTopNode()->GetVolume();
  double p[3] = { 380.0, 0.0, 0.0 }; //ok for normal Belle II geometry
  while (!top_node->Contains(p)) {
    p[0] *= 0.8;
  }
  m_trackpropagator->SetMaxR(p[0]); //don't draw tracks outside detector

  //TODO is this actually needed?
  m_trackpropagator->SetMaxStep(1.0); //make sure to reeval magnetic field often enough

  m_tracklist = new TEveTrackList(m_trackpropagator);
  m_tracklist->IncDenyDestroy();
  m_tracklist->SetName("MCParticles");
  m_tracklist->SelectByP(MIN_P_CUT, 15.0); //don't show too many particles by default...

  m_gftrackpropagator = new TEveTrackPropagator();
  m_gftrackpropagator->IncDenyDestroy();
  m_gftrackpropagator->SetMagFieldObj(&m_bfield, false);
  m_gftrackpropagator->SetMaxOrbs(0.5); //stop after track markers

  m_calo3d = new TEveCalo3D(NULL, "ECLClusters");
  m_calo3d->SetBarrelRadius(125.80); //inner radius of ECL barrel
  m_calo3d->SetForwardEndCapPos(196.5); //inner edge of forward endcap
  m_calo3d->SetBackwardEndCapPos(-102.0); //inner edge of backward endcap
  m_calo3d->SetMaxValAbs(2.1);
  m_calo3d->SetRnrFrame(false, false); //don't show crystal grid
  m_calo3d->IncDenyDestroy();

  //Stop eve from deleting contents... (which might already be deleted)
  gEve->GetSelection()->IncDenyDestroy();
  gEve->GetHighlight()->IncDenyDestroy();

  clearEvent();
}

void EVEVisualization::setOptions(const std::string& opts) { m_options = opts; }

void EVEVisualization::setErrScale(double errScale) { m_errorScale = errScale; }

EVEVisualization::~EVEVisualization()
{
  //my objects
  delete m_visualRepMap;

  if (!gEve)
    return; //objects are probably already freed by Eve

  //Eve objects
  delete m_eclData;
  delete m_unassignedRecoHits;
  delete m_tracklist;
  delete m_trackcandlist;
  delete m_trackpropagator;
  delete m_gftrackpropagator;
  delete m_calo3d;
}

void EVEVisualization::enableVolume(const char* name, bool only_daughters, bool enable)
{
  TGeoVolume* vol = gGeoManager->FindVolumeFast(name);
  if (vol) {
    if (!only_daughters)
      vol->SetVisibility(enable);
    vol->SetVisDaughters(enable);
  } else {
    B2DEBUG(100, "Volume " << name << " not found?");
  }
}

void EVEVisualization::setVolumeColor(const char* name, Color_t col)
{
  TGeoVolume* vol = gGeoManager->FindVolumeFast(name);
  if (vol) {
    //while TGeoVolume derives from TAttFill, the line color actually is important here
    vol->SetLineColor(col);
  } else {
    B2DEBUG(100, "Volume " << name << " not found?");
  }
}

void EVEVisualization::addGeometry()
{
  if (!gGeoManager)
    return;

  B2DEBUG(100, "Setting up geometry for TEve...");
  //set colours by atomic mass number
  gGeoManager->DefaultColors();

  //Set transparency of geometry
  TObjArray* volumes = gGeoManager->GetListOfVolumes();
  for (int i = 0; i < volumes->GetEntriesFast(); i++) {
    TGeoVolume* volume = static_cast<TGeoVolume*>(volumes->At(i));
    volume->SetTransparency(70); //0: opaque, 100: fully transparent
  }

  /*
  //disable display of especially detailed subdetectors
  disableVolume("logical_ecl", true);
  disableVolume("LVCryo");
  //disable all daughters of logical_CDC individually
  TGeoVolume* cdc = gGeoManager->FindVolumeFast("logicalCDC");
  if (!cdc) {
    B2WARNING("Volume logicalCDC not found!");
  } else {
    const int nNodes = cdc->GetNdaughters();
    for (int i = 0; i < nNodes; i++) {
      cdc->GetNode(i)->GetVolume()->SetVisibility(false);
    }
  }
  //reenable CDC boundaries
  enableVolume("logicalInnerWall_0_Shield", false);
  enableVolume("logicalOuterWall_0_Shield", false);
  enableVolume("logicalBackwardCover2", false);
  enableVolume("logicalForwardCover3", false);
  enableVolume("logicalForwardCover4", false);
  //ring things in endcap region
  disableVolume("LVPoleTip");
  disableVolume("LVPoleTip2");
  disableVolume("LVPoleTip3");
  //magnets and beam pipes outside IR (shielding left in)
  disableVolume("logi_A1spc1_name");
  disableVolume("logi_B1spc1_name");
  disableVolume("logi_D1spc1_name");
  disableVolume("logi_E1spc1_name");
  */

  //set some nicer colors (at top level only)
  setVolumeColor("PXD.Envelope", kGreen + 3);
  setVolumeColor("SVD.Envelope", kOrange + 8);
  setVolumeColor("logical_ecl", kOrange - 3);
  setVolumeColor("BKLM.EnvelopeLogical", kGreen + 3);
  setVolumeColor("Endcap_1", kGreen + 3);
  setVolumeColor("Endcap_2", kGreen + 3);

  //make magnets more transparent
  for (int i = 0; i < volumes->GetEntriesFast(); i++) {
    TGeoVolume* volume = static_cast<TGeoVolume*>(volumes->At(i));
    if (TString(volume->GetName()).BeginsWith("Magnet_")) {
      volume->SetTransparency(97); //0: opaque, 100: fully transparent
    }
  }

  TGeoNode* top_node = gGeoManager->GetTopNode();
  assert(top_node != NULL);
  TEveGeoTopNode* eve_top_node = new TEveGeoTopNode(gGeoManager, top_node);
  eve_top_node->IncDenyDestroy();
  eve_top_node->SetVisLevel(2);
  gEve->AddGlobalElement(eve_top_node);

  //don't show full geo unless turned on by user
  eve_top_node->SetRnrSelfChildren(m_fullgeo, m_fullgeo);

  //expand geometry in eve list (otherwise one needs three clicks to see that it has children)
  eve_top_node->ExpandIntoListTreesRecursively();

  B2DEBUG(100, "Loading geometry projections...");

  const std::string extractPath = FileSystem::findFile("/data/display/geometry_extract.root");
  TFile* f = TFile::Open(extractPath.c_str(), "READ");
  TEveGeoShapeExtract* gse = dynamic_cast<TEveGeoShapeExtract*>(f->Get("Extract"));
  TEveGeoShape* gs = TEveGeoShape::ImportShapeExtract(gse, 0);
  gs->SetRnrSelf(false);
  gs->IncDenyDestroy();
  gs->SetName("Minimal geometry extract");
  delete f;

  //I want to show full geo in unprojected view,
  //but I still need to add the extract to the geometry scene...
  gEve->AddGlobalElement(gs);

  gs->SetRnrSelfChildren(false, !m_fullgeo);
  B2DEBUG(100, "Done.");
}

void EVEVisualization::addTrack(const Belle2::Track* belle2Track)
{
  using namespace genfit;
  using genfit::Track;

  const TrackFitResult* fitResult = belle2Track->getTrackFitResult(Const::pion);
  if (!fitResult) {
    B2ERROR("Track without TrackFitResult skipped.");
    return;
  }
  const Track* track = fitResult->getRelated<genfit::Track>();
  TString label = TString::Format("Track %d (FitResult: %d)", belle2Track->getArrayIndex(), fitResult->getArrayIndex());

  // parse the option string ------------------------------------------------------------------------
  bool drawDetectors = false;
  bool drawHits = false;
  bool drawPlanes = false;

  if (m_options != "") {
    for (size_t i = 0; i < m_options.length(); i++) {
      if (m_options.at(i) == 'D') drawDetectors = true;
      if (m_options.at(i) == 'H') drawHits = true;
      if (m_options.at(i) == 'P') drawPlanes = true;
    }
  }
  // finished parsing the option string -------------------------------------------------------------

  //TODO: move these
  bool refit_ = false;
  bool drawCardinalRep_ = true;
  bool drawErrors = false;
  bool drawRefTrack_ = false;
  bool drawForward_ = false;
  bool drawBackward_ = false;
  eFitterType fitterId_ = RefKalman;
  double errorScale_ = 1.0;
  unsigned int repId_ = 0;
  eMultipleMeasurementHandling mmHandling_ = unweightedClosestToPrediction;
  double dPVal_ = 1.E-3;
  int nMaxIter_ = 4;
  bool resort_ = true;
  int i = 0; //just for printing

  unsigned int numpoints = track ? track->getNumPointsWithMeasurement() : 0;
  bool isPruned = (track == nullptr);


  TEveRecTrackD recTrack;
  const TVector3& poca = fitResult->getPosition();
  const TVector3& poca_momentum = fitResult->getMomentum();

  recTrack.fV.Set(poca);
  recTrack.fP.Set(poca_momentum);
  recTrack.fSign = fitResult->getChargeSign();
  TEveTrack* eveTrack = new TEveTrack(&recTrack, m_gftrackpropagator);
  eveTrack->SetName(label);


  if (track) {
    if (! track->checkConsistency()) {
      B2ERROR("track is not consistent");
      return;
    }

    boost::scoped_ptr<Track> refittedTrack(NULL);
    if (refit_) {

      boost::scoped_ptr<AbsKalmanFitter> fitter;
      switch (fitterId_) {
        case SimpleKalman:
          fitter.reset(new KalmanFitter(nMaxIter_, dPVal_));
          fitter->setMultipleMeasurementHandling(mmHandling_);
          break;

        case RefKalman:
          fitter.reset(new KalmanFitterRefTrack(nMaxIter_, dPVal_));
          fitter->setMultipleMeasurementHandling(mmHandling_);
          break;

        case DafSimple: {
          genfit::AbsKalmanFitter* DafsKalman = new genfit::KalmanFitter();
          fitter.reset(new DAF(DafsKalman));
        }
        break;
        case DafRef:
          fitter.reset(new DAF());
          break;

      }
      fitter->setMaxIterations(nMaxIter_);


      refittedTrack.reset(new Track(*track));
      refittedTrack->deleteFitterInfo();

      try {
        fitter->processTrack(refittedTrack.get(), resort_);
      } catch (genfit::Exception& e) {
        B2ERROR("Exception (" << e.what() << ") encountered, could not refit track");
        return;
      }

      track = refittedTrack.get();
    }


    AbsTrackRep* rep;

    if (drawCardinalRep_) {
      rep = track->getCardinalRep();
      B2DEBUG(100, "Draw cardinal rep");
    } else {
      if (repId_ >= track->getNumReps())
        repId_ = track->getNumReps() - 1;
      rep = track->getTrackRep(repId_);
      B2DEBUG(100, "Draw rep" << repId_);
    }

    const FitStatus* fitStatus = track->getFitStatus(rep);
    //track->Print();
    //track->Print("C");
    //fitStatus->Print();

    isPruned = fitStatus->isTrackPruned();


    KalmanFitterInfo* fi;
    KalmanFitterInfo* prevFi = 0;
    const MeasuredStateOnPlane* fittedState(NULL);
    const MeasuredStateOnPlane* prevFittedState(NULL);


    for (unsigned int j = 0; j < numpoints; j++) { // loop over all hits in the track

      TrackPoint* tp = track->getPointWithMeasurement(j);
      if (! tp->hasRawMeasurements()) {
        B2ERROR("trackPoint has no raw measurements");
        continue;
      }

      // get the fitter infos ------------------------------------------------------------------
      if (! tp->hasFitterInfo(rep)) {
        B2ERROR("trackPoint has no fitterInfo for rep");
        continue;
      }

      AbsFitterInfo* fitterInfo = tp->getFitterInfo(rep);

      fi = dynamic_cast<KalmanFitterInfo*>(fitterInfo);
      if (fi == NULL) {
        B2ERROR("can only display KalmanFitterInfo");
        continue;
      }
      if (! fi->hasPredictionsAndUpdates()) {
        B2ERROR("KalmanFitterInfo does not have all predictions and updates");
        continue;
      }
      try {
        fittedState = &(fi->getFittedState(true));
      } catch (Exception& e) {
        B2ERROR(e.what() << " - can not get fitted state");
        continue;
      }

      TVector3 track_pos = rep->getPos(*fittedState);

      // draw track if corresponding option is set ------------------------------------------
      if (prevFittedState != NULL) {
        TEvePathMark::EType_e markType = TEvePathMark::kReference;
        if (j + 1 == numpoints) //track should stop here.
          markType = TEvePathMark::kDecay;

        makeLines(eveTrack, prevFittedState, fittedState, rep, markType, drawErrors);
        if (drawErrors) { // make sure to draw errors in both directions
          makeLines(eveTrack, prevFittedState, fittedState, rep, markType, drawErrors, 0);
        }
        //these are currently disabled.
        //TODO: if activated, I want to have a separate TEveStraightLineSet instead of eveTrack (different colors/options)
        if (drawForward_)
          makeLines(eveTrack, prevFi->getForwardUpdate(), fi->getForwardPrediction(), rep, markType, drawErrors, 0);
        if (drawBackward_)
          makeLines(eveTrack, prevFi->getBackwardPrediction(), fi->getBackwardUpdate(), rep, markType, drawErrors);
        // draw reference track if corresponding option is set ------------------------------------------
        if (drawRefTrack_ && fi->hasReferenceState() && prevFi->hasReferenceState())
          makeLines(eveTrack, prevFi->getReferenceState(), fi->getReferenceState(), rep, markType, false);
      }
      prevFi = fi;
      prevFittedState = fittedState;


      //loop over all measurements for this point (e.g. u and v-type strips)
      const int numMeasurements = tp->getNumRawMeasurements();
      for (int iMeasurement = 0; iMeasurement < numMeasurements; iMeasurement++) {
        const AbsMeasurement* m = tp->getRawMeasurement(iMeasurement);


        const MeasurementOnPlane* mop = fi->getMeasurementOnPlane(iMeasurement);
        const TVectorT<double>& hit_coords = mop->getState();
        const TMatrixTSym<double>& hit_cov = mop->getCov();

        // finished getting the hit infos -----------------------------------------------------

        // sort hit infos into variables ------------------------------------------------------
        TVector3 o = fittedState->getPlane()->getO();
        TVector3 u = fittedState->getPlane()->getU();
        TVector3 v = fittedState->getPlane()->getV();

        bool planar_hit = false;
        bool planar_pixel_hit = false;
        bool space_hit = false;
        bool wire_hit = false;
        bool wirepoint_hit = false;
        double_t hit_u = 0;
        double_t hit_v = 0;
        double_t plane_size = 4;

        int hit_coords_dim = m->getDim();

        if (dynamic_cast<const PlanarMeasurement*>(m) != NULL) {
          planar_hit = true;
          if (hit_coords_dim == 1) {
            hit_u = hit_coords(0);
          } else if (hit_coords_dim == 2) {
            planar_pixel_hit = true;
            hit_u = hit_coords(0);
            hit_v = hit_coords(1);
          }
        } else if (dynamic_cast<const SpacepointMeasurement*>(m) != NULL) {
          space_hit = true;
          plane_size = 4;
        } else if (dynamic_cast<const WireMeasurement*>(m) != NULL) {
          wire_hit = true;
          hit_u = fabs(hit_coords(0));
          hit_v = v * (track_pos - o); // move the covariance tube so that the track goes through it
          plane_size = 4;
          if (dynamic_cast<const WirePointMeasurement*>(m) != NULL) {
            wirepoint_hit = true;
            hit_v = hit_coords(1);
          }
        } else {
          B2ERROR("Track " << i << ", Hit " << j << ", Measurement " << iMeasurement << ": Unknown measurement type: skipping hit!");
          break;
        }

        if (plane_size < 4) plane_size = 4;
        // finished setting variables ---------------------------------------------------------

        // draw planes if corresponding option is set -----------------------------------------
        if (drawPlanes || (drawDetectors && planar_hit)) {
          TVector3 move(0, 0, 0);
          if (wire_hit) move = v * (v * (track_pos - o)); // move the plane along the wire until the track goes through it
          TEveBox* box = boxCreator(o + move, u, v, plane_size, plane_size, 0.01);
          if (drawDetectors && planar_hit) {
            box->SetMainColor(kCyan);
          } else {
            box->SetMainColor(kGray);
          }
          box->SetMainTransparency(50);
          eveTrack->AddElement(box);
        }
        // finished drawing planes ------------------------------------------------------------

        // draw detectors if option is set, only important for wire hits ----------------------
        if (drawDetectors) {

          if (wire_hit) {
            TEveGeoShape* det_shape = new TEveGeoShape("det_shape");
            det_shape->IncDenyDestroy();
            det_shape->SetShape(new TGeoTube(std::max(0., (double)(hit_u - 0.0105 / 2.)), hit_u + 0.0105 / 2., plane_size));

            TVector3 norm = u.Cross(v);
            TGeoRotation det_rot("det_rot", (u.Theta() * 180) / TMath::Pi(), (u.Phi() * 180) / TMath::Pi(),
                                 (norm.Theta() * 180) / TMath::Pi(), (norm.Phi() * 180) / TMath::Pi(),
                                 (v.Theta() * 180) / TMath::Pi(), (v.Phi() * 180) / TMath::Pi()); // move the tube to the right place and rotate it correctly
            TVector3 move = v * (v * (track_pos - o)); // move the tube along the wire until the track goes through it
            TGeoCombiTrans det_trans(o(0) + move.X(),
                                     o(1) + move.Y(),
                                     o(2) + move.Z(),
                                     &det_rot);
            det_shape->SetTransMatrix(det_trans);
            det_shape->SetMainColor(kCyan);
            det_shape->SetMainTransparency(25);
            if ((drawHits && (hit_u + 0.0105 / 2 > 0)) || !drawHits) {
              eveTrack->AddElement(det_shape);
            }
          }

        }
        // finished drawing detectors ---------------------------------------------------------

        if (drawHits) {

          // draw planar hits, with distinction between strip and pixel hits ----------------
          if (planar_hit) {
            if (!planar_pixel_hit) {
              //strip hit
              static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
              const SVDRecoHit* recoHit = dynamic_cast<const SVDRecoHit*>(m);
              if (!recoHit) {
                B2WARNING("SVD recohit couldn't be converted... ");
                continue;
              }

              const VXD::SensorInfoBase& sensor = geo.get(recoHit->getSensorID());
              double du, dv;
              TVector3 a = o; //defines position of sensor plane
              double hit_res_u = hit_cov(0, 0);
              if (recoHit->isU()) {
                du = std::sqrt(hit_res_u);
                dv = sensor.getLength();
                a += hit_u * u;
              } else {
                du = sensor.getWidth();
                dv = std::sqrt(hit_res_u);
                a += hit_u * v;
              }
              double depth = sensor.getThickness();
              TEveBox* hit_box = boxCreator(a, u, v, du, dv, depth);
              hit_box->SetName("SVDRecoHit");
              hit_box->SetMainColor(c_recoHitColor);
              hit_box->SetMainTransparency(0);
              eveTrack->AddElement(hit_box);
            } else {
              // calculate eigenvalues to draw error-ellipse ----------------------------
              TMatrixDEigen eigen_values(hit_cov);
              TEveGeoShape* cov_shape = new TEveGeoShape("cov_shape");
              cov_shape->IncDenyDestroy();
              TMatrixT<double> ev = eigen_values.GetEigenValues();
              TMatrixT<double> eVec = eigen_values.GetEigenVectors();
              double pseudo_res_0 = errorScale_ * std::sqrt(ev(0, 0));
              double pseudo_res_1 = errorScale_ * std::sqrt(ev(1, 1));
              // finished calcluating, got the values -----------------------------------

              // calculate the semiaxis of the error ellipse ----------------------------
              cov_shape->SetShape(new TGeoEltu(pseudo_res_0, pseudo_res_1, 0.0105));
              TVector3 pix_pos = o + hit_u * u + hit_v * v;
              TVector3 u_semiaxis = (pix_pos + eVec(0, 0) * u + eVec(1, 0) * v) - pix_pos;
              TVector3 v_semiaxis = (pix_pos + eVec(0, 1) * u + eVec(1, 1) * v) - pix_pos;
              TVector3 norm = u.Cross(v);
              // finished calculating ---------------------------------------------------

              // rotate and translate everything correctly ------------------------------
              TGeoRotation det_rot("det_rot", (u_semiaxis.Theta() * 180) / TMath::Pi(), (u_semiaxis.Phi() * 180) / TMath::Pi(),
                                   (v_semiaxis.Theta() * 180) / TMath::Pi(), (v_semiaxis.Phi() * 180) / TMath::Pi(),
                                   (norm.Theta() * 180) / TMath::Pi(), (norm.Phi() * 180) / TMath::Pi());
              TGeoCombiTrans det_trans(pix_pos(0), pix_pos(1), pix_pos(2), &det_rot);
              cov_shape->SetTransMatrix(det_trans);
              // finished rotating and translating --------------------------------------

              cov_shape->SetMainColor(c_recoHitColor);
              cov_shape->SetMainTransparency(0);
              eveTrack->AddElement(cov_shape);
            }
          }
          // finished drawing planar hits ---------------------------------------------------

          // draw spacepoint hits -----------------------------------------------------------
          if (space_hit) {

            // get eigenvalues of covariance to know how to draw the ellipsoid ------------
            TMatrixDEigen eigen_values(m->getRawHitCov());
            TEveGeoShape* cov_shape = new TEveGeoShape("cov_shape");
            cov_shape->IncDenyDestroy();
            cov_shape->SetShape(new TGeoSphere(0., 1.));
            TMatrixT<double> ev = eigen_values.GetEigenValues();
            TMatrixT<double> eVec = eigen_values.GetEigenVectors();
            TVector3 eVec1(eVec(0, 0), eVec(1, 0), eVec(2, 0));
            TVector3 eVec2(eVec(0, 1), eVec(1, 1), eVec(2, 1));
            TVector3 eVec3(eVec(0, 2), eVec(1, 2), eVec(2, 2));
            TVector3 norm = u.Cross(v);
            // got everything we need -----------------------------------------------------


            TGeoRotation det_rot("det_rot", (eVec1.Theta() * 180) / TMath::Pi(), (eVec1.Phi() * 180) / TMath::Pi(),
                                 (eVec2.Theta() * 180) / TMath::Pi(), (eVec2.Phi() * 180) / TMath::Pi(),
                                 (eVec3.Theta() * 180) / TMath::Pi(), (eVec3.Phi() * 180) / TMath::Pi()); // the rotation is already clear

            // set the scaled eigenvalues -------------------------------------------------
            double pseudo_res_0 = errorScale_ * std::sqrt(ev(0, 0));
            double pseudo_res_1 = errorScale_ * std::sqrt(ev(1, 1));
            double pseudo_res_2 = errorScale_ * std::sqrt(ev(2, 2));
            // finished scaling -----------------------------------------------------------

            // rotate and translate -------------------------------------------------------
            TGeoGenTrans det_trans(o(0), o(1), o(2),
                                   //std::sqrt(pseudo_res_0/pseudo_res_1/pseudo_res_2), std::sqrt(pseudo_res_1/pseudo_res_0/pseudo_res_2), std::sqrt(pseudo_res_2/pseudo_res_0/pseudo_res_1), // this workaround is necessary due to the "normalization" performed in  TGeoGenTrans::SetScale
                                   //1/(pseudo_res_0),1/(pseudo_res_1),1/(pseudo_res_2),
                                   pseudo_res_0, pseudo_res_1, pseudo_res_2,
                                   &det_rot);
            cov_shape->SetTransMatrix(det_trans);
            // finished rotating and translating ------------------------------------------

            cov_shape->SetMainColor(c_recoHitColor);
            cov_shape->SetMainTransparency(10);
            eveTrack->AddElement(cov_shape);
          }
          // finished drawing spacepoint hits -----------------------------------------------

          // draw wire hits -----------------------------------------------------------------
          if (wire_hit) {
            TEveGeoShape* cov_shape = new TEveGeoShape("cov_shape");
            cov_shape->IncDenyDestroy();
            double pseudo_res_0 = errorScale_ * std::sqrt(hit_cov(0, 0));
            double pseudo_res_1 = plane_size;
            if (wirepoint_hit) pseudo_res_1 = errorScale_ * std::sqrt(hit_cov(1, 1));

            cov_shape->SetShape(new TGeoTube(std::max(0., (double)(hit_u - pseudo_res_0)), hit_u + pseudo_res_0, pseudo_res_1));
            TVector3 norm = u.Cross(v);

            // rotate and translate -------------------------------------------------------
            TGeoRotation det_rot("det_rot", (u.Theta() * 180) / TMath::Pi(), (u.Phi() * 180) / TMath::Pi(),
                                 (norm.Theta() * 180) / TMath::Pi(), (norm.Phi() * 180) / TMath::Pi(),
                                 (v.Theta() * 180) / TMath::Pi(), (v.Phi() * 180) / TMath::Pi());
            TGeoCombiTrans det_trans(o(0) + hit_v * v.X(),
                                     o(1) + hit_v * v.Y(),
                                     o(2) + hit_v * v.Z(),
                                     &det_rot);
            cov_shape->SetTransMatrix(det_trans);
            // finished rotating and translating ------------------------------------------

            cov_shape->SetMainColor(c_recoHitColor);
            cov_shape->SetMainTransparency(50);
            eveTrack->AddElement(cov_shape);
          }
          // finished drawing wire hits -----------------------------------------------------
        }

      }
    }
  }

  eveTrack->SetTitle(TString::Format("%s\n"
                                     "#points: %u %s\n"
                                     //"pT=%.3f, pZ=%.3f\n"
                                     "pVal: %e",
                                     label.Data(),
                                     numpoints, (isPruned ? " (pruned)" : ""),
                                     //track_mom.Pt(), track_mom.Pz(),
                                     fitResult->getPValue()
                                    ));
  eveTrack->SetLineColor(c_trackColor);
  eveTrack->SetLineStyle(1);
  eveTrack->SetLineWidth(3.0);


  addToGroup("Fitted Tracks", eveTrack);
  addObject(track, eveTrack);
  addObject(belle2Track, eveTrack);
}

TEveBox* EVEVisualization::boxCreator(const TVector3& o, TVector3 u, TVector3 v, float ud, float vd, float depth)
{
  TEveBox* box = new TEveBox;
  box->SetPickable(true);
  float vertices[24];

  TVector3 norm = u.Cross(v);
  u *= (0.5 * ud);
  v *= (0.5 * vd);
  norm *= (0.5 * depth);

  for (int i = 0; i < 3; i++) {
    vertices[i + 0]  = (float)(o(i) - u(i) - v(i) - norm(i));
    vertices[i + 3]  = (float)(o(i) + u(i) - v(i) - norm(i));
    vertices[i + 6]  = (float)(o(i) + u(i) - v(i) + norm(i));
    vertices[i + 9]  = (float)(o(i) - u(i) - v(i) + norm(i));
    vertices[i + 12] = (float)(o(i) - u(i) + v(i) - norm(i));
    vertices[i + 15] = (float)(o(i) + u(i) + v(i) - norm(i));
    vertices[i + 18] = (float)(o(i) + u(i) + v(i) + norm(i));
    vertices[i + 21] = (float)(o(i) - u(i) + v(i) + norm(i));
  }

  box->SetVertices(vertices);
  return box;
}

void EVEVisualization::makeLines(TEveTrack* eveTrack, const genfit::StateOnPlane* prevState, const genfit::StateOnPlane* state, const genfit::AbsTrackRep* rep,
                                 TEvePathMark::EType_e markType, bool drawErrors, int markerPos)
{
  using namespace genfit;

  TVector3 pos, dir, oldPos, oldDir;
  rep->getPosDir(*state, pos, dir);
  rep->getPosDir(*prevState, oldPos, oldDir);

  double distA = (pos - oldPos).Mag();
  double distB = distA;
  if ((pos - oldPos)*oldDir < 0)
    distA *= -1.;
  if ((pos - oldPos)*dir < 0)
    distB *= -1.;

  TEvePathMark mark(
    markType,
    TEveVector(pos(0), pos(1), pos(2)),
    TEveVector(dir(0), dir(1), dir(2))
  );
  eveTrack->AddPathMark(mark);


  if (drawErrors) {
    const MeasuredStateOnPlane* measuredState;
    if (markerPos == 0)
      measuredState = dynamic_cast<const MeasuredStateOnPlane*>(prevState);
    else
      measuredState = dynamic_cast<const MeasuredStateOnPlane*>(state);

    if (measuredState != NULL) {

      // step for evaluate at a distance from the original plane
      TVector3 eval;
      if (markerPos == 0)
        eval = 0.2 * distA * oldDir;
      else
        eval = -0.2 * distB * dir;


      // get cov at first plane
      TMatrixDSym cov;
      TVector3 position, direction;
      rep->getPosMomCov(*measuredState, position, direction, cov);

      // get eigenvalues & -vectors
      TMatrixDEigen eigen_values(cov.GetSub(0, 2, 0, 2));
      TMatrixT<double> ev = eigen_values.GetEigenValues();
      TMatrixT<double> eVec = eigen_values.GetEigenVectors();
      TVector3 eVec1, eVec2;
      // limit
      static const double maxErr = 1000.;
      double ev0 = std::min(ev(0, 0), maxErr);
      double ev1 = std::min(ev(1, 1), maxErr);
      double ev2 = std::min(ev(2, 2), maxErr);

      // get two largest eigenvalues/-vectors
      if (ev0 < ev1 && ev0 < ev2) {
        eVec1.SetXYZ(eVec(0, 1), eVec(1, 1), eVec(2, 1));
        eVec1 *= sqrt(ev1);
        eVec2.SetXYZ(eVec(0, 2), eVec(1, 2), eVec(2, 2));
        eVec2 *= sqrt(ev2);
      } else if (ev1 < ev0 && ev1 < ev2) {
        eVec1.SetXYZ(eVec(0, 0), eVec(1, 0), eVec(2, 0));
        eVec1 *= sqrt(ev0);
        eVec2.SetXYZ(eVec(0, 2), eVec(1, 2), eVec(2, 2));
        eVec2 *= sqrt(ev2);
      } else {
        eVec1.SetXYZ(eVec(0, 0), eVec(1, 0), eVec(2, 0));
        eVec1 *= sqrt(ev0);
        eVec2.SetXYZ(eVec(0, 1), eVec(1, 1), eVec(2, 1));
        eVec2 *= sqrt(ev1);
      }

      if (eVec1.Cross(eVec2)*eval < 0)
        eVec2 *= -1;
      //assert(eVec1.Cross(eVec2)*eval > 0);

      const TVector3 oldEVec1(eVec1);
      const TVector3 oldEVec2(eVec2);

      const int nEdges = 24;
      std::vector<TVector3> vertices;

      vertices.push_back(position);

      // vertices at plane
      for (int i = 0; i < nEdges; ++i) {
        const double angle = 2 * TMath::Pi() / nEdges * i;
        vertices.push_back(position + cos(angle)*eVec1 + sin(angle)*eVec2);
      }



      DetPlane* newPlane = new DetPlane(*(measuredState->getPlane()));
      newPlane->setO(position + eval);

      MeasuredStateOnPlane stateCopy(*measuredState);
      try {
        rep->extrapolateToPlane(stateCopy, SharedPlanePtr(newPlane));
      } catch (Exception& e) {
        B2ERROR(e.what());
        return;
      }

      // get cov at 2nd plane
      rep->getPosMomCov(stateCopy, position, direction, cov);

      // get eigenvalues & -vectors
      TMatrixDEigen eigen_values2(cov.GetSub(0, 2, 0, 2));
      ev = eigen_values2.GetEigenValues();
      eVec = eigen_values2.GetEigenVectors();
      // limit
      ev0 = std::min(ev(0, 0), maxErr);
      ev1 = std::min(ev(1, 1), maxErr);
      ev2 = std::min(ev(2, 2), maxErr);

      // get two largest eigenvalues/-vectors
      if (ev0 < ev1 && ev0 < ev2) {
        eVec1.SetXYZ(eVec(0, 1), eVec(1, 1), eVec(2, 1));
        eVec1 *= sqrt(ev1);
        eVec2.SetXYZ(eVec(0, 2), eVec(1, 2), eVec(2, 2));
        eVec2 *= sqrt(ev2);
      } else if (ev1 < ev0 && ev1 < ev2) {
        eVec1.SetXYZ(eVec(0, 0), eVec(1, 0), eVec(2, 0));
        eVec1 *= sqrt(ev0);
        eVec2.SetXYZ(eVec(0, 2), eVec(1, 2), eVec(2, 2));
        eVec2 *= sqrt(ev2);
      } else {
        eVec1.SetXYZ(eVec(0, 0), eVec(1, 0), eVec(2, 0));
        eVec1 *= sqrt(ev0);
        eVec2.SetXYZ(eVec(0, 1), eVec(1, 1), eVec(2, 1));
        eVec2 *= sqrt(ev1);
      }

      if (eVec1.Cross(eVec2)*eval < 0)
        eVec2 *= -1;
      //assert(eVec1.Cross(eVec2)*eval > 0);

      if (oldEVec1 * eVec1 < 0) {
        eVec1 *= -1;
        eVec2 *= -1;
      }

      // vertices at 2nd plane
      double angle0 = eVec1.Angle(oldEVec1);
      if (eVec1 * (eval.Cross(oldEVec1)) < 0)
        angle0 *= -1;
      for (int i = 0; i < nEdges; ++i) {
        const double angle = 2 * TMath::Pi() / nEdges * i - angle0;
        vertices.push_back(position + cos(angle)*eVec1 + sin(angle)*eVec2);
      }

      vertices.push_back(position);


      TEveTriangleSet* error_shape = new TEveTriangleSet(vertices.size(), nEdges * 2);
      for (unsigned int k = 0; k < vertices.size(); ++k) {
        error_shape->SetVertex(k, vertices[k].X(), vertices[k].Y(), vertices[k].Z());
      }

      assert(vertices.size() == 2 * nEdges + 2);

      int iTri(0);
      for (int i = 0; i < nEdges; ++i) {
        //error_shape->SetTriangle(iTri++,  0,             i+1,        (i+1)%nEdges+1);
        error_shape->SetTriangle(iTri++,  i + 1,           i + 1 + nEdges, (i + 1) % nEdges + 1);
        error_shape->SetTriangle(iTri++, (i + 1) % nEdges + 1, i + 1 + nEdges, (i + 1) % nEdges + 1 + nEdges);
        //error_shape->SetTriangle(iTri++,  2*nEdges+1,    i+1+nEdges, (i+1)%nEdges+1+nEdges);
      }

      //assert(iTri == nEdges*4);

      error_shape->SetMainColor(c_trackColor);
      error_shape->SetMainTransparency(25);
      eveTrack->AddElement(error_shape);
    }
  }
}

void EVEVisualization::addSimHit(const CDCSimHit* hit, const MCParticle* particle)
{
  addSimHit(hit->getPosWire(), particle);
}
void EVEVisualization::addSimHit(const PXDSimHit* hit, const MCParticle* particle)
{
  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  const TVector3& global_pos = geo.get(hit->getSensorID()).pointToGlobal(hit->getPosIn());
  addSimHit(global_pos, particle);
}
void EVEVisualization::addSimHit(const SVDSimHit* hit, const MCParticle* particle)
{
  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  const TVector3& global_pos = geo.get(hit->getSensorID()).pointToGlobal(hit->getPosIn());
  addSimHit(global_pos, particle);
}
void EVEVisualization::addSimHit(const BKLMSimHit* hit, const MCParticle* particle)
{
  TVector3 global_pos;
  BKLMSimHitPosition* p = hit->getRelatedFrom<BKLMSimHitPosition>();
  if (p) global_pos = p->getPosition();
  addSimHit(global_pos, particle);
}
void EVEVisualization::addSimHit(const EKLMSimHit* hit, const MCParticle* particle)
{
  const TVector3& global_pos = hit->getPosition();
  addSimHit(global_pos, particle);
}
void EVEVisualization::addSimHit(const TVector3& v, const MCParticle* particle)
{
  MCTrack* track = addMCParticle(particle);
  if (!track)
    return; //hide hits from this particle
  track->simhits->SetNextPoint(v.x(), v.y(), v.z());
}

EVEVisualization::MCTrack* EVEVisualization::addMCParticle(const MCParticle* particle)
{
  if (!particle) {
    if (!m_mcparticleTracks[particle].simhits) {
      const TString pointsTitle("Unassigned SimHits");
      m_mcparticleTracks[particle].simhits = new TEvePointSet(pointsTitle);
      m_mcparticleTracks[particle].simhits->SetTitle(pointsTitle);
      m_mcparticleTracks[particle].simhits->SetMarkerStyle(6);
      m_mcparticleTracks[particle].simhits->SetMainColor(c_unassignedHitColor);
      //m_mcparticleTracks[particle].simhits->SetMainTransparency(50);
      m_mcparticleTracks[particle].track = NULL;
    }
    return &m_mcparticleTracks[particle];
  }

  if (m_hideSecondaries and !particle->hasStatus(MCParticle::c_PrimaryParticle)) {
    return NULL;
  }
  //don't show Upsilon(4S)
  if (particle->getDecayTime() == 0.0) {
    return NULL;
  }
  if (m_assignToPrimaries) {
    while (!particle->hasStatus(MCParticle::c_PrimaryParticle) and particle->getMother())
      particle = particle->getMother();
  }

  if (!m_mcparticleTracks[particle].track) {
    const TVector3& p = particle->getMomentum();
    const TVector3& vertex = particle->getProductionVertex();
    const int pdg = particle->getPDG();
    TParticle tparticle(pdg, particle->getStatus(),
                        (particle->getMother() ? particle->getMother()->getIndex() : 0), 0, particle->getFirstDaughter(), particle->getLastDaughter(),
                        p.x(), p.y(), p.z(), particle->getEnergy(),
                        vertex.x(), vertex.y(), vertex.z(), particle->getProductionTime());
    TEveMCTrack mctrack;
    mctrack = tparticle;
    mctrack.fTDecay = particle->getDecayTime();
    mctrack.fVDecay.Set(particle->getDecayVertex());
    mctrack.fDecayed = !boost::math::isinf(mctrack.fTDecay);
    mctrack.fIndex = particle->getIndex();
    m_mcparticleTracks[particle].track = new TEveTrack(&mctrack, m_trackpropagator);

    //Check if there is a trajectory stored for this particle
    const auto mcTrajectories = particle->getRelationsTo<MCParticleTrajectory>();
    bool hasTrajectory(false);
    for (auto rel : mcTrajectories.relations()) {
      //Trajectories with negative weight are from secondary daughters which
      //were ignored so we don't use them.
      if (rel.weight <= 0)  continue;
      //Found one, let's add tose point as reference points to the TEveTrack.
      //This will force the track propagation to visit all points in order but
      //provide smooth helix interpolation between the points
      const MCParticleTrajectory& trajectory = dynamic_cast<const MCParticleTrajectory&>(*rel.object);
      for (const MCTrajectoryPoint & p : trajectory) {
        m_mcparticleTracks[particle].track->AddPathMark(
          TEvePathMark(
            //Add the last trajectory point as decay point to prevent TEve to
            //propagate beyond the end of the track. So lets compare the adress
            //to the address of last point and choose the pathmark accordingly
            (&p == &trajectory.back()) ? TEvePathMark::kDecay : TEvePathMark::kReference,
            TEveVector(p.x, p.y, p.z),
            TEveVector(p.px, p.py, p.pz)
          ));
      }
      //"There can only be One" -> found a trajectory, stop the loop
      hasTrajectory = true;
      break;
    }

    //If we have the full trajectory there is no need to add additional path marks
    if (!hasTrajectory) {
      //add daughter vertices - improves track rendering as lost momentum is taken into account
      for (int iDaughter = particle->getFirstDaughter(); iDaughter <= particle->getLastDaughter(); iDaughter++) {
        if (iDaughter == 0)
          continue; //no actual daughter

        const MCParticle* daughter = StoreArray<MCParticle>()[iDaughter - 1];

        TEvePathMarkD refMark(TEvePathMarkD::kDaughter);
        refMark.fV.Set(daughter->getProductionVertex());
        refMark.fP.Set(daughter->getMomentum());
        refMark.fTime = daughter->getProductionTime();
        m_mcparticleTracks[particle].track->AddPathMark(refMark);
      }

      //neutrals and very short-lived particles should stop somewhere
      //(can result in wrong shapes for particles stopped in the detector, so not used there)
      if ((TMath::Nint(particle->getCharge()) == 0 or !particle->hasStatus(MCParticle::c_StoppedInDetector))
          and mctrack.fDecayed) {
        TEvePathMarkD decayMark(TEvePathMarkD::kDecay);
        decayMark.fV.Set(particle->getDecayVertex());
        m_mcparticleTracks[particle].track->AddPathMark(decayMark);
      }
    }
    TString particle_name(mctrack.GetName());

    //set track title (for popup)
    TString momLabel = "";
    if (particle->getMother()) {
      momLabel = TString::Format("\nMother: Idx=%d, PDG=%d)", particle->getMother()->getIndex(), particle->getMother()->getPDG());
      m_mcparticleTracks[particle].parentParticle = particle->getMother();
    }

    if (!hasTrajectory) {
      //Hijack the mother label to show that the track position is only
      //extrapolated, not known from simulation
      momLabel += "\n(track estimated from initial momentum)";
      //Also, show those tracks with dashed lines
      m_mcparticleTracks[particle].track->SetLineStyle(2);
    }

    m_mcparticleTracks[particle].track->SetTitle(TString::Format(
                                                   "MCParticle Index=%d\n"
                                                   "Chg=%d, PDG=%d (%s)\n"
                                                   "pT=%.3f, pZ=%.3f\nV=(%.3f, %.3f, %.3f)"
                                                   "%s",
                                                   particle->getIndex(),
                                                   (int)particle->getCharge(), particle->getPDG(), particle_name.Data(),
                                                   mctrack.Pt(), mctrack.Pz(), mctrack.Vx(), mctrack.Vy(), mctrack.Vz(),
                                                   momLabel.Data()));


    //add some color (avoid black & white)
    switch (abs(pdg)) {
      case 11:
        m_mcparticleTracks[particle].track->SetLineColor(kAzure);
        break;
      case 13:
        m_mcparticleTracks[particle].track->SetLineColor(kCyan + 1);
        break;
      case 22:
        m_mcparticleTracks[particle].track->SetLineColor(kSpring);
        break;
      case 211:
        m_mcparticleTracks[particle].track->SetLineColor(kGray + 1);
        break;
      case 321:
        m_mcparticleTracks[particle].track->SetLineColor(kRed + 1);
        break;
      case 2212:
        m_mcparticleTracks[particle].track->SetLineColor(kOrange - 2);
        break;
      default:
        m_mcparticleTracks[particle].track->SetLineColor(kMagenta);
    }

    //create point set for hits
    const TString pointsTitle = TString::Format("SimHits for MCParticle %d (%s)", particle->getIndex(), particle_name.Data());
    m_mcparticleTracks[particle].simhits = new TEvePointSet(pointsTitle);
    m_mcparticleTracks[particle].simhits->SetTitle(pointsTitle);
    m_mcparticleTracks[particle].simhits->SetMarkerStyle(6);
    m_mcparticleTracks[particle].simhits->SetMainColor(m_mcparticleTracks[particle].track->GetLineColor());
    //m_mcparticleTracks[particle].simhits->SetMainTransparency(50);
    addObject(particle, m_mcparticleTracks[particle].track);
  }
  return &m_mcparticleTracks[particle];
}

void EVEVisualization::makeTracks()
{
  std::map<const MCParticle*, MCTrack>::iterator it = m_mcparticleTracks.begin();
  const std::map<const MCParticle*, MCTrack>::iterator& end = m_mcparticleTracks.end();
  for (; it != end; ++it) {
    if (it->second.track) {
      if (it->second.simhits->Size() > 0) {
        it->second.track->AddElement(it->second.simhits);
      } else {
        //if we don't add it, remove empty collection
        it->second.simhits->Destroy();
        it->second.simhits = nullptr;
      }

      TEveElement* parent = m_tracklist;
      if (it->second.parentParticle) {
        const auto& parentIt = m_mcparticleTracks.find(it->second.parentParticle);
        if (parentIt != m_mcparticleTracks.end()) {
          parent = parentIt->second.track;
        }
      }
      parent->AddElement(it->second.track);
    } else { //add simhits directly
      gEve->AddElement(it->second.simhits);
    }
  }
  gEve->AddElement(m_tracklist);
  m_tracklist->MakeTracks();
  m_tracklist->SelectByP(MIN_P_CUT, 15.0); //don't show too many particles by default...

  for (size_t i = 0; i < m_options.length(); i++) {
    if (m_options.at(i) == 'M') {
      m_gftrackpropagator->SetRnrDaughters(true);
      m_gftrackpropagator->SetRnrReferences(true);
      //m_gftrackpropagator->SetRnrFV(true); //TODO: this crashes?
      TMarker m;
      m.SetMarkerColor(c_trackMarkerColor);
      m.SetMarkerStyle(1); //a single pixel
      m.SetMarkerSize(1); //ignored.
      m_gftrackpropagator->RefPMAtt() = m;
      m_gftrackpropagator->RefFVAtt() = m;
    }
  }

  if (m_trackcandlist)
    gEve->AddElement(m_trackcandlist);

  m_eclData->DataChanged(); //update limits (Empty() won't work otherwise)
  if (!m_eclData->Empty()) {
    m_eclData->SetAxisFromBins();
    m_calo3d->SetData(m_eclData);
  }
  gEve->AddElement(m_calo3d);

  if (m_unassignedRecoHits) {
    gEve->AddElement(m_unassignedRecoHits);
  }
}

void EVEVisualization::clearEvent()
{
  if (!gEve)
    return;

  m_visualRepMap->clear();
  m_groups.clear();
  m_mcparticleTracks.clear();
  m_shownRecohits.clear();
  m_tracklist->DestroyElements();
  if (m_trackcandlist)
    m_trackcandlist->DestroyElements();


  //remove ECL data from event
  m_calo3d->SetData(NULL);
  m_calo3d->DestroyElements();

  //lower energy threshold for ECL
  float ecl_threshold = 0.01;
  if (m_eclData)
    ecl_threshold = m_eclData->GetSliceThreshold(0);

  delete m_eclData;
  m_eclData = new TEveCaloDataVec(1); //#slices
  m_eclData->IncDenyDestroy();
  m_eclData->RefSliceInfo(0).Setup("ECL", ecl_threshold, kRed);

  delete m_unassignedRecoHits;
  m_unassignedRecoHits = 0;

  gEve->GetSelection()->RemoveElements();
  gEve->GetHighlight()->RemoveElements();
  //other things are cleaned up by TEve...
}




void EVEVisualization::addVertex(const genfit::GFRaveVertex* vertex, const TString& name)
{
  TVector3 v = vertex->getPos();
  TEvePointSet* vertexPoint = new TEvePointSet(name);
  //sadly, setting a title for a TEveGeoShape doesn't result in a popup...
  vertexPoint->SetTitle(TString::Format("%s\n"
                                        "V=(%.3f, %.3f, %.3f)\n"
                                        "pVal=%e",
                                        name.Data(), v.x(), v.y(), v.z(),
                                        TMath::Prob(vertex->getChi2(), vertex->getNdf())));
  vertexPoint->SetMainColor(c_recoHitColor);
  vertexPoint->SetNextPoint(v.x(), v.y(), v.z());

  TMatrixDEigen eigen_values(vertex->getCov());
  TEveGeoShape* det_shape = new TEveGeoShape(name + " Error");
  det_shape->IncDenyDestroy();
  det_shape->SetShape(new TGeoSphere(0., 1.));   //Initially created as a sphere, then "scaled" into an ellipsoid.
  TMatrixT<double> ev = eigen_values.GetEigenValues(); //Assigns the eigenvalues into the "ev" matrix.
  TMatrixT<double> eVec = eigen_values.GetEigenValues();  //Assigns the eigenvalues into the "eVec" matrix.
  TVector3 eVec1(eVec(0, 0), eVec(1, 0), eVec(2, 0));   //Define the 3 eigenvectors of the covariance matrix as objects of the TVector3 class using constructor.
  TVector3 eVec2(eVec(0, 1), eVec(1, 1), eVec(2, 1));   //eVec(i,j) uses the method/overloaded operator ( . ) of the TMatrixT class to return the matrix entry.
  TVector3 eVec3(eVec(0, 2), eVec(1, 2), eVec(2, 2));
  // got everything we need -----------------------------------------------------   //Eigenvalues(semi axis) of the covariance matrix accquired!


  TGeoRotation det_rot("det_rot", (eVec1.Theta() * 180) / TMath::Pi(), (eVec1.Phi() * 180) / TMath::Pi(),
                       (eVec2.Theta() * 180) / TMath::Pi(), (eVec2.Phi() * 180) / TMath::Pi(),
                       (eVec3.Theta() * 180) / TMath::Pi(), (eVec3.Phi() * 180) / TMath::Pi()); // the rotation is already clear

  // set the scaled eigenvalues -------------------------------------------------
  double pseudo_res_0 = std::sqrt(ev(0, 0));
  double pseudo_res_1 = std::sqrt(ev(1, 1));
  double pseudo_res_2 = std::sqrt(ev(2, 2));    //"Scaled" eigenvalues pseudo_res (lengths of the semi axis) are the sqrt of the real eigenvalues.

  //B2INFO("The pseudo_res_0/1/2 are " << pseudo_res_0 << "," << pseudo_res_1 << "," << pseudo_res_2); //shows the scaled eigenvalues



  // rotate and translate -------------------------------------------------------
  TGeoGenTrans det_trans(v(0), v(1), v(2), pseudo_res_0, pseudo_res_1, pseudo_res_2, &det_rot); //Puts the ellipsoid at the position of the vertex, v(0)=v.x(), operator () overloaded.
  det_shape->SetTransMatrix(det_trans);
  // finished rotating and translating ------------------------------------------

  det_shape->SetMainColor(kOrange);   //The color of the error ellipsoid.
  det_shape->SetMainTransparency(0);

  vertexPoint->AddElement(det_shape);
  addToGroup("Vertices", vertexPoint);
  addObject(vertex, vertexPoint);
}


void EVEVisualization::addECLCluster(const ECLCluster* cluster)
{
  const float phi = cluster->getPhi();
  const float dPhi = cluster->getErrorPhi();
  const float dTheta = cluster->getErrorTheta();

  if (!std::isfinite(dPhi) or !std::isfinite(dTheta)) {
    B2ERROR("ECLCluster phi or theta error is NaN or infinite, skipping cluster!");
    return;
  }

  //convert theta +- dTheta into eta +- dEta
  TVector3 thetaLow;
  thetaLow.SetPtThetaPhi(1.0, cluster->getTheta() - dTheta, phi);
  TVector3 thetaHigh;
  thetaHigh.SetPtThetaPhi(1.0, cluster->getTheta() + dTheta, phi);
  float etaLow = thetaLow.Eta();
  float etaHigh = thetaHigh.Eta();
  if (etaLow > etaHigh) {
    std::swap(etaLow, etaHigh);
  }

  m_eclData->AddTower(etaLow, etaHigh, phi - dPhi, phi + dPhi);
  m_eclData->FillSlice(0, cluster->getEnergy());
}

void EVEVisualization::addROI(const ROIid* roi, const TString& name)
{

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  VxdID sensorID = roi->getSensorID();
  const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensorID);

  double minU = aSensorInfo.getUCellPosition(roi->getMinUid(), roi->getMinVid());
  double minV = aSensorInfo.getVCellPosition(roi->getMinVid());
  double maxU = aSensorInfo.getUCellPosition(roi->getMaxUid(), roi->getMaxVid());
  double maxV = aSensorInfo.getVCellPosition(roi->getMaxVid());


  TVector3 localA(minU, minV, 0);
  TVector3 localB(minU, maxV, 0);
  TVector3 localC(maxU, minV, 0);

  TVector3 globalA = aSensorInfo.pointToGlobal(localA);
  TVector3 globalB = aSensorInfo.pointToGlobal(localB);
  TVector3 globalC = aSensorInfo.pointToGlobal(localC);

  TEveBox* ROIbox = boxCreator((globalB + globalC) * 0.5 , globalB - globalA, globalC - globalA, 1, 1, 0.01);

  ROIbox->SetName(name);
  ROIbox->SetMainColor(kSpring - 9);
  ROIbox->SetMainTransparency(50);

  addToGroup("ROIs", ROIbox);
  addObject(roi, ROIbox);

}

void EVEVisualization::addRecoHit(const SVDCluster* hit, TEveStraightLineSet* lines)
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

  lines->AddLine(a.x(), a.y(), a.z(), b.x(), b.y(), b.z());
  m_shownRecohits.insert(hit);
}

void EVEVisualization::addRecoHit(const CDCHit* hit, TEveStraightLineSet* lines)
{
  static CDC::CDCGeometryPar& cdcgeo = CDC::CDCGeometryPar::Instance();
  const TVector3& wire_pos_f = cdcgeo.wireForwardPosition(WireID(hit->getID()));
  const TVector3& wire_pos_b = cdcgeo.wireBackwardPosition(WireID(hit->getID()));

  lines->AddLine(wire_pos_f.x(), wire_pos_f.y(), wire_pos_f.z(), wire_pos_b.x(), wire_pos_b.y(), wire_pos_b.z());
  m_shownRecohits.insert(hit);
}

void EVEVisualization::showUserData(const DisplayData& displayData)
{
  for (const auto & labelPair : displayData.m_labels) {
    TEveText* text = new TEveText(labelPair.first.c_str());
    text->SetName(labelPair.first.c_str());
    text->SetTitle(labelPair.first.c_str());
    text->SetMainColor(kGray + 1);
    const TVector3& p = labelPair.second;
    text->PtrMainTrans()->SetPos(p.x(), p.y(), p.z());
    addToGroup("DisplayData", text);
  }

  for (const auto & pointPair : displayData.m_pointSets) {
    TEvePointSet* points = new TEvePointSet(pointPair.first.c_str());
    points->SetTitle(pointPair.first.c_str());
    points->SetMarkerStyle(7);
    points->SetMainColor(kGreen);
    for (const TVector3 & p : pointPair.second) {
      points->SetNextPoint(p.x(), p.y(), p.z());
    }
    addToGroup("DisplayData", points);
  }

  int randomColor = 2; //primary colours, changing rapidly with index
  for (const auto & arrow : displayData.m_arrows) {
    const TVector3 pos = arrow.start;
    const TVector3 dir = arrow.end - pos;
    TEveArrow* eveArrow = new TEveArrow(dir.x(), dir.y(), dir.z(), pos.x(), pos.y(), pos.z());
    eveArrow->SetName(arrow.name.c_str());
    eveArrow->SetTitle(arrow.name.c_str());
    int arrowColor = arrow.color;
    if (arrowColor == -1) {
      arrowColor = randomColor;
      randomColor++;
    }
    eveArrow->SetMainColor(arrowColor);

    //add label
    TEveText* text = new TEveText(arrow.name.c_str());
    text->SetMainColor(arrowColor);
    //in middle of arrow, with some slight offset
    const TVector3& labelPos = pos + 0.5 * dir + 0.1 * dir.Orthogonal();
    text->PtrMainTrans()->SetPos(labelPos.x(), labelPos.y(), labelPos.z());
    eveArrow->AddElement(text);
    addToGroup("DisplayData", eveArrow);
  }

}
void EVEVisualization::addObject(const TObject* dataStoreObject, TEveElement* visualRepresentation)
{
  m_visualRepMap->add(dataStoreObject, visualRepresentation);
}

void EVEVisualization::addToGroup(const std::string& name, TEveElement* elem)
{
  //slashes at beginning and end are ignored
  const std::string& groupName = boost::algorithm::trim_copy_if(name, boost::algorithm::is_any_of("/"));

  TEveElementList* group = m_groups[groupName];
  if (!group) {
    group = new TEveElementList(groupName.c_str(), groupName.c_str());
    m_groups[groupName] = group;

    //if groupName contains '/', remove last bit and add to parent group
    //e.g. if groupName=A/B/C, call addToGroup("A/B", groupC)
    auto lastSlash = boost::algorithm::find_last(groupName, "/");
    if (lastSlash) {
      const std::string parentGroup(groupName.begin(), lastSlash.begin());
      const std::string thisGroup(lastSlash.end(), groupName.end());
      group->SetElementName(thisGroup.c_str());
      addToGroup(parentGroup, group);
    } else {
      gEve->AddElement(group);
    }
  }
  group->AddElement(elem);
}

void EVEVisualization::addTrackCandidateTFInfo(TrackCandidateTFInfo* info)
{

  TEveLine* line = new TEveLine(info->getCoordinates().size());
  line->SetName(TString::Format("VXDTF TC: %d", info->getOwnID()));
  line->SetTitle(info->getDisplayInformation());
  line->SetMainColor(info->getColor());
  for (auto & v : info->getCoordinates()) {
    line->SetNextPoint(v.x(), v.y(), v.z());
  }

  addToGroup("VXDTF/Track Candidates", line);
  addObject(info, line);
}

void EVEVisualization::addCellTFInfo(CellTFInfo* info)
{
  TEveLine* line = new TEveLine(info->getCoordinates().size());
  line->SetName(TString::Format("Cell idx %d", info->getArrayIndex()));
  line->SetTitle(info->getDisplayInformation());
  line->SetMainColor(info->getColor());
  for (auto & v : info->getCoordinates()) {
    line->SetNextPoint(v.x(), v.y(), v.z());
  }

  addToGroup("VXDTF/Cells", line);
  addObject(info, line);
}

void EVEVisualization::addSectorTFInfo(SectorTFInfo* info)
{
  TString name = TString::Format("Sector %d", info->getSectorID());

  const std::vector<TVector3>& vertices = info->getCoordinates();
  TVector3 u = vertices[1] - vertices[0];
  TVector3 v = vertices[2] - vertices[0];
  TVector3 center = vertices[0] + 0.5 * u + 0.5 * v;

  //u, v are the correct length, so use du = dv = 1.0
  TEveBox* box = boxCreator(center, u, v, 1.0, 1.0, 0.01);
  box->SetName(name);
  box->SetTitle(info->getDisplayInformation());
  box->SetMainColor(info->getColor());
  box->SetMainTransparency(60);

  addToGroup("VXDTF/Sectors", box);
  addObject(info, box);
}
