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

#include <display/VisualRepMap.h>
#include <display/EveGeometry.h>
#include <display/EveVisBField.h>

#include <vxd/geometry/GeoCache.h>
#include <bklm/dataobjects/BKLMSimHitPosition.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <bklm/geometry/GeometryPar.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <top/geometry/TOPGeometryPar.h>

#include <genfit/AbsMeasurement.h>
#include <genfit/PlanarMeasurement.h>
#include <genfit/SpacepointMeasurement.h>
#include <genfit/WireMeasurement.h>
#include <genfit/WireMeasurementNew.h>
#include <genfit/WirePointMeasurement.h>
#include <genfit/DetPlane.h>
#include <genfit/Exception.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/GblFitterInfo.h>

#include <framework/dataobjects/DisplayData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/ColorPalette.h>

#include <TEveArrow.h>
#include <TEveBox.h>
#include <TEveCalo.h>
#include <TEveManager.h>
#include <TEveGeoShape.h>
#include <TEveLine.h>
#include <TEvePointSet.h>
#include <TEveSelection.h>
#include <TEveStraightLineSet.h>
#include <TEveTriangleSet.h>
#include <TEveText.h>
#include <TEveTrack.h>
#include <TEveTrackPropagator.h>
#include <TGeoEltu.h>
#include <TGeoMatrix.h>
#include <TGeoNode.h>
#include <TGeoManager.h>
#include <TGeoSphere.h>
#include <TGeoTube.h>
#include <TGLLogicalShape.h>
#include <TParticle.h>
#include <TMath.h>
#include <TVectorD.h>
#include <TMatrixD.h>
#include <TMatrixDSymEigen.h>

#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <cassert>
#include <cmath>

using namespace Belle2;
using namespace Belle2::TangoPalette;

namespace {
  /** Destroys 'el', zeroes pointer. Supposed to also remove el from global lists, which 'delete el' would not. */
  template <class T> void destroyEveElement(T*& el)
  {
    if (!el) return;
    if (el->GetDenyDestroy() > 1)
      B2WARNING("destroyEveElement(): Element " << el->GetName() << " has unexpected refcount " << el->GetDenyDestroy());

    el->DecDenyDestroy(); //also deletes el when refcount <= 0
    el = nullptr;
  }

  /** TEveGeoShape contains a TGeoShape and feels responsible for deletion; but TGeoShape is owned by global TGeoManager.
   *
   * What we want is deletion by TEve (after changing events), so we explictly remove the object
   * from TGeoManager's garbage collection list. (some objects might not be in the list because these
   * things are horribly inconsistent. doesn't hurt to try though.)
   */
  void fixGeoShapeRefCount(TEveGeoShape* eveshape)
  {
    TGeoShape* s = eveshape->GetShape();
    //Remove(obj) usually takes about 20us, but since usually s is at the end, we can shorten this
    if (gGeoManager->GetListOfShapes()->Last() == s)
      gGeoManager->GetListOfShapes()->RemoveAt(gGeoManager->GetListOfShapes()->GetLast());
    else
      gGeoManager->GetListOfShapes()->Remove(s);
  }
}

const int EVEVisualization::c_recoHitColor = getTColorID("Orange", 1);
const int EVEVisualization::c_unassignedHitColor = getTColorID("Plum", 1);
const int EVEVisualization::c_trackColor = getTColorID("Sky Blue", 2);
const int EVEVisualization::c_recoTrackColor = getTColorID("Sky Blue", 1);
const int EVEVisualization::c_trackMarkerColor = getTColorID("Chameleon", 3);
const int EVEVisualization::c_klmClusterColor = getTColorID("Chameleon", 1);

EVEVisualization::EVEVisualization():
  m_assignToPrimaries(false),
  m_eclData(0),
  m_bfield(new EveVisBField())
{
  setErrScale();

  TGLLogicalShape::SetIgnoreSizeForCameraInterest(kTRUE);     // Allows the visualization of the "small" error ellipsoid.

  //create new containers
  m_trackpropagator = new TEveTrackPropagator();
  m_trackpropagator->IncDenyDestroy();
  m_trackpropagator->SetMagFieldObj(m_bfield, false);
  m_trackpropagator->SetFitDaughters(false); //most secondaries are no longer immediate daughters since we might discard those!
  m_trackpropagator->SetMaxR(EveGeometry::getMaxR()); //don't draw tracks outside detector
  //TODO is this actually needed?
  m_trackpropagator->SetMaxStep(1.0); //make sure to reeval magnetic field often enough

  m_tracklist = new TEveTrackList(m_trackpropagator);
  m_tracklist->IncDenyDestroy();
  m_tracklist->SetName("MCParticles");
  m_tracklist->SelectByP(c_minPCut, FLT_MAX); //don't show too many particles by default...

  m_gftrackpropagator = new TEveTrackPropagator();
  m_gftrackpropagator->IncDenyDestroy();
  m_gftrackpropagator->SetMagFieldObj(m_bfield, false);
  m_gftrackpropagator->SetMaxOrbs(0.5); //stop after track markers

  m_consttrackpropagator = new TEveTrackPropagator();
  m_consttrackpropagator->IncDenyDestroy();
  m_consttrackpropagator->SetMagField(0, 0, -1.5);
  m_consttrackpropagator->SetMaxR(EveGeometry::getMaxR());

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
  if (!gEve)
    return; //objects are probably already freed by Eve

  //Eve objects
  destroyEveElement(m_eclData);
  destroyEveElement(m_unassignedRecoHits);
  destroyEveElement(m_tracklist);
  destroyEveElement(m_trackpropagator);
  destroyEveElement(m_gftrackpropagator);
  destroyEveElement(m_consttrackpropagator);
  destroyEveElement(m_calo3d);
  delete m_bfield;
}

void EVEVisualization::addTrackCandidate(const std::string& collectionName,
                                         const RecoTrack& recoTrack)
{
  const TString label = ObjectInfo::getIdentifier(&recoTrack);
  // parse the option string ------------------------------------------------------------------------
  bool drawHits = false;

  if (m_options != "") {
    for (size_t i = 0; i < m_options.length(); i++) {
      if (m_options.at(i) == 'H') drawHits = true;
    }
  }
  // finished parsing the option string -------------------------------------------------------------


  //track seeds
  TVector3 track_pos = recoTrack.getPositionSeed();
  TVector3 track_mom = recoTrack.getMomentumSeed();

  TEveStraightLineSet* lines = new TEveStraightLineSet("RecoHits for " + label);
  lines->SetMainColor(c_recoTrackColor);
  lines->SetMarkerColor(c_recoTrackColor);
  lines->SetMarkerStyle(6);
  lines->SetMainTransparency(60);

  if (drawHits) {
    // Loop over all hits in the track (three different types)
    for (const RecoHitInformation::UsedPXDHit* pxdHit : recoTrack.getPXDHitList()) {
      addRecoHit(pxdHit, lines);
    }

    for (const RecoHitInformation::UsedSVDHit* svdHit : recoTrack.getSVDHitList()) {
      addRecoHit(svdHit, lines);
    }

    for (const RecoHitInformation::UsedCDCHit* cdcHit : recoTrack.getCDCHitList()) {
      addRecoHit(cdcHit, lines);
    }
  }

  TEveRecTrack rectrack;
  rectrack.fP.Set(track_mom);
  rectrack.fV.Set(track_pos);

  TEveTrack* track_lines = new TEveTrack(&rectrack, m_gftrackpropagator);
  track_lines->SetName(label); //popup label set at end of function
  track_lines->SetPropagator(m_gftrackpropagator);
  track_lines->SetLineColor(c_recoTrackColor);
  track_lines->SetLineWidth(1);
  track_lines->SetTitle(ObjectInfo::getTitle(&recoTrack));

  track_lines->SetCharge((int)recoTrack.getChargeSeed());


  track_lines->AddElement(lines);
  addToGroup(collectionName, track_lines);
  addObject(&recoTrack, track_lines);
}

void EVEVisualization::addCDCTriggerTrack(const std::string& collectionName,
                                          const CDCTriggerTrack& trgTrack)
{
  const TString label = ObjectInfo::getIdentifier(&trgTrack);

  TVector3 track_pos = TVector3(0, 0, trgTrack.getZ0());
  TVector3 track_mom = (trgTrack.getChargeSign() == 0) ?
                       trgTrack.getDirection() * 1000 :
                       trgTrack.getMomentum(1.5);

  TEveRecTrack rectrack;
  rectrack.fP.Set(track_mom);
  rectrack.fV.Set(track_pos);

  TEveTrack* track_lines = new TEveTrack(&rectrack, m_consttrackpropagator);
  track_lines->SetName(label); //popup label set at end of function
  track_lines->SetPropagator(m_consttrackpropagator);
  track_lines->SetLineColor(kOrange + 2);
  track_lines->SetLineWidth(1);
  track_lines->SetTitle(ObjectInfo::getTitle(&trgTrack) +
                        TString::Format("\ncharge: %d, phi: %.2fdeg, pt: %.2fGeV, theta: %.2fdeg, z: %.2fcm",
                                        trgTrack.getChargeSign(),
                                        trgTrack.getPhi0() * 180 / M_PI,
                                        trgTrack.getTransverseMomentum(1.5),
                                        trgTrack.getDirection().Theta() * 180 / M_PI,
                                        trgTrack.getZ0()));


  track_lines->SetCharge(trgTrack.getChargeSign());

  // show 2D tracks with dashed lines
  if (trgTrack.getZ0() == 0 && trgTrack.getCotTheta() == 0)
    track_lines->SetLineStyle(2);

  addToGroup(collectionName, track_lines);
  addObject(&trgTrack, track_lines);
}

void EVEVisualization::addTrack(const Belle2::Track* belle2Track)
{
  // load the pion fit hypothesis or the hypothesis which is the closest in mass to a pion
  // the tracking will not always successfully fit with a pion hypothesis
  const TrackFitResult* fitResult = belle2Track->getTrackFitResultWithClosestMass(Const::pion);
  if (!fitResult) {
    B2ERROR("Track without TrackFitResult skipped.");
    return;
  }
  const RecoTrack* track = belle2Track->getRelated<RecoTrack>();

  const TString label = ObjectInfo::getIdentifier(belle2Track) + " (" + ObjectInfo::getIdentifier(fitResult) + ")";

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

  // We loop over all points (scattering non-measurement points for GBL)
  // and for Kalman we skip those with no measurements, which should
  // not be there
  bool isPruned = (track == nullptr);


  TEveRecTrackD recTrack;
  const TVector3& poca = fitResult->getPosition();
  recTrack.fV.Set(poca);

  const TVector3& poca_momentum = fitResult->getMomentum();
  if (std::isfinite(poca_momentum.Mag()))
    recTrack.fP.Set(poca_momentum);
  else //use 1TeV momentum for tracks without curvature
    recTrack.fP.Set(fitResult->getHelix().getDirection() * 1000);

  recTrack.fSign = fitResult->getChargeSign();
  TEveTrack* eveTrack = new TEveTrack(&recTrack, m_gftrackpropagator);
  eveTrack->SetName(label);


  if (track) {
    const genfit::AbsTrackRep* representation;

    if (m_drawCardinalRep) {
      representation = track->getCardinalRepresentation();
      B2DEBUG(100, "Draw cardinal rep");
    } else {
      const auto& representations = track->getRepresentations();
      if (representations.empty()) {
        B2ERROR("No representations found in the reco track!");
        return;
      }
      B2DEBUG(100, "Draw representation number 0.");
      representation = representations.front();
    }

    const genfit::FitStatus* fitStatus = track->getTrackFitStatus(representation);

    isPruned = fitStatus->isTrackPruned();

    // GBL and Kalman cannot mix in a track.
    // What is 0 after first loop will stay 0:
    genfit::KalmanFitterInfo* fi = 0;
    genfit::KalmanFitterInfo* prevFi = 0;
    genfit::GblFitterInfo* gfi = 0;
    genfit::GblFitterInfo* prevGFi = 0;
    const genfit::MeasuredStateOnPlane* fittedState(NULL);
    const genfit::MeasuredStateOnPlane* prevFittedState(NULL);


    const auto& hitPoints = track->getHitPointsWithMeasurement();
    const unsigned int numpoints = hitPoints.size();

    int hitCounter = -1;
    for (genfit::TrackPoint* tp : hitPoints) { // loop over all points in the track
      hitCounter++;

      // get the fitter infos ------------------------------------------------------------------
      if (! tp->hasFitterInfo(representation)) {
        B2ERROR("trackPoint has no fitterInfo for rep");
        continue;
      }

      genfit::AbsFitterInfo* fitterInfo = tp->getFitterInfo(representation);

      fi = dynamic_cast<genfit::KalmanFitterInfo*>(fitterInfo);
      gfi = dynamic_cast<genfit::GblFitterInfo*>(fitterInfo);

      if (!gfi && !fi) {
        B2ERROR("Can only display KalmanFitterInfo or GblFitterInfo");
        continue;
      }

      if (gfi && fi)
        B2FATAL("AbsFitterInfo dynamic-casted to both KalmanFitterInfo and GblFitterInfo!");


      if (fi && ! tp->hasRawMeasurements()) {
        B2ERROR("trackPoint has no raw measurements");
        continue;
      }

      if (fi && ! fi->hasPredictionsAndUpdates()) {
        B2ERROR("KalmanFitterInfo does not have all predictions and updates");
        continue;
      }

      try {
        if (gfi)
          fittedState = &(gfi->getFittedState(true));
        if (fi)
          fittedState = &(fi->getFittedState(true));
      } catch (genfit::Exception& e) {
        B2ERROR(e.what() << " - can not get fitted state");
        continue;
      }

      TVector3 track_pos = representation->getPos(*fittedState);

      // draw track if corresponding option is set ------------------------------------------
      if (prevFittedState != NULL) {

        TEvePathMark::EType_e markType = TEvePathMark::kReference;
        if (hitCounter + 1 == static_cast<int>(numpoints)) //track should stop here.
          markType = TEvePathMark::kDecay;

        // Kalman: non-null prevFi ensures that the previous fitter info was also KalmanFitterInfo
        if (fi && prevFi) {
          makeLines(eveTrack, prevFittedState, fittedState, representation, markType, m_drawErrors);
          if (m_drawErrors) { // make sure to draw errors in both directions
            makeLines(eveTrack, prevFittedState, fittedState, representation, markType, m_drawErrors, 0);
          }
          //these are currently disabled.
          //TODO: if activated, I want to have a separate TEveStraightLineSet instead of eveTrack (different colors/options)
          if (m_drawForward)
            makeLines(eveTrack, prevFi->getForwardUpdate(), fi->getForwardPrediction(), representation, markType, m_drawErrors, 0);
          if (m_drawBackward)
            makeLines(eveTrack, prevFi->getBackwardPrediction(), fi->getBackwardUpdate(), representation, markType, m_drawErrors);
          // draw reference track if corresponding option is set ------------------------------------------
          if (m_drawRefTrack && fi->hasReferenceState() && prevFi->hasReferenceState())
            makeLines(eveTrack, prevFi->getReferenceState(), fi->getReferenceState(), representation, markType, false);
        }

        // GBL: non-null prevGFi ensures that the previous fitter info was also GblFitterInfo
        if (gfi && prevGFi) {
          makeLines(eveTrack, prevFittedState, fittedState, representation, markType, m_drawErrors);
          if (m_drawErrors) {
            makeLines(eveTrack, prevFittedState, fittedState, representation, markType, m_drawErrors, 0);
          }

          if (m_drawRefTrack && gfi->hasReferenceState() && prevGFi->hasReferenceState()) {
            genfit::StateOnPlane prevSop = prevGFi->getReferenceState();
            genfit::StateOnPlane sop = gfi->getReferenceState();
            makeLines(eveTrack, &prevSop, &sop, representation, markType, false);
          }
        }

      }
      prevFi = fi; // Kalman
      prevGFi = gfi; // GBL
      prevFittedState = fittedState;


      //loop over all measurements for this point (e.g. u and v-type strips)
      const int numMeasurements = tp->getNumRawMeasurements();
      for (int iMeasurement = 0; iMeasurement < numMeasurements; iMeasurement++) {
        const genfit::AbsMeasurement* m = tp->getRawMeasurement(iMeasurement);

        TVectorT<double> hit_coords;
        TMatrixTSym<double> hit_cov;

        if (fi) {
          // Kalman
          genfit::MeasurementOnPlane* mop = fi->getMeasurementOnPlane(iMeasurement);
          hit_coords.ResizeTo(mop->getState());
          hit_cov.ResizeTo(mop->getCov());
          hit_coords = mop->getState();
          hit_cov = mop->getCov();
        }
        if (gfi) {
          // GBL - has only one measurement (other should be already merged before the track is constructed)
          // That means tp->getNumRawMeasurements() returns 1 for tracks fitted by GBL, because GBLfit Module
          // merges all corresponding SVD strips to 2D combined clusters.
          genfit::MeasurementOnPlane gblMeas = gfi->getMeasurement();
          hit_coords.ResizeTo(gblMeas.getState());
          hit_cov.ResizeTo(gblMeas.getCov());
          hit_coords = gblMeas.getState();
          hit_cov = gblMeas.getCov();
        }

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

        if (dynamic_cast<const genfit::PlanarMeasurement*>(m) != NULL) {
          planar_hit = true;
          if (hit_coords_dim == 1) {
            hit_u = hit_coords(0);
          } else if (hit_coords_dim == 2) {
            planar_pixel_hit = true;
            hit_u = hit_coords(0);
            hit_v = hit_coords(1);
          }
        } else if (dynamic_cast<const genfit::SpacepointMeasurement*>(m) != NULL) {
          space_hit = true;
          plane_size = 4;
        } else if (dynamic_cast<const CDCRecoHit*>(m)
                   || dynamic_cast<const genfit::WireMeasurement*>(m)
                   || dynamic_cast<const genfit::WireMeasurementNew*>(m)) {
          wire_hit = true;
          hit_u = fabs(hit_coords(0));
          hit_v = v * (track_pos - o); // move the covariance tube so that the track goes through it
          plane_size = 4;
          if (dynamic_cast<const genfit::WirePointMeasurement*>(m) != NULL) {
            wirepoint_hit = true;
            hit_v = hit_coords(1);
          }
        } else {
          B2ERROR("Hit " << hitCounter << ", Measurement " << iMeasurement << ": Unknown measurement type: skipping hit!");
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
            det_shape->SetShape(new TGeoTube(std::max(0., (double)(hit_u - 0.0105 / 2.)), hit_u + 0.0105 / 2., plane_size));
            fixGeoShapeRefCount(det_shape);

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
              //pixel hit
              // calculate eigenvalues to draw error-ellipse ----------------------------
              TMatrixDSymEigen eigen_values(hit_cov);
              TEveGeoShape* cov_shape = new TEveGeoShape("PXDRecoHit");
              const TVectorD& ev = eigen_values.GetEigenValues();
              const TMatrixD& eVec = eigen_values.GetEigenVectors();
              double pseudo_res_0 = m_errorScale * std::sqrt(ev(0));
              double pseudo_res_1 = m_errorScale * std::sqrt(ev(1));
              // finished calcluating, got the values -----------------------------------

              // calculate the semiaxis of the error ellipse ----------------------------
              cov_shape->SetShape(new TGeoEltu(pseudo_res_0, pseudo_res_1, 0.0105));
              fixGeoShapeRefCount(cov_shape);
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
            TMatrixDSymEigen eigen_values(m->getRawHitCov());
            TEveGeoShape* cov_shape = new TEveGeoShape("SpacePoint Hit");
            cov_shape->SetShape(new TGeoSphere(0., 1.));
            fixGeoShapeRefCount(cov_shape);
            const TVectorD& ev = eigen_values.GetEigenValues();
            const TMatrixD& eVec = eigen_values.GetEigenVectors();
            TVector3 eVec1(eVec(0, 0), eVec(1, 0), eVec(2, 0));
            TVector3 eVec2(eVec(0, 1), eVec(1, 1), eVec(2, 1));
            TVector3 eVec3(eVec(0, 2), eVec(1, 2), eVec(2, 2));
            TVector3 norm = u.Cross(v);
            // got everything we need -----------------------------------------------------


            TGeoRotation det_rot("det_rot", (eVec1.Theta() * 180) / TMath::Pi(), (eVec1.Phi() * 180) / TMath::Pi(),
                                 (eVec2.Theta() * 180) / TMath::Pi(), (eVec2.Phi() * 180) / TMath::Pi(),
                                 (eVec3.Theta() * 180) / TMath::Pi(), (eVec3.Phi() * 180) / TMath::Pi()); // the rotation is already clear

            // set the scaled eigenvalues -------------------------------------------------
            double pseudo_res_0 = m_errorScale * std::sqrt(ev(0));
            double pseudo_res_1 = m_errorScale * std::sqrt(ev(1));
            double pseudo_res_2 = m_errorScale * std::sqrt(ev(2));
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
            const double cdcErrorScale = 1.0;
            TEveGeoShape* cov_shape = new TEveGeoShape("CDCRecoHit");
            double pseudo_res_0 = cdcErrorScale * std::sqrt(hit_cov(0, 0));
            double pseudo_res_1 = plane_size;
            if (wirepoint_hit) pseudo_res_1 = cdcErrorScale * std::sqrt(hit_cov(1, 1));

            cov_shape->SetShape(new TGeoTube(std::max(0., (double)(hit_u - pseudo_res_0)), hit_u + pseudo_res_0, pseudo_res_1));
            fixGeoShapeRefCount(cov_shape);
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

    auto& firstref = eveTrack->RefPathMarks().front();
    auto& lastref = eveTrack->RefPathMarks().back();
    double f = firstref.fV.Distance(recTrack.fV);
    double b = lastref.fV.Distance(recTrack.fV);
    if (f > 100 and f > b) {
      B2WARNING("Decay vertex is much closer to POCA than first vertex, reversing order of track points... (this is intended for cosmic tracks, if you see this message in other context it might indicate a problem)");
      //last ref is better than first...
      lastref.fType = TEvePathMarkD::kReference;
      firstref.fType = TEvePathMarkD::kDecay;
      std::reverse(eveTrack->RefPathMarks().begin(), eveTrack->RefPathMarks().end());
    }
  }
  eveTrack->SetTitle(TString::Format("%s\n"
                                     "pruned: %s\n"
                                     "pT=%.3f, pZ=%.3f\n"
                                     "pVal: %e",
                                     label.Data(),
                                     isPruned ? " yes" : "no",
                                     poca_momentum.Pt(), poca_momentum.Pz(),
                                     fitResult->getPValue()
                                    ));
  eveTrack->SetLineColor(c_trackColor);
  eveTrack->SetLineStyle(1);
  eveTrack->SetLineWidth(3.0);


  addToGroup("Fitted Tracks", eveTrack);
  if (track)
    addObject(track, eveTrack);
  addObject(belle2Track, eveTrack);
}

TEveBox* EVEVisualization::boxCreator(const TVector3& o, TVector3 u, TVector3 v, float ud, float vd, float depth)
{
  //force minimum width of polygon to deal with Eve limits
  float min = 0.04;
  if (vd < min)
    vd = min;
  if (ud < min)
    ud = min;
  if (depth < min)
    depth = min;

  TEveBox* box = new TEveBox;
  box->SetPickable(true);

  TVector3 norm = u.Cross(v);
  u *= (0.5 * ud);
  v *= (0.5 * vd);
  norm *= (0.5 * depth);


  for (int k = 0; k < 8; ++k) {
    // Coordinates for all eight corners of the box
    // as two parallel rectangles, with vertices specified in clockwise direction
    int signU = ((k + 1) & 2) ? -1 : 1;
    int signV = (k & 4) ? -1 : 1;
    int signN = (k & 2) ? -1 : 1;
    float vertex[3];
    for (int i = 0; i < 3; ++i) {
      vertex[i] = o(i) + signU * u(i) + signV * v(i) + signN * norm(i);
    }
    box->SetVertex(k, vertex);
  }

  return box;
}

void EVEVisualization::makeLines(TEveTrack* eveTrack, const genfit::StateOnPlane* prevState, const genfit::StateOnPlane* state,
                                 const genfit::AbsTrackRep* rep,
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
      TMatrixDSymEigen eigen_values(cov.GetSub(0, 2, 0, 2));
      const TVectorD& ev = eigen_values.GetEigenValues();
      const TMatrixD& eVec = eigen_values.GetEigenVectors();
      TVector3 eVec1, eVec2;
      // limit
      static const double maxErr = 1000.;
      double ev0 = std::min(ev(0), maxErr);
      double ev1 = std::min(ev(1), maxErr);
      double ev2 = std::min(ev(2), maxErr);

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

      TVector3 oldEVec1(eVec1);

      const int nEdges = 24;
      std::vector<TVector3> vertices;

      vertices.push_back(position);

      // vertices at plane
      for (int i = 0; i < nEdges; ++i) {
        const double angle = 2 * TMath::Pi() / nEdges * i;
        vertices.push_back(position + cos(angle)*eVec1 + sin(angle)*eVec2);
      }



      SharedPlanePtr newPlane(new DetPlane(*(measuredState->getPlane())));
      newPlane->setO(position + eval);

      MeasuredStateOnPlane stateCopy(*measuredState);
      try {
        rep->extrapolateToPlane(stateCopy, newPlane);
      } catch (Exception& e) {
        B2ERROR(e.what());
        return;
      }

      // get cov at 2nd plane
      rep->getPosMomCov(stateCopy, position, direction, cov);

      // get eigenvalues & -vectors
      {
        TMatrixDSymEigen eigen_values2(cov.GetSub(0, 2, 0, 2));
        const TVectorD& eVal = eigen_values2.GetEigenValues();
        const TMatrixD& eVect = eigen_values2.GetEigenVectors();
        // limit
        ev0 = std::min(eVal(0), maxErr);
        ev1 = std::min(eVal(1), maxErr);
        ev2 = std::min(eVal(2), maxErr);

        // get two largest eigenvalues/-vectors
        if (ev0 < ev1 && ev0 < ev2) {
          eVec1.SetXYZ(eVect(0, 1), eVect(1, 1), eVect(2, 1));
          eVec1 *= sqrt(ev1);
          eVec2.SetXYZ(eVect(0, 2), eVect(1, 2), eVect(2, 2));
          eVec2 *= sqrt(ev2);
        } else if (ev1 < ev0 && ev1 < ev2) {
          eVec1.SetXYZ(eVect(0, 0), eVect(1, 0), eVect(2, 0));
          eVec1 *= sqrt(ev0);
          eVec2.SetXYZ(eVect(0, 2), eVect(1, 2), eVect(2, 2));
          eVec2 *= sqrt(ev2);
        } else {
          eVec1.SetXYZ(eVect(0, 0), eVect(1, 0), eVect(2, 0));
          eVec1 *= sqrt(ev0);
          eVec2.SetXYZ(eVect(0, 1), eVect(1, 1), eVect(2, 1));
        } eVec2 *= sqrt(ev1);
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
    if (!m_mcparticleTracks[nullptr].simhits) {
      const TString pointsTitle("Unassigned SimHits");
      m_mcparticleTracks[nullptr].simhits = new TEvePointSet(pointsTitle);
      m_mcparticleTracks[nullptr].simhits->SetTitle(pointsTitle);
      m_mcparticleTracks[nullptr].simhits->SetMarkerStyle(6);
      m_mcparticleTracks[nullptr].simhits->SetMainColor(c_unassignedHitColor);
      //m_mcparticleTracks[nullptr].simhits->SetMainTransparency(50);
      m_mcparticleTracks[nullptr].track = NULL;
    }
    return &m_mcparticleTracks[nullptr];
  }

  if (m_hideSecondaries and !particle->hasStatus(MCParticle::c_PrimaryParticle)) {
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
      for (const MCTrajectoryPoint& pt : trajectory) {
        m_mcparticleTracks[particle].track->AddPathMark(
          TEvePathMark(
            //Add the last trajectory point as decay point to prevent TEve to
            //propagate beyond the end of the track. So lets compare the adress
            //to the address of last point and choose the pathmark accordingly
            (&pt == &trajectory.back()) ? TEvePathMark::kDecay : TEvePathMark::kReference,
            TEveVector(pt.x, pt.y, pt.z),
            TEveVector(pt.px, pt.py, pt.pz)
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
    const MCParticle* mom = particle->getMother();
    if (mom) {
      m_mcparticleTracks[particle].parentParticle = mom;
      addMCParticle(mom);
    }

    TString title = ObjectInfo::getTitle(particle);
    if (!hasTrajectory) {
      //Hijack the mother label to show that the track position is only
      //extrapolated, not known from simulation
      title += "\n(track estimated from initial momentum)";
      //Also, show those tracks with dashed lines
      m_mcparticleTracks[particle].track->SetLineStyle(2);
    }

    m_mcparticleTracks[particle].track->SetTitle(title);

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
    const TString pointsTitle = "SimHits for " + ObjectInfo::getIdentifier(particle) + " - " + particle_name;
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
  for (auto& mcTrackPair : m_mcparticleTracks) {
    MCTrack& mcTrack = mcTrackPair.second;
    if (mcTrack.track) {
      if (mcTrack.simhits->Size() > 0) {
        mcTrack.track->AddElement(mcTrack.simhits);
      } else {
        //if we don't add it, remove empty collection
        destroyEveElement(mcTrack.simhits);
      }

      TEveElement* parent = m_tracklist;
      if (mcTrack.parentParticle) {
        const auto& parentIt = m_mcparticleTracks.find(mcTrack.parentParticle);
        if (parentIt != m_mcparticleTracks.end()) {
          parent = parentIt->second.track;
        }
      }
      parent->AddElement(mcTrack.track);
    } else { //add simhits directly
      gEve->AddElement(mcTrack.simhits);
    }
  }
  gEve->AddElement(m_tracklist);
  m_tracklist->MakeTracks();
  m_tracklist->SelectByP(c_minPCut, FLT_MAX); //don't show too many particles by default...

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

  m_consttrackpropagator->SetMagField(0, 0, -1.5);

  m_eclData->DataChanged(); //update limits (Empty() won't work otherwise)
  if (!m_eclData->Empty()) {
    m_eclData->SetAxisFromBins(0.0,
                               0.0); //epsilon_x/y = 0 so we don't merge neighboring bins. This avoids some rendering issues with projections of small clusters.
    m_calo3d->SetData(m_eclData);
  }
  gEve->AddElement(m_calo3d);

  if (m_unassignedRecoHits) {
    m_unassignedRecoHits->SetRnrState(m_unassignedRecoHitsVisibility);
    gEve->AddElement(m_unassignedRecoHits);
  }

}

void EVEVisualization::clearEvent()
{
  if (!gEve)
    return;

  VisualRepMap::getInstance()->clear();
  for (auto& groupPair : m_groups) {
    //store visibility, invalidate pointers
    if (groupPair.second.group)
      groupPair.second.visible = groupPair.second.group->GetRnrState();
    groupPair.second.group = nullptr;
  }

  m_mcparticleTracks.clear();
  m_shownRecohits.clear();
  m_tracklist->DestroyElements();

  //remove ECL data from event
  m_calo3d->SetData(NULL);
  m_calo3d->DestroyElements();

  //lower energy threshold for ECL
  float ecl_threshold = 0.01;
  if (m_eclData)
    ecl_threshold = m_eclData->GetSliceThreshold(0);

  destroyEveElement(m_eclData);
  m_eclData = new TEveCaloDataVec(1); //#slices
  m_eclData->IncDenyDestroy();
  m_eclData->RefSliceInfo(0).Setup("ECL", ecl_threshold, kRed);

  if (m_unassignedRecoHits)
    m_unassignedRecoHitsVisibility = m_unassignedRecoHits->GetRnrState();
  destroyEveElement(m_unassignedRecoHits);

  gEve->GetSelection()->RemoveElements();
  gEve->GetHighlight()->RemoveElements();
  //other things are cleaned up by TEve...
}




void EVEVisualization::addVertex(const genfit::GFRaveVertex* vertex)
{
  TVector3 v = vertex->getPos();
  TEvePointSet* vertexPoint = new TEvePointSet(ObjectInfo::getInfo(vertex));
  //sadly, setting a title for a TEveGeoShape doesn't result in a popup...
  vertexPoint->SetTitle(ObjectInfo::getTitle(vertex));
  vertexPoint->SetMainColor(c_recoHitColor);
  vertexPoint->SetNextPoint(v.x(), v.y(), v.z());

  TMatrixDSymEigen eigen_values(vertex->getCov());
  TEveGeoShape* det_shape = new TEveGeoShape(ObjectInfo::getInfo(vertex) + " Error");
  det_shape->SetShape(new TGeoSphere(0., 1.));   //Initially created as a sphere, then "scaled" into an ellipsoid.
  fixGeoShapeRefCount(det_shape);
  const TVectorD& ev = eigen_values.GetEigenValues(); //Assigns the eigenvalues into the "ev" matrix.
  const TMatrixD& eVec = eigen_values.GetEigenVectors();  //Assigns the eigenvalues into the "eVec" matrix.
  TVector3 eVec1(eVec(0, 0), eVec(1, 0), eVec(2,
                                              0));   //Define the 3 eigenvectors of the covariance matrix as objects of the TVector3 class using constructor.
  TVector3 eVec2(eVec(0, 1), eVec(1, 1), eVec(2,
                                              1));   //eVec(i,j) uses the method/overloaded operator ( . ) of the TMatrixT class to return the matrix entry.
  TVector3 eVec3(eVec(0, 2), eVec(1, 2), eVec(2, 2));
  // got everything we need -----------------------------------------------------   //Eigenvalues(semi axis) of the covariance matrix accquired!


  TGeoRotation det_rot("det_rot", (eVec1.Theta() * 180) / TMath::Pi(), (eVec1.Phi() * 180) / TMath::Pi(),
                       (eVec2.Theta() * 180) / TMath::Pi(), (eVec2.Phi() * 180) / TMath::Pi(),
                       (eVec3.Theta() * 180) / TMath::Pi(), (eVec3.Phi() * 180) / TMath::Pi()); // the rotation is already clear

  // set the scaled eigenvalues -------------------------------------------------
  //"Scaled" eigenvalues pseudo_res (lengths of the semi axis) are the sqrt of the eigenvalues.
  double pseudo_res_0 = std::sqrt(ev(0));
  double pseudo_res_1 = std::sqrt(ev(1));
  double pseudo_res_2 = std::sqrt(ev(2));

  //B2INFO("The pseudo_res_0/1/2 are " << pseudo_res_0 << "," << pseudo_res_1 << "," << pseudo_res_2); //shows the scaled eigenvalues



  // rotate and translate -------------------------------------------------------
  TGeoGenTrans det_trans(v(0), v(1), v(2), pseudo_res_0, pseudo_res_1, pseudo_res_2,
                         &det_rot); //Puts the ellipsoid at the position of the vertex, v(0)=v.x(), operator () overloaded.
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
  float dPhi = cluster->getUncertaintyPhi();
  float dTheta = cluster->getUncertaintyTheta();
  if (dPhi >= M_PI / 4 or dTheta >= M_PI / 4 or cluster->getUncertaintyEnergy() == 1.0) {
    B2WARNING("Found ECL cluster with broken errors (unit matrix or too large). Using 0.05 as error in phi/theta. The 3x3 error matrix previously was:");
    cluster->getCovarianceMatrix3x3().Print();
    dPhi = dTheta = 0.05;
  }

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

  int id = m_eclData->AddTower(etaLow, etaHigh, phi - dPhi, phi + dPhi);
  m_eclData->FillSlice(0, cluster->getEnergy());
  VisualRepMap::getInstance()->addCluster(cluster, m_eclData, id);
}

void EVEVisualization::addKLMCluster(const KLMCluster* cluster)
{
  const double layerThicknessCm = 3.16; //TDR, Fig 10.2
  const double layerDistanceCm = 9.1 - layerThicknessCm;

  TVector3 startPos = cluster->getClusterPosition(); //position of first RPC plane
  TVector3 dir; //direction of cluster stack, Mag() == distance between planes
  TVector3 a, b; //defines RPC plane
  bool isBarrel = (startPos.Z() > -175.0 and startPos.Z() < 270.0);
  if (isBarrel) {
    //barrel
    b = TVector3(0, 0, 1);
    a = startPos.Cross(b).Unit();
    double c = M_PI / 4.0;
    double offset = c / 2.0 + M_PI;
    a.SetPhi(int((a.Phi() + offset) / (c))*c - M_PI);
    TVector3 perp = b.Cross(a);

    const double barrelRadiusCm = 204.0;
    startPos.SetMag(barrelRadiusCm / perp.Dot(startPos.Unit()));

    dir = startPos.Unit();
    dir.SetMag((layerDistanceCm + layerThicknessCm) / perp.Dot(dir));
  } else {
    //endcap
    b = TVector3(startPos.x(), startPos.y(), 0).Unit();
    a = startPos.Cross(b).Unit();
    double endcapStartZ = 284;
    if (startPos.z() < 0)
      endcapStartZ = -189.5;

    double scaleFac = endcapStartZ / startPos.z();
    startPos.SetMag(startPos.Mag() * scaleFac);

    dir = startPos.Unit();
    dir.SetMag((layerDistanceCm + layerThicknessCm) / fabs(dir.z()));
  }

  for (int i = 0; i < cluster->getLayers(); i++) {
    TVector3 layerPos = startPos;
    layerPos += (cluster->getInnermostLayer() + i) * dir;
    auto* layer = boxCreator(layerPos, a, b, 20.0, 20.0, layerThicknessCm / 2);
    layer->SetMainColor(c_klmClusterColor);
    layer->SetMainTransparency(70);
    layer->SetName(ObjectInfo::getIdentifier(cluster));
    layer->SetTitle(ObjectInfo::getTitle(cluster));

    addToGroup(std::string("KLMClusters/") + ObjectInfo::getIdentifier(cluster).Data(), layer);
    addObject(cluster, layer);
  }
}

void EVEVisualization::addBKLMHit2d(const BKLMHit2d* bklm2dhit)
{
  //TVector3 globalPosition=  bklm2dhit->getGlobalPosition();
  bklm::GeometryPar*  m_GeoPar = Belle2::bklm::GeometryPar::instance();
  const bklm::Module* module = m_GeoPar->findModule(bklm2dhit->isForward(), bklm2dhit->getSector(), bklm2dhit->getLayer());

  CLHEP::Hep3Vector global;
  //+++ global coordinates of the hit
  global[0] = bklm2dhit->getGlobalPosition()[0];
  global[1] = bklm2dhit->getGlobalPosition()[1];
  global[2] = bklm2dhit->getGlobalPosition()[2];

  //+++ local coordinates of the hit
  CLHEP::Hep3Vector local = module->globalToLocal(global);
  //double localU = local[1]; //phi
  //double localV = local[2]; //z
  int Nphistrip = bklm2dhit->getPhiStripMax() - bklm2dhit->getPhiStripMin() + 1;
  int Nztrip = bklm2dhit->getZStripMax() - bklm2dhit->getZStripMin() + 1;
  double du = module->getPhiStripWidth() * Nphistrip;
  double dv = module->getZStripWidth() * Nztrip;

  //Let's do some simple thing
  CLHEP::Hep3Vector localU(local[0], local[1] + 1.0, local[2]);
  CLHEP::Hep3Vector localV(local[0], local[1], local[2] + 1.0);

  CLHEP::Hep3Vector globalU = module->localToGlobal(localU);
  CLHEP::Hep3Vector globalV = module->localToGlobal(localV);

  TVector3 o(global[0], global[1], global[2]);
  TVector3 u(globalU[0], globalU[1], globalU[2]);
  TVector3 v(globalV[0], globalV[1], globalV[2]);

  //Lest's just assign the depth is 1.0 cm (thickness of a layer), better to update
  TEveBox* bklmbox = boxCreator(o, u - o, v - o, du, dv, 1.0);

  bklmbox->SetMainColor(kGreen);
  //bklmbox->SetName((std::to_string(hitModule)).c_str());
  bklmbox->SetName("BKLMHit2d");

  addToGroup("BKLM2dHits", bklmbox);
  addObject(bklm2dhit, bklmbox);
}

void EVEVisualization::addEKLMHit2d(const EKLMHit2d* eklm2dhit)
{
  const double du = 2.0;
  const double dv = 2.0;
  TVector3 o = eklm2dhit->getPosition();
  TVector3 u(1.0, 0.0, 0.0);
  TVector3 v(0.0, 1.0, 0.0);
  TEveBox* eklmbox = boxCreator(o, u, v, du, dv, 4.0);
  eklmbox->SetMainColor(kGreen);
  eklmbox->SetName("EKLMHit2d");
  addToGroup("EKLM2dHits", eklmbox);
  addObject(eklm2dhit, eklmbox);
}

void EVEVisualization::addROI(const ROIid* roi)
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

  ROIbox->SetName(ObjectInfo::getIdentifier(roi));
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

void EVEVisualization::addCDCHit(const CDCHit* hit, bool showTriggerHits)
{
  static CDC::CDCGeometryPar& cdcgeo = CDC::CDCGeometryPar::Instance();
  const TVector3& wire_pos_f = cdcgeo.wireForwardPosition(WireID(hit->getID()));
  const TVector3& wire_pos_b = cdcgeo.wireBackwardPosition(WireID(hit->getID()));
  static CDC::RealisticTDCCountTranslator tdcTranslator;
  TEveGeoShape* cov_shape = new TEveGeoShape("cov_shape");
  //TODO: leftrightflag not set! (same for other parameters, unsure which ones should be set)
  double driftLength = tdcTranslator.getDriftLength(hit->getTDCCount(), WireID(hit->getID()));
  double driftLengthRes = tdcTranslator.getDriftLengthResolution(driftLength, WireID(hit->getID()));
  driftLengthRes = std::max(driftLengthRes, 0.005);
  const double lengthOfWireSection = 3.0;

  //z in wire direction, x,y orthogonal
  const TVector3 zaxis = wire_pos_b - wire_pos_f;
  const TVector3 xaxis = zaxis.Orthogonal();
  const TVector3 yaxis = xaxis.Cross(zaxis);

  // move to z=0
  const TVector3 midPoint = wire_pos_f - zaxis * (wire_pos_f.z() / zaxis.z());

  cov_shape->SetShape(new TGeoTube(std::max(0., (double)(driftLength - driftLengthRes)), driftLength + driftLengthRes,
                                   lengthOfWireSection));
  fixGeoShapeRefCount(cov_shape);

  TGeoRotation det_rot("det_rot",
                       xaxis.Theta() * 180 / TMath::Pi(), xaxis.Phi() * 180 / TMath::Pi(),
                       yaxis.Theta() * 180 / TMath::Pi(), yaxis.Phi() * 180 / TMath::Pi(),
                       zaxis.Theta() * 180 / TMath::Pi(), zaxis.Phi() * 180 / TMath::Pi()
                      );

  TGeoCombiTrans det_trans(midPoint(0), midPoint(1), midPoint(2), &det_rot);
  cov_shape->SetTransMatrix(det_trans);

  // get relation to trigger track segments
  bool isPartOfTS = false;
  const auto segments = hit->getRelationsFrom<CDCTriggerSegmentHit>();
  if (showTriggerHits && segments.size() > 0) {
    isPartOfTS = true;
  }

  if (hit->getISuperLayer() % 2 == 0) {
    if (isPartOfTS)
      cov_shape->SetMainColor(kCyan + 3);
    else
      cov_shape->SetMainColor(kCyan);
  } else {
    if (isPartOfTS)
      cov_shape->SetMainColor(kPink + 6);
    else
      cov_shape->SetMainColor(kPink + 7);
  }

  cov_shape->SetMainTransparency(50);
  cov_shape->SetName(ObjectInfo::getIdentifier(hit));
  cov_shape->SetTitle(ObjectInfo::getInfo(hit) + TString::Format("\nWire ID: %d\nADC: %d\nTDC: %d",
                      hit->getID(), hit->getADCCount(), hit->getTDCCount()));

  addToGroup("CDCHits", cov_shape);
  addObject(hit, cov_shape);
  if (isPartOfTS) {
    addToGroup("CDCTriggerSegmentHits", cov_shape);
    for (auto rel : segments.relations()) {
      addObject(rel.object, cov_shape);
    }
  }
}

void EVEVisualization::addCDCTriggerSegmentHit(const CDCTriggerSegmentHit* hit)
{
  static CDC::CDCGeometryPar& cdcgeo = CDC::CDCGeometryPar::Instance();
  TEveStraightLineSet* shape = new TEveStraightLineSet();

  // get center wire
  unsigned iL = WireID(hit->getID()).getICLayer();
  if (hit->getPriorityPosition() < 3) iL -= 1;
  unsigned nWires = cdcgeo.nWiresInLayer(iL);
  unsigned iCenter = hit->getIWire();
  if (hit->getPriorityPosition() == 1) iCenter += 1;

  // a track segment consists of 11 wires (15 in SL0) in a special configuration
  // -> get the shift with respect to the center wire (*) for all wires
  // SL 1-8:
  //  _ _ _
  // |_|_|_|
  //  |_|_|
  //   |*|
  //  |_|_|
  // |_|_|_|
  std::vector<int> layershift = { -2, -1, 0, 1, 2};
  std::vector<std::vector<float>> cellshift = {
    { -1, 0, 1},
    { -0.5, 0.5},
    { 0},
    { -0.5, 0.5},
    { -1, 0, 1}
  };
  // SL 0:
  //  _ _ _ _ _
  // |_|_|_|_|_|
  //  |_|_|_|_|
  //   |_|_|_|
  //    |_|_|
  //     |*|
  if (hit->getISuperLayer() == 0) {
    layershift = { 0, 1, 2, 3, 4};
    cellshift = {
      { 0},
      { -0.5, 0.5},
      { -1, 0, 1},
      { -1.5, -0.5, 0.5, 1.5},
      { -2, -1, 0, 1, 2}
    };
  }

  // draw all cells in segment
  for (unsigned il = 0; il < layershift.size(); ++il) {
    for (unsigned ic = 0; ic < cellshift[il].size(); ++ic) {
      TVector3 corners[2][2];
      for (unsigned ir = 0; ir < 2; ++ir) {
        double r = cdcgeo.fieldWireR(iL + layershift[il] - ir);
        double fz = cdcgeo.fieldWireFZ(iL + layershift[il] - ir);
        double bz = cdcgeo.fieldWireBZ(iL + layershift[il] - ir);
        for (unsigned iphi = 0; iphi < 2; ++iphi) {
          double phib = (iCenter + cellshift[il][ic] + iphi - 0.5) * 2 * M_PI / nWires;
          double phif = phib + cdcgeo.nShifts(iL + layershift[il]) * M_PI / nWires;

          TVector3 pos_f = TVector3(cos(phif) * r, sin(phif) * r, fz);
          TVector3 pos_b = TVector3(cos(phib) * r, sin(phib) * r, bz);
          TVector3 zaxis = pos_b - pos_f;
          corners[ir][iphi] = pos_f - zaxis * (pos_f.z() / zaxis.z());
        }
      }

      shape->AddLine(corners[0][0].x(), corners[0][0].y(), 0,
                     corners[0][1].x(), corners[0][1].y(), 0);
      shape->AddLine(corners[0][1].x(), corners[0][1].y(), 0,
                     corners[1][1].x(), corners[1][1].y(), 0);
      shape->AddLine(corners[1][1].x(), corners[1][1].y(), 0,
                     corners[1][0].x(), corners[1][0].y(), 0);
      shape->AddLine(corners[1][0].x(), corners[1][0].y(), 0,
                     corners[0][0].x(), corners[0][0].y(), 0);
    }
  }

  if (hit->getISuperLayer() % 2 == 0) {
    shape->SetMainColor(kCyan + 3);
  } else {
    shape->SetMainColor(kPink + 6);
  }

  shape->SetName(ObjectInfo::getIdentifier(hit));
  shape->SetTitle(ObjectInfo::getTitle(hit) +
                  TString::Format("\nPriority: %d\nLeft/Right: %d",
                                  hit->getPriorityPosition(), hit->getLeftRight()));
  addToGroup("CDCTriggerSegmentHits", shape);
  addObject(hit, shape);
}

void EVEVisualization::addARICHHit(const ARICHHit* hit)
{
  DBObjPtr<ARICHGeometryConfig> arichGeo;

  int hitModule = hit->getModule();
  float fi = arichGeo->getDetectorPlane().getSlotPhi(hitModule);

  TVector3 centerPos3D =  hit->getPosition();

  TVector3 channelX(1, 0, 0);    channelX.RotateZ(fi);
  TVector3 channelY(0, 1, 0);    channelY.RotateZ(fi);

  auto* arichbox = boxCreator(centerPos3D, arichGeo->getMasterVolume().momentumToGlobal(channelX),
                              arichGeo->getMasterVolume().momentumToGlobal(channelY), 0.49, 0.49, 0.05);
  arichbox->SetMainColor(kOrange + 10);
  arichbox->SetName((std::to_string(hitModule)).c_str());

  addToGroup("ARICHHits", arichbox);
  addObject(hit, arichbox);
}

void EVEVisualization::addTOPDigits(const StoreArray<TOPDigit>& digits)
{
  /** TOP module ID -> #digits */
  std::map<int, int> m_topSummary;
  for (const TOPDigit& hit : digits) {
    int mod = hit.getModuleID();
    ++m_topSummary[mod];
  }
  int maxcount = 0;
  for (auto modCountPair : m_topSummary) {
    if (modCountPair.second > maxcount)
      maxcount = modCountPair.second;
  }
  for (auto modCountPair : m_topSummary) {
    const auto& topmod = TOP::TOPGeometryPar::Instance()->getGeometry()->getModule(modCountPair.first);
    double phi = topmod.getPhi();
    double r_center = topmod.getRadius();
    double z = topmod.getZc();

    TVector3 centerPos3D;
    centerPos3D.SetMagThetaPhi(r_center, M_PI / 2, phi);
    centerPos3D.SetZ(z);

    TVector3 channelX(1, 0, 0);    channelX.RotateZ(phi);
    TVector3 channelY(0, 1, 0);    channelY.RotateZ(phi);

    //bar is a bit thicker so we can mouse over without getting the geometry
    auto* moduleBox = boxCreator(centerPos3D, channelX, channelY,
                                 3.0 * topmod.getBarThickness(), topmod.getBarWidth() , topmod.getBarLength());
    moduleBox->SetMainColor(kAzure + 10);
    double weight = double(modCountPair.second) / maxcount;
    moduleBox->SetMainTransparency(90 - weight * 50);
    moduleBox->SetName(("TOP module " + std::to_string(modCountPair.first)).c_str());
    moduleBox->SetTitle(TString::Format("#TOPDigits: %d ", modCountPair.second));

    addToGroup("TOP Modules", moduleBox);
    //associate all TOPDigits with this module.
    for (const TOPDigit& hit : digits) {
      if (modCountPair.first == hit.getModuleID())
        addObject(&hit, moduleBox);
    }
  }
}


void EVEVisualization::showUserData(const DisplayData& displayData)
{
  for (const auto& labelPair : displayData.m_labels) {
    TEveText* text = new TEveText(labelPair.first.c_str());
    text->SetName(labelPair.first.c_str());
    text->SetTitle(labelPair.first.c_str());
    text->SetMainColor(kGray + 1);
    const TVector3& p = labelPair.second;
    text->PtrMainTrans()->SetPos(p.x(), p.y(), p.z());
    addToGroup("DisplayData", text);
  }

  for (const auto& pointPair : displayData.m_pointSets) {
    TEvePointSet* points = new TEvePointSet(pointPair.first.c_str());
    points->SetTitle(pointPair.first.c_str());
    points->SetMarkerStyle(7);
    points->SetMainColor(kGreen);
    for (const TVector3& p : pointPair.second) {
      points->SetNextPoint(p.x(), p.y(), p.z());
    }
    addToGroup("DisplayData", points);
  }

  int randomColor = 2; //primary colours, changing rapidly with index
  for (const auto& arrow : displayData.m_arrows) {
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
  VisualRepMap::getInstance()->add(dataStoreObject, visualRepresentation);
}

void EVEVisualization::addToGroup(const std::string& name, TEveElement* elem)
{
  //slashes at beginning and end are ignored
  const std::string& groupName = boost::algorithm::trim_copy_if(name, boost::algorithm::is_any_of("/"));

  TEveElementList* group = m_groups[groupName].group;
  if (!group) {
    group = new TEveElementList(groupName.c_str(), groupName.c_str());
    group->SetRnrState(m_groups[groupName].visible);
    m_groups[groupName].group = group;

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

