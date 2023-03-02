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




  /** Find Gaussian compoments in a Histogram
   *
   * 1. Highest peak is found
   * 2. It is fitted in a given range
   * 3. Fit paramter is stored
   * 4. Gaus peak is removed from histogram
   * 5. Process is repeated until a few criterias are met.
   */
  class FindGausPeaks {

  public:

    /*! default constructor */
    FindGausPeaks(TH1D& h, std::vector<GroupInfo>& groupInfo)
      : m_Hist(h), m_groupInfoVector(groupInfo) {}

    /*! expected min and max signal time */
    void setSignalTime(const double& rangeCenter, const double& rangeMin, const double& rangeMax)
    {
      m_expectedSignalTimeCenter = rangeCenter;
      m_expectedSignalTimeMin    = rangeMin;
      m_expectedSignalTimeMax    = rangeMax;
    }
    /*! set minSigma and maxSigma */
    void setSigmaLimits(const double& minSigma, const double& maxSigma)
    {
      m_minSigma = minSigma;
      m_maxSigma = maxSigma;
    }
    /*! set fit half range */
    void setFitHalfWidth(const double& range) { m_fitRangeHalfWidth = range; }
    /*! set sigmaN */
    void setSigmaN(const double& N) { m_removeSigmaN = N; }
    /*! set fractional threshold */
    void setFracThreshold(const double& fraction) { m_fracThreshold = fraction; }
    /*! set max groups */
    void setMaxGroups(const int& groups) { m_maxGroups = groups; }
    /*! set signal lifetime for exponential-weighted sort */
    void setSignalLifeTime(const double& val) { m_signalLifetime = val; }

    /*! now find the peaks */
    void doTheSearch();

    /*! increase the size of vector to max, this helps in sorting */
    void resizeToMaxSize()
    {
      m_groupInfoVector.resize(m_maxGroups, GroupInfo(0., 0., 0.));
    }

    /** Sort Background Groups
     *
     * The probability of being signal is max at groupID = 0 and decreases with group number increasing.
     * The probability of being background is max at groupID = 19 and increases with group number decreasing.
     */
    void sortBackgroundGroups();

    /** Sort Signals
     *
     * Sorting signal groups based on exponential weight.
     * This decreases chance of near-signal bkg groups getting picked.
     * The probability of being signal is max at groupID = 0 and decreases with group number increasing.
     */
    void sortSignalGroups();


  private:

    TH1D& m_Hist;   /**< Reference to the histogram to be used in the search */
    std::vector<GroupInfo>& m_groupInfoVector; /**< Group (or Gauss Fit) Info to be stored */

    double m_expectedSignalTimeCenter =   0; /**< Expected time of the signal [ns]. */
    double m_expectedSignalTimeMin    = -50; /**< Expected low range of signal hits [ns]. */
    double m_expectedSignalTimeMax    =  50; /**< Expected high range of signal hits [ns]. */
    double m_minSigma           = 1.; /**< Lower limit of cluster time sigma for the fit for the
           peak-search [ns]. */
    double m_maxSigma           = 15.; /**< Upper limit of cluster time sigma for the fit for the
            peak-search [ns]. */
    double m_fitRangeHalfWidth  = 5.; /**< Half width of the range in which the fit for the
           peak-search is performed [ns]. */
    double m_removeSigmaN       = 5.; /**< Remove upto this sigma of fitted gaus from histogram. */
    double m_fracThreshold      = 0.05; /**< Minimum fraction of candidates in a peak (wrt to the
             highest peak) considered for fitting in the peak-search. */
    int    m_maxGroups          = 20; /**< maximum number of groups to be accepted. */
    double m_signalLifetime     = 30.; /**< Group prominence is weighted with exponential weight
            with a lifetime defined by this parameter [ns]. */

  };

  void FindGausPeaks::doTheSearch()
  {

    double maxPeak     = 0.;  //   height of the highest peak in signal region [expectedSignalTimeMin, expectedSignalTimeMax]
    double maxIntegral = 0.;  // integral of the highest peak in signal region [expectedSignalTimeMin, expectedSignalTimeMax]

    bool amDone = false;
    int  roughCleaningCounter = 0; // handle to take care when fit does not conserves
    while (!amDone) {

      // take the bin corresponding to the highest peak
      int    maxBin        = m_Hist.GetMaximumBin();
      double maxBinCenter  = m_Hist.GetBinCenter(maxBin);
      double maxBinContent = m_Hist.GetBinContent(maxBin);

      // Set maxPeak for the first time
      if (maxPeak == 0 &&
          maxBinCenter > m_expectedSignalTimeMin && maxBinCenter < m_expectedSignalTimeMax)
        maxPeak = maxBinContent;
      // we are done if the the height of the this peak is below threshold
      if (maxPeak != 0 && maxBinContent < maxPeak * m_fracThreshold) { amDone = true; continue;}



      // preparing the gaus function for fitting the peak
      TF1 ngaus("ngaus", myGaus,
                m_Hist.GetXaxis()->GetXmin(), m_Hist.GetXaxis()->GetXmax(), 3);

      // setting the parameters according to the maxBinCenter and maxBinContnet
      double maxPar0 = maxBinContent * 2.50662827463100024 * m_fitRangeHalfWidth;
      ngaus.SetParameter(0, maxBinContent);
      ngaus.SetParLimits(0,
                         maxPar0 * 0.01,
                         maxPar0 * 2.);
      ngaus.SetParameter(1, maxBinCenter);
      ngaus.SetParLimits(1,
                         maxBinCenter - m_fitRangeHalfWidth * 0.2,
                         maxBinCenter + m_fitRangeHalfWidth * 0.2);
      ngaus.SetParameter(2, m_fitRangeHalfWidth);
      ngaus.SetParLimits(2,
                         m_minSigma,
                         m_maxSigma);


      // fitting the gauss at the peak the in range [-fitRangeHalfWidth, fitRangeHalfWidth]
      int status = m_Hist.Fit("ngaus", "NQ0", "",
                              maxBinCenter - m_fitRangeHalfWidth,
                              maxBinCenter + m_fitRangeHalfWidth);


      if (!status) {    // if fit converges

        double pars[3] = {
          ngaus.GetParameter(0),     // integral
          ngaus.GetParameter(1),     // center
          std::fabs(ngaus.GetParameter(2)) // sigma
        };

        // fit converges but paramters are at limit
        // Do a rough cleaning
        if (pars[2] <= m_minSigma + 0.01 || pars[2] >= m_maxSigma - 0.01) {
          // subtract the faulty part from the histogram
          subtractGausFromHistogram(m_Hist, maxPar0, maxBinCenter, m_fitRangeHalfWidth, m_removeSigmaN);
          if (roughCleaningCounter++ > m_maxGroups) amDone = true;
          continue;
        }

        // Set maxIntegral for the first time
        if (maxPeak != 0 && maxIntegral == 0) maxIntegral = pars[0];
        // we are done if the the integral of the this peak is below threshold
        if (maxIntegral != 0 && pars[0] < maxIntegral * m_fracThreshold) { amDone = true; continue;}


        // now subtract the fitted gaussian from the histogram
        subtractGausFromHistogram(m_Hist, pars[0], pars[1], pars[2], m_removeSigmaN);

        // store group information (integral, position, width)
        m_groupInfoVector.push_back(std::make_tuple(pars[0], pars[1], pars[2]));
        B2DEBUG(1, " group " << int(m_groupInfoVector.size())
                << " pars[0] " << pars[0] << " pars[1] " << pars[1] << " pars[2] " << pars[2]);

        if (int(m_groupInfoVector.size()) >= m_maxGroups) { amDone = true; continue;}

      } else {    // fit did not converges
        // subtract the faulty part from the histogram
        subtractGausFromHistogram(m_Hist, maxPar0, maxBinCenter, m_fitRangeHalfWidth, m_removeSigmaN);
        if (roughCleaningCounter++ > m_maxGroups) amDone = true;
        continue;
      }
    }

  } // end of doTheSearch()

  void FindGausPeaks::sortBackgroundGroups()
  {
    GroupInfo key;
    for (int ij = int(m_groupInfoVector.size()) - 2; ij >= 0; ij--) {
      key = m_groupInfoVector[ij];
      double keynorm = std::get<0>(key);
      double keymean = std::get<1>(key);
      bool isKeySignal = true;
      if (keynorm != 0. && (keymean < m_expectedSignalTimeMin || keymean > m_expectedSignalTimeMax)) isKeySignal = false;
      if (isKeySignal) continue;
      int kj = ij + 1;
      while (1) {
        if (kj >= int(m_groupInfoVector.size())) break;
        double grnorm = std::get<0>(m_groupInfoVector[kj]);
        double grmean = std::get<1>(m_groupInfoVector[kj]);
        bool isGrSignal = true;
        if (grnorm != 0. && (grmean < m_expectedSignalTimeMin || grmean > m_expectedSignalTimeMax)) isGrSignal = false;
        if (!isGrSignal && (grnorm > keynorm)) break;
        m_groupInfoVector[kj - 1] = m_groupInfoVector[kj];
        kj++;
      }
      m_groupInfoVector[kj - 1] = key;
    }
  }

  void FindGausPeaks::sortSignalGroups()
  {
    if (m_signalLifetime > 0.) {
      GroupInfo key;
      for (int ij = 1; ij < int(m_groupInfoVector.size()); ij++) {
        key = m_groupInfoVector[ij];
        double keynorm = std::get<0>(key);
        if (keynorm <= 0) break;
        double keymean = std::get<1>(key);
        bool isKeySignal = true;
        if (keynorm > 0 && (keymean < m_expectedSignalTimeMin || keymean > m_expectedSignalTimeMax)) isKeySignal = false;
        if (!isKeySignal) break;
        double keyWt = keynorm * TMath::Exp(-std::fabs(keymean - m_expectedSignalTimeCenter) / m_signalLifetime);
        int kj = ij - 1;
        while (1) {
          if (kj < 0) break;
          double grnorm = std::get<0>(m_groupInfoVector[kj]);
          double grmean = std::get<1>(m_groupInfoVector[kj]);
          double grWt = grnorm * TMath::Exp(-std::fabs(grmean - m_expectedSignalTimeCenter) / m_signalLifetime);
          if (grWt > keyWt) break;
          m_groupInfoVector[kj + 1] = m_groupInfoVector[kj];
          kj--;
        }
        m_groupInfoVector[kj + 1] = key;
      }
    }
  }


} /* end of namespace Belle2 */
