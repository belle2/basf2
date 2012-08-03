#include <tracking/modules/trackingDisplay/GenfitDisplay.h>

#include <framework/core/InputController.h>
#include <framework/logging/Logger.h>
#include <geometry/GeometryManager.h>

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
#include <TEveManager.h>
#include <TEveEventManager.h>
#include <TEveGeoNode.h>
#include <TEveGeoShape.h>
#include <TEveProjectionAxes.h>
#include <TEveScene.h>
#include <TEvePointSet.h>
#include <TEveProjectionManager.h>
#include <TEveStraightLineSet.h>
#include <TEveViewer.h>
#include <TGFileDialog.h>
#include <TGLViewer.h>
#include <TGeoEltu.h>
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TGeoNode.h>
#include <TGeoSphere.h>
#include <TGeoTube.h>
#include <TMath.h>
#include <TMatrixT.h>
#include <TMatrixDEigen.h>
#include <TSystem.h>
#include <TVector2.h>
#include <TVectorD.h>
#include "TGeoManager.h"

#include <assert.h>
#include <cmath>
#include <exception>
#include <iostream>


using namespace Belle2;

GenfitDisplay::GenfitDisplay():
  fEventId(0),
  m_guiInitialized(false),
  m_prevButton(0),
  m_nextButton(0)
{
  if ((!gApplication) || (gApplication && gApplication->TestBit(TApplication::kDefaultApplication))) {
    B2INFO("gApplication not found, creating...");
    new TApplication("ROOT_application", 0, 0);
  }
  if (!gEve) {
    B2INFO("gEve not found, creating...");
    TEveManager::Create(true, "IV"); //hide file browser
  }
  gGeoManager->DefaultColors();

  fEventId = 0;
  m_guiInitialized = false;
  setOptions();
  setErrScale();
}

void GenfitDisplay::setOptions(const std::string& opts) { fOption = opts; }

void GenfitDisplay::setErrScale(double errScale) { fErrorScale = errScale; }

double GenfitDisplay::getErrScale() const { return fErrorScale; }

GenfitDisplay::~GenfitDisplay() { reset(); }

void GenfitDisplay::reset()
{
  for (unsigned int j = 0; j < m_tracks.size(); j++) {
    //delete GFTrack
    delete m_tracks.at(j);
  }

  m_tracks.clear();
  fHits.clear();
}

void GenfitDisplay::addEvent(const std::vector<GFTrack*>& evts)
{
  for (unsigned int i = 0; i < evts.size(); i++) {
    m_tracks.push_back(new GFTrack(*(evts.at(i))));
  }
}

void GenfitDisplay::next(unsigned int stp)
{
  goToEvent(fEventId + stp);
}

void GenfitDisplay::prev(unsigned int stp)
{
  if (!InputController::canControlInput())
    return;
  if (fEventId < (int)stp) {
    goToEvent(0);
  } else {
    goToEvent(fEventId - stp);
  }
}

void GenfitDisplay::goToEvent(unsigned int id)
{
  const long numEntries = InputController::numEntries();

  //change UI state?
  m_prevButton->SetEnabled(id > 0);
  m_nextButton->SetEnabled((id + 1 < numEntries) or !InputController::canControlInput());

  if (fEventId == id) return;

  if (!InputController::canControlInput() && fEventId != id - 1) {
    B2ERROR("Cannot switch to event " << id << ", only works in conjunction with SimpleInput.");
  }

  fEventId = id;

  if (gEve->GetCurrentEvent())
    gEve->GetCurrentEvent()->DestroyElements();
  if (numEntries > 0 && InputController::canControlInput()) {
    B2DEBUG(100, "Switching to event " << fEventId);
    InputController::setNextEntry(fEventId);
  }

  B2DEBUG(100, "exiting event loop now.");
  //exit event loop to allow basf2 to go to next event
  gSystem->ExitLoop();

  /*
  double old_error_scale = fErrorScale;
  drawEvent(fEventId);
  if (old_error_scale != fErrorScale) {
    if (gEve->GetCurrentEvent())
      gEve->GetCurrentEvent()->DestroyElements();
    drawEvent(fEventId); // if autoscaling changed the error, draw again.
  }
  fErrorScale = old_error_scale;
  */
}

void GenfitDisplay::open()
{
  if (m_guiInitialized && !gEve) {
    //window closed?
    B2WARNING("no TEveManager found, skipping display");
    return;
  }

  bool drawSilent = false;
  bool drawGeometry = false;

  // parse the global options
  for (size_t i = 0; i < fOption.length(); i++) {
    if (fOption.at(i) == 'X') drawSilent = true;
    if (fOption.at(i) == 'G') drawGeometry = true;
  }

  // draw the geometry, does not really work yet. If it's fixed, the docu in the header file should be changed.
  if (!m_guiInitialized && drawGeometry) {
    TGeoNode* top_node = gGeoManager->GetTopNode();
    assert(top_node != NULL);

    //Set transparency of geometry
    TObjArray* volumes = gGeoManager->GetListOfVolumes();
    for (int i = 0; i < volumes->GetEntriesFast(); i++) {
      TGeoVolume* volume = static_cast<TGeoVolume*>(volumes->At(i));
      volume->SetTransparency(50);
    }

    TEveGeoTopNode* eve_top_node = new TEveGeoTopNode(gGeoManager, top_node);
    eve_top_node->IncDenyDestroy();
    eve_top_node->SetVisLevel(2);
    gEve->AddGlobalElement(eve_top_node);
  }
  TGeoVolume* ecl = gGeoManager->FindVolumeFast("logical_ecl");
  if (ecl)
    ecl->SetVisDaughters(false);
  else
    B2WARNING("ECL not found?");
  TGeoVolume* cdc = gGeoManager->FindVolumeFast("logicalCDC");
  if (cdc)
    cdc->SetVisDaughters(false);
  else
    B2WARNING("CDC not found?");

  double old_error_scale = fErrorScale;
  drawEvent();
  //if (old_error_scale != fErrorScale) goToEvent(0); // if autoscaling changed the error, draw again.
  fErrorScale = old_error_scale;

  if (!drawSilent) {
    if (!m_guiInitialized) {
      makeGui();
      goToEvent(fEventId); //update button state
      m_guiInitialized = true;
    }
    //make display interactive
    gApplication->Run(true); //return from Run()
    //interactive part done, event data removed from scene
  }
}

void GenfitDisplay::drawEvent()
{
  // parse the option string ------------------------------------------------------------------------
  bool drawAutoScale = false;
  bool drawDetectors = false;
  bool drawHits = false;
  bool drawScaleMan = false;
  bool drawTrackMarkers = false;
  bool drawPlanes = false;
  bool drawTrack = false;
  bool drawRawHits = false;

  if (fOption != "") {
    for (size_t i = 0; i < fOption.length(); i++) {
      if (fOption.at(i) == 'A') drawAutoScale = true;
      if (fOption.at(i) == 'D') drawDetectors = true;
      if (fOption.at(i) == 'H') drawHits = true;
      if (fOption.at(i) == 'M') drawTrackMarkers = true;
      if (fOption.at(i) == 'P') drawPlanes = true;
      if (fOption.at(i) == 'S') drawScaleMan = true;
      if (fOption.at(i) == 'T') drawTrack = true;
      if (fOption.at(i) == 'R') drawRawHits = true;
    }
  }
  // finished parsing the option string -------------------------------------------------------------

  gEve->SetStatusLine(TString::Format("Showing event %ld, with %d tracks and %lu raw hits.", fEventId, (int)m_tracks.size(), (drawHits ? fHits.size() : 0)).Data());

  // draw SPHits  // quick n dirty hack
  if (drawRawHits && !fHits.empty()) {
    TEvePointSet* pointSet = new TEvePointSet("Raw hits", fHits.size());
    pointSet->SetMainColor(kGray);
    pointSet->SetMainTransparency(70);
    for (unsigned int j = 0; j < fHits.size(); ++j) {
      pointSet->SetNextPoint(
        fHits[j][0],
        fHits[j][1],
        fHits[j][2]);
    }
    gEve->AddElement(pointSet);
  }


  for (unsigned int i = 0; i < m_tracks.size(); i++) { // loop over all tracks in an event
    GFTrack* track = m_tracks.at(i);

    const int irep = 0;
    GFAbsTrackRep* rep = track->getTrackRep(irep);

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

    TVector3 old_track_pos;

    TEveStraightLineSet* track_lines = NULL;

    // saving the initial state of the representation -----------------------------------------
    GFDetPlane initial_plane = rep->getReferencePlane();
    TMatrixT<double> initial_state(rep->getState());
    TMatrixT<double> initial_cov(rep->getCov());
    TMatrixT<double> initial_auxInfo;
    if (rep->hasAuxInfo()) {
      initial_auxInfo.ResizeTo(*(rep->getAuxInfo(initial_plane)));
      initial_auxInfo = (*(rep->getAuxInfo(initial_plane)));
    }
    // saved initial state --------------------------------------------------------------------

    for (unsigned int j = 0; j < numhits; j++) { // loop over all hits in the track

      GFAbsRecoHit* hit = track->getHit(j);
      GFDetPlane plane;

      // get the hit infos ------------------------------------------------------------------
      if (smoothing) {
        TMatrixT<double> state;
        TMatrixT<double> cov;
        TMatrixT<double> auxInfo;
        GFTools::getSmoothedData(track, irep, j, state, cov, plane, auxInfo);
        rep->setData(state, plane, &cov, &auxInfo);
      } else {
        try {
          plane = hit->getDetPlane(rep);
          rep->extrapolate(plane);
        } catch (GFException& e) {
          B2WARNING("Exception caught (getDetPlane): Hit " << j << " in Track " << i << " skipped! " << e.what());
          if (e.isFatal()) {
            B2WARNING("Fatal exception, skipping track");
            break;
          } else continue;
        }
      }
      const TVector3& track_pos = rep->getPos(plane);
      const TVector3& plane_pos = plane.getO();
      TMatrixT<double> hit_coords;
      TMatrixT<double> hit_cov;
      hit->getMeasurement(rep, plane, rep->getState(), rep->getCov(), hit_coords, hit_cov);
      // finished getting the hit infos -----------------------------------------------------

      // sort hit infos into variables ------------------------------------------------------
      TVector3 o = plane.getO();
      TVector3 u = plane.getU();
      TVector3 v = plane.getV();

      const std::string& hit_type = hit->getPolicyName();

      bool planar_hit = false;
      bool planar_pixel_hit = false;
      bool space_hit = false;
      bool wire_hit = false;
      double_t hit_u = 0;
      double_t hit_v = 0;
      double_t plane_size = 4;
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
        plane_size = 4;
      } else if (hit_type == "GFWireHitPolicy") {
        wire_hit = true;
        hit_u = hit_coords(0, 0);
        plane_size = 4;
      } else {
        B2WARNING("Track " << i << ", Hit " << j << ": Unknown policy name: skipping hit!");
        break;
      }

      if (plane_size < 4) plane_size = 4;
      // finished setting variables ---------------------------------------------------------

      // draw track if corresponding option is set ------------------------------------------
      if (drawTrack) {
        if (track_lines == NULL) {
          track_lines = new TEveStraightLineSet(TString::Format("GFTrack %d (pVal: %e)", i, TMath::Prob(track->getChiSqu(), track->getNDF())));
        }
        if (j > 0) track_lines->AddLine(old_track_pos(0), old_track_pos(1), old_track_pos(2), track_pos(0), track_pos(1), track_pos(2));
        old_track_pos = track_pos;
        if (charge > 0) {
          track_lines->SetLineColor(kRed);
        } else {
          track_lines->SetLineColor(kBlue);
        }
        track_lines->SetLineWidth(2);
        if (drawTrackMarkers) {
          track_lines->AddMarker(track_pos(0), track_pos(1), track_pos(2));
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
          double pseudo_res_0 = fErrorScale * std::sqrt(hit_cov(0, 0));
          if (!drawHits) { // if the hits are also drawn, make the tube smaller to avoid intersecting volumes
            det_shape->SetShape(new TGeoTube(0, hit_u, plane_size));
          } else {
            det_shape->SetShape(new TGeoTube(0, hit_u - pseudo_res_0, plane_size));
          }
          TVector3 norm = u.Cross(v);
          TGeoRotation* det_rot = new TGeoRotation("det_rot", (u.Theta() * 180) / TMath::Pi(), (u.Phi() * 180) / TMath::Pi(),
                                                   (norm.Theta() * 180) / TMath::Pi(), (norm.Phi() * 180) / TMath::Pi(),
                                                   (v.Theta() * 180) / TMath::Pi(), (v.Phi() * 180) / TMath::Pi()); // move the tube to the right place and rotate it correctly
          TGeoMatrix* det_trans = new TGeoCombiTrans(o(0), o(1), o(2), det_rot);
          det_shape->SetTransMatrix(*det_trans);
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
          TVector2 plane_coords = plane.LabToPlane(plane_pos);
          if (!planar_pixel_hit) {
            TEveBox* hit_box;
            hit_box = boxCreator((plane_pos + (plane_coords.Px() - hit_u) * u), u, v, fErrorScale * std::sqrt(hit_res_u), plane_size, 0.0105);
            hit_box->SetMainColor(kYellow);
            hit_box->SetMainTransparency(0);
            if (track_lines)
              track_lines->AddElement(hit_box);
            else
              gEve->AddElement(hit_box);
          } else {
            // calculate eigenvalues to draw error-ellipse ----------------------------
            TMatrixDEigen eigen_values(hit_cov);
            TEveGeoShape* det_shape = new TEveGeoShape("planar hit");
            det_shape->IncDenyDestroy();
            TMatrixT<double> ev = eigen_values.GetEigenValues();
            TMatrixT<double> eVec = eigen_values.GetEigenVectors();
            double pseudo_res_0 = fErrorScale * std::sqrt(ev(0, 0));
            double pseudo_res_1 = fErrorScale * std::sqrt(ev(1, 1));
            // finished calcluating, got the values -----------------------------------

            // do autoscaling if necessary --------------------------------------------
            if (drawAutoScale) {
              double min_cov = std::min(pseudo_res_0, pseudo_res_1);
              if (min_cov < 1e-5) {
                std::cout << "Track " << i << ", Hit " << j << ": Invalid covariance matrix (Eigenvalue < 1e-5), autoscaling not possible!" << std::endl;
              } else {
                if (min_cov < 0.049) {
                  double cor = 0.05 / min_cov;
                  std::cout << "Track " << i << ", Hit " << j << ": Pixel covariance too small, rescaling by " << cor;
                  fErrorScale *= cor;
                  pseudo_res_0 *= cor;
                  pseudo_res_1 *= cor;
                  std::cout << " to " << fErrorScale << std::endl;
                }
              }
            }
            // finished autoscaling ---------------------------------------------------

            // calculate the semiaxis of the error ellipse ----------------------------
            det_shape->SetShape(new TGeoEltu(pseudo_res_0, pseudo_res_1, 0.0105));
            TVector3 pix_pos = plane_pos + (plane_coords.Px() - hit_u) * u + (plane_coords.Py() - hit_v) * v;
            TVector3 u_semiaxis = (pix_pos + eVec(0, 0) * u + eVec(1, 0) * v) - pix_pos;
            TVector3 v_semiaxis = (pix_pos + eVec(0, 1) * u + eVec(1, 1) * v) - pix_pos;
            TVector3 norm = u.Cross(v);
            // finished calculating ---------------------------------------------------

            // rotate and translate everything correctly ------------------------------
            TGeoRotation* det_rot = new TGeoRotation("det_rot", (u_semiaxis.Theta() * 180) / TMath::Pi(), (u_semiaxis.Phi() * 180) / TMath::Pi(),
                                                     (v_semiaxis.Theta() * 180) / TMath::Pi(), (v_semiaxis.Phi() * 180) / TMath::Pi(),
                                                     (norm.Theta() * 180) / TMath::Pi(), (norm.Phi() * 180) / TMath::Pi());
            TGeoMatrix* det_trans = new TGeoCombiTrans(pix_pos(0), pix_pos(1), pix_pos(2), det_rot);
            det_shape->SetTransMatrix(*det_trans);
            // finished rotating and translating --------------------------------------

            det_shape->SetMainColor(kYellow);
            det_shape->SetMainTransparency(0);
            if (track_lines)
              track_lines->AddElement(det_shape);
            else
              gEve->AddElement(det_shape);
          }
        }
        // finished drawing planar hits ---------------------------------------------------

        // draw spacepoint hits -----------------------------------------------------------
        if (space_hit) {

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


          TGeoRotation* det_rot = new TGeoRotation("det_rot", (eVec1.Theta() * 180) / TMath::Pi(), (eVec1.Phi() * 180) / TMath::Pi(),
                                                   (eVec2.Theta() * 180) / TMath::Pi(), (eVec2.Phi() * 180) / TMath::Pi(),
                                                   (eVec3.Theta() * 180) / TMath::Pi(), (eVec3.Phi() * 180) / TMath::Pi()); // the rotation is already clear

          // set the scaled eigenvalues -------------------------------------------------
          double pseudo_res_0 = fErrorScale * std::sqrt(ev(0, 0));
          double pseudo_res_1 = fErrorScale * std::sqrt(ev(1, 1));
          double pseudo_res_2 = fErrorScale * std::sqrt(ev(2, 2));
          if (drawScaleMan) { // override again if necessary
            pseudo_res_0 = fErrorScale * 0.5;
            pseudo_res_1 = fErrorScale * 0.5;
            pseudo_res_2 = fErrorScale * 0.5;
          }
          // finished scaling -----------------------------------------------------------

          // autoscale if necessary -----------------------------------------------------
          if (drawAutoScale) {
            double min_cov = std::min(pseudo_res_0, std::min(pseudo_res_1, pseudo_res_2));
            if (min_cov < 1e-5) {
              std::cout << "Track " << i << ", Hit " << j << ": Invalid covariance matrix (Eigenvalue < 1e-5), autoscaling not possible!" << std::endl;
            } else {
              if (min_cov <= 0.149) {
                double cor = 0.15 / min_cov;
                std::cout << "Track " << i << ", Hit " << j << ": Space hit covariance too small, rescaling by " << cor;
                fErrorScale *= cor;
                pseudo_res_0 *= cor;
                pseudo_res_1 *= cor;
                pseudo_res_2 *= cor;
                std::cout << " to " << fErrorScale << std::endl;
              }
            }
          }
          // finished autoscaling -------------------------------------------------------

          // rotate and translate -------------------------------------------------------
          TGeoMatrix* det_trans = new TGeoGenTrans(o(0), o(1), o(2), 1 / (pseudo_res_0), 1 / (pseudo_res_1), 1 / (pseudo_res_2), det_rot);
          det_shape->SetTransMatrix(*det_trans);
          // finished rotating and translating ------------------------------------------

          det_shape->SetMainColor(kYellow);
          det_shape->SetMainTransparency(0);
          if (track_lines)
            track_lines->AddElement(det_shape);
          else
            gEve->AddElement(det_shape);
        }
        // finished drawing spacepoint hits -----------------------------------------------

        // draw wire hits -----------------------------------------------------------------
        if (wire_hit) {
          TEveGeoShape* det_shape = new TEveGeoShape("wire hit");
          det_shape->IncDenyDestroy();
          double pseudo_res_0 = fErrorScale * std::sqrt(hit_cov(0, 0));

          // autoscale if necessary -----------------------------------------------------
          if (drawAutoScale) {
            if (pseudo_res_0 < 1e-5) {
              std::cout << "Track " << i << ", Hit " << j << ": Invalid wire resolution (< 1e-5), autoscaling not possible!" << std::endl;
            } else {
              if (pseudo_res_0 < 0.0049) {
                double cor = 0.005 / pseudo_res_0;
                std::cout << "Track " << i << ", Hit " << j << ": Wire covariance too small, rescaling by " << cor;
                fErrorScale *= cor;
                pseudo_res_0 *= cor;
                std::cout << " to " << fErrorScale << std::endl;
              }
            }
          }
          // finished autoscaling -------------------------------------------------------

          det_shape->SetShape(new TGeoTube(std::min(0., (double)(hit_u - pseudo_res_0)), hit_u + pseudo_res_0, plane_size));
          TVector3 norm = u.Cross(v);

          // rotate and translate -------------------------------------------------------
          TGeoRotation* det_rot = new TGeoRotation("det_rot", (u.Theta() * 180) / TMath::Pi(), (u.Phi() * 180) / TMath::Pi(),
                                                   (norm.Theta() * 180) / TMath::Pi(), (norm.Phi() * 180) / TMath::Pi(),
                                                   (v.Theta() * 180) / TMath::Pi(), (v.Phi() * 180) / TMath::Pi());
          TGeoMatrix* det_trans = new TGeoCombiTrans(o(0), o(1), o(2), det_rot);
          det_shape->SetTransMatrix(*det_trans);
          // finished rotating and translating ------------------------------------------

          det_shape->SetMainColor(kYellow);
          det_shape->SetMainTransparency(50);
          if (track_lines)
            track_lines->AddElement(det_shape);
          else
            gEve->AddElement(det_shape);
        }
        // finished drawing wire hits -----------------------------------------------------

      }

    }

    // reseting to the initial state ----------------------------------------------------------
    rep->setData(initial_state, initial_plane, &initial_cov, &initial_auxInfo);

    try {
      rep->extrapolate(initial_plane);
    } catch (GFException& e) {
      B2WARNING("Error: Exception caught: could not extrapolate back to initial plane " << e.what());
      continue;
    }
    // done resetting -------------------------------------------------------------------------

    if (track_lines != NULL) gEve->AddElement(track_lines);

  }

  gEve->Redraw3D(false); //do not reset camera when redrawing
}


void GenfitDisplay::setHits(const std::vector<std::vector<double> > &hits) { fHits = hits; }


TEveBox* GenfitDisplay::boxCreator(const TVector3& o, TVector3 u, TVector3 v, float ud, float vd, float depth)
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

void GenfitDisplay::makeGui()
{
  TEveBrowser* browser = gEve->GetBrowser();
  browser->HideBottomTab();

  browser->StartEmbedding(TRootBrowser::kLeft);

  TGMainFrame* frmMain = new TGMainFrame(gClient->GetRoot(), 1000, 600);
  frmMain->SetWindowName("XX GUI");
  frmMain->SetCleanup(kDeepCleanup);

  TGHorizontalFrame* hf = new TGHorizontalFrame(frmMain);
  {
    TString icondir(Form("%s/icons/", gSystem->Getenv("ROOTSYS")));

    m_prevButton = new TGPictureButton(hf, gClient->GetPicture(icondir + "GoBack.gif"));
    hf->AddFrame(m_prevButton);
    m_prevButton->Connect("Clicked()", "Belle2::GenfitDisplay", this, "prev()");

    m_nextButton = new TGPictureButton(hf, gClient->GetPicture(icondir + "GoForward.gif"));
    hf->AddFrame(m_nextButton);
    m_nextButton->Connect("Clicked()", "Belle2::GenfitDisplay", this, "next()");
  }
  frmMain->AddFrame(hf);

  hf = new TGHorizontalFrame(frmMain);
  {
    TGButton* b = 0;
    b = new TGTextButton(hf, "r-phi");
    hf->AddFrame(b);
    b->Connect("Clicked()", "Belle2::GenfitDisplay", this, "setRPhiProjection()");

    b = new TGTextButton(hf, "3D");
    hf->AddFrame(b);
    b->Connect("Clicked()", "Belle2::GenfitDisplay", this, "setDefaultProjection()");

    b = new TGTextButton(hf, "Dark/light colors");
    hf->AddFrame(b);
    b->Connect("Clicked()", "TGLViewer", gEve->GetDefaultGLViewer(), "SwitchColorSet()");
  }
  frmMain->AddFrame(hf);

  hf = new TGHorizontalFrame(frmMain);
  {
    TGButton* b = 0;
    b = new TGTextButton(hf, "Save Picture");
    hf->AddFrame(b);
    b->Connect("Clicked()", "Belle2::GenfitDisplay", this, "savePicture()");

    b = new TGTextButton(hf, "Save Hi-Res Picture");
    hf->AddFrame(b);
    b->Connect("Clicked()", "Belle2::GenfitDisplay", this, "saveHiResPicture()");

  }
  frmMain->AddFrame(hf);

  frmMain->MapSubwindows();
  frmMain->Resize();
  frmMain->MapWindow();

  browser->StopEmbedding();
  browser->SetTabTitle("Event Control", 0);
}

void GenfitDisplay::setRPhiProjection()
{
  TGLViewer* v = gEve->GetDefaultGLViewer();
  v->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
}

void GenfitDisplay::setDefaultProjection()
{
  TGLViewer* v = gEve->GetDefaultGLViewer();
  v->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
}

void GenfitDisplay::savePicture(bool highres)
{
  TGFileInfo fi;
  //deleting the pointer crashes, so I'm assuming this is magically cleaned up at some point
  new TGFileDialog(gEve->GetBrowser()->GetClient()->GetDefaultRoot(), gEve->GetBrowser(), kFDSave, &fi);
  if (!fi.fFilename)
    return; //cancelled
  TGLViewer* v = gEve->GetDefaultGLViewer();
  if (!highres)
    v->SavePicture(fi.fFilename);
  else
    v->SavePictureWidth(fi.fFilename, 4000);

  B2INFO("Saved picture in: " << fi.fFilename)

  //file dialog leaves empty box, redraw
  gEve->Redraw3D(false); //do not reset camera when redrawing
}

ClassImp(GenfitDisplay)
