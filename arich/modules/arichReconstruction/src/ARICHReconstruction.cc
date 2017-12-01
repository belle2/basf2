/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "arich/modules/arichReconstruction/ARICHReconstruction.h"
#include "arich/dbobjects/ARICHGeometryConfig.h"
#include "arich/modules/arichReconstruction/Utility.h"
#include "arich/geometry/ARICHBtestGeometryPar.h"
#include "arich/dataobjects/ARICHHit.h"
#include "arich/dataobjects/ARICHTrack.h"
#include "arich/dataobjects/ARICHPhoton.h"

// DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <vector>
#include <TRotation.h>
#include <TRandom3.h>
#include <TFile.h>
#include <TGraph2D.h>

using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace arich;

  ARICHReconstruction::ARICHReconstruction(int storePhot):
    m_arichgp(),
    m_recPars(),
    m_trackPosRes(0),
    m_trackAngRes(0),
    m_nAerogelLayers(0),
    m_storePhot(storePhot)
  {
    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {p_mass[i] = 0;}
    for (unsigned i = 0; i < c_noOfAerogels; i++) {
      m_refractiveInd[i] = 0;
      m_zaero[i] = 0;
      m_thickness[i] = 0;
      m_transmissionLen[i] = 0;
      m_n0[i] = 0;
      m_anorm[i] = TVector3();
    }

  }

  void ARICHReconstruction::initialize()
  {

    for (const auto& part : Const::chargedStableSet) {
      p_mass[part.getIndex()] = part.getMass();
    }

    m_nAerogelLayers = m_arichgp->getAerogelPlane().getNLayers();
    m_thickness[m_nAerogelLayers] = 0;
    for (unsigned int i = 0; i < m_nAerogelLayers; i++) {
      m_refractiveInd[i] = m_arichgp->getAerogelPlane().getLayerRefIndex(i + 1);
      m_anorm[i] = TVector3(0, 0, 1);
      m_thickness[i] = m_arichgp->getAerogelPlane().getLayerThickness(i + 1);
      if (i == 0) m_zaero[i] = m_arichgp->getAerogelPlane().getAerogelZPosition() + m_thickness[i];
      else  m_zaero[i] = m_zaero[i - 1] +  m_thickness[i];

      m_transmissionLen[i] = m_arichgp->getAerogelPlane().getLayerTrLength(i + 1) ; // aerogel transmission length;

      // measured FOM; aeroMerit is number of detected photons for beam of beta=1 and perpedicular incidence to aerogel tile
      // (corrected for geometrical acceptance). n0[i] is then calculated from
      // aeroMerit[i]=n0[i]*sin2(thc)*transmissionLen[i] * (1 - exp(-thickness[i] / transmissionLen[i])
      m_n0[i] = m_recPars->getAerogelFOM(i) / ((1. - 1. / m_refractiveInd[i] / m_refractiveInd[i]) * m_transmissionLen[i] * (1 - exp(
                                                 -m_thickness[i] / m_transmissionLen[i])));
      m_thickness[m_nAerogelLayers]   += m_thickness[i];
    }
    m_refractiveInd[m_nAerogelLayers  ]   = 1.0;
    m_refractiveInd[m_nAerogelLayers + 1]   = m_arichgp->getHAPDGeometry().getWinRefIndex();
    m_zaero[m_nAerogelLayers  ] = m_arichgp->getDetectorZPosition();
    m_zaero[m_nAerogelLayers + 1] = m_zaero[m_nAerogelLayers] + m_arichgp->getHAPDGeometry().getWinThickness();

  }


  int ARICHReconstruction::InsideDetector(TVector3 a, int copyno)
  {
    if (copyno == -1) return 0;
    TVector2 origin;
    origin.SetMagPhi(m_arichgp->getDetectorPlane().getSlotR(copyno), m_arichgp->getDetectorPlane().getSlotPhi(copyno));
    TVector2 a2(a.X(), a.Y());
    double phi = m_arichgp->getDetectorPlane().getSlotPhi(copyno);
    TVector2 diff = a2 - origin;
    diff = diff.Rotate(-phi);
    const double size = m_arichgp->getHAPDGeometry().getAPDSizeX();
    if (fabs(diff.X()) < size / 2. && fabs(diff.Y()) < size / 2.) {
      return 1;
    }
    return 0;
  }


  int ARICHReconstruction::smearTrack(ARICHTrack& arichTrack)
  {
    double a = gRandom->Gaus(0, m_trackAngRes);
    double b = gRandom->Gaus(0, m_trackAngRes);
    TVector3 dirf(a, b, sqrt(1 - a * a - b * b));
    double dx = gRandom->Gaus(0, m_trackPosRes);
    double dy = gRandom->Gaus(0, m_trackPosRes);
    TVector3 mod = TVector3(dx, dy, 0);
    TVector3 rpoint = arichTrack.getPosition() + mod;
    TVector3 odir  = arichTrack.getDirection();
    double omomentum  = arichTrack.getMomentum();
    TVector3 rdir = TransformFromFixed(odir) * dirf;  // global system
    double rmomentum = omomentum;
    arichTrack.setReconstructedValues(rpoint, rdir, rmomentum);
    return 1;
  }


  TVector3 ARICHReconstruction::FastTracking(TVector3 dirf, TVector3 r,  double* refractiveInd, double* z, int n, int opt)
  {
    //
    // Description:
    // The method calculates the intersection  of the cherenkov photon
    // with the detector plane

    //  z[n+1]
    //  z[0] .. 1st aerogel exit
    //  z[n-1] .. 2nd aerogel exit

    double rmir = 0; double angmir = 0; int section[2] = {0, 0};

    unsigned tileID = m_arichgp->getAerogelPlane().getAerogelTileID(r.X(), r.Y());

    if (tileID == 0 && opt == 1) return TVector3();

    int nmir = m_arichgp->getMirrors().getNMirrors();
    if (nmir > 0) {
      rmir = m_arichgp->getMirrors().getPoint(1).XYvector().Mod();
      double dangle = 2 * M_PI / nmir;
      angmir = m_arichgp->getMirrors().getStartAngle() - dangle / 2.;

      double trkangle = r.XYvector().Phi() - angmir;
      if (trkangle < 0) trkangle += 2 * M_PI;
      if (trkangle > 2 * M_PI) trkangle -= 2 * M_PI;

      section[1]  = int(trkangle / dangle) + 1;


    }

    bool reflok = false; bool refl = false;
    double path = (z[0] - r.z()) / dirf.z();
    r   += dirf * path;
    for (int a = 1; a <= n + 1 ; a++) {
      double rind = refractiveInd[a] / refractiveInd[a - 1];
      dirf = Refraction(dirf, rind);
      if (dirf.Mag() == 0) return TVector3();
      path = (z[a] - r.z()) / dirf.z();
      TVector3 r0 = r;
      if (a == n && opt == 1) {
        if (m_arichgp->getAerogelPlane().getAerogelTileID(r.X(), r.Y()) != tileID) return TVector3();
      }
      r += dirf * path;
      TVector2 rxy = r.XYvector();
      // check for possible reflections
      if (a != n || rxy.Mod() < rmir || nmir == 0) continue;
      double angle = rxy.Phi() - angmir;
      if (angle < 0) angle += 2 * M_PI;
      if (angle > 2 * M_PI) angle -= 2 * M_PI;
      double dangle = 2 * M_PI / nmir;
      section[0] = int(angle / dangle) + 1;
      if (r.Mag() > (r - 2 * m_arichgp->getMirrors().getPoint(section[0])).Mag()) {
        refl = true;
        int nrefl = 2;
        if (section[0] == section[1]) nrefl = 1;
        for (int k = 0; k < nrefl; k++) {

          if (!HitsMirror(r0, dirf, section[k])) continue;

          TVector3 mirpoint = m_arichgp->getMirrors().getPoint(section[k]);
          TVector3 mirnorm = m_arichgp->getMirrors().getNormVector(section[k]);
          double s = dirf * mirnorm;
          double s1 = (mirpoint - r0) * mirnorm;
          r = r0 + s1 / s * dirf;
          dirf = dirf - 2 * (dirf * mirnorm) * mirnorm;
          path = (z[a] - r.z()) / dirf.z();
          r += dirf * path;
          reflok = true;
          break;
        }
      }
    }

    if (!reflok && refl) return TVector3();
    return r;
  }

  TVector3 ARICHReconstruction::HitVirtualPosition(const TVector3& hitpos, int mirrorID)
  {

    if (mirrorID == 0) return hitpos;
    TVector3 mirpoint = m_arichgp->getMirrors().getPoint(mirrorID);
    TVector3 mirnorm = m_arichgp->getMirrors().getNormVector(mirrorID);
    return hitpos - 2 * ((hitpos - mirpoint) * mirnorm) * mirnorm;
  }


  bool ARICHReconstruction::HitsMirror(const TVector3& pos, const TVector3& dir, int mirrorID)
  {

    TVector3 mirnorm = m_arichgp->getMirrors().getNormVector(mirrorID);
    TVector3 mirpoint = m_arichgp->getMirrors().getPoint(mirrorID);
    TRotation rot = TransformToFixed(mirnorm);
    TVector3 dirTr = rot * dir;
    if (dirTr.Z() < 0) return 0; // comes from outter side
    TVector3 posTr =  rot * (pos - mirpoint);
    TVector3 pointOnMirr = posTr - (posTr.Z() / dirTr.Z()) * dirTr;
    if (fabs(pointOnMirr.Y()) < m_arichgp->getMirrors().getPlateLength() / 2.
        && fabs(pointOnMirr.X()) < m_arichgp->getMirrors().getPlateWidth() / 2.) return 1;

    return 0;
  }


  int ARICHReconstruction::CherenkovPhoton(TVector3 r, TVector3 rh,
                                           TVector3& rf, TVector3& dirf,
                                           double* refractiveInd, double* z, int n, int mirrorID)
  {
    //
    // Description:
    // The method calculates the direction of the cherenkov photon
    // and intersection with the aerogel exit point
    //
    // Called by: CerenkovAngle


    // Arguments:
    // Input:
    // dir, r track position
    // rh photon hit


    // Output:
    // rf aerogel exit from which the  photon was emitted
    // dirf photon direction in aerogel
    static TVector3 norm(0, 0, 1); // detector plane normal vector

    double dwin    = m_arichgp->getHAPDGeometry().getWinThickness();
    double refractiveInd0 = m_arichgp->getHAPDGeometry().getWinRefIndex();

    // iteration is stoped when the difference of photon positions on first aerogel exit
    // between two iterations is smaller than this value.
    const double dmin  = 0.0000001;
    const int    niter = 100; // maximal number of iterations
    TVector3  dirw;
    TVector3 rh1 = rh - dwin * norm;

    std::vector <TVector3> rf0(n + 2);
    //  rf0[0] .. track point
    //  rf0[1] 1. 1st aerogel exit
    //  rf0[n] n.  aerogel exit ...
    std::vector <TVector3> dirf0(n + 2);
    //  dirf0[0] .. 1st aerogel direction
    //  dirf0[1] .. 2nd aerogel direction
    //  dirf0[n] .. direction after aerogels

    //  z[0] .. 1st aerogel exit
    //  z[n-1] .. 2nd aerogel exit
    //  z[n]    .. detector position
    //  z[n+1]  .. detector + window

    rf0[0] = r;
    rf0[1] = rf;

    double rind = 0;

    for (int iter = 0; iter < niter; iter++) {

      // direction in the space between aerogels and detector
      // *************************************
      if (iter == 0) dirf0[n] = (rh1 - rf0[1]).Unit();
      else  dirf0[n] = (rh1 - rf0[n]).Unit();

      // *************************************
      // n-layers of aerogel // refractiveInd relative refractive index
      for (int a = n - 1; a >= 0 ; a--) {
        rind = refractiveInd[a] / refractiveInd[a + 1];
        dirf0[a] = Refraction(dirf0[a + 1], rind);
      }

      double path = (z[0] - r.z()) / dirf0[0].z();
      double x1 = rf0[1].x();
      double y1 = rf0[1].y();
      for (int a = 0; a < n ; a++) {
        rf0[a + 1] = rf0[a] + dirf0[a] * path;
        path = (z[a + 1] - rf0[a + 1].z()) / dirf0[a + 1].z();
      }

      Refraction(dirf0[n], norm, refractiveInd0, dirw);

      // *************************************

      path = dwin / (dirw * norm);
      rh1 = rh - dirw * path;

      double d2 = (rf0[1].x() - x1) * (rf0[1].x() - x1) + (rf0[1].y() - y1) * (rf0[1].y() - y1);

      if ((d2 < dmin) && (iter)) {

        // if mirror hypothesis check if reflection point lies on mirror plate
        if (mirrorID) {
          if (!HitsMirror(rf0[n], dirf0[n], mirrorID)) return -1;
        }

        rf = rf0[1];
        dirf = dirf0[0];
        return iter;
      }
    }
    return -1;
  }

  int ARICHReconstruction::likelihood2(ARICHTrack& arichTrack, StoreArray<ARICHHit>& arichHits, ARICHLikelihood& arichLikelihood)
  {

    const unsigned int nPhotonHits = arichHits.getEntries(); // number of detected photons

    if (m_nAerogelLayers + 1 > c_noOfAerogels) B2ERROR("ARICHReconstrucion: number of aerogel layers defined in the xml file exceeds "
                                                         << c_noOfAerogels);

    double  logL[c_noOfHypotheses] = {0.0};
    double  nSig_w_acc[c_noOfHypotheses][c_noOfAerogels] = { {0.0} }; // expected no. of signal photons, including geometrical acceptance
    double  nSig_wo_acc[c_noOfHypotheses][c_noOfAerogels][20] = { { {0.0} } }; // expected no. of signal photons, without geometrical acceptance, divided in 20 phi bins (used for PDF normalization)
    double  nSig_wo_accInt[c_noOfHypotheses][c_noOfAerogels] = { {0.0} }; // expected no. of signal photons, without geometrical acceptance, integrated over phi
    double  esigi[c_noOfHypotheses] = {0.0}; // expected number of signal photons in hit pixel
    double  thetaCh[c_noOfHypotheses][c_noOfAerogels] = { {0.0} }; // expected Cherenkov angle

    // read some geometry parameters
    double padSize = m_arichgp->getHAPDGeometry().getPadSize();
    int nMirSeg = m_arichgp->getMirrors().getNMirrors();
    double angmir  = m_arichgp->getMirrors().getStartAngle();

    // Detected photons in cherenkov ring (integrated over 0.1<theta<0.5)
    int nDetPhotons = 0;

    double ebgri[c_noOfHypotheses] = {0.0}; // number of expected background photons per pad
    double  nBgr[c_noOfHypotheses] = {0.0}; // total number of expected background photons (in 0.1-0.5 rad ring)

    // reconstructed track direction
    TVector3 edir = arichTrack.getDirection();
    if (edir.z() < 0) return 0;
    double momentum = arichTrack.getMomentum();

    double thcResolution = m_recPars->getThcResolution(momentum);
    if (thcResolution < 0) thcResolution = 0.01; // happens for spurious tracks with 100 GeV momentum!

    double wideGaussFract = (m_recPars->getParameters())[0];
    double wideGaussSigma = (m_recPars->getParameters())[1];

    //------------------------------------------------------
    // Calculate number of expected detected photons (emmited x geometrical acceptance).
    // -----------------------------------------------------

    float nphot_scaling = 20.; // number of photons to be traced is (expected number of emitted photons * nphot_scaling)
    int nStep = 5;             // number of steps in one aerogel layer

    // loop over all particle hypotheses
    for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {

      // absorbtion factor (scattering)
      double abs = 1;

      // loop over aerogel layers
      for (int iAerogel = m_nAerogelLayers - 1; iAerogel >= 0; iAerogel--) {

        thetaCh[iHyp][iAerogel] = ExpectedCherenkovAngle(momentum,
                                                         p_mass[iHyp],
                                                         m_refractiveInd[iAerogel]);

        // track length in the radiator
        double pathLengthRadiator = arichTrack.getDirection() * m_anorm[iAerogel];
        if (pathLengthRadiator)  pathLengthRadiator = m_thickness[iAerogel] / pathLengthRadiator;

        // step length
        double dxx = pathLengthRadiator / double(nStep);
        // number of photons to be emmited per step (number of expected photons * nphot_scaling)
        double nPhot = m_n0[iAerogel] * sin(thetaCh[iHyp][iAerogel]) * sin(thetaCh[iHyp][iAerogel]) * dxx * nphot_scaling;
        TVector3 exit_point = getTrackPositionAtZ(arichTrack, m_zaero[iAerogel]);

        // loop over emmision point steps
        for (int iepoint = 0; iepoint < nStep; iepoint++) {

          TVector3 epoint = exit_point - (0.5 + iepoint) * dxx * edir;
          abs *= exp(-dxx / m_transmissionLen[iAerogel]);
          unsigned int genPhot = nPhot * abs; // number of photons to emmit in current step, including scattering  correction

          // loop over emmited "photons"
          for (unsigned int iPhoton = 0; iPhoton < genPhot; iPhoton++) {
            double fi = 2 * M_PI * iPhoton / float(genPhot); // uniformly distributed in phi
            TVector3 adirf = setThetaPhi(thetaCh[iHyp][iAerogel], fi); // photon direction in track system
            adirf =  TransformFromFixed(edir) * adirf;  // photon direction in global system
            int ifi = int (fi * 20 / 2. / M_PI); // phi bin
            // track photon from emission point to the detector plane
            TVector3 dposition = FastTracking(adirf, epoint, &m_refractiveInd[iAerogel], &m_zaero[iAerogel], m_nAerogelLayers - iAerogel, 1);
            if (dposition.Mag() > 1.0) {nSig_wo_acc[iHyp][iAerogel][ifi] += 1; nSig_wo_accInt[iHyp][iAerogel] += 1;}
            else continue;
            unsigned  copyno =  m_arichgp->getDetectorPlane().pointSlotID(dposition.X(), dposition.Y());
            if (!copyno) continue;
            // check if photon fell on photosensitive area
            if (InsideDetector(dposition, copyno)) nSig_w_acc[iHyp][iAerogel] += 1;
          }
        }

        // scale the obtained numbers
        for (int ik = 0; ik < 20; ik++) {
          nSig_wo_acc[iHyp][iAerogel][ik] /= nphot_scaling;
        }
        nSig_w_acc[iHyp][iAerogel] /= nphot_scaling;
        nSig_wo_accInt[iHyp][iAerogel] /= nphot_scaling;
      } // for (unsigned int iAerogel=0;iAerogel<m_nAerogelLayers;iAerogel++)

      // sum up contribution from all aerogel layers
      for (unsigned int iAerogel = 0; iAerogel < m_nAerogelLayers; iAerogel++) {
        nSig_w_acc[iHyp][m_nAerogelLayers] += nSig_w_acc[iHyp][iAerogel];
        nSig_wo_accInt[iHyp][m_nAerogelLayers] += nSig_wo_accInt[iHyp][iAerogel];

        for (int ik = 0; ik < 20; ik++) {
          nSig_wo_acc[iHyp][m_nAerogelLayers][ik] += nSig_wo_acc[iHyp][iAerogel][ik];
        }
      }

      // get number of expected background photons in ring (integrated from 0.1 to 0.5 rad)
      std::vector<double> bkgPars = {momentum / sqrt(p_mass[iHyp]*p_mass[iHyp] + momentum * momentum), double(arichTrack.hitsWindow())};
      nBgr[iHyp] = m_recPars->getExpectedBackgroundHits(bkgPars);

    }  // for (int iHyp=0;iHyp < c_noOfHypotheses; iHyp++ )
    //#####################################################

    TVector3 track_at_detector = getTrackPositionAtZ(arichTrack, m_zaero[m_nAerogelLayers + 1]);

    // the id numbers of mirrors from which the photons could possibly reflect are calculated
    int mirrors[3];
    mirrors[0] = 0; // for no reflection
    int refl = 1;

    // only if particle track on detector is at radius larger than 850mm (for now hardcoded)
    // possible reflections are taken into account.
    if (track_at_detector.XYvector().Mod() > 85.0) {
      double trackang = track_at_detector.Phi() - angmir;
      if (trackang < 0) trackang += 2 * M_PI;
      if (trackang > 2 * M_PI) trackang -= 2 * M_PI;
      int section1 = int(trackang * nMirSeg / 2. / M_PI) + 1;
      int section2 = section1 + 1;
      if (section1 == nMirSeg)  section2 = 1;
      mirrors[1] = section1; mirrors[2] = section2;
      refl = 3;
    }

    // loop over all detected photon hits

    for (unsigned int iPhoton = 0; iPhoton < nPhotonHits; iPhoton++) {

      ARICHHit* h = arichHits[iPhoton];
      int modID = h->getModule();
      TVector3 hitpos = m_arichgp->getMasterVolume().pointToLocal(h->getPosition());
      bool bkgAdded = false;
      int nfoo = nDetPhotons;
      for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) { esigi[iHyp] = 0; ebgri[iHyp] = 0;}

      // loop over possible mirror reflections
      for (int mirr = 0; mirr < refl; mirr++) {
        // calculate fi_ch for a given track refl
        TVector3 virthitpos =  HitVirtualPosition(hitpos, mirrors[mirr]);

        // if hit is more than 15cm from the track position on the detector plane, skip it.
        // (not reconstructing hits with irrelevantly large Cherenkov angle)
        if ((track_at_detector - virthitpos).Mag() > 15.0) continue;

        double sigExpArr[c_noOfHypotheses] = {0.0}; // esigi for given mirror hypothesis only
        double th_cer_1st = 0;
        double fi_cer_1st = 0;
        int proc = 0;

        // loop over all arogel layers
        for (unsigned int iAerogel = 0; iAerogel < m_nAerogelLayers; iAerogel++) {

          TVector3 initialrf = getTrackPositionAtZ(arichTrack, m_zaero[iAerogel]);
          TVector3 epoint = getTrackMeanEmissionPosition(arichTrack, iAerogel);
          TVector3 edirr  = arichTrack.getDirection();
          TVector3 photonDirection; // calculated photon direction

          if (CherenkovPhoton(epoint, virthitpos, initialrf, photonDirection, &m_refractiveInd[iAerogel], &m_zaero[iAerogel],
                              m_nAerogelLayers - iAerogel, mirrors[mirr]) < 0) break;

          TVector3 dirch = TransformToFixed(edirr) * photonDirection;
          double fi_cer = dirch.Phi();
          double th_cer = dirch.Theta();

          // skip photons with irrelevantly large/small Cherenkov angle
          if ((th_cer > 0.5 || th_cer < 0.1) && iAerogel == 0) break;

          // count photons with 0.1<thc<0.5
          if (nfoo == nDetPhotons) nDetPhotons++;

          if (iAerogel == 0) { th_cer_1st = th_cer; fi_cer_1st = fi_cer;}

          if (fi_cer < 0) fi_cer += 2 * M_PI;
          double fii = fi_cer;
          if (mirr > 0) {
            double fi_mir = m_arichgp->getMirrors().getNormVector(mirrors[mirr]).XYvector().Phi();
            fii = 2 * fi_mir - fi_cer - M_PI;
          }


          // loop over all particle hypotheses
          for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {

            // track a photon from the mean emission point to the detector surface
            TVector3  photonDirection1 = setThetaPhi(thetaCh[iHyp][iAerogel], fi_cer);  // particle system
            photonDirection1 = TransformFromFixed(edirr) * photonDirection1;  // global system
            int ifi = int (fi_cer * 20 / 2. / M_PI);
            TVector3  photonAtAerogelExit = photonDirection1 * (m_thickness[iAerogel] / photonDirection1.z());
            TVector3  trackAtAerogelExit = edirr * (m_thickness[iAerogel] / edirr.z());
            TVector3  dtrackphoton = photonAtAerogelExit - trackAtAerogelExit;
            TVector3 detector_position;

            detector_position = FastTracking(photonDirection1, epoint, &m_refractiveInd[iAerogel], &m_zaero[iAerogel],
                                             m_nAerogelLayers - iAerogel, 0);

            TVector3 meanr             = detector_position - epoint;
            double   path              = meanr.Mag();
            meanr                      = meanr.Unit();

            double   detector_sigma    = thcResolution * path / meanr.z();
            double wide_sigma = wideGaussSigma * path / meanr.z();
            // calculate pad orientation and distance relative to that photon
            double modphi =  m_arichgp->getDetectorPlane().getSlotPhi(modID);

            double      pad_fi = fii - modphi;
            double      dx     = (detector_position - hitpos).Mag();
            double  dr = (track_at_detector - detector_position).Mag();

            if (dr > 0.01) {
              double normalizacija = nSig_wo_acc[iHyp][iAerogel][ifi] * padSize / (0.1 * M_PI * dr);
              double integralMain = SquareInt(padSize, pad_fi, dx, detector_sigma) / sqrt(2.);
              double integralWide = SquareInt(padSize, pad_fi, dx, wide_sigma) / sqrt(2.);
              // expected number of signal photons in each pixel
              double exp = normalizacija * ((1 - wideGaussFract) * integralMain + wideGaussFract * integralWide);
              esigi[iHyp] += exp;
              sigExpArr[iHyp] += exp;
            } // if (dr>0 && thetaCh[iHyp][iAerogel])

          }// for (int iHyp=0;iHyp< c_noOfHypotheses; iHyp++)
          if (iAerogel == m_nAerogelLayers - 1) proc = 1; // successfully processed for all layers
        }// for (unsigned int iAerogel=0; iAerogel<m_nAerogelLayers;iAerogel++)

        if (proc) {
          // add background contribution if not yet (add only once)
          if (!bkgAdded) {
            for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {
              std::vector<double> pars = {momentum / sqrt(p_mass[iHyp]*p_mass[iHyp] + momentum * momentum), double(arichTrack.hitsWindow())};
              ebgri[iHyp] += m_recPars->getBackgroundPerPad(th_cer_1st, pars);
            }
            bkgAdded = true;
          }
          // create ARICHPhoton if desired
          if (m_storePhot) {
            ARICHPhoton phot(iPhoton, th_cer_1st, fi_cer_1st, mirrors[mirr]); // th_cer of the first aerogel layer assumption is stored
            phot.setBkgExp(ebgri); // store expected number of background hits
            phot.setSigExp(sigExpArr); // store expected number of signal hits
            arichTrack.addPhoton(phot);
          }
        }

      }// for (int mirr = 0; mirr < refl; mirr++)

      //******************************************
      // LIKELIHOOD construction
      //*******************************************

      for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {
        double expected = esigi[iHyp] + ebgri[iHyp];
        if (bkgAdded) logL[iHyp] += expected + log(1 - exp(-expected));
      }

    } // for (unsigned  int iPhoton=0; iPhoton< nPhotonHits; iPhoton++)

    //*********************************************
    // add constant term to the LIKELIHOOD function
    //*********************************************
    double exppho[6] = {0.0};
    for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {
      exppho[iHyp] = nSig_w_acc[iHyp][m_nAerogelLayers] * (1 - wideGaussFract) + wideGaussFract * 0.7 *
                     nSig_wo_accInt[iHyp][m_nAerogelLayers] + nBgr[iHyp];
      logL[iHyp] -= exppho[iHyp];
      if (isnan(logL[iHyp]) || isinf(logL[iHyp])) {
        B2WARNING("ARICHReconstruction: log likelihood value infinite! Flat background hit probability is " << ebgri[iHyp] << "!");
        logL[iHyp] = 0;
      }
    }

    //******************************************
    // store LikeliHOOD info
    //******************************************

    int flag = 1;
    if ((thetaCh[0][0] > 0 || thetaCh[0][1] > 0) &&  nSig_w_acc[0][m_nAerogelLayers] == 0) flag = 0;

    // set values of ARICHLikelihood
    arichLikelihood.setValues(flag, logL, nDetPhotons, exppho);

    return 1;
  }

  void ARICHReconstruction::setTrackPositionResolution(double pRes)
  {
    m_trackPosRes = pRes;
  }
  void ARICHReconstruction::setTrackAngleResolution(double aRes)
  {
    m_trackAngRes = aRes;
  }

  TVector3 ARICHReconstruction::getTrackMeanEmissionPosition(const ARICHTrack& track, int iAero)
  {
    // Emission length measured from aerogel exit

    TVector3 dir = track.getDirection();
    if (dir.Z() == 0) return TVector3();
    double d   = m_thickness[iAero] / dir.Z() / m_transmissionLen[iAero];
    double dmean = 1 - d / expm1(d);
    //double dmean = -log((1 + exp(-d)) / 2.);
    double mel = dmean * m_transmissionLen[iAero];

    return (getTrackPositionAtZ(track, m_zaero[iAero]) - mel * dir);
  }

  TVector3 ARICHReconstruction::getTrackPositionAtZ(const ARICHTrack& track, double zout)
  {
    TVector3 dir = track.getDirection();
    TVector3 pos = track.getPosition();
    if (dir.Z() == 0) return TVector3(0, 0, 0);
    double path = (zout - pos.Z()) / dir.Z();
    return pos + dir * path;
  }

  void ARICHReconstruction::transformTrackToLocal(ARICHTrack& arichTrack, bool)
  {
    // tranform track from BelleII to local ARICH frame
    TVector3 locPos = m_arichgp->getMasterVolume().pointToLocal(arichTrack.getPosition());
    TVector3 locDir = m_arichgp->getMasterVolume().momentumToLocal(arichTrack.getDirection());

    /*if(align && m_alignp.isValid()){
      // apply global alignment correction
      //   locPos = m_alignp->pointToLocal(locPos);
      //locDir = m_alignp->momentumToLocal(locDir);
    } */

    // set parameters and return
    // is it needed to extrapolate to z of aerogel in local frame?? tabun
    arichTrack.setReconstructedValues(locPos, locDir, arichTrack.getMomentum());
    return;
  }


}
