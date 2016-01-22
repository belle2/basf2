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
#include "arich/modules/arichReconstruction/Utility.h"
#include "arich/geometry/ARICHGeometryPar.h"
#include "arich/geometry/ARICHBtestGeometryPar.h"
#include "arich/dataobjects/ARICHDigit.h"
#include "arich/dataobjects/ARICHTrack.h"

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

using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace arich;

  ARICHReconstruction::ARICHReconstruction(int storePhot, int beamtest):
    m_arichGeoParameters(ARICHGeometryPar::Instance()),
    m_beamtest(beamtest),
    m_bkgLevel(0),
    m_trackPosRes(0),
    m_trackAngRes(0),
    m_singleRes(0),
    m_aeroMerit(0),
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

    if (!m_arichGeoParameters->isInit()) {
      GearDir content("/Detector/DetectorComponent[@name='ARICH']/Content");
      m_arichGeoParameters->Initialize(content);
    }
    if (!m_arichGeoParameters->isInit()) {
      B2ERROR("Component ARICH not found in Gearbox");
      return;
    }

    for (const auto& part : Const::chargedStableSet) {
      p_mass[part.getIndex()] = part.getMass();
    }

    m_nAerogelLayers = m_arichGeoParameters->getNumberOfAerogelRadiators();

    m_thickness[m_nAerogelLayers] = 0;
    for (unsigned int i = 0; i < m_nAerogelLayers; i++) {
      m_refractiveInd[i] = m_arichGeoParameters->getAerogelRefIndex(i);
      m_anorm[i] = TVector3(0, 0, 1);
      m_thickness[i] = m_arichGeoParameters->getAerogelThickness(i);
      m_zaero[i] = m_arichGeoParameters->getAerogelZPosition(i) + m_thickness[i];
      m_transmissionLen[i] = m_arichGeoParameters->getAerogelTransmissionLength(i) ; // aerogel transmission length;

      // measured FOM; aeroMerit is number of detected photons for beam of beta=1 and perpedicular incidence to aerogel tile
      // (corrected for geometrical acceptance). n0[i] is then calculated from
      // aeroMerit[i]=n0[i]*sin2(thc)*transmissionLen[i] * (1 - exp(-thickness[i] / transmissionLen[i])
      m_n0[i] = m_aeroMerit[i] / ((1. - 1. / m_refractiveInd[i]) * m_transmissionLen[i] * (1 - exp(
                                    -m_thickness[i] / m_transmissionLen[i])));
      m_thickness[m_nAerogelLayers]   += m_thickness[i];
    }
    m_refractiveInd[m_nAerogelLayers  ]   = 1.0;
    m_refractiveInd[m_nAerogelLayers + 1]   = m_arichGeoParameters->getDetectorWindowRefIndex();
    m_zaero[m_nAerogelLayers  ] = m_arichGeoParameters->getDetectorZPosition();
    m_zaero[m_nAerogelLayers + 1] = m_zaero[m_nAerogelLayers] + m_arichGeoParameters->getDetectorWindowThickness();

  }


  int ARICHReconstruction::InsideDetector(TVector3 a, int copyno)
  {
    if (copyno == -1) return 0;
    TVector3 origin = m_arichGeoParameters->getOrigin(copyno);
    TVector2 origin2(origin.X(), origin.Y());
    TVector2 a2(a.X(), a.Y());
    double phi = m_arichGeoParameters->getModAngle(copyno);
    TVector2 diff = a2 - origin2;
    diff = diff.Rotate(-phi);
    const double size = m_arichGeoParameters->getSensitiveSurfaceSize();
    if (fabs(diff.X()) < size / 2. && fabs(diff.Y()) < size / 2.) {
      return 1;
    }
    return 0;
  }


  int ARICHReconstruction::smearTrack(ARICHTrack& arichTrack)
  {
    TRotation rot;  TVector3  rc; TVector3 rrel; TVector3 offset;
    if (m_beamtest % 2 == 0 && m_beamtest != 0) {
      static ARICHBtestGeometryPar* _arichbtgp = ARICHBtestGeometryPar::Instance();
      rot  =  _arichbtgp->getFrameRotation();
      rc   =  _arichbtgp->getRotationCenter();
      offset = _arichbtgp->getOffset();
      rrel  =  rc - rot * rc;
    }

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
    if (m_beamtest % 2 == 0 && m_beamtest != 0) {
      rdir = rot * rdir;
      rpoint = rot * rpoint + rrel - (rc + offset);
    }
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

    int tileID = m_arichGeoParameters->getAerogelTileID(r.XYvector());
    if (tileID == 0 && opt == 1) return TVector3();

    int nmir = m_arichGeoParameters->getNMirrors();
    if (nmir > 0) {
      rmir = m_arichGeoParameters->getMirrorPoint(0).XYvector().Mod();
      angmir = m_arichGeoParameters->getMirrorsStartAngle();
      double dangle = 2 * M_PI / nmir;
      double trkangle = r.XYvector().Phi() - angmir;
      if (trkangle < 0) trkangle += 2 * M_PI;
      section[1]  = int(trkangle / dangle);
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
        if (m_arichGeoParameters->getAerogelTileID(r.XYvector()) != tileID) return TVector3();
      }
      r += dirf * path;
      TVector2 rxy = r.XYvector();
      // check for possible reflections
      if (a != n || rxy.Mod() < rmir || nmir == 0) continue;
      double angle = rxy.Phi() - angmir;
      if (angle < 0) angle += 2 * M_PI;
      double dangle = 2 * M_PI / nmir;
      section[0] = int(angle / dangle);
      if (r.Mag() > (r - 2 * m_arichGeoParameters->getMirrorPoint(section[0])).Mag()) {
        refl = true;
        for (int k = 0; k < 2; k++) {
          TVector3 mirpoint = m_arichGeoParameters->getMirrorPoint(section[k]);
          TVector3 mirnorm = m_arichGeoParameters->getMirrorNormal(section[k]);
          double s = dirf * mirnorm;
          double s1 = (mirpoint - r0) * mirnorm;
          r = r0 + s1 / s * dirf;
          if (r.Z() < m_arichGeoParameters->getMirrorsZPosition()) return TVector3();
          if (fabs(r.XYvector().DeltaPhi(mirnorm.XYvector())) > double(M_PI / nmir)) { r = r0; continue;}
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

    if (mirrorID == -1) return hitpos;
    TVector3 mirpoint = m_arichGeoParameters->getMirrorPoint(mirrorID);
    TVector3 mirnorm = m_arichGeoParameters->getMirrorNormal(mirrorID);
    return hitpos - 2 * ((hitpos - mirpoint) * mirnorm) * mirnorm;
  }


  int ARICHReconstruction::CherenkovPhoton(TVector3 r, TVector3 rh,
                                           TVector3& rf, TVector3& dirf,
                                           double* refractiveInd, double* z, int n)
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

    double dwin    = m_arichGeoParameters->getDetectorWindowThickness();
    double refractiveInd0 = m_arichGeoParameters->getDetectorWindowRefIndex();

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
        rf = rf0[1];
        dirf = dirf0[0];
        return iter;
      }
    }
    return -1;
  }


  int ARICHReconstruction::likelihood2(ARICHTrack& arichTrack, StoreArray<ARICHDigit>& arichDigits, ARICHLikelihood& arichLikelihood)
  {

    const unsigned int nPhotonHits = arichDigits.getEntries(); // number of detected photons

    if (m_nAerogelLayers + 1 > c_noOfAerogels) B2ERROR("ARICHReconstrucion: number of aerogel layers defined in the xml file exceeds "
                                                         << c_noOfAerogels);

    double  logL[c_noOfHypotheses] = {0.0};
    double  nBgr[c_noOfHypotheses] = {0.0};
    double  nSig_w_acc[c_noOfHypotheses][c_noOfAerogels] = { {0.0} }; // expected no. of signal photons, including geometrical acceptance
    double  nSig_wo_acc[c_noOfHypotheses][c_noOfAerogels][20] = { { {0.0} } }; // expected no. of signal photons, without geometrical acceptance, divided in 20 phi bins (used for PDF normalization)
    double  esigi[c_noOfHypotheses] = {0.0}; // expected number of signal photons in hit pixel
    double  thetaCh[c_noOfHypotheses][c_noOfAerogels] = { {0.0} }; // expected Cherenkov angle

    // read some geometry parameters
    double padSize = m_arichGeoParameters->getDetectorPadSize();
    double padArea = padSize / Unit::m * padSize / Unit::m;
    //int padNum = m_arichGeoParameters->getDetectorXPadNumber() * m_arichGeoParameters->getDetectorXPadNumber();
    int nMirSeg = m_arichGeoParameters->getNMirrors();
    double angmir  = m_arichGeoParameters->getMirrorsStartAngle();

    // Detected photons within expected cherenkov ring (within +/-3 sigma)
    int nDetPhotons[c_noOfHypotheses] = {0};

    // reconstructed track direction
    TVector3 edir = arichTrack.getDirection();


    //------------------------------------------------------
    // Calculate number of expected detected photons (emmited x geometrical acceptance).
    // -----------------------------------------------------

    float nphot_scaling = 20.;
    int nStep = 5;

    // loop over all particle hypotheses
    for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {

      double abs = 1;

      // loop over aerogel layers
      for (int iAerogel = m_nAerogelLayers - 1; iAerogel >= 0; iAerogel--) {

        thetaCh[iHyp][iAerogel] = ExpectedCherenkovAngle(arichTrack.getMomentum(),
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
          unsigned int genPhot = nPhot * abs; // number of photons to emmit in current step, including absorbtion correction

          // loop over emmited "photons"
          for (unsigned int iPhoton = 0; iPhoton < genPhot; iPhoton++) {
            double fi = 2 * M_PI * iPhoton / float(genPhot);
            TVector3 adirf = setThetaPhi(thetaCh[iHyp][iAerogel], fi); // particle system
            adirf =  TransformFromFixed(edir) * adirf;  // global system
            int ifi = int (fi * 20 / 2. / M_PI);

            if (!m_arichGeoParameters->isSimple()) {
              TVector3 dposition = FastTracking(adirf, epoint, &m_refractiveInd[iAerogel], &m_zaero[iAerogel], m_nAerogelLayers - iAerogel, 1);
              int copyno =  m_arichGeoParameters->getCopyNo(dposition);
              if (dposition.Mag() > 1.0) nSig_wo_acc[iHyp][iAerogel][ifi] += 1;
              if (InsideDetector(dposition, copyno)) nSig_w_acc[iHyp][iAerogel] += 1;

            } else {

              TVector3 dposition = FastTrackingSimple(adirf, epoint, &m_refractiveInd[iAerogel], &m_zaero[iAerogel], m_nAerogelLayers - iAerogel);
              for (int ik = 1; ik <= m_arichGeoParameters->getNMCopies(); ik++) {
                if (InsideDetector(dposition, ik)) {
                  nSig_w_acc[iHyp][iAerogel] += 1;
                  break;
                }
              }
            }
          }
        }

        // scale the obtained numbers
        for (int ik = 0; ik < 20; ik++) {
          nSig_wo_acc[iHyp][iAerogel][ik] /= nphot_scaling;
        }
        nSig_w_acc[iHyp][iAerogel] /= nphot_scaling;

      } // for (unsigned int iAerogel=0;iAerogel<m_nAerogelLayers;iAerogel++)

      // sum up contribution from all aerogel layers
      for (unsigned int iAerogel = 0; iAerogel < m_nAerogelLayers; iAerogel++) {
        nSig_w_acc[iHyp][m_nAerogelLayers] += nSig_w_acc[iHyp][iAerogel];
        for (int ik = 0; ik < 20; ik++) {
          nSig_wo_acc[iHyp][m_nAerogelLayers][ik] += nSig_wo_acc[iHyp][iAerogel][ik];
        }
      }

      // implement method to calculate expected number of background hits (for now set to 0, same for all particle hypotheses)
      nBgr[iHyp] = 0; //m_bkgLevel * padArea * padNum * m_arichGeoParameters->getNMCopies();

    }  // for (int iHyp=0;iHyp < c_noOfHypotheses; iHyp++ )
    //#####################################################


    double ebgri = m_bkgLevel * padArea;
    TVector3 track_at_detector = getTrackPositionAtZ(arichTrack, m_zaero[m_nAerogelLayers + 1]);

    // the id numbers of mirrors from which the photons could possibly reflect are calculated
    int mirrors[3];
    mirrors[0] = -1; // for no reflection
    int refl = 1;

    if (m_arichGeoParameters->isSimple()) {refl += nMirSeg; mirrors[1] = 0; mirrors[2] = 1;} else {
      // only if particle track on detector is at radius larger than 850mm (for now hardcoded)
      // possible reflections are taken into account.
      if (track_at_detector.XYvector().Mod() > 85.0) {
        double trackang = track_at_detector.Phi() - angmir + M_PI / double(nMirSeg);
        if (trackang < 0) trackang += 2 * M_PI;
        int section1 = int(trackang / 2 / M_PI * nMirSeg) % nMirSeg;
        int section2 = section1 - 1;
        if (section1 == 0)  section2 = nMirSeg - 1;
        mirrors[1] = section1; mirrors[2] = section2;
        refl = 3;
      }
    }

    // loop over all detected photon hits

    for (unsigned int iPhoton = 0; iPhoton < nPhotonHits; iPhoton++) {

      ARICHDigit* h = arichDigits[iPhoton];
      int chID = h->getChannelID();
      int modID = h->getModuleID();

      TVector3 hitpos = m_arichGeoParameters->getChannelCenterGlob(modID, chID);

      int nfoo[c_noOfHypotheses];
      for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) { esigi[iHyp] = 0; nfoo[iHyp] = nDetPhotons[iHyp];}

      // loop over all arogel layers
      for (unsigned int iAerogel = 0; iAerogel < m_nAerogelLayers; iAerogel++) {

        TVector3 initialrf = getTrackPositionAtZ(arichTrack, m_zaero[iAerogel]);
        TVector3 epoint = getTrackMeanEmissionPosition(arichTrack, iAerogel);
        TVector3 edir  = arichTrack.getDirection();

        TVector3 photonDirection; // calculated photon direction

        // loop over possible mirror reflections
        for (int mirr = 0; mirr < refl; mirr++) {
          // calculate fi_ch for a given track refl
          TVector3 virthitpos =  HitVirtualPosition(hitpos, mirrors[mirr]);

          // if hit is more than 15cm from the track position on the detector plane, skip it.
          // (not reconstructing hits with irrelevantly large Cherenkov angle)
          if ((track_at_detector - virthitpos).Mag() > 15.0) continue;

          if (CherenkovPhoton(epoint, virthitpos, initialrf, photonDirection, &m_refractiveInd[iAerogel], &m_zaero[iAerogel],
                              m_nAerogelLayers - iAerogel) < 0)  continue;

          TVector3 dirch = TransformToFixed(edir) * photonDirection;
          double fi_cer = dirch.Phi();
          double th_cer = dirch.Theta();

          if (th_cer > 0.5) continue;

          // add photon to ARICHTrack
          if (m_storePhot) arichTrack.addPhoton(th_cer, fi_cer, iAerogel, mirr);

          // if beamtest analysis skip likelihood calculation
          if (m_beamtest) continue;

          if (fi_cer < 0) fi_cer += 2 * M_PI;
          double fii = fi_cer;
          if (mirr > 0) {
            double fi_mir = m_arichGeoParameters->getMirrorNormal(mirrors[mirr]).XYvector().Phi();
            fii = 2 * fi_mir - fi_cer - M_PI;
          }

          // loop over all particle hypotheses
          for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {

            // if reconstruced cherekov angle is within the expected cherenkov ring (+/-3 sigma) add it to number of detected photons
            if (fabs(th_cer - thetaCh[iHyp][iAerogel]) < 0.042 && nfoo[iHyp] == nDetPhotons[iHyp] && th_cer > 0.07) nDetPhotons[iHyp]++;

            // track a photon from the mean emission point to the detector surface
            TVector3  photonDirection1 = setThetaPhi(thetaCh[iHyp][iAerogel], fi_cer);  // particle system
            photonDirection1 = TransformFromFixed(edir) * photonDirection1;  // global system
            int ifi = int (fi_cer * 20 / 2. / M_PI);
            TVector3  photonAtAerogelExit = photonDirection1 * (m_thickness[iAerogel] / photonDirection1.z());
            TVector3  trackAtAerogelExit = edir * (m_thickness[iAerogel] / edir.z());
            TVector3  dtrackphoton = photonAtAerogelExit - trackAtAerogelExit;
            TVector3 detector_position;
            if (!m_arichGeoParameters->isSimple()) {
              detector_position = FastTracking(photonDirection1, epoint, &m_refractiveInd[iAerogel], &m_zaero[iAerogel],
                                               m_nAerogelLayers - iAerogel, 0);
            } else {
              detector_position = FastTrackingSimple(photonDirection1, epoint, &m_refractiveInd[iAerogel], &m_zaero[iAerogel],
                                                     m_nAerogelLayers - iAerogel);
            }
            TVector3 meanr             = detector_position - epoint;
            double   path              = meanr.Mag();
            meanr                      = meanr.Unit();

            double   detector_sigma    = m_singleRes * path / meanr.z();

            // calculate pad orientation and distance relative to that photon
            TVector3 modorigin = m_arichGeoParameters->getOrigin(modID);
            double modphi =  m_arichGeoParameters->getModAngle(modID);

            double      pad_fi = fii - modphi;

            double      dx     = (detector_position - hitpos).Mag();
            // calculate pad acceptance
            double  dr = (track_at_detector - detector_position).Mag();

            if (dr > 0 && thetaCh[iHyp][iAerogel]) {
              double padSizemm = padSize / Unit::mm;
              double normalizacija = nSig_wo_acc[iHyp][iAerogel][ifi] * padSizemm / (0.1 * M_PI * dr / Unit::mm);
              double integral = SquareInt(padSizemm, pad_fi, dx / Unit::mm, detector_sigma * 10.) / sqrt(2.);
              // expected number of signal photons in each pixel
              esigi[iHyp] += normalizacija * integral;
            } // if (dr>0 && thetaCh[iHyp][iAerogel])

          }// for (int iHyp=0;iHyp< c_noOfHypotheses; iHyp++)

        }// for (int mirr = 0; mirr < refl; mirr++)

      }// for (unsigned int iAerogel=0; iAerogel<m_nAerogelLayers;iAerogel++)

      //******************************************
      // LIKELIHOOD construction
      //*******************************************

      for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {
        double expected = esigi[iHyp] + ebgri;
        logL[iHyp] += expected + log(1 - exp(-expected));
      }

    } // for (unsigned  int iPhoton=0; iPhoton< nPhotonHits; iPhoton++)

    //*********************************************
    // add constant term to the LIKELIHOOD function
    //*********************************************

    for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {
      logL[iHyp] -= (nSig_w_acc[iHyp][m_nAerogelLayers] + nBgr[iHyp]);
    }

    //******************************************
    // store LikeliHOOD info
    //******************************************
    int flag = 1;
    double exppho[6] = {0.0};
    for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {
      exppho[iHyp] = nSig_w_acc[iHyp][m_nAerogelLayers];
      if ((thetaCh[iHyp][0] > 0 || thetaCh[iHyp][1] > 0) &&  nSig_w_acc[iHyp][m_nAerogelLayers] == 0) flag = 0;
    }

    // set values of ARICHLikelihood
    arichLikelihood.setValues(flag, logL, nDetPhotons, exppho);

    return 0;
  }

  void ARICHReconstruction::setBackgroundLevel(double bkgLevel)
  {
    m_bkgLevel = bkgLevel;
  }
  void ARICHReconstruction::setTrackPositionResolution(double pRes)
  {
    m_trackPosRes = pRes;
  }
  void ARICHReconstruction::setTrackAngleResolution(double aRes)
  {
    m_trackAngRes = aRes;
  }
  void ARICHReconstruction::setSinglePhotonResolution(double sRes)
  {
    m_singleRes = sRes;
  }
  void ARICHReconstruction::setAerogelFigureOfMerit(std::vector<double>& merit)
  {
    m_aeroMerit = merit;
  }

  TVector3 ARICHReconstruction::FastTrackingSimple(TVector3 dirf, TVector3 r,  double* refractiveInd, double* z, int n)
  {
    //
    // Description:
    // The method calculates the intersection  of the cherenkov photon
    // with the detector plane

    //  z[n+1]
    //  z[0] .. 1st aerogel exit
    //  z[n-1] .. 2nd aerogel exit
    //  z[n-1] .. 2nd aerogel exit

    int nmir = m_arichGeoParameters->getNMirrors();
    double path = 0;
    bool reflok = false; bool refl = false;
    path = (z[0] - r.z()) / dirf.z();
    r   += dirf * path;
    for (int a = 1; a <= n + 1 ; a++) {
      double rind = refractiveInd[a] / refractiveInd[a - 1];
      dirf = Refraction(dirf, rind);
      if (dirf.Mag() == 0) return TVector3();
      path = (z[a] - r.z()) / dirf.z();
      TVector3 r0 = r;
      r += dirf * path;
      TVector2 rxy = r.XYvector();
      // check for possible reflections
      if (a != n) continue;
      for (int k = 0; k < nmir; k++) {
        TVector3 mirpoint = m_arichGeoParameters->getMirrorPoint(k);
        TVector3 mirnorm = m_arichGeoParameters->getMirrorNormal(k);
        TVector2 dr = rxy - mirpoint.XYvector();
        if (dr.X()*mirnorm.X() + dr.Y()*mirnorm.Y() > 0) {
          refl = true;
          double s = dirf * mirnorm;
          double s1 = (mirpoint - r0) * mirnorm;
          r = r0 + s1 / s * dirf;
          if (r.Z() < m_arichGeoParameters->getMirrorsZPosition()) return TVector3();
          if ((r.XYvector() - mirpoint.XYvector()).Mod() > 20) { r = r0; continue;}
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

  TVector3 ARICHReconstruction::getTrackMeanEmissionPosition(const ARICHTrack& track, int iAero)
  {
    // Emission length measured from aerogel exit

    TVector3 dir = track.getDirection();
    if (dir.Z() == 0) return TVector3();
    double d   = m_thickness[iAero] / dir.Z() / m_transmissionLen[iAero];
    double dmean = 1 - d / expm1(d);
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

}
