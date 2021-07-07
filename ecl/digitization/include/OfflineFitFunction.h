/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/digitization/algorithms.h>

#pragma once

namespace Belle2 {
  namespace ECL {
    double WaveFuncTwoComponent(double* x, double* par)
    {
      //Used for fitting with ROOT::FIT and drawing offline fit results
      //p0 = trigger time
      //p1 = baseline
      //p2 = photon component amp
      //p3 = hadron component amp
      //p4-p13 =photon shape template parameters (fixed)
      //p14-p23 = hadron or didoe shape template parameters (fixed)
      //p24 = photon scale (fixed)
      //p25 = hadron scale (fixed)

      const double time = x[0] - par[0];
      const double PhotonScale = par[24];
      const double HadronScale = par[25];

      double GammaComponent = 0;
      double HadronComponent = 0;
      double photonTemplate10[10];
      double hadronTemplate10[10];
      for (int k = 0; k < 10; k++) {
        photonTemplate10[k] = par[4 + k];
        hadronTemplate10[k] = par[10 + 4 + k];
      }
      GammaComponent  = Belle2::ECL::ShaperDSPofflineFit(time, photonTemplate10, PhotonScale);
      HadronComponent = Belle2::ECL::ShaperDSPofflineFit(time, hadronTemplate10, HadronScale);
      return (par[2] * GammaComponent + par[3] * HadronComponent) + par[1];
    }
  }
}
