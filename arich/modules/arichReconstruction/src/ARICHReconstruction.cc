/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "arich/modules/arichReconstruction/ARICHReconstruction.h"
#include "arich/dbobjects/ARICHGeometryConfig.h"
#include "arich/modules/arichReconstruction/Utility.h"
#include "arich/dataobjects/ARICHHit.h"
#include "arich/dataobjects/ARICHTrack.h"
#include "arich/dataobjects/ARICHPhoton.h"

// DataStore
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/geometry/VectorUtil.h>

#include <vector>
#include <Math/Vector3D.h>
#include <TRandom3.h>

using namespace boost;

namespace Belle2 {

  using namespace arich;

  ARICHReconstruction::ARICHReconstruction(int storePhot):
    m_arichgp(),
    m_recPars(),
    m_trackPosRes(0),
    m_trackAngRes(0),
    m_alignMirrors(true),
    m_nAerogelLayers(0),
    m_storePhot(storePhot)
  {
    for (unsigned i = 0; i < c_noOfHypotheses; i++) {p_mass[i] = 0;}
    for (unsigned i = 0; i < c_noOfAerogels; i++) {
      m_refractiveInd[i] = 0;
      m_zaero[i] = 0;
      m_thickness[i] = 0;
      m_transmissionLen[i] = 0;
      m_n0[i] = 0;
      m_anorm[i] = ROOT::Math::XYZVector();
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
      m_anorm[i] = ROOT::Math::XYZVector(0, 0, 1);
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

    if (m_mirrAlign.hasChanged()) {
      m_mirrorNorms.clear();
      m_mirrorPoints.clear();
      for (unsigned i = 1; i < m_arichgp->getMirrors().getNMirrors() + 1; i++) {
        m_mirrorNorms.push_back(getMirrorNorm(i));
        m_mirrorPoints.push_back(getMirrorPoint(i));
      }
    }

    if (m_tileAlign) {
      if (m_tileAlign.hasChanged()) {
        for (int iTile = 1; iTile < 125; iTile++) {
          m_tilePars[iTile - 1][0] = m_tileAlign->getAlignmentElement(iTile).getAlpha();
          m_tilePars[iTile - 1][1] = m_tileAlign->getAlignmentElement(iTile).getBeta();
        }
      }
    }
  }


  int ARICHReconstruction::InsideDetector(ROOT::Math::XYZVector a, int copyno)
  {
    if (copyno == -1) return 0;
    ROOT::Math::XYVector origin;
    origin.SetXY(m_arichgp->getDetectorPlane().getSlotR(copyno) * std::cos(m_arichgp->getDetectorPlane().getSlotPhi(copyno)),
                 m_arichgp->getDetectorPlane().getSlotR(copyno) * std::sin(m_arichgp->getDetectorPlane().getSlotPhi(copyno)));
    ROOT::Math::XYVector a2(a);
    double phi = m_arichgp->getDetectorPlane().getSlotPhi(copyno);
    ROOT::Math::XYVector diff = a2 - origin;
    diff.Rotate(-phi);
    const double size = m_arichgp->getHAPDGeometry().getAPDSizeX();
    if (fabs(diff.X()) < size / 2. && fabs(diff.Y()) < size / 2.) {
      int chX, chY;
      m_arichgp->getHAPDGeometry().getXYChannel(diff.X(), diff.Y(), chX, chY);
      if (chX < 0 || chY < 0) return 0;
      int asicChannel = m_chnMap->getAsicFromXY(chX, chY);
      // eliminate un-active channels
      if (asicChannel < 0 || !m_chnMask->isActive(copyno, asicChannel)) return 0;
      return 1;
    }
    return 0;
  }


  int ARICHReconstruction::smearTrack(ARICHTrack& arichTrack)
  {
    double a = gRandom->Gaus(0, m_trackAngRes);
    double b = gRandom->Gaus(0, m_trackAngRes);
    ROOT::Math::XYZVector dirf(a, b, sqrt(1 - a * a - b * b));
    double dx = gRandom->Gaus(0, m_trackPosRes);
    double dy = gRandom->Gaus(0, m_trackPosRes);
    ROOT::Math::XYZVector mod(dx, dy, 0);
    ROOT::Math::XYZVector rpoint = arichTrack.getPosition() + mod;
    ROOT::Math::XYZVector odir  = arichTrack.getDirection();
    double omomentum  = arichTrack.getMomentum();
    ROOT::Math::XYZVector rdir = TransformFromFixed(odir) * dirf;  // global system
    double rmomentum = omomentum;
    arichTrack.setReconstructedValues(rpoint, ROOT::Math::XYZVector(rdir), rmomentum);
    return 1;
  }


  ROOT::Math::XYZVector ARICHReconstruction::FastTracking(ROOT::Math::XYZVector dirf, ROOT::Math::XYZVector r,
                                                          double* refractiveInd, double* z, int n, int opt)
  {
    //
    // Description:
    // The method calculates the intersection  of the cherenkov photon
    // with the detector plane

    //  z[n+1]
    //  z[0] .. 1st aerogel exit
    //  z[n-1] .. 2nd aerogel exit

    double angmir = 0; int section[2] = {0, 0};

    unsigned tileID = m_arichgp->getAerogelPlane().getAerogelTileID(r.X(), r.Y());

    if (tileID == 0 && opt == 1) return ROOT::Math::XYZVector();

    int nmir = m_arichgp->getMirrors().getNMirrors();
    if (nmir > 0) {
      double dangle = 2 * M_PI / nmir;
      angmir = m_arichgp->getMirrors().getStartAngle() - dangle / 2.;

      double trkangle = r.Phi() - angmir;
      if (trkangle < 0) trkangle += 2 * M_PI;
      if (trkangle > 2 * M_PI) trkangle -= 2 * M_PI;

      section[1]  = int(trkangle / dangle) + 1;
    }

    bool reflok = false; bool refl = false;
    double path = (z[0] - r.Z()) / dirf.Z();
    r   += dirf * path;
    for (int a = 1; a <= n + 1 ; a++) {
      double rind = refractiveInd[a] / refractiveInd[a - 1];
      dirf = Refraction(dirf, rind);
      if (dirf.R() == 0) return ROOT::Math::XYZVector();
      path = (z[a] - r.Z()) / dirf.Z();
      ROOT::Math::XYZVector r0 = r;
      if (a == n && opt == 1) {
        if (m_arichgp->getAerogelPlane().getAerogelTileID(r.X(), r.Y()) != tileID) return ROOT::Math::XYZVector();
      }
      r += dirf * path;
      ROOT::Math::XYVector rxy(r);
      // check for possible reflections
      if (a != n || nmir == 0) continue;
      double angle = rxy.Phi() - angmir;
      if (angle < 0) angle += 2 * M_PI;
      if (angle > 2 * M_PI) angle -= 2 * M_PI;
      double dangle = 2 * M_PI / nmir;
      section[0] = int(angle / dangle) + 1;
      if (r.R() > (r - 2 * m_mirrorPoints[section[0] - 1]).R()) {
        refl = true;
        int nrefl = 2;
        if (section[0] == section[1]) nrefl = 1;
        for (int k = 0; k < nrefl; k++) {
          if (!HitsMirror(r0, dirf, section[k])) continue;
          ROOT::Math::XYZVector mirpoint = m_mirrorPoints[section[k] - 1];
          ROOT::Math::XYZVector mirnorm = m_mirrorNorms[section[k] - 1];
          double s = dirf.Dot(mirnorm);
          double s1 = (mirpoint - r0).Dot(mirnorm);
          r = r0 + s1 / s * dirf;
          dirf = dirf - 2 * (dirf.Dot(mirnorm)) * mirnorm;
          path = (z[a] - r.Z()) / dirf.Z();
          r += dirf * path;
          reflok = true;
          break;
        }
      }
    }

    if (!reflok && refl) return ROOT::Math::XYZVector();
    return r;
  }

  ROOT::Math::XYZVector ARICHReconstruction::HitVirtualPosition(const ROOT::Math::XYZVector& hitpos, int mirrorID)
  {

    if (mirrorID == 0) return hitpos;
    ROOT::Math::XYZVector mirpoint = m_mirrorPoints[mirrorID - 1];
    ROOT::Math::XYZVector mirnorm = m_mirrorNorms[mirrorID - 1];
    return hitpos - 2 * ((hitpos - mirpoint).Dot(mirnorm)) * mirnorm;
  }


  bool ARICHReconstruction::HitsMirror(const ROOT::Math::XYZVector& pos, const ROOT::Math::XYZVector& dir, int mirrorID)
  {

    ROOT::Math::XYZVector mirnorm = m_mirrorNorms[mirrorID - 1];
    ROOT::Math::XYZVector mirpoint = m_mirrorPoints[mirrorID - 1];
    ROOT::Math::Rotation3D rot = TransformToFixed(mirnorm);
    ROOT::Math::XYZVector dirTr = rot * dir;
    if (dirTr.Z() < 0) return 0; // comes from outter side
    ROOT::Math::XYZVector posTr =  rot * (pos - mirpoint);
    ROOT::Math::XYZVector pointOnMirr = posTr - (posTr.Z() / dirTr.Z()) * dirTr;
    if (fabs(pointOnMirr.Y()) < m_arichgp->getMirrors().getPlateLength() / 2.
        && fabs(pointOnMirr.X()) < m_arichgp->getMirrors().getPlateWidth() / 2.) return 1;

    return 0;
  }


  int ARICHReconstruction::CherenkovPhoton(ROOT::Math::XYZVector r, ROOT::Math::XYZVector rh,
                                           ROOT::Math::XYZVector& rf, ROOT::Math::XYZVector& dirf,
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
    static ROOT::Math::XYZVector norm(0, 0, 1); // detector plane normal vector

    double dwin    = m_arichgp->getHAPDGeometry().getWinThickness();
    double refractiveInd0 = m_arichgp->getHAPDGeometry().getWinRefIndex();

    // iteration is stoped when the difference of photon positions on first aerogel exit
    // between two iterations is smaller than this value.
    const double dmin  = 0.0000001;
    const int    niter = 100; // maximal number of iterations
    ROOT::Math::XYZVector dirw;
    ROOT::Math::XYZVector rh1 = rh - dwin * norm;

    std::vector <ROOT::Math::XYZVector > rf0(n + 2);
    //  rf0[0] .. track point
    //  rf0[1] 1. 1st aerogel exit
    //  rf0[n] n.  aerogel exit ...
    std::vector <ROOT::Math::XYZVector > dirf0(n + 2);
    //  dirf0[0] .. 1st aerogel direction
    //  dirf0[1] .. 2nd aerogel direction
    //  dirf0[n] .. direction after aerogels

    //  z[0] .. 1st aerogel exit
    //  z[n-1] .. 2nd aerogel exit
    //  z[n]    .. detector position
    //  z[n+1]  .. detector + window

    rf0[0] = r;
    rf0[1] = rf;

    for (int iter = 0; iter < niter; iter++) {

      // direction in the space between aerogels and detector
      // *************************************
      if (iter == 0) dirf0[n] = (rh1 - rf0[1]).Unit();
      else  dirf0[n] = (rh1 - rf0[n]).Unit();

      // *************************************
      // n-layers of aerogel // refractiveInd relative refractive index
      for (int a = n - 1; a >= 0 ; a--) {
        double rind = refractiveInd[a] / refractiveInd[a + 1];
        dirf0[a] = Refraction(dirf0[a + 1], rind);
      }

      double path = (z[0] - r.Z()) / dirf0[0].Z();
      double x1 = rf0[1].X();
      double y1 = rf0[1].Y();
      for (int a = 0; a < n ; a++) {
        rf0[a + 1] = rf0[a] + dirf0[a] * path;
        path = (z[a + 1] - rf0[a + 1].Z()) / dirf0[a + 1].Z();
      }

      Refraction(dirf0[n], norm, refractiveInd0, dirw);

      // *************************************

      path = dwin / (dirw.Dot(norm));
      rh1 = rh - dirw * path;

      double d2 = (rf0[1].X() - x1) * (rf0[1].X() - x1) + (rf0[1].Y() - y1) * (rf0[1].Y() - y1);

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

  int ARICHReconstruction::likelihood2(ARICHTrack& arichTrack, const StoreArray<ARICHHit>& arichHits,
                                       ARICHLikelihood& arichLikelihood)
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
    ROOT::Math::XYZVector edir = arichTrack.getDirection();
    if (edir.Z() < 0) return 0;
    double momentum = arichTrack.getMomentum();

    double thcResolution = m_recPars->getThcResolution(momentum);
    if (thcResolution < 0) thcResolution = 0.01; // happens for spurious tracks with 100 GeV momentum!

    double wideGaussFract = (m_recPars->getParameters())[0];
    double wideGaussSigma = (m_recPars->getParameters())[1];

    unsigned tileID = m_arichgp->getAerogelPlane().getAerogelTileID(arichTrack.getPosition().X(), arichTrack.getPosition().Y());
    double r = arichTrack.getPosition().Rho();
    if (tileID > 0) correctEmissionPoint(tileID, r);

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
        double pathLengthRadiator = arichTrack.getDirection().Dot(m_anorm[iAerogel]);
        if (pathLengthRadiator)  pathLengthRadiator = m_thickness[iAerogel] / pathLengthRadiator;

        // step length
        double dxx = pathLengthRadiator / double(nStep);
        // number of photons to be emmited per step (number of expected photons * nphot_scaling)
        double nPhot = m_n0[iAerogel] * sin(thetaCh[iHyp][iAerogel]) * sin(thetaCh[iHyp][iAerogel]) * dxx * nphot_scaling;
        ROOT::Math::XYZVector exit_point = getTrackPositionAtZ(arichTrack, m_zaero[iAerogel]);

        // loop over emmision point steps
        for (int iepoint = 0; iepoint < nStep; iepoint++) {

          ROOT::Math::XYZVector epoint = exit_point - (0.5 + iepoint) * dxx * edir;
          abs *= exp(-dxx / m_transmissionLen[iAerogel]);
          unsigned int genPhot = nPhot * abs; // number of photons to emmit in current step, including scattering  correction

          // loop over emmited "photons"
          for (unsigned int iPhoton = 0; iPhoton < genPhot; iPhoton++) {
            double fi = 2 * M_PI * iPhoton / float(genPhot); // uniformly distributed in phi
            ROOT::Math::XYZVector adirf = setThetaPhi(thetaCh[iHyp][iAerogel], fi); // photon direction in track system
            adirf =  TransformFromFixed(edir) * adirf;  // photon direction in global system
            int ifi = int (fi * 20 / 2. / M_PI); // phi bin
            // track photon from emission point to the detector plane
            ROOT::Math::XYZVector dposition = FastTracking(adirf, epoint, &m_refractiveInd[iAerogel], &m_zaero[iAerogel],
                                                           m_nAerogelLayers - iAerogel, 1);
            if (dposition.R() > 1.0) {nSig_wo_acc[iHyp][iAerogel][ifi] += 1; nSig_wo_accInt[iHyp][iAerogel] += 1;}
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

    ROOT::Math::XYZVector track_at_detector = getTrackPositionAtZ(arichTrack, m_zaero[m_nAerogelLayers + 1]);

    // the id numbers of mirrors from which the photons could possibly reflect are calculated
    int mirrors[3];
    mirrors[0] = 0; // for no reflection
    int refl = 1;

    // only if particle track on detector is at radius larger than 850mm (for now hardcoded)
    // possible reflections are taken into account.
    if (track_at_detector.Rho() > 85.0) {
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
      int channel = h->getChannel();
      ROOT::Math::XYZVector hitpos = m_arichgp->getMasterVolume().pointToLocal(h->getPosition());
      bool bkgAdded = false;
      int nfoo = nDetPhotons;
      for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) { esigi[iHyp] = 0; ebgri[iHyp] = 0;}

      bool reflOK = true; // remove window photons from reflected hypothesis

      // loop over possible mirror reflections
      for (int mirr = 0; mirr < refl; mirr++) {

        if (!reflOK) break; // photon from window so break

        // calculate fi_ch for a given track refl
        ROOT::Math::XYZVector virthitpos =  HitVirtualPosition(hitpos, mirrors[mirr]);

        // if hit is more than 25cm from the track position on the detector plane, skip it.
        // (not reconstructing hits with irrelevantly large Cherenkov angle)
        if ((track_at_detector - virthitpos).R() > 25.0) continue;

        double sigExpArr[c_noOfHypotheses] = {0.0}; // esigi for given mirror hypothesis only
        double th_cer_all[c_noOfAerogels] = {0.0};
        double fi_cer_all[c_noOfAerogels] = {0.0};

        double weight[c_noOfHypotheses][c_noOfAerogels] = { {0.0} };
        double weight_sum[c_noOfHypotheses] = {0.0};
        int proc = 0;
        double fi_cer_trk = 0.;

        // loop over all aerogel layers
        for (unsigned int iAerogel = 0; iAerogel < m_nAerogelLayers; iAerogel++) {

          ROOT::Math::XYZVector initialrf = getTrackPositionAtZ(arichTrack, m_zaero[iAerogel]);
          ROOT::Math::XYZVector epoint = getTrackMeanEmissionPosition(arichTrack, iAerogel);
          ROOT::Math::XYZVector edirr  = arichTrack.getDirection();
          ROOT::Math::XYZVector photonDirection; // calculated photon direction

          if (CherenkovPhoton(epoint, virthitpos, initialrf, photonDirection, &m_refractiveInd[iAerogel], &m_zaero[iAerogel],
                              m_nAerogelLayers - iAerogel, mirrors[mirr]) < 0) break;

          ROOT::Math::XYZVector dirch = TransformToFixed(edirr) * photonDirection;
          double fi_cer = dirch.Phi();
          double th_cer = dirch.Theta();


          th_cer_all[iAerogel] = th_cer;
          fi_cer_all[iAerogel] = fi_cer;
          auto deltaPhi = dirch.Phi() - edir.Phi();
          if (deltaPhi > M_PI)
            deltaPhi -= 2 * M_PI;
          if (deltaPhi < -M_PI)
            deltaPhi += 2 * M_PI;
          fi_cer_trk = deltaPhi;

          if (mirr == 0 && th_cer < 0.1) reflOK = false;
          // skip photons with irrelevantly large/small Cherenkov angle
          if (th_cer > 0.5 || th_cer < 0.1) continue;

          // count photons with 0.1<thc<0.5
          if (nfoo == nDetPhotons) nDetPhotons++;

          if (fi_cer < 0) fi_cer += 2 * M_PI;
          double fii = fi_cer;
          if (mirr > 0) {
            double fi_mir = m_mirrorNorms[mirrors[mirr] - 1].Phi();
            fii = 2 * fi_mir - fi_cer - M_PI;
          }


          // loop over all particle hypotheses
          for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {

            // track a photon from the mean emission point to the detector surface
            ROOT::Math::XYZVector photonDirection1 = setThetaPhi(thetaCh[iHyp][iAerogel], fi_cer);  // particle system
            photonDirection1 = TransformFromFixed(edirr) * photonDirection1;  // global system
            int ifi = int (fi_cer * 20 / 2. / M_PI);
            ROOT::Math::XYZVector detector_position;

            detector_position = FastTracking(photonDirection1, epoint, &m_refractiveInd[iAerogel], &m_zaero[iAerogel],
                                             m_nAerogelLayers - iAerogel, 0);

            ROOT::Math::XYZVector meanr             = detector_position - epoint;
            double   path              = meanr.R();
            meanr                      = meanr.Unit();

            double meanpath = (m_recPars->getParameters())[2];
            if (iAerogel == 1) meanpath = meanpath - m_thickness[iAerogel];

            double detector_sigma    = thcResolution * meanpath / meanr.Z();
            double wide_sigma = wideGaussSigma * path / meanr.Z();
            // calculate pad orientation and distance relative to that photon
            double modphi =  m_arichgp->getDetectorPlane().getSlotPhi(modID);

            double      pad_fi = fii - modphi;
            double      dx     = (detector_position - hitpos).R();
            double  dr = (track_at_detector - detector_position).R();

            if (dr > 0.01) {
              double normalizacija = nSig_wo_acc[iHyp][iAerogel][ifi] * padSize / (0.1 * M_PI * dr * meanr.Z());
              weight[iHyp][iAerogel] = normalizacija;
              weight_sum[iHyp] += weight[iHyp][iAerogel];
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
              ebgri[iHyp] += m_recPars->getBackgroundPerPad(th_cer_all[1], pars);
            }
            bkgAdded = true;
          }
        }
        // create ARICHPhoton if desired
        if (m_storePhot && th_cer_all[1] > 0 && th_cer_all[1] < 0.6) {
          double n_cos_theta_ch[c_noOfHypotheses] = {0.0};
          double phi_ch[c_noOfHypotheses] = {0.0};
          for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {
            if (weight_sum[iHyp] > 0) {
              for (unsigned int iAerogel = 0; iAerogel < m_nAerogelLayers; iAerogel++) {
                double emission_prob = weight[iHyp][iAerogel] / weight_sum[iHyp];
                n_cos_theta_ch[iHyp] += emission_prob * m_refractiveInd[iAerogel] * cos(th_cer_all[iAerogel]);
                phi_ch[iHyp] += emission_prob * fi_cer_all[iAerogel];
              }
              //std::cout << iHyp << " " <<  n_cos_theta_ch[iHyp] << " " << phi_ch[iHyp] << std::endl;
            } else {
              n_cos_theta_ch[iHyp] = -99999.;
              phi_ch[iHyp] = -99999.;
            }
          }
          ARICHPhoton phot(iPhoton, th_cer_all[1], fi_cer_all[1], mirrors[mirr]); // th_cer of the first aerogel layer assumption is stored
          phot.setBkgExp(ebgri); // store expected number of background hits
          phot.setSigExp(sigExpArr); // store expected number of signal hits
          phot.setPhiCerTrk(fi_cer_trk); // store phi angle in track coordinates
          phot.setNCosThetaCh(n_cos_theta_ch); // store n cos(theta_th) for all particle hypotheses
          phot.setPhiCh(phi_ch); // store phi_ch for all particle hypotheses
          phot.setXY(hitpos.X(), hitpos.Y()); // store x-y hit position
          phot.setModuleID(modID); // store module id
          phot.setChannel(channel); // store channel
          arichTrack.addPhoton(phot);
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
    double exppho[c_noOfHypotheses] = {0.0};
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

  ROOT::Math::XYZVector ARICHReconstruction::getTrackMeanEmissionPosition(const ARICHTrack& track, int iAero)
  {
    // Emission length measured from aerogel exit

    ROOT::Math::XYZVector dir = track.getDirection();
    if (dir.Z() == 0) return ROOT::Math::XYZVector();
    double d   = m_thickness[iAero] / dir.Z() / m_transmissionLen[iAero];
    double dmean = 1 - d / expm1(d);
    //double dmean = -log((1 + exp(-d)) / 2.);
    double mel = dmean * m_transmissionLen[iAero];

    return (getTrackPositionAtZ(track, m_zaero[iAero]) - mel * dir);
  }

  ROOT::Math::XYZVector ARICHReconstruction::getTrackPositionAtZ(const ARICHTrack& track, double zout)
  {
    ROOT::Math::XYZVector dir = track.getDirection();
    ROOT::Math::XYZVector pos = track.getPosition();
    if (dir.Z() == 0) return ROOT::Math::XYZVector(0, 0, 0);
    double path = (zout - pos.Z()) / dir.Z();
    return pos + dir * path;
  }

  void ARICHReconstruction::transformTrackToLocal(ARICHTrack& arichTrack, bool align)
  {
    // tranform track from Belle II to local ARICH frame
    ROOT::Math::XYZVector locPos = m_arichgp->getMasterVolume().pointToLocal(arichTrack.getPosition());
    ROOT::Math::XYZVector locDir = m_arichgp->getMasterVolume().momentumToLocal(arichTrack.getDirection());

    // apply the alignment correction
    if (align && m_alignp.isValid()) {
      // apply global alignment correction
      locPos = m_alignp->pointToLocal(locPos);
      locDir = m_alignp->momentumToLocal(locDir);
    }

    // set parameters and return
    // is it needed to extrapolate to z of aerogel in local frame?? tabun
    arichTrack.setReconstructedValues(locPos, locDir, arichTrack.getMomentum());
    return;
  }


  ROOT::Math::XYZVector ARICHReconstruction::getMirrorPoint(int mirrorID)
  {

    ROOT::Math::XYZVector mirpoint = m_arichgp->getMirrors().getPoint(mirrorID);
    if (m_alignMirrors && m_mirrAlign.isValid()) mirpoint += m_mirrAlign->getAlignmentElement(mirrorID).getTranslation();
    return mirpoint;

  }

  ROOT::Math::XYZVector ARICHReconstruction::getMirrorNorm(int mirrorID)
  {
    if (m_alignMirrors && m_mirrAlign.isValid()) {

      ROOT::Math::XYZVector mirnorm = m_arichgp->getMirrors().getNormVector(mirrorID);

      VectorUtil::setMagThetaPhi(mirnorm,
                                 mirnorm.R(),
                                 mirnorm.Theta() + m_mirrAlign->getAlignmentElement(mirrorID).getAlpha(),
                                 mirnorm.Phi() + m_mirrAlign->getAlignmentElement(mirrorID).getBeta());

      return mirnorm;

    }
    return m_arichgp->getMirrors().getNormVector(mirrorID);
  }

  void ARICHReconstruction::correctEmissionPoint(int tileID, double r)
  {

    double ang = m_tilePars[tileID - 1][0] + m_tilePars[tileID - 1][1] * r;
    m_zaero[0] = m_arichgp->getAerogelPlane().getAerogelZPosition() + m_thickness[0] - ang * 50.;
    m_zaero[1] = m_zaero[0] +  m_thickness[1];

  }

}
