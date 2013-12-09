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
#include <display/modules/display/EVEVisualization.h>

#include <display/dataobjects/DisplayData.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <geometry/GeometryManager.h>
#include <vxd/geometry/GeoCache.h>
#include <ecl/geometry/ECLGeometryPar.h>

#include <svd/reconstruction/SVDRecoHit.h>


#include <TApplication.h>
#include <TEveBox.h>
#include <TEveCalo.h>
#include <TEveManager.h>
#include <TEveEventManager.h>
#include <TEveGeoNode.h>
#include <TEveGeoShape.h>
#include <TEveGeoShapeExtract.h>
#include <TEveScene.h>
#include <TEvePointSet.h>
#include <TEveProjectionManager.h>
#include <TEveStraightLineSet.h>
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
#include <TROOT.h>
#include <TSystem.h>
#include <TVector2.h>
#include <TVectorD.h>

#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/scoped_ptr.hpp>

#include <assert.h>
#include <cmath>
#include <exception>
#include <iostream>


using namespace Belle2;

const double MIN_P_CUT = 0.00;

EVEVisualization::EVEVisualization():
  m_fullgeo(false),
  m_assignToPrimaries(false),
  m_trackcandlist(0),
  m_eclsimhitdata(0),
  m_unassignedRecoHits(0)
{
  setErrScale();

  TGLLogicalShape::SetIgnoreSizeForCameraInterest(kTRUE);     // Allows the visualization of the "small" error ellipsoid.

  //create new containers
  m_trackpropagator = new TEveTrackPropagator();
  m_trackpropagator->IncDenyDestroy();
  m_trackpropagator->SetMagFieldObj(&m_bfield, false);

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
  m_gftrackpropagator->SetMaxOrbs(0.01); //stop after track markers

  m_gftracklist = new TEveTrackList("Fitted tracks", m_gftrackpropagator);
  m_gftracklist->IncDenyDestroy();

  m_calo3d = new TEveCalo3D(NULL, "ECLHits");
  m_calo3d->SetBarrelRadius(125.80); //inner radius of ECL barrel
  m_calo3d->SetForwardEndCapPos(196.5); //inner edge of forward endcap
  m_calo3d->SetBackwardEndCapPos(-102.0); //inner edge of backward endcap
  m_calo3d->SetMaxValAbs(2.1);
  m_calo3d->SetRnrFrame(false, false); //don't show crystal grid
  m_calo3d->IncDenyDestroy();

  clearEvent();
}

void EVEVisualization::setOptions(const std::string& opts) { m_options = opts; }

void EVEVisualization::setErrScale(double errScale) { m_errorScale = errScale; }

EVEVisualization::~EVEVisualization()
{
  if (!gEve)
    return; //objects are probably already freed by Eve

  delete m_eclsimhitdata;
  delete m_unassignedRecoHits;
  delete m_tracklist;
  delete m_gftracklist;
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

  TGeoNode* top_node = gGeoManager->GetTopNode();
  assert(top_node != NULL);
  TEveGeoTopNode* eve_top_node = new TEveGeoTopNode(gGeoManager, top_node);
  eve_top_node->IncDenyDestroy();
  eve_top_node->SetVisLevel(2);
  gEve->AddGlobalElement(eve_top_node);

  //don't show full geo unless turned on by user
  eve_top_node->SetRnrSelfChildren(m_fullgeo, m_fullgeo);


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

void EVEVisualization::addTrack(const genfit::Track* gftrack, const TString& label)
{
  // parse the option string ------------------------------------------------------------------------
  bool drawDetectors = false;
  bool drawHits = false;
  bool drawScaleMan = false;
  bool drawMarkers = false;
  bool drawPlanes = false;
  bool drawTrack = false;

  if (m_options != "") {
    for (size_t i = 0; i < m_options.length(); i++) {
      if (m_options.at(i) == 'D') drawDetectors = true;
      if (m_options.at(i) == 'H') drawHits = true;
      if (m_options.at(i) == 'M') drawMarkers = true;
      if (m_options.at(i) == 'P') drawPlanes = true;
      if (m_options.at(i) == 'S') drawScaleMan = true;
      if (m_options.at(i) == 'T') drawTrack = true;
    }
  }
  // finished parsing the option string -------------------------------------------------------------

  /*
    //copy original track
    boost::scoped_ptr<genfit::Track> track(new genfit::Track(*gftrack));

    //copy original track rep
    boost::scoped_ptr<genfit::AbsTrackRep> rep(track->getCardinalRep()->clone());
    const unsigned int irep = track->getCardinalRepID();

    unsigned int numhits = track->getNumHits();
    double charge = rep->getCharge();

    bool smoothing = track->getSmoothing();

    if (rep->getStatusFlag()) {
      B2WARNING("Trying to display a track with status flag != 0...");
      if (smoothing) {
        B2WARNING("trying without smoothing!");
        smoothing = false;
      }
    }

    TEveTrack* track_lines = NULL;
    if (drawTrack) {
      GFDetPlane plane = rep->getFirstPlane();
      rep->extrapolate(plane);
      const TVector3& track_pos = rep->getPos(plane);
      const TVector3& track_mom = rep->getMom(plane);

      TEveRecTrack rectrack;
      rectrack.fP.Set(track_mom);
      rectrack.fV.Set(track_pos);
      rectrack.fSign = (int)charge;

      track_lines = new TEveTrack(&rectrack, m_gftrackpropagator);
      track_lines->SetName(label); //popup label set at end of function
      track_lines->SetPropagator(m_gftrackpropagator);
      track_lines->SetLineColor(c_trackColor);
      track_lines->SetLineWidth(2);
      track_lines->SetTitle(TString::Format("%s\n"
                                            "#hits: %u\n"
                                            "pT=%.3f, pZ=%.3f\n"
                                            "pVal: %e",
                                            label.Data(), numhits,
                                            track_mom.Pt(), track_mom.Pz(),
                                            TMath::Prob(track->getChiSqu(), track->getNDF())));

      track_lines->SetCharge((int)charge);

      if (numhits == 0) {
        //most likely the track was read from file, so we generate entire track shape from initial plane
        m_gftrackpropagator->SetMaxOrbs(0.5);
        track_lines->MakeTrack();
      } else if (drawMarkers) {
        track_lines->SetRnrPoints(true);
      }
    }*/

  /*
    for (unsigned int j = 0; j < numhits; j++) { // loop over all hits in the track

      GFAbsRecoHit* hit = track->getHit(j);
      GFDetPlane plane;

      // get the hit infos ------------------------------------------------------------------
      try {
        if (smoothing) {
          TVectorD state;
          TMatrixDSym cov;
          TMatrixD auxInfo;
          GFTools::getBiasedSmoothedData(track.get(), irep, j, state, cov, plane, auxInfo);
          rep->setData(state, plane, &cov, &auxInfo);
        } else {
          plane = hit->getDetPlane(rep.get());
          rep->extrapolate(plane);
        }
      } catch (GFException& e) {
        B2WARNING("Exception caught (getDetPlane): Hit " << j << " skipped! " << e.what());
        if (e.isFatal()) {
          B2WARNING("Fatal exception, skipping track");
          break;
        } else continue;
      }
      const TVector3& track_pos = rep->getPos(plane);
      TVectorD hit_coords;
      TMatrixDSym hit_cov;
      hit->getMeasurement(rep.get(), plane, rep->getState(), rep->getCov(), hit_coords, hit_cov);
      // finished getting the hit infos -----------------------------------------------------

      // sort hit infos into variables ------------------------------------------------------
      const TVector3& o = plane.getO();
      const TVector3& u = plane.getU();
      const TVector3& v = plane.getV();

      bool planar_hit = false;
      bool planar_pixel_hit = false;
      bool space_hit = false;
      bool wire_hit = false;
      double_t hit_u = 0;
      double_t hit_v = 0;
      const float plane_size = 4.0;
      double_t hit_res_u = 0.5;
      //double_t hit_res_v = 0.5; //unused

      int hit_coords_dim = hit_coords.GetNrows();

      if (dynamic_cast<GFAbsPlanarHit*>(hit) != NULL) {
        planar_hit = true;
        if (hit_coords_dim == 1) {
          hit_u = hit_coords(0);
          hit_res_u = hit_cov(0, 0);
        } else if (hit_coords_dim == 2) {
          planar_pixel_hit = true;
          hit_u = hit_coords(0);
          hit_v = hit_coords(1);
          hit_res_u = hit_cov(0, 0);
          //hit_res_v = hit_cov(1, 1);
        }
      } else if (dynamic_cast<GFAbsSpacepointHit*>(hit) != NULL) {
        space_hit = true;
      } else if (dynamic_cast<GFAbsWireHit*>(hit) != NULL) {
        wire_hit = true;
        hit_u = hit_coords(0);
        hit_v = v * (track_pos - o); // move the covariance tube so that the track goes through it
        hit_res_u = hit_cov(0, 0);
        //hit_res_v = 4;
        //GFAbsWirepointHit not used
      } else {
        B2WARNING("Hit " << j << ": Unknown policy name: skipping hit!");
        continue;
      }

      // finished setting variables ---------------------------------------------------------

      // draw track if corresponding option is set ------------------------------------------
      if (drawTrack) {
        TEvePathMarkD refMark(TEvePathMarkD::kDaughter); //doesn't need momentum
        refMark.fV.Set(track_pos);
        refMark.fTime = track_pos.Mag(); //path marks can later be sorted by 'time'...
        track_lines->AddPathMark(refMark);
      }
      // finished drawing track -------------------------------------------------------------

      // draw planes if corresponding option is set -----------------------------------------
      if (drawPlanes || (drawDetectors && planar_hit)) {
        TVector3 move(0, 0, 0);
        if (wire_hit)
          move = v * (v * (track_pos - o)); // move the plane along the wire until the track goes through it
        TEveBox* box = boxCreator(o + move, u, v, plane_size, plane_size, 0.01);
        box->SetName("Detector Plane");

        if (drawDetectors && planar_hit) {
          box->SetMainColor(kCyan);
        } else {
          box->SetMainColor(kGray);
        }
        box->SetMainTransparency(50);
        if (track_lines)
          track_lines->AddElement(box);
        else
          gEve->AddElement(box);
      }
      // finished drawing planes ------------------------------------------------------------


      // draw detectors if option is set, only important for wire hits ----------------------
      if (drawDetectors) {

        if (wire_hit && !drawHits) { //make sure to only draw once, even with both drawHits and drawDetectors
          TEveGeoShape* det_shape = new TEveGeoShape("wire hit");
          det_shape->IncDenyDestroy();
          double pseudo_res_0 = std::sqrt(hit_cov(0, 0));
          if (!drawHits) { // if the hits are also drawn, make the tube smaller to avoid intersecting volumes
            det_shape->SetShape(new TGeoTube(0, hit_u, plane_size));
          } else {
            det_shape->SetShape(new TGeoTube(0, hit_u - pseudo_res_0, plane_size));
          }
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
          if ((drawHits && (hit_u - pseudo_res_0 > 0)) || !drawHits) {
            if (track_lines)
              track_lines->AddElement(det_shape);
            else
              gEve->AddElement(det_shape);
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
            const SVDRecoHit* recoHit = static_cast<SVDRecoHit*>(hit);
            const VXD::SensorInfoBase& sensor = geo.get(recoHit->getSensorID());
            double du, dv;
            TVector3 a = o; //defines position of sensor plane
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
            hit_box->SetName(TString::Format("SVDRecoHit %u", j));
            hit_box->SetMainColor(c_recoHitColor);
            hit_box->SetMainTransparency(0);
            if (track_lines)
              track_lines->AddElement(hit_box);
            else
              gEve->AddElement(hit_box);
          } else {
            // calculate eigenvalues to draw error-ellipse ----------------------------
            TEveGeoShape* det_shape = new TEveGeoShape("planar pixel hit");
            det_shape->IncDenyDestroy();
            TMatrixDEigen eigen_values(hit_cov);
            TMatrixT<double> ev = eigen_values.GetEigenValues();
            TMatrixT<double> eVec = eigen_values.GetEigenVectors();
            double pseudo_res_0 = m_errorScale * std::sqrt(ev(0, 0));
            double pseudo_res_1 = m_errorScale * std::sqrt(ev(1, 1));
            // finished calcluating, got the values -----------------------------------

            // calculate the semiaxis of the error ellipse ----------------------------
            det_shape->SetShape(new TGeoEltu(pseudo_res_0, pseudo_res_1, 0.0105));
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
            det_shape->SetTransMatrix(det_trans);
            // finished rotating and translating --------------------------------------

            det_shape->SetMainColor(c_recoHitColor);
            det_shape->SetMainTransparency(0);
            if (track_lines)
              track_lines->AddElement(det_shape);
            else
              gEve->AddElement(det_shape);
          }
        } else if (space_hit) {
          B2WARNING("Space hit used (shouldn't happen in basf2)");
          //currently unused in Belle2 {{{
          // get eigenvalues of covariance to know how to draw the ellipsoid ------------
          TMatrixDEigen eigen_values(hit->getRawHitCov());
          TEveGeoShape* det_shape = new TEveGeoShape("spacepoint hit");
          det_shape->IncDenyDestroy();
          det_shape->SetShape(new TGeoSphere(0., 1.));
          TMatrixT<double> ev = eigen_values.GetEigenValues();
          TMatrixT<double> eVec = eigen_values.GetEigenVectors();
          TVector3 eVec1(eVec(0, 0), eVec(1, 0), eVec(2, 0));
          TVector3 eVec2(eVec(0, 1), eVec(1, 1), eVec(2, 1));
          TVector3 eVec3(eVec(0, 2), eVec(1, 2), eVec(2, 2));
          // got everything we need -----------------------------------------------------


          TGeoRotation det_rot("det_rot", (eVec1.Theta() * 180) / TMath::Pi(), (eVec1.Phi() * 180) / TMath::Pi(),
                               (eVec2.Theta() * 180) / TMath::Pi(), (eVec2.Phi() * 180) / TMath::Pi(),
                               (eVec3.Theta() * 180) / TMath::Pi(), (eVec3.Phi() * 180) / TMath::Pi()); // the rotation is already clear

          // set the scaled eigenvalues -------------------------------------------------
          double pseudo_res_0 = m_errorScale * std::sqrt(ev(0, 0));
          double pseudo_res_1 = m_errorScale * std::sqrt(ev(1, 1));
          double pseudo_res_2 = m_errorScale * std::sqrt(ev(2, 2));
          if (drawScaleMan) { // override again if necessary
            pseudo_res_0 = m_errorScale * 0.5;
            pseudo_res_1 = m_errorScale * 0.5;
            pseudo_res_2 = m_errorScale * 0.5;
          }
          // finished scaling -----------------------------------------------------------

          // rotate and translate -------------------------------------------------------
          TGeoGenTrans det_trans(o(0), o(1), o(2), 1 / (pseudo_res_0), 1 / (pseudo_res_1), 1 / (pseudo_res_2), &det_rot);
          det_shape->SetTransMatrix(det_trans);
          // finished rotating and translating ------------------------------------------

          det_shape->SetMainColor(c_recoHitColor);
          det_shape->SetMainTransparency(0);
          if (track_lines)
            track_lines->AddElement(det_shape);
          else
            gEve->AddElement(det_shape);
          //}}}
        } else if (wire_hit) {
          TEveGeoShape* det_shape = new TEveGeoShape("wire hit");
          det_shape->IncDenyDestroy();
          double pseudo_res_0 = std::sqrt(hit_cov(0, 0));

          det_shape->SetShape(new TGeoTube(std::min(0., (double)(hit_u - pseudo_res_0)), hit_u + pseudo_res_0, plane_size));
          TVector3 norm = u.Cross(v);

          // rotate and translate -------------------------------------------------------
          TGeoRotation det_rot("det_rot", (u.Theta() * 180) / TMath::Pi(), (u.Phi() * 180) / TMath::Pi(),
                               (norm.Theta() * 180) / TMath::Pi(), (norm.Phi() * 180) / TMath::Pi(),
                               (v.Theta() * 180) / TMath::Pi(), (v.Phi() * 180) / TMath::Pi());
          TGeoCombiTrans det_trans(o(0) + hit_v * v.X(),
                                   o(1) + hit_v * v.Y(),
                                   o(2) + hit_v * v.Z(),
                                   &det_rot);
          det_shape->SetTransMatrix(det_trans);
          // finished rotating and translating ------------------------------------------

          det_shape->SetMainColor(c_recoHitColor);
          det_shape->SetMainTransparency(50);
          if (track_lines)
            track_lines->AddElement(det_shape);
          else
            gEve->AddElement(det_shape);
        }
        // finished drawing planar hits -----------------------------------------------------
      }
    }

    if (track_lines) {
      m_gftracklist->AddElement(track_lines);
    }
    */
}


TEveBox* EVEVisualization::boxCreator(const TVector3& o, TVector3 u, TVector3 v, float ud, float vd, float depth)
{
  TEveBox* box = new TEveBox;
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
  const TVector3& global_pos = hit->getHitPosition();
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

void EVEVisualization::addSimHit(const ECLHit* hit, const MCParticle* particle)
{
  addMCParticle(particle);

  const int cell = hit->getCellId();
  const TVector3& pos = ECL::ECLGeometryPar::Instance()->GetCrystalPos(cell - 1);
  const float eta = (float)pos.Eta();
  const float phi = (float)pos.Phi();

  //crystals are ~6 by 6 cm in crossection
  TVector3 dPhiVec = pos.Cross(TVector3(0., 0., 1.0));
  dPhiVec.SetMag(2.8); //half a crystal width in phi
  const float dPhi = (float)pos.DeltaPhi(pos + dPhiVec);
  TVector3 dEtaVec = pos.Cross(dPhiVec);
  dEtaVec.SetMag(2.8); //half a crystal width in eta
  const float dEta = (float)TMath::Abs((pos + dEtaVec).Eta() - eta);

  m_eclsimhitdata->AddTower(eta - dEta, eta + dEta, phi - dPhi, phi + dPhi);
  m_eclsimhitdata->FillSlice(0, hit->getEnergyDep());
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
  //workaround for workaround, remove once bug #937 is fixed
  if (particle->getDecayTime() == 0.0 and particle->getDecayTime() == particle->getProductionTime()) {
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
                        (particle->getMother() ? particle->getMother()->getIndex() - 1 : 0), 0, particle->getFirstDaughter(), particle->getLastDaughter(),
                        p.x(), p.y(), p.z(), particle->getEnergy(),
                        vertex.x(), vertex.y(), vertex.z(), particle->getProductionTime());
    TEveMCTrack mctrack;
    mctrack = tparticle;
    mctrack.fTDecay = particle->getDecayTime();
    mctrack.fVDecay.Set(particle->getDecayVertex());
    mctrack.fDecayed = !boost::math::isinf(mctrack.fTDecay);
    mctrack.fIndex = particle->getIndex() - 1;
    m_mcparticleTracks[particle].track = new TEveTrack(&mctrack, m_trackpropagator);

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
    //also make sure a decay vertex is set, and that it is not set to (0,0,0) (as with particle gun)
    //last bit is workaround for particlegun bug #937
    if ((TMath::Nint(particle->getCharge()) == 0 or !particle->hasStatus(MCParticle::c_StoppedInDetector))
        and mctrack.fDecayed and mctrack.fTDecay != 0.0) {
      TEvePathMarkD decayMark(TEvePathMarkD::kDecay);
      decayMark.fV.Set(particle->getDecayVertex());
      m_mcparticleTracks[particle].track->AddPathMark(decayMark);
    }
    TString particle_name(mctrack.GetName());

    //set track title (for popup)
    TString momLabel = "";
    if (particle->getMother())
      momLabel = TString::Format("\nMother: Idx=%d, PDG=%d)", particle->getMother()->getIndex() - 1, particle->getMother()->getPDG());

    m_mcparticleTracks[particle].track->SetTitle(TString::Format(
                                                   "MCParticle Index=%d\n"
                                                   "Chg=%d, PDG=%d (%s)\n"
                                                   "pT=%.3f, pZ=%.3f\nV=(%.3f, %.3f, %.3f)"
                                                   "%s",
                                                   particle->getIndex() - 1,
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
    const TString pointsTitle = TString::Format("SimHits for MCParticle %d (%s)", particle->getIndex() - 1, particle_name.Data());
    m_mcparticleTracks[particle].simhits = new TEvePointSet(pointsTitle);
    m_mcparticleTracks[particle].simhits->SetTitle(pointsTitle);
    m_mcparticleTracks[particle].simhits->SetMarkerStyle(6);
    m_mcparticleTracks[particle].simhits->SetMainColor(m_mcparticleTracks[particle].track->GetLineColor());
    //m_mcparticleTracks[particle].simhits->SetMainTransparency(50);
    m_mcparticleTracks[particle].track->AddElement(m_mcparticleTracks[particle].simhits);
  }
  return &m_mcparticleTracks[particle];
}

void EVEVisualization::makeTracks()
{
  std::map<const MCParticle*, MCTrack>::iterator it = m_mcparticleTracks.begin();
  const std::map<const MCParticle*, MCTrack>::iterator& end = m_mcparticleTracks.end();
  for (; it != end; ++it) {
    if (it->second.track)
      m_tracklist->AddElement(it->second.track);
    else //add simhits directly
      gEve->AddElement(it->second.simhits);
  }
  gEve->AddElement(m_tracklist);
  m_tracklist->MakeTracks();
  m_tracklist->SelectByP(MIN_P_CUT, 15.0); //don't show too many particles by default...

  for (size_t i = 0; i < m_options.length(); i++) {
    if (m_options.at(i) == 'M') {
      m_gftrackpropagator->SetRnrDaughters(true);
      //m_gftrackpropagator->SetRnrFV(true); //TODO: this crashes?
      TMarker m;
      m.SetMarkerColor(c_trackMarkerColor);
      m.SetMarkerStyle(1); //a single pixel
      m.SetMarkerSize(1); //ignored.
      m_gftrackpropagator->RefPMAtt() = m;
      m_gftrackpropagator->RefFVAtt() = m;
    } else if (m_options.at(i) == 'T') {
      m_gftracklist->SetRnrLine(true);
    }
  }
  gEve->AddElement(m_gftracklist);

  if (m_trackcandlist)
    gEve->AddElement(m_trackcandlist);

  m_eclsimhitdata->DataChanged(); //update limits (Empty() won't work otherwise)
  if (!m_eclsimhitdata->Empty()) {
    m_eclsimhitdata->SetAxisFromBins();
    m_calo3d->SetData(m_eclsimhitdata);
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
  m_mcparticleTracks.clear();
  m_shownRecohits.clear();
  m_tracklist->DestroyElements();
  m_gftracklist->DestroyElements();
  if (m_trackcandlist)
    m_trackcandlist->DestroyElements();


  //remove ECL data from event
  m_calo3d->SetData(NULL);
  m_calo3d->DestroyElements();

  //lower energy threshold for ECL
  float ecl_threshold = 0.01;
  if (m_eclsimhitdata)
    ecl_threshold = m_eclsimhitdata->GetSliceThreshold(0);

  delete m_eclsimhitdata;
  m_eclsimhitdata = new TEveCaloDataVec(1); //#slices
  m_eclsimhitdata->IncDenyDestroy();
  m_eclsimhitdata->RefSliceInfo(0).Setup("ECL", ecl_threshold, kRed);

  delete m_unassignedRecoHits;
  m_unassignedRecoHits = 0;

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
  gEve->AddElement(vertexPoint);
}


void EVEVisualization::addGamma(const ECLGamma* gamma, const TString& name)
{
  TVector3 Momentum = gamma->getMomentum();
  Momentum.SetMag(200);

  float energy = gamma->getEnergy();
  float pX = gamma->getPx();
  float pY = gamma->getPy();
  float pZ = gamma->getPz();

  TEveLine* gammaVis = new TEveLine(name);
  gammaVis->SetNextPoint(0, 0, 0); //assuming gamma came from IP
  gammaVis->SetNextPoint(Momentum.x(), Momentum.y(), Momentum.z());
  gammaVis->SetTitle(TString::Format("%s\n"
                                     "Energy=%.3f\n"
                                     "p=(%.3f, %.3f, %.3f)",
                                     name.Data(), energy, pX, pY, pZ));

  gammaVis->SetMainColor(kGreen + 2);
  gammaVis->SetLineWidth(2.0);
  m_calo3d->AddElement(gammaVis);
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
    text->SetTitle(labelPair.first.c_str());
    text->SetMainColor(kGray + 1);
    const TVector3& p = labelPair.second;
    text->PtrMainTrans()->SetPos(p.x(), p.y(), p.z());
    gEve->AddElement(text);
  }

  for (const auto & pointPair : displayData.m_pointSets) {
    TEvePointSet* points = new TEvePointSet(pointPair.first.c_str());
    points->SetTitle(pointPair.first.c_str());
    points->SetMarkerStyle(7);
    points->SetMainColor(kGreen);
    for (const TVector3 & p : pointPair.second) {
      points->SetNextPoint(p.x(), p.y(), p.z());
    }
    gEve->AddElement(points);
  }

}
