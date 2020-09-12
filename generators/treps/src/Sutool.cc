// -*- C++ -*-
//
// Package:     <package>
// Module:      Sutool
//
// Description: <one line class summary>
//   Tools collected by S.Uehara for kinematics
// Implimentation:
//     <Notes on implimentation>
//
// Author:      Sadaharu Uehara
// Created:     Thu May  8 15:04:07 JST 1997
// $Id$
//
// Revision history
// Update on 31-Mar-1998 S.U. for 3-body case for impoving approximation
//     of phase-space volume
// Modofied to be suitable to Belle II style
// $Log$

// system include files
#include <iostream>
#include <fstream>
#include <math.h>
#include <TVector3.h>
#include <TLorentzVector.h>

#include <framework/logging/Logger.h>

// user include files
#include <generators/treps/Sutool.h>

using namespace std;

namespace Belle2 {

  extern "C" {
    extern void aaubrn_(float*, float[20], int*, float*);
  }

  //
  // member functions
  //


  double Sutool::p2bdy(double m0, double m1, double m2, int& jcode)
  {
    // two-body decay kinematics   translated from EPOCS P2BDY$
    if (m0 <= 0.0 || m1 < 0.0 || m2 < 0.0) {
      B2FATAL(" Invalid mass value in two-body decay:" <<
              m0 << " " << m1 << " " << m2) ;
      jcode = 0;
      return 0.0 ;
    }

    if (m0 >= (m1 + m2)) {
      double pp = (m0 - m1 - m2) * (m0 + m1 - m2)
                  * (m0 - m1 + m2) * (m0 + m1 + m2);
      if (pp < 0.0) pp = 0.0 ;
      jcode = 1;
      double pcm = sqrt(pp) / 2. / m0 ;
      return pcm ;
    } else {
      jcode = 0;
      return 0.0 ;
    }
    return 0.0 ;
  }

  int Sutool::pdecy(double mpr, double* mse,
                    const TVector3& ppri, TLorentzVector* psec, int nums)
  // multi-body decay by phase space
  // translated from EPOCS PDECY$
  // original EPOCS version gives a good approximation for nums = 3
  // but, bad approximation for nums > 3.
  // I improved that to get an exact answer for nums = 3, and to get
  // good approximations for nums > 3  using AAUB    (S.Uehara)
  {
    if (nums < 2) {
      B2FATAL(" Number of secondaries must be >=2");
      return 0;
    }

    double m0 = mpr ;
    int n = nums ;
    TVector3 pcms = ppri ;

    int jcode;
    double w, p, pf;

    do {
      if (n == 2) {
        p = p2bdy(m0, mse[0], mse[1], jcode);
        if (jcode == 0) return 0 ;
        w = mse[0];
      } else {
        double mx = m0 - mse[n - 1];
        double mi = 0.0 ;
        for (int l = 1 ; l <= n - 1 ; l++) mi = mi + mse[l - 1];
        double bmx = mx * mx ;
        double bmi = mi * mi ;
        double pmx = p2bdy(m0, mi, mse[n - 1], jcode);
        if (jcode == 0) return 0 ;

        do {
          double w2 = bmi + (bmx - bmi) * gRandom->Uniform() ;
          w = sqrt(w2) ;
          p = p2bdy(m0, w, mse[n - 1], jcode);
          if (jcode == 0) return 0;

          if (n == 3) {
            pf = 2.*p2bdy(w, mse[0], mse[1], jcode) / w;
            if (jcode == 0) return 0;
          } else {
            // Use AAUB
            float pfmax, pff;
            float m0f = (float)m0;
            float wf = (float)w;
            int nsub = n - 1;
            float msef[20];
            for (int l = 0 ; l < nsub ; l++) msef[l] = (float)mse[l];
            aaubrn_(&m0f, msef, &nsub, &pfmax);
            aaubrn_(&wf, msef, &nsub, &pff);
            pf = (double)(pff / pfmax);
          }
        } while (pmx * gRandom->Uniform() > p * pf);

      }
      const double twopi = 2.*3.14159265 ;
      double phi = twopi * gRandom->Uniform();
      double cth = 2.*gRandom->Uniform() - 1. ;
      double sth = sqrt(1. - cth * cth);

      psec[n - 1] = TLorentzVector(p * sth * cos(phi), p * sth * sin(phi),
                                   p * cth, sqrt(p * p + mse[n - 1] * mse[n - 1]));
      psec[n - 2] = TLorentzVector(-p * sth * cos(phi), -p * sth * sin(phi),
                                   -p * cth, sqrt(p * p + w * w));

      psec[n - 1].Boost(pcms);
      psec[n - 2].Boost(pcms);

      if (n == 2) break ;

      m0 = w;
      pcms = (1. / psec[n - 2].T()) * psec[n - 2].Vect();

      n--;
    } while (1);
    return jcode;
  }

  void Sutool::rotate(TLorentzVector& p, double th, double phi)
  {
    // Some rotation of the 3Vector in a LorentzVector
    double ct = cos(th);
    double st = sin(th);
    double cp = cos(phi);
    double sp = sin(phi);

    TLorentzVector q =  TLorentzVector(
                          cp * ct * p.X() + sp * p.Y() + cp * st * p.Z(),
                          -sp * ct * p.X() + cp * p.Y() - sp * st * p.Z(),
                          -st * p.X() + ct * p.Z(),
                          p.T());
    p = q;
  }

  float Sutool::interp(float a1, float a2, float a3, float a4, float f)
  {
    float b1, b2, b3, c1, c2, d1;
    float g2, g3;
    // added by S.Uehara Sept.26,1997
    b1 = a2 - a1; b2 = a3 - a2 ; b3 = a4 - a3;
    c1 = b2 - b1; c2 = b3 - b2 ; d1 = c2 - c1;
    g2 = 0.25 * f * (f - 1.);
    g3 = 2. / 3. * g2 * (f - 0.5);

    return  a2 + f * b2 + g2 * (c1 + c2) + g3 * d1;
  }

  int Sutool::poisson(double m)
  {
    return gRandom->Poisson(m);
  }

}
