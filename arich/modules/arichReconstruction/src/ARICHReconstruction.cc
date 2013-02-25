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
#include "arich/modules/arichReconstruction/ARICHTrack.h"

// DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <vector>
#include <TRotation.h>
#include <TRandom3.h>

using namespace std;
using namespace boost;


namespace Belle2 {
  namespace arich {

#define MAXAERO 5 // maximal number of aerogel layers
#define NHYP 5 // number of particle hypotheses



    ARICHReconstruction::ARICHReconstruction():
      _arichgp(ARICHGeometryPar::Instance()), m_bkgLevel(0), m_trackPosRes(0),
      m_trackAngRes(0), m_singleRes(0), m_aeroMerit(0)
    {

    }


    int ARICHReconstruction::InsideDetector(TVector3 a, int copyno)
    {
      if (copyno == -1) return 0;
      TVector3 origin = _arichgp->getOrigin(copyno);
      TVector2 origin2(origin.X(), origin.Y());
      TVector2 a2(a.X(), a.Y());
      double phi = _arichgp->getModAngle(copyno);
      TVector2 diff = a2 - origin2;
      diff = diff.Rotate(-phi);
      const double size = _arichgp->getSensitiveSurfaceSize();
      if (fabs(diff.X()) < size / 2. && fabs(diff.Y()) < size / 2.) {
        return 1;
      }
      return 0;
    }


    int ARICHReconstruction::ReconstructParticles()
    {

      StoreArray<ARICHTrack> arichTracks;
      unsigned int tsize = arichTracks.getEntries();

      for (unsigned  int i = 0; i < tsize; i++) {
        ARICHTrack* track = arichTracks[i];
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


    TVector3 ARICHReconstruction::FastTracking(TVector3 dirf, TVector3 r,  double* refind, double* z, int n)
    {
      //
      // Description:
      // The method calculates the intersection  of the cherenkov photon
      // with the detector plane

      //  z[n+1]
      //  z[0] .. 1st aerogel exit
      //  z[n-1] .. 2nd aerogel exit
      //  z[n-1] .. 2nd aerogel exit

      double rmir = 0; double angmir = 0; int section[2] = {0, 0}; double dangle = 0;
      double trkangle = 0;
      int nmir = _arichgp->getNMirrors();
      if (nmir > 0) {
        rmir = _arichgp->getMirrorPoint(0).XYvector().Mod();
        angmir = _arichgp->getMirrorsStartAngle();
        dangle = 2 * M_PI / nmir;
        trkangle = r.XYvector().Phi() - angmir;
        if (trkangle < 0) trkangle += 2 * M_PI;
        section[1]  = int(trkangle / dangle);
      }
      double path = 0;
      double rind = 0;

      bool reflok = false; bool refl = false;
      path = (z[0] - r.z()) / dirf.z();
      r   += dirf * path;

      for (int a = 1; a <= n + 1 ; a++) {
        rind = refind[a] / refind[a - 1];
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
        if (r.Mag() > (r - 2 * _arichgp->getMirrorPoint(section[0])).Mag()) {
          refl = true;
          for (int k = 0; k < 2; k++) {
            TVector3 mirpoint = _arichgp->getMirrorPoint(section[k]);
            TVector3 mirnorm = _arichgp->getMirrorNormal(section[k]);
            double s = dirf * mirnorm;
            double s1 = (mirpoint - r0) * mirnorm;
            r = r0 + s1 / s * dirf;
            if (r.Z() < _arichgp->getMirrorsZPosition()) return TVector3();
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
      TVector3 mirpoint = _arichgp->getMirrorPoint(mirrorID);
      TVector3 mirnorm = _arichgp->getMirrorNormal(mirrorID);
      return hitpos - 2 * ((hitpos - mirpoint) * mirnorm) * mirnorm;
    }


    int ARICHReconstruction::CherenkovPhoton(TVector3 r, TVector3 rh,
                                             TVector3& rf, TVector3& dirf,
                                             double* refind, double* z, int n)
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

      double dwin    = _arichgp->getDetectorWindowThickness();
      double refind0 = _arichgp->getDetectorWindowRefIndex();

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

      double path = 0;
      double rind = 0;

      for (int iter = 0; iter < niter; iter++) {

        // direction in the space between aerogels and detector
        // *************************************
        if (iter == 0) dirf0[n] = (rh1 - rf0[1]).Unit();
        else  dirf0[n] = (rh1 - rf0[n]).Unit();

        // *************************************
        // n-layers of aerogel // refind relative refractive index
        for (int a = n - 1; a >= 0 ; a--) {
          rind = refind[a] / refind[a + 1];
          dirf0[a] = Refraction(dirf0[a + 1], rind);
        }

        path = (z[0] - r.z()) / dirf0[0].z();
        double x1 = rf0[1].x();
        double y1 = rf0[1].y();
        for (int a = 0; a < n ; a++) {
          rf0[a + 1] = rf0[a] + dirf0[a] * path;
          path = (z[a + 1] - rf0[a + 1].z()) / dirf0[a + 1].z();
        }

        Refraction(dirf0[n], norm, refind0, dirw);

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


    int ARICHReconstruction::Likelihood2()
    {

      const double p_mass[5] = { 0.000511, 0.10566, 0.13957, 0.49368, 0.93827};// mass of particles in GeV

      StoreArray<ARICHTrack> arichTracks;
      unsigned int tsize = arichTracks.getEntries();
      StoreArray<ARICHDigit> arichDigits;
      unsigned int hsize = arichDigits.getEntries();

      if (tsize == 0) return 0;

      unsigned int asize = _arichgp->getNumberOfAerogelRadiators();
      const int maxhyp(5);
      double  esigi[maxhyp];

      static double  refind[MAXAERO];
      static double  zaero[MAXAERO];
      static double  thickness[MAXAERO];
      static double  trlen[MAXAERO];
      static double  n0[MAXAERO];
      static TVector3 anorm[MAXAERO];
      static double pad_size;
      static double angmir;
      static int nMirSeg;
      static int first = 1;

      double  lrad[MAXAERO];
      double  acceptance[NHYP][MAXAERO];
      double  lkh[NHYP];
      double  nbgr[NHYP];
      double  nsig[MAXAERO][NHYP];
      double  thc[NHYP][MAXAERO];

      if (first) {

        pad_size = _arichgp->getDetectorPadSize();
        nMirSeg = _arichgp->getNMirrors();
        angmir  = _arichgp->getMirrorsStartAngle();
        thickness[asize] = 0;
        for (unsigned int i = 0; i < asize; i++) {
          refind[i] = _arichgp->getAerogelRefIndex(i) ;
          anorm[i] = TVector3(0, 0, 1);
          thickness[i] = _arichgp->getAerogelThickness(i);
          zaero[i] = _arichgp->getAerogelZPosition(i) + thickness[i];
          trlen[i] = _arichgp->getAerogelTransmissionLength(i) ; // aerogel transmission length;
          // measured FOM
          n0[i] = m_aeroMerit[i] / (0.1516 * trlen[i] * (1 - exp(-thickness[i] / trlen[i])));
          thickness[asize]   += thickness[i];
        }
        refind[asize  ]   = 1.0;
        refind[asize + 1]   = _arichgp->getDetectorWindowRefIndex();
        zaero[asize  ] = _arichgp->getDetectorZPosition();
        zaero[asize + 1] = zaero[asize] + _arichgp->getDetectorWindowThickness();
        first = 0;
      }

      // loop over all tracks
      for (unsigned  int i = 0; i < tsize; i++) {

        ARICHTrack* track =  arichTracks[i];
        int nfot = 0;
        double padArea = pad_size / Unit::m * pad_size / Unit::m;
        int padNum = _arichgp->getDetectorXPadNumber() * _arichgp->getDetectorXPadNumber();

        // loop over all particle hypotheses
        for (int hyp = 0; hyp < maxhyp; hyp++) {
          lkh[hyp] = 0;

          // loop over aerogel layers
          for (unsigned int a = 0; a < asize; a++) {
            lrad[a] = track->getMeanEmissionDirection(a) * anorm[a];    // track length in the radiator;
            if (lrad[a]) lrad[a] = thickness[a] / lrad[a];
            thc[hyp][a] = ExpectedCherenkovAngle(track->getReconstructedMomentum(), p_mass[hyp], refind[a]);

            //----------------------------------
            // acceptance calculation
            //----------------------------------

            acceptance[hyp][a] = 0;
            const int maxmca = 100; // number of photons emitted in cone to calculate the geometrical acceptance

            for (int mca = 0; mca < maxmca; mca++) {
              double fi = 2 * M_PI * mca / float(maxmca);
              TVector3 epoint = track->getMeanEmissionPosition(a);
              TVector3 edir  = track->getMeanEmissionDirection(a);
              TVector3 adirf = setThetaPhi(thc[hyp][a], fi); // particle system
              adirf =  TransformFromFixed(edir) * adirf;  // global system

              if (!_arichgp->isSimple()) {
                TVector3 dposition = FastTracking(adirf, epoint, &refind[a], &zaero[a], asize - a);
                int copyno =  _arichgp->getCopyNo(dposition);
                if (InsideDetector(dposition, copyno)) acceptance[hyp][a] += 1;
              } else {
                TVector3 dposition = FastTrackingSimple(adirf, epoint, &refind[a], &zaero[a], asize - a);
                for (int i = 1; i <= _arichgp->getNMCopies(); i++) {
                  if (InsideDetector(dposition, i)) {
                    acceptance[hyp][a] += 1;
                    break;
                  }
                }
              }
            }

            acceptance[hyp][a] /= float(maxmca);

            //----------------------------------
            // expected number of photons
            //----------------------------------

            nsig[hyp][a] = n0[a] * trlen[a] * sin(thc[hyp][a]) * sin(thc[hyp][a]) * (1 - exp(-lrad[a] / trlen[a])) * acceptance[hyp][a];

            if (a > 0)  nsig[hyp][a - 1] *= exp(-lrad[a] / trlen[a]);

          } // for (unsigned int a=0;a<asize;a++)
          //*********************************************************

          nsig[hyp][asize] = 0;
          thc[hyp][asize] = thc[hyp][0];

          for (unsigned int a = 0; a < asize; a++) nsig[hyp][asize] += nsig[hyp][a];

          nbgr[hyp] = m_bkgLevel * padArea * padNum * _arichgp->getNMCopies();

        }  // for (int hyp=0;hyp < maxhyp; hyp++ )
        //#####################################################

        double ebgri = m_bkgLevel * padArea;
        TVector3 track_at_detector = track->getPositionAtZ(zaero[asize + 1]);

        // the id numbers of mirrors from which the photons could possibly reflect are calculated
        int mirrors[3];
        mirrors[0] = -1; // for no reflection
        int refl = 1;

        if (_arichgp->isSimple()) {refl += nMirSeg; mirrors[1] = 0; mirrors[2] = 1;} else {
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
        // loop over all photon hits
        for (unsigned int ifot = 0; ifot < hsize; ifot++) {

          ARICHDigit* h = arichDigits[ifot];
          int chID = h->getChannelID();
          int modID = h->getModuleID();

          TVector3 hitpos = _arichgp->getChannelCenterGlob(modID, chID);
          for (int hyp = 0; hyp < maxhyp; hyp++) esigi[hyp] = 0;
          int nfoo = nfot;
          // loop over all arogel layers
          for (unsigned int ar = 0; ar < asize; ar++) {

            TVector3 initialrf = track->getAerogelExit(ar);
            TVector3 epoint = track->getMeanEmissionPosition(ar);
            TVector3 edir  = track->getMeanEmissionDirection(ar);

            TVector3 dirf; // calculated photon direction

            // loop over possible mirror reflections
            for (int mirr = 0; mirr < refl; mirr++) {
              // calculate fi_ch for a given track refl
              TVector3 virthitpos =  HitVirtualPosition(hitpos, mirrors[mirr]);

              if (CherenkovPhoton(epoint, virthitpos, initialrf, dirf, &refind[ar], &zaero[ar], asize - ar) < 0)  continue;

              TVector3 dirch = TransformToFixed(edir) * dirf;
              double fi_cer = dirch.Phi();
              double th_cer = dirch.Theta();

              if (fabs(th_cer - thc[track->getIdentity()][0]) < 0.05 && ar == 0 && nfoo == nfot) nfot++;
              if (fi_cer < 0) fi_cer += 2 * M_PI;
              double fii;
              if (mirr > 0) {
                double fi_mir = _arichgp->getMirrorNormal(mirrors[mirr]).XYvector().Phi();
                fii = 2 * fi_mir - fi_cer - M_PI;
              }
              // loop over all particle hypotheses
              for (int hyp = 0; hyp < maxhyp; hyp++) {

                // track a photon from the mean emission point to the detector surface
                TVector3  dirf1 = setThetaPhi(thc[hyp][ar], fi_cer);  // particle system
                dirf1 = TransformFromFixed(edir) * dirf1;  // global system

                TVector3  photonAtAerogelExit = dirf1 * (thickness[ar] / dirf1.z());
                TVector3  trackAtAerogelExit = edir * (thickness[ar] / edir.z());
                TVector3  dtrackphoton = photonAtAerogelExit - trackAtAerogelExit;
                TVector3 detector_position;
                if (!_arichgp->isSimple()) {
                  detector_position = FastTracking(dirf1, epoint, &refind[ar], &zaero[ar], asize - ar);
                } else {
                  detector_position = FastTrackingSimple(dirf1, epoint, &refind[ar], &zaero[ar], asize - ar);
                }
                TVector3 meanr             = detector_position - epoint;
                double   path              = meanr.Mag();
                meanr                      = meanr.Unit();

                double   detector_sigma    = m_singleRes * path / meanr.z();

                // calculate pad orientation and distance relative to that photon
                TVector3 modorigin = _arichgp->getOrigin(modID);
                double modphi =  _arichgp->getModAngle(modID);

                double      pad_fi = fii - modphi;

                double      dx     = (detector_position - hitpos).Mag();
                // calculate pad acceptance
                double  dr = (track_at_detector - detector_position).Mag();

                if (dr > 0 && thc[hyp][ar]) {
                  double pad_sizemm = pad_size / Unit::mm;
                  double normalizacija = nsig[hyp][ar] * pad_sizemm / (2 * M_PI * dr / Unit::mm);
                  double integral = SquareInt(pad_sizemm, pad_fi, dx / Unit::mm, detector_sigma * 0.1);
                  // expected number of signal photons in each pixel
                  esigi[hyp] += normalizacija * integral;
                }// if (dr>0 && thc[hyp][ar])

              }// for (int hyp=0;hyp< maxhyp; hyp++ )

            }// for (int mirr = 0; mirr < refl; mirr++)

          }// for (unsigned int ar=0; ar<asize;ar++)

          //******************************************
          // LIKELIHOOD construction
          //*******************************************

          for (int hyp = 0; hyp < maxhyp; hyp++) {
            double expected = esigi[hyp] + ebgri;
            if (expected > 1e-13)    lkh[hyp] += expected + log(1 - exp(-expected));
            else {
              lkh[hyp] += 1e-14 + log(1 - exp(-(1e-14)));
            }
          }

          //*******************************************

        }// for (unsigned  int ifot=0; ifot< hsize; ifot++)

        //******************************************
        // add constant term to the LIKELIHOOD function
        //*******************************************


        for (int hyp = 0; hyp < maxhyp; hyp++) {
          lkh[hyp] -= (nsig[hyp][asize] + nbgr[hyp]);
        }

        //******************************************
        // store LikeliHOOD info
        //******************************************
        track->setExpectedNOfPhotons(0, nfot);
        track->setLikelihood(maxhyp, lkh);

      } // for (unsigned  int i=0; i< tsize; i++){

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

    TVector3 ARICHReconstruction::FastTrackingSimple(TVector3 dirf, TVector3 r,  double* refind, double* z, int n)
    {
      //
      // Description:
      // The method calculates the intersection  of the cherenkov photon
      // with the detector plane

      //  z[n+1]
      //  z[0] .. 1st aerogel exit
      //  z[n-1] .. 2nd aerogel exit
      //  z[n-1] .. 2nd aerogel exit

      int nmir = _arichgp->getNMirrors();
      double path = 0;
      double rind = 0;
      bool reflok = false; bool refl = false;
      path = (z[0] - r.z()) / dirf.z();
      r   += dirf * path;
      for (int a = 1; a <= n + 1 ; a++) {
        rind = refind[a] / refind[a - 1];
        dirf = Refraction(dirf, rind);
        if (dirf.Mag() == 0) return TVector3();
        path = (z[a] - r.z()) / dirf.z();
        TVector3 r0 = r;
        r += dirf * path;
        TVector2 rxy = r.XYvector();
        // check for possible reflections
        if (a != n) continue;
        for (int k = 0; k < nmir; k++) {
          TVector3 mirpoint = _arichgp->getMirrorPoint(k);
          TVector3 mirnorm = _arichgp->getMirrorNormal(k);
          TVector2 dr = rxy - mirpoint.XYvector();
          if (dr.X()*mirnorm.X() + dr.Y()*mirnorm.Y() > 0) {
            refl = true;
            double s = dirf * mirnorm;
            double s1 = (mirpoint - r0) * mirnorm;
            r = r0 + s1 / s * dirf;
            if (r.Z() < _arichgp->getMirrorsZPosition()) return TVector3();
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


