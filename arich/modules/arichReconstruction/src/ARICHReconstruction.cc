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
#include "arich/dataobjects/ARICHDigit.h"
#include <arich/modules/arichReconstruction/ARICHTrack.h>

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


using namespace std;
using namespace boost;

namespace Belle2 {
  namespace arich {

    ARICHReconstruction::ARICHReconstruction(int beamtest):
      m_arichGeoParameters(ARICHGeometryPar::Instance()),
      m_beamtest(beamtest),
      m_bkgLevel(0),
      m_trackPosRes(0),
      m_trackAngRes(0),
      m_singleRes(0),
      m_aeroMerit(0)
    {
      B2INFO("ARICHReconstruction::ARICHReconstruction()");
      if (m_beamtest) {
        m_hitstuple = new TNtuple("hits", "Btest Cherenkov angle", "n:agel:mir:thc:fic:x:y:z:tx:ty:tz:sx:sy");
        m_tracktuple = new TNtuple("tracks", "Btest tracks", "id:p:nexp:acc:ndet:le:lmu:lpi:lk:lp");
      }
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


    int ARICHReconstruction::smearTracks(std::vector<ARICHTrack>& arichTracks)
    {
      unsigned int nTracks = arichTracks.size();

      for (unsigned  int i = 0; i < nTracks; i++) {
        ARICHTrack* track = &arichTracks[i];
        double thc = gRandom->Gaus(0, m_trackAngRes);
        double fic = gRandom->Uniform(2 * M_PI);
        TVector3 dirf = setThetaPhi(thc, fic);  // particle system
        double dr = gRandom->Gaus(0, m_trackPosRes);
        double dfi = gRandom->Uniform(2 * M_PI);
        double dx = dr * cos(dfi);
        double dy = dr * sin(dfi);
        TVector3 mod = TVector3(dx, dy, 0);
        TVector3 rpoint = track->getOriginalPosition() + mod;
        TVector3 odir  = track->getOriginalDirection();
        double omomentum  = track->getOriginalMomentum();
        TVector3 rdir = TransformFromFixed(odir) * dirf;  // global system
        double rmomentum = omomentum;
        track->setReconstructedValues(rpoint, rdir, rmomentum);
      }
      return 0;
    }


    TVector3 ARICHReconstruction::FastTracking(TVector3 dirf, TVector3 r,  double* refractiveInd, double* z, int n)
    {
      //
      // Description:
      // The method calculates the intersection  of the cherenkov photon
      // with the detector plane

      //  z[n+1]
      //  z[0] .. 1st aerogel exit
      //  z[n-1] .. 2nd aerogel exit
      //  z[n-1] .. 2nd aerogel exit

      double rmir = 0; double angmir = 0; int section[2] = {0, 0};

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

    TVector3 ARICHReconstruction::HitVirtualPosition(const TVector3 hitpos, int mirrorID)
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


    int ARICHReconstruction::likelihood2(std::vector<ARICHTrack>& arichTracks)
    {
      static int ncount = 0;
      ncount++;
      //const double p_mass[5] = { 0.000511, 0.10566, 0.13957, 0.49368, 0.93827};// mass of particles in GeV
      const double p_mass[5] = {Const::electron.getMass(), Const::muon.getMass(), Const::pion.getMass(), Const::kaon.getMass(), Const::proton.getMass()};

      const unsigned int nTracks = arichTracks.size();
      StoreArray<ARICHDigit> arichDigits;
      const unsigned int nPhotonHits = arichDigits.getEntries(); // detected photons

      if (nTracks == 0) return 0;

      const unsigned int nAerogelLayers = m_arichGeoParameters->getNumberOfAerogelRadiators();

      static double  refractiveInd[c_noOfAerogels];
      static double  zaero[c_noOfAerogels];
      static double  thickness[c_noOfAerogels];
      static double  transmissionLen[c_noOfAerogels];
      static double  n0[c_noOfAerogels]; //const factor in dN/dE formula
      static TVector3 anorm[c_noOfAerogels];
      static double padSize;
      static double angmir;
      static int nMirSeg;
      static int first = 1;

      double  pathLengthRadiator[c_noOfAerogels];
      double  acceptance[c_noOfHypotheses][c_noOfAerogels];
      double  logL[c_noOfHypotheses];
      double  nBgr[c_noOfHypotheses];
      double  nSig[c_noOfAerogels][c_noOfHypotheses]; // expected no. of photons
      double  esigi[c_noOfHypotheses]; // expected number of signal photons in each pixel
      double  thetaCh[c_noOfHypotheses][c_noOfAerogels]; // expected Ch. angle

      if (first) {

        padSize = m_arichGeoParameters->getDetectorPadSize();
        nMirSeg = m_arichGeoParameters->getNMirrors();
        angmir  = m_arichGeoParameters->getMirrorsStartAngle();
        thickness[nAerogelLayers] = 0;
        for (unsigned int i = 0; i < nAerogelLayers; i++) {
          refractiveInd[i] = m_arichGeoParameters->getAerogelRefIndex(i);
          anorm[i] = TVector3(0, 0, 1);
          thickness[i] = m_arichGeoParameters->getAerogelThickness(i);
          zaero[i] = m_arichGeoParameters->getAerogelZPosition(i) + thickness[i];
          transmissionLen[i] = m_arichGeoParameters->getAerogelTransmissionLength(i) ; // aerogel transmission length;
          // measured FOM
          n0[i] = m_aeroMerit[i] / (0.1516 * transmissionLen[i] * (1 - exp(-thickness[i] / transmissionLen[i])));
          thickness[nAerogelLayers]   += thickness[i];
        }
        refractiveInd[nAerogelLayers  ]   = 1.0;
        refractiveInd[nAerogelLayers + 1]   = m_arichGeoParameters->getDetectorWindowRefIndex();
        zaero[nAerogelLayers  ] = m_arichGeoParameters->getDetectorZPosition();
        zaero[nAerogelLayers + 1] = zaero[nAerogelLayers] + m_arichGeoParameters->getDetectorWindowThickness();
        first = 0;
      }

      // loop over all tracks
      for (unsigned  int i = 0; i < nTracks; i++) {

        ARICHTrack* track =  &arichTracks[i];
        // Reconstructed photons within expected cone
        int nDetPhotons = 0;
        double padArea = padSize / Unit::m * padSize / Unit::m;
        int padNum = m_arichGeoParameters->getDetectorXPadNumber() * m_arichGeoParameters->getDetectorXPadNumber();

        // loop over all particle hypotheses
        for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {
          logL[iHyp] = 0;

          // loop over aerogel layers
          for (unsigned int iAerogel = 0; iAerogel < nAerogelLayers; iAerogel++) {
            // track length in the radiator
            pathLengthRadiator[iAerogel] = track->getMeanEmissionDirection(iAerogel) * anorm[iAerogel];
            if (pathLengthRadiator[iAerogel]) {
              pathLengthRadiator[iAerogel] = thickness[iAerogel] / pathLengthRadiator[iAerogel];
            }
            thetaCh[iHyp][iAerogel] = ExpectedCherenkovAngle(track->getReconstructedMomentum(),
                                                             p_mass[iHyp],
                                                             refractiveInd[iAerogel]);

            //----------------------------------
            // acceptance calculation
            //----------------------------------

            acceptance[iHyp][iAerogel] = 0;
            const unsigned int c_maxPhotonsCone = 100; // number of photons emitted in cone to calculate the geometrical acceptance

            for (unsigned int iPhoton = 0; iPhoton < c_maxPhotonsCone; iPhoton++) {
              double fi = 2 * M_PI * iPhoton / float(c_maxPhotonsCone);
              TVector3 epoint = track->getMeanEmissionPosition(iAerogel);
              TVector3 edir  = track->getMeanEmissionDirection(iAerogel);
              TVector3 adirf = setThetaPhi(thetaCh[iHyp][iAerogel], fi); // particle system
              adirf =  TransformFromFixed(edir) * adirf;  // global system

              if (!m_arichGeoParameters->isSimple()) {
                TVector3 dposition = FastTracking(adirf, epoint, &refractiveInd[iAerogel], &zaero[iAerogel], nAerogelLayers - iAerogel);
                int copyno =  m_arichGeoParameters->getCopyNo(dposition);
                if (InsideDetector(dposition, copyno)) acceptance[iHyp][iAerogel] += 1;
              } else {
                TVector3 dposition = FastTrackingSimple(adirf, epoint, &refractiveInd[iAerogel], &zaero[iAerogel], nAerogelLayers - iAerogel);
                for (int i = 1; i <= m_arichGeoParameters->getNMCopies(); i++) {
                  if (InsideDetector(dposition, i)) {
                    acceptance[iHyp][iAerogel] += 1;
                    break;
                  }
                }
              }
            }

            acceptance[iHyp][iAerogel] /= float(c_maxPhotonsCone);

            //----------------------------------
            // expected number of photons
            //----------------------------------

            nSig[iHyp][iAerogel] = n0[iAerogel] * transmissionLen[iAerogel] *
                                   sin(thetaCh[iHyp][iAerogel]) * sin(thetaCh[iHyp][iAerogel]) *
                                   (1 - exp(-pathLengthRadiator[iAerogel] / transmissionLen[iAerogel])) *
                                   acceptance[iHyp][iAerogel];

            if (iAerogel > 0)  {
              nSig[iHyp][iAerogel - 1] *= exp(-pathLengthRadiator[iAerogel] / transmissionLen[iAerogel]);
            }

          } // for (unsigned int iAerogel=0;iAerogel<nAerogelLayers;iAerogel++)
          //*********************************************************

          nSig[iHyp][nAerogelLayers] = 0;
          thetaCh[iHyp][nAerogelLayers] = thetaCh[iHyp][0];

          for (unsigned int iAerogel = 0; iAerogel < nAerogelLayers; iAerogel++) nSig[iHyp][nAerogelLayers] += nSig[iHyp][iAerogel];

          nBgr[iHyp] = m_bkgLevel * padArea * padNum * m_arichGeoParameters->getNMCopies();

        }  // for (int iHyp=0;iHyp < c_noOfHypotheses; iHyp++ )
        //#####################################################

        double ebgri = m_bkgLevel * padArea;
        TVector3 track_at_detector = track->getPositionAtZ(zaero[nAerogelLayers + 1]);

        // the id numbers of mirrors from which the photons could possibly reflect are calculated
        int mirrors[3];
        mirrors[0] = -1; // for no reflection
        int refl = 1;

        if (m_arichGeoParameters->isSimple()) {refl += nMirSeg; mirrors[1] = 0; mirrors[2] = 1;} else {
          // only if particle track on detector is at radius larger than 850mm (for now hardcoded)
          // possible reflections are taken into account.
          if (track_at_detector.XYvector().Mod() > 85) {
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
          for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) esigi[iHyp] = 0;
          int nfoo = nDetPhotons;
          // loop over all arogel layers
          for (unsigned int iAerogel = 0; iAerogel < nAerogelLayers; iAerogel++) {

            TVector3 initialrf = track->getAerogelExit(iAerogel);
            TVector3 epoint = track->getMeanEmissionPosition(iAerogel);
            TVector3 edir  = track->getMeanEmissionDirection(iAerogel);

            TVector3 photonDirection; // calculated photon direction

            // loop over possible mirror reflections
            for (int mirr = 0; mirr < refl; mirr++) {
              // calculate fi_ch for a given track refl
              TVector3 virthitpos =  HitVirtualPosition(hitpos, mirrors[mirr]);

              if (CherenkovPhoton(epoint, virthitpos, initialrf, photonDirection, &refractiveInd[iAerogel], &zaero[iAerogel], nAerogelLayers - iAerogel) < 0)  continue;

              TVector3 dirch = TransformToFixed(edir) * photonDirection;
              double fi_cer = dirch.Phi();
              double th_cer = dirch.Theta();
              if (th_cer > 0.5 || th_cer < 0.05) continue;

              if (m_beamtest) m_hitstuple->Fill(ncount, iAerogel, mirr, th_cer, fi_cer, hitpos.x(), hitpos.y(), hitpos.z(), epoint.x(), epoint.y(), epoint.z() , edir.x(), edir.y());
              if (m_beamtest > 2) continue;
              if (fabs(th_cer - thetaCh[track->getIdentity()][0]) < 0.042 && iAerogel == 0 && nfoo == nDetPhotons && th_cer > 0.07) nDetPhotons++;
              if (fi_cer < 0) fi_cer += 2 * M_PI;
              double fii = fi_cer;
              if (mirr > 0) {
                double fi_mir = m_arichGeoParameters->getMirrorNormal(mirrors[mirr]).XYvector().Phi();
                fii = 2 * fi_mir - fi_cer - M_PI;
              }

              // loop over all particle hypotheses
              for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {

                // track a photon from the mean emission point to the detector surface
                TVector3  photonDirection1 = setThetaPhi(thetaCh[iHyp][iAerogel], fi_cer);  // particle system
                photonDirection1 = TransformFromFixed(edir) * photonDirection1;  // global system

                TVector3  photonAtAerogelExit = photonDirection1 * (thickness[iAerogel] / photonDirection1.z());
                TVector3  trackAtAerogelExit = edir * (thickness[iAerogel] / edir.z());
                TVector3  dtrackphoton = photonAtAerogelExit - trackAtAerogelExit;
                TVector3 detector_position;
                if (!m_arichGeoParameters->isSimple()) {
                  detector_position = FastTracking(photonDirection1, epoint, &refractiveInd[iAerogel], &zaero[iAerogel], nAerogelLayers - iAerogel);
                } else {
                  detector_position = FastTrackingSimple(photonDirection1, epoint, &refractiveInd[iAerogel], &zaero[iAerogel], nAerogelLayers - iAerogel);
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
                  double normalizacija = nSig[iHyp][iAerogel] * padSizemm / (2 * M_PI * dr / Unit::mm);
                  double integral = SquareInt(padSizemm, pad_fi, dx / Unit::mm, detector_sigma * 10.) / sqrt(2.);
                  // expected number of signal photons in each pixel
                  esigi[iHyp] += normalizacija * integral;
                }// if (dr>0 && thetaCh[iHyp][iAerogel])

              }// for (int iHyp=0;iHyp< c_noOfHypotheses; iHyp++ )

            }// for (int mirr = 0; mirr < refl; mirr++)

          }// for (unsigned int iAerogel=0; iAerogel<nAerogelLayers;iAerogel++)

          //******************************************
          // LIKELIHOOD construction
          //*******************************************

          for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {
            double expected = esigi[iHyp] + ebgri;
            logL[iHyp] += expected + log(1 - exp(-expected));
          }

          //*******************************************

        }// for (unsigned  int iPhoton=0; iPhoton< nPhotonHits; iPhoton++)

        //******************************************
        // add constant term to the LIKELIHOOD function
        //*******************************************


        for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {
          logL[iHyp] -= (nSig[iHyp][nAerogelLayers] + nBgr[iHyp]);
        }

        //******************************************
        // store LikeliHOOD info
        //******************************************
        for (int iHyp = 0; iHyp < c_noOfHypotheses; iHyp++) {
          track->setExpectedPhotons(iHyp, nSig[iHyp][nAerogelLayers]); // sum for all agels
        }
        track->setDetectedPhotons(nDetPhotons);
        track->setLikelihood(logL);
        //**************************************

        if (m_beamtest) {
          int id = track->getIdentity();
          if (id < 0) id = 0;
          m_tracktuple->Fill(id, track->getReconstructedMomentum(), nSig[2][nAerogelLayers], acceptance[2][0], nDetPhotons, logL[0], logL[1], logL[2], logL[3], logL[4]);
        }

      } // for (unsigned  int i=0; i< nTracks; i++){

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

  }
}


