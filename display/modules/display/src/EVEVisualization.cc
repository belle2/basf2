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

#include <framework/logging/Logger.h>
#include <framework/core/Environment.h>
#include <geometry/GeometryManager.h>
#include <vxd/geometry/GeoCache.h>
#include <ecl/geometry/ECLGeometryPar.h>

#include <GFAbsRecoHit.h>
#include <GFAbsTrackRep.h>
#include <GFConstField.h>
#include <GFDetPlane.h>
#include <GFException.h>
#include <GFFieldManager.h>
#include <GFTools.h>

#include <TApplication.h>
#include <TEveBrowser.h>
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
#include <TEveTrack.h>
#include <TEveTrackPropagator.h>
#include <TFile.h>
#include <TGeoEltu.h>
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TGeoNode.h>
#include <TGeoSphere.h>
#include <TGeoTube.h>
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

EVEVisualization::EVEVisualization():
  m_assignToPrimaries(false),
  m_eclsimhitdata(0)
{
  setOptions();
  setErrScale();
  clearEvent();
}

void EVEVisualization::setOptions(const std::string& opts) { m_options = opts; }

void EVEVisualization::setErrScale(double errScale) { m_errorScale = errScale; }

double EVEVisualization::getErrScale() const { return m_errorScale; }

EVEVisualization::~EVEVisualization()
{
  if (!gEve)
    return; //objects are probably already freed by Eve

  delete m_eclsimhitdata;
  delete m_trackpropagator;
  delete m_gftrackpropagator;
}

void EVEVisualization::enableVolume(const char* name, bool only_daughters, bool enable)
{
  TGeoVolume* vol = gGeoManager->FindVolumeFast(name);
  if (vol) {
    if (!only_daughters)
      vol->SetVisibility(enable);
    vol->SetVisDaughters(enable);
  } else {
    B2WARNING("Volume " << name << " not found?");
  }
}

void EVEVisualization::setVolumeColor(const char* name, Color_t col)
{
  TGeoVolume* vol = gGeoManager->FindVolumeFast(name);
  if (vol) {
    //while TGeoVolume derives from TAttFill, the line color actually is important here
    vol->SetLineColor(col);
  } else {
    B2WARNING("Volume " << name << " not found?");
  }
}

void EVEVisualization::addGeometry()
{
  const bool saveExtract = false;

  B2INFO("Setting up geometry for TEve...");
  //set colours by atomic mass number
  gGeoManager->DefaultColors();

  //Set transparency of geometry
  TObjArray* volumes = gGeoManager->GetListOfVolumes();
  for (int i = 0; i < volumes->GetEntriesFast(); i++) {
    TGeoVolume* volume = static_cast<TGeoVolume*>(volumes->At(i));
    volume->SetTransparency(50);
  }

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
  if (saveExtract) {
    //Endcaps look strange on top level
    disableVolume("Endcap_1");
    disableVolume("Endcap_2");
  }

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

  if (saveExtract) {
    TGeoManager* my_tgeomanager = gGeoManager;
    eve_top_node->ExpandIntoListTrees();
    eve_top_node->SaveExtract("geometry_extract.root", "Extract", false);

    //this doesn't work too well...
    //eve_top_node->ExpandIntoListTreesRecursively();
    //eve_top_node->SaveExtract("display_geometry_full.root", "Extract", false);
    gGeoManager = my_tgeomanager;
  }

  //don't show full geo unless turned on by user
  eve_top_node->SetRnrSelfChildren(false, false);


  B2INFO("Loading geometry projections...");

  //Since TEveGeoShapeExtract() replaces the global TGeoManager (??!), we'll make a backup copy
  //TODO: remove once 5.34.2+ is in externals
  TGeoManager* my_tgeomanager = gGeoManager;

  const std::string extractPath = Environment::Instance().getDataSearchPath() + std::string("/display/geometry_extract.root");
  TFile* f = TFile::Open(extractPath.c_str(), "READ");
  TEveGeoShapeExtract* gse = dynamic_cast<TEveGeoShapeExtract*>(f->Get("Extract"));
  TEveGeoShape* gs = TEveGeoShape::ImportShapeExtract(gse, 0);
  gs->SetRnrSelf(false);
  delete f;

  //I want to show full geo in unprojected view,
  //but I still need to add the extract to the geometry scene...
  gEve->AddGlobalElement(gs);

  //restore old TGeoManager
  gGeoManager = my_tgeomanager;
}

void EVEVisualization::saveGeometry(const std::string& name)
{
  gGeoManager->Export(name.c_str());
}

void EVEVisualization::addTrack(const GFTrack* gftrack, const TString& label)
{
  // parse the option string ------------------------------------------------------------------------
  bool drawAutoScale = false;
  bool drawDetectors = false;
  bool drawHits = false;
  bool drawScaleMan = false;
  bool drawPlanes = false;
  bool drawTrack = false;

  if (m_options != "") {
    for (size_t i = 0; i < m_options.length(); i++) {
      if (m_options.at(i) == 'A') drawAutoScale = true;
      if (m_options.at(i) == 'D') drawDetectors = true;
      if (m_options.at(i) == 'H') drawHits = true;
      if (m_options.at(i) == 'P') drawPlanes = true;
      if (m_options.at(i) == 'S') drawScaleMan = true;
      if (m_options.at(i) == 'T') drawTrack = true;
    }
  }
  // finished parsing the option string -------------------------------------------------------------

  //copy original track
  boost::scoped_ptr<GFTrack> track(new GFTrack(*gftrack));

  const int irep = 0;
  //copy original track rep
  boost::scoped_ptr<GFAbsTrackRep> rep(track->getTrackRep(irep)->clone());

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

  for (unsigned int j = 0; j < numhits; j++) { // loop over all hits in the track

    GFAbsRecoHit* hit = track->getHit(j);
    GFDetPlane plane;

    // get the hit infos ------------------------------------------------------------------
    try {
      if (smoothing) {
        TMatrixT<double> state;
        TMatrixT<double> cov;
        TMatrixT<double> auxInfo;
        GFTools::getSmoothedData(track.get(), irep, j, state, cov, plane, auxInfo);
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
    const TVector3& track_mom = rep->getMom(plane);
    const TVector3& plane_pos = plane.getO();
    TMatrixT<double> hit_coords;
    TMatrixT<double> hit_cov;
    hit->getMeasurement(rep.get(), plane, rep->getState(), rep->getCov(), hit_coords, hit_cov);
    // finished getting the hit infos -----------------------------------------------------

    // sort hit infos into variables ------------------------------------------------------
    const TVector3& o = plane.getO();
    const TVector3& u = plane.getU();
    const TVector3& v = plane.getV();

    const std::string& hit_type = hit->getPolicyName();

    bool planar_hit = false;
    bool planar_pixel_hit = false;
    bool space_hit = false;
    bool wire_hit = false;
    double_t hit_u = 0;
    double_t hit_v = 0;
    const float plane_size = 4.0;
    double_t hit_res_u = 0.5;
    //double_t hit_res_v = 0.5; //not actually used

    int hit_coords_dim = hit_coords.GetNrows();

    if (hit_type == "GFPlanarHitPolicy") {
      planar_hit = true;
      if (hit_coords_dim == 1) {
        hit_u = hit_coords(0, 0);
        hit_res_u = hit_cov(0, 0);
      } else if (hit_coords_dim == 2) {
        planar_pixel_hit = true;
        hit_u = hit_coords(0, 0);
        hit_v = hit_coords(1, 0);
        hit_res_u = hit_cov(0, 0);
        //hit_res_v = hit_cov(1, 1);
      }
    } else if (hit_type == "GFSpacepointHitPolicy") {
      space_hit = true;
    } else if (hit_type == "GFWireHitPolicy") {
      wire_hit = true;
      hit_u = hit_coords(0, 0);
    } else {
      B2WARNING("Hit " << j << ": Unknown policy name: skipping hit!");
      continue;
    }

    // finished setting variables ---------------------------------------------------------

    // draw track if corresponding option is set ------------------------------------------
    if (drawTrack) {
      if (!track_lines) {
        TEveRecTrack rectrack;
        rectrack.fP.Set(track_mom);
        rectrack.fV.Set(track_pos);

        track_lines = new TEveTrack(&rectrack, m_gftrackpropagator);
        track_lines->SetName(label); //popup label set at end of function
        track_lines->SetPropagator(m_gftrackpropagator);
        if (charge > 0) {
          track_lines->SetLineColor(kRed);
        } else {
          track_lines->SetLineColor(kBlue);
        }
        track_lines->SetLineWidth(2);
        track_lines->SetTitle(TString::Format("%s\n"
                                              "#hits: %u\n"
                                              "pT=%.3f, pZ=%.3f\n"
                                              "pVal: %e",
                                              label.Data(), numhits,
                                              track_mom.Pt(), track_mom.Pz(),
                                              TMath::Prob(track->getChiSqu(), track->getNDF())));

        track_lines->SetCharge((int)charge);
      } else {
        TEvePathMarkD refMark(TEvePathMarkD::kDaughter); //doesn't need momentum
        refMark.fV.Set(track_pos);
        refMark.fTime = track_pos.Mag(); //path marks can later be sorted by 'time'...
        track_lines->AddPathMark(refMark);
      }
    }
    // finished drawing track -------------------------------------------------------------

    // draw planes if corresponding option is set -----------------------------------------
    if (drawPlanes || (drawDetectors && planar_hit)) {
      TEveBox* box = boxCreator(plane_pos, u, v, plane_size, plane_size, 0.01);
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
        TGeoCombiTrans det_trans(o(0), o(1), o(2), &det_rot);
        det_shape->SetTransMatrix(det_trans);
        det_shape->SetMainColor(kCyan);
        det_shape->SetMainTransparency(0);
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
          //currently unused in Belle2 {{{
          TEveBox* hit_box;
          hit_box = boxCreator((plane_pos + hit_u * u), u, v, m_errorScale * std::sqrt(hit_res_u), plane_size, 0.0105);
          hit_box->SetMainColor(kYellow);
          hit_box->SetMainTransparency(0);
          if (track_lines)
            track_lines->AddElement(hit_box);
          else
            gEve->AddElement(hit_box);
          //}}}
        } else {
          // calculate eigenvalues to draw error-ellipse ----------------------------
          TEveGeoShape* det_shape = new TEveGeoShape("planar hit");
          det_shape->IncDenyDestroy();
          TMatrixDEigen eigen_values(hit_cov);
          TMatrixT<double> ev = eigen_values.GetEigenValues();
          TMatrixT<double> eVec = eigen_values.GetEigenVectors();
          double pseudo_res_0 = m_errorScale * std::sqrt(ev(0, 0));
          double pseudo_res_1 = m_errorScale * std::sqrt(ev(1, 1));
          // finished calcluating, got the values -----------------------------------

          // do autoscaling if necessary --------------------------------------------
          if (drawAutoScale) {
            double min_cov = std::min(pseudo_res_0, pseudo_res_1);
            if (min_cov < 1e-5) {
              std::cout << "Hit " << j << ": Invalid covariance matrix (Eigenvalue < 1e-5), autoscaling not possible!" << std::endl;
            } else {
              if (min_cov < 0.049) {
                double cor = 0.05 / min_cov;
                std::cout << "Hit " << j << ": Pixel covariance too small, rescaling by " << cor;
                m_errorScale *= cor;
                pseudo_res_0 *= cor;
                pseudo_res_1 *= cor;
                std::cout << " to " << m_errorScale << std::endl;
              }
            }
          }
          // finished autoscaling ---------------------------------------------------

          // calculate the semiaxis of the error ellipse ----------------------------
          det_shape->SetShape(new TGeoEltu(pseudo_res_0, pseudo_res_1, 0.0105));
          TVector3 pix_pos = plane_pos + hit_u * u + hit_v * v;
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

          det_shape->SetMainColor(kYellow);
          det_shape->SetMainTransparency(0);
          if (track_lines)
            track_lines->AddElement(det_shape);
          else
            gEve->AddElement(det_shape);
        }
      } else if (space_hit) {
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

        // autoscale if necessary -----------------------------------------------------
        if (drawAutoScale) {
          double min_cov = std::min(pseudo_res_0, std::min(pseudo_res_1, pseudo_res_2));
          if (min_cov < 1e-5) {
            std::cout << "Hit " << j << ": Invalid covariance matrix (Eigenvalue < 1e-5), autoscaling not possible!" << std::endl;
          } else {
            if (min_cov <= 0.149) {
              double cor = 0.15 / min_cov;
              std::cout << "Hit " << j << ": Space hit covariance too small, rescaling by " << cor;
              m_errorScale *= cor;
              pseudo_res_0 *= cor;
              pseudo_res_1 *= cor;
              pseudo_res_2 *= cor;
              std::cout << " to " << m_errorScale << std::endl;
            }
          }
        }
        // finished autoscaling -------------------------------------------------------

        // rotate and translate -------------------------------------------------------
        TGeoGenTrans det_trans(o(0), o(1), o(2), 1 / (pseudo_res_0), 1 / (pseudo_res_1), 1 / (pseudo_res_2), &det_rot);
        det_shape->SetTransMatrix(det_trans);
        // finished rotating and translating ------------------------------------------

        det_shape->SetMainColor(kYellow);
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
        TGeoCombiTrans det_trans(o(0), o(1), o(2), &det_rot);
        det_shape->SetTransMatrix(det_trans);
        // finished rotating and translating ------------------------------------------

        det_shape->SetMainColor(kYellow);
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
void EVEVisualization::addSimHit(const TOPSimHit* hit, const MCParticle* particle)
{
  const TVector3& global_pos = hit->getVposition(); //at photon emission
  addSimHit(global_pos, particle);
}
void EVEVisualization::addSimHit(const BKLMSimHit* hit, const MCParticle* particle)
{
  const TVector3& global_pos = hit->getHitPosition();
  addSimHit(global_pos, particle);
}
/*
void EVEVisualization::addSimHit(const EKLMSimHit* hit, const MCParticle* particle)
{
  const TVector3* global_pos = hit->getPosition();
  addSimHit(*global_pos, particle);
}
*/
void EVEVisualization::addSimHit(const TVector3& v, const MCParticle* particle)
{
  TEvePointSet* simhits = addMCParticle(particle).simhits;
  simhits->SetNextPoint(v.x(), v.y(), v.z());
}

void EVEVisualization::addECLHit(const HitECL* hit)
{
  const int cell = hit->getCellId();
  const TVector3& pos = ECL::ECLGeometryPar::Instance()->GetCrystalPos(cell);
  const float eta = (float)pos.Eta();
  const float phi = (float)pos.Phi();
  //maybe these should depend on position?
  //crystals are ~6 by 6 cm in crossection
  static const float dEta = 0.02;
  static const float dPhi = 0.02;

  m_eclsimhitdata->AddTower(eta - dEta, eta + dEta, phi - dPhi, phi + dPhi);
  m_eclsimhitdata->FillSlice(0, hit->getEnergyDep());
}

EVEVisualization::MCTrack& EVEVisualization::addMCParticle(const MCParticle* particle)
{
  if (m_assignToPrimaries) {
    while (!particle->hasStatus(MCParticle::c_PrimaryParticle) and particle->getMother())
      particle = particle->getMother();
  }

  if (!m_mcparticleTracks[particle].track) {
    const TVector3& p = particle->getMomentum();
    const TVector3& vertex = particle->getProductionVertex();
    int pdg = particle->getPDG();
    //TODO: remove this workaround once a fix is in the externals
    static const bool unknown_pdg_is_unsafe = gROOT->GetVersionInt() <= 53401;
    bool workaround_active = false;
    if (unknown_pdg_is_unsafe) {
      switch (abs(pdg)) {
        case 11:
        case 13:
        case 22:
        case 211:
        case 321:
        case 2212:
          break;
        default:
          workaround_active = true;
          //let TEveTrack pretend it's something safe.
          if (TMath::Nint(particle->getCharge()) == 0)
            pdg = 22;
          else
            pdg = (particle->getCharge() > 0) ? -11 : 11;
          break;
      }
    }
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

    //neutrals and very short-lived particles should stop somewhere
    //(can result in wrong shapes for particles stopped in the detector, so not used there)
    if (TMath::Nint(particle->getCharge()) == 0 or !particle->hasStatus(MCParticle::c_StoppedInDetector)) {
      TEvePathMarkD decayMark(TEvePathMarkD::kDecay);
      decayMark.fV.Set(particle->getDecayVertex());
      m_mcparticleTracks[particle].track->AddPathMark(decayMark);
    }
    TString particle_name(mctrack.GetName());
    //and since we don't want Ups(4S) to show up as 'gamma'...
    if (workaround_active) {
      particle_name = TString::Format("PDG: %d", particle->getPDG());
      m_mcparticleTracks[particle].track->SetName(particle_name);
    }

    //set track title (for popup)
    TString momLabel = "";
    if (particle->getMother())
      momLabel = TString::Format("\nMother: Idx=%d, PDG=%d)", particle->getMother()->getIndex(), particle->getMother()->getPDG());

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
    switch (abs(particle->getPDG())) {
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
    m_mcparticleTracks[particle].simhits->SetMainColor(m_mcparticleTracks[particle].track->GetLineColor());
    m_mcparticleTracks[particle].simhits->SetMainTransparency(70);
    m_mcparticleTracks[particle].track->AddElement(m_mcparticleTracks[particle].simhits);
  }
  return m_mcparticleTracks[particle];
}

void EVEVisualization::makeTracks()
{
  TEveTrackList* tracks = new TEveTrackList(m_trackpropagator);
  tracks->SetName("MCParticles");
  std::map<const MCParticle*, MCTrack>::iterator it = m_mcparticleTracks.begin();
  const std::map<const MCParticle*, MCTrack>::iterator& end = m_mcparticleTracks.end();
  for (; it != end; ++it) {
    tracks->AddElement(it->second.track);
  }
  gEve->AddElement(tracks);
  tracks->MakeTracks();
  tracks->SelectByP(0.01, 15.0); //don't show too many particles by default...

  for (size_t i = 0; i < m_options.length(); i++) {
    if (m_options.at(i) == 'M') {
      m_gftrackpropagator->SetRnrDaughters(true);
      m_gftracklist->SetRnrPoints(true);
      //m_gftrackpropagator->SetRnrFV(true); //TODO: this crashes?
      TMarker m;
      m.SetMarkerColor(kSpring);
      m.SetMarkerStyle(1); //a single pixel
      m.SetMarkerSize(1); //ignored.
      m_gftrackpropagator->RefPMAtt() = m;
      m_gftrackpropagator->RefFVAtt() = m;
    } else if (m_options.at(i) == 'T') {
      m_gftracklist->SetRnrLine(true);
    }
  }
  gEve->AddElement(m_gftracklist);

  m_eclsimhitdata->DataChanged(); //update limits (Empty() wont' work otherwise)
  if (!m_eclsimhitdata->Empty()) {
    m_eclsimhitdata->SetAxisFromBins();
    TEveCalo3D* calo3d = new TEveCalo3D(m_eclsimhitdata, "ECLHits");
    calo3d->SetBarrelRadius(127.00); //inner radius of ECL barrel
    calo3d->SetEndCapPos(196.5); //inner edge of forward endcap
    //TODO: needs a patched ROOT version
    //calo3d->SetForwardEndCapPos(196.5); //inner edge of forward endcap
    //calo3d->SetBackwardEndCapPos(-102.0); //inner edge of backward endcap
    calo3d->SetMaxValAbs(2.1);
    gEve->AddElement(calo3d);
  }
}

void EVEVisualization::clearEvent()
{
  if (!gEve)
    return;
  m_mcparticleTracks.clear();
  //other things are cleaned up by TEve...

  //also create new containers
  m_trackpropagator = new TEveTrackPropagator();
  m_trackpropagator->SetMagFieldObj(&m_bfield, false);
  m_trackpropagator->SetMaxR(380); //don't draw tracks outside detector
  m_gftracklist = new TEveTrackList("Fitted tracks", m_trackpropagator);
  m_gftrackpropagator = new TEveTrackPropagator();
  //TODO: add more complex magnetic field (important for BKLM)
  m_gftrackpropagator->SetMagFieldObj(&m_bfield, false);
  m_gftrackpropagator->SetMaxOrbs(0.01); //stop after track markers
  delete m_eclsimhitdata;
  m_eclsimhitdata = new TEveCaloDataVec(1); //#slices
  m_eclsimhitdata->IncDenyDestroy();
  m_eclsimhitdata->RefSliceInfo(0).Setup("ECL", 0.01, kRed); //set lower energy threshold here
}
