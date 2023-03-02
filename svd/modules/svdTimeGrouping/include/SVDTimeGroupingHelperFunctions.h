/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TH1D.h>
#include <TF1.h>
#include <TMath.h>


namespace Belle2 {


  /*! typedef to be used to store Gauss paramerers (integral, center, sigma) */
  typedef std::tuple<double, double, double> GroupInfo;


  /*! Gaus function to be used in the fit. */
  inline double myGaus(const double* x, const double* par)
  {
    return par[0] * TMath::Gaus(x[0], par[1], par[2], true);
  }


  /** Add (or Subtract) a Gaussian to (or from) a histogram
   *
   * The gauss is calculated upto the sigmaN passed to the function.
   */
  inline void addGausToHistogram(TH1D& hist,
                                 const double& integral, const double& center, const double& sigma,
                                 const double& sigmaN, const bool& isAddition = true)
  {
    int startBin = hist.FindBin(center - sigmaN * sigma);
    int   endBin = hist.FindBin(center + sigmaN * sigma);
    if (startBin < 1) startBin = 1;
    if (endBin > (hist.GetNbinsX())) endBin = hist.GetNbinsX();

    for (int ijx = startBin; ijx <= endBin; ijx++) {
      double tbinc       = hist.GetBinCenter(ijx);
      double tbincontent = hist.GetBinContent(ijx);

      if (isAddition) tbincontent += integral * TMath::Gaus(tbinc, center, sigma, true);
      else tbincontent -= integral * TMath::Gaus(tbinc, center, sigma, true);

      hist.SetBinContent(ijx, tbincontent);
    }
  }


  /**  Subtract a Gaussian from a histogram
   *
   * The gauss is calculated upto the sigmaN passed to the function.
   */
  inline void subtractGausFromHistogram(TH1D& hist,
                                        const double& integral, const double& center, const double& sigma,
                                        const double& sigmaN)
  {
    addGausToHistogram(hist, integral, center, sigma, sigmaN, false);
  }

} /* end of namespace Belle2 */
