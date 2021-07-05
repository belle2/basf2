/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <top/calibration/TOPLocalCalFitter.h>
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace TOP {

    /**
     * This module is the fitter for the CAF collector TOPLaserCalibratorCollector. It
     * analyzes the tree containing the timing of the laser and pulser hits produced by the collector,
     * returning a tree  with the fit results and the histograms for each channel.
     * It can be used to produce both channelT0 calibrations and to analyze
     * the daily, low statistics laser runs
    */
    class TOPLocalCalFitter : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPLocalCalFitter();

      /** Destructor */
      virtual ~TOPLocalCalFitter() {}

      /** Sets the minimum number of entries to perform the calibration in one channel */
      void setMinEntries(int minEntries)
      {
        m_minEntries = minEntries;
      }

      /** Sets the name of the output root file */
      void setOutputFileName(const std::string& output)
      {
        m_output = output;
      }

      /** Sets the name of the root file containing the laser MC time corrections and the fit constraints.
       * If the monitoringFit option is used (low statistics sample), this file must be the result of an high-statistics fit.
       */
      void setFitConstraintsFileName(const std::string& fitConstraints)
      {
        m_fitConstraints = fitConstraints;
      }

      /** Sets the name of the root file containing the TTS parameters */
      void setTTSFileName(const std::string& TTSData)
      {
        m_TTSData = TTSData;
      }

      /** Sets the fitter mode. The options are 'calibration' (default), 'monitoring' or 'MC'.
       *  The mode affects the number of parameters that are fixed.
       *  Use calibration if you are fitting a large sample (1 M  events or more) to derive  a set of channelT0 calibrations.
       *  Use monitoring if you are fitting a smaller sample. The light path fractions and the tail parameters
       *  will be constrained according to the constraint file you passed to the fitter (usually teh result fo a high-statistics fit).
       *  Use MC to fit the MC sample and calculate a new set of prism corrections. No parameter is fixed, but the
       *  tail components are removed form the fit.
       */
      void setFitMode(const std::string& fitterMode)
      {
        if (fitterMode == "calibration")
          B2INFO("Fitter set to calibration mode");
        else if (fitterMode == "monitoring")
          B2INFO("Fitter set to monitoring mode");
        else if (fitterMode == "MC")
          B2INFO("Fitter set to MC mode");
        else
          B2ERROR("Unknown fitter type " << fitterMode  << ". The valid options are calibration, monitoring or MC");

        m_fitterMode = fitterMode;
      }

      /** Enables the fit amplitude bins.  */
      void fitInAmpliduteBins(bool isFitInAmplitudeBins)
      {
        m_isFitInAmplitudeBins = isFitInAmplitudeBins;
      }


    protected:

      /** prepares the output tree */
      void setupOutputTreeAndFile();

      /** loads the TTS parameters and the MC truth info */
      void loadMCInfoTrees();

      /** determines if the constant obtained by the fit are good or not */
      void determineFitStatus();

      /** Fits the laser light on one channel*/
      void fitChannel(short, short, TH1*);

      /** Fits the two pulsers */
      void fitPulser(TH1*, TH1*);

      /** Calculates the commonT0 calibration after the fits have been done.
       *  It also saves the constants in a localDB and in the output tree
       */
      void calculateChennelT0();

      /** Runs the algorithm on events. Currently, it always returns c_OK despite of the actual result of the fitting procedure.
       *  This is not an issue since this moduleis not intended to be used in the automatic calibration. */
      virtual EResult calibrate() override;

    private:

      int m_minEntries = 50; /**<  Minimum number of entries to perform the fit. Currently not used*/
      std::string m_output = "laserFitResult.root"; /**< Name of the output file */
      std::string m_fitConstraints =
        "/group/belle2/group/detector/TOP/calibration/MCreferences/LaserMCParameters.root"; /**< File with the TTS parametrization*/
      std::string m_TTSData =
        "/group/belle2/group/detector/TOP/calibration/MCreferences/TTSParametrization.root"; /**< File with the Fit constraints and MC info */
      std::string m_fitterMode = "calibration";/**< Fit mode. Can be 'calibration', 'monitoring' or 'MC' */
      bool m_isFitInAmplitudeBins = false; /**< Enables the fit in amplitude bins */
      std::vector<float> m_binEdges = {50, 100, 130, 160, 190, 220, 250, 280, 310, 340, 370, 400, 430, 460, 490, 520, 550, 580, 610, 640, 670, 700, 800, 900, 1000, 1200, 1500, 2000}; /**< Amplitude bins */
      TFile* m_inputTTS = nullptr; /**< File containing m_treeTTS */
      TFile* m_inputConstraints = nullptr; /**< File containing m_treeConstraints */

      TTree* m_treeTTS = nullptr; /**< Input to the fitter. A tree containing the TTS parametrization for each channel */
      TTree* m_treeConstraints =
        nullptr; /**< Input to the fitter. A tree containing the laser MC corrections and all the paraeters to be fixed in the fit*/
      TFile* m_histFile = nullptr; /**< Output of the fitter. The file containing the output trees and histograms*/
      TTree* m_fitTree = nullptr; /**< Output of the fitter. The tree containg the fit results. */
      TTree* m_timewalkTree =
        nullptr; /**< Output of the fitter. The tree containg the fit results to be used to study timewalk and asymptotic time resolution. */


      // Variables for the TTS parametrization tree
      float m_mean2 = 0;  /**< Position of the second gaussian of the TTS parametrization with respect to the first one*/
      float m_sigma1 = 0; /**< Width of the first gaussian on the TTS parametrization */
      float m_sigma2 = 0; /**< Width of the second gaussian on the TTS parametrization */
      float m_f1 = 0; /**< Fraction of the first gaussian on the TTS parametrization */
      float m_f2 = 0; /**< Fraction of the second gaussian on the TTS parametrization */
      short m_pixelRow = 0; /**< Pixel row */
      short m_pixelCol = 0; /**< Pixel column */

      // Variables for the MC truth infos
      float m_peakTimeConstraints = 0; /**< Time of the main laser peak in the MC simulation (aka MC correction)  */
      float m_deltaTConstraints = 0; /**< Distance between the main and the secondary laser peak */
      float m_fractionConstraints = 0; /**< Fraction of the main peak*/
      float m_timeExtraConstraints = 0; /**< Position of the guassian used to describe the extra peak on the timing distribution tail */
      float m_sigmaExtraConstraints = 0; /**< Width of the guassian used to describe the extra peak on the timing distribution tail */
      float m_alphaExtraConstraints = 0.; /**< alpha parameter of the tail of the extra peak. */
      float m_nExtraConstraints = 0.; /**< parameter n of the tail of the extra peak */
      float m_timeBackgroundConstraints = 0.; /**< Position of the gaussian used to describe the background, w/ respect to peakTime */
      float m_sigmaBackgroundConstraints = 0.; /**< Sigma of the gaussian used to describe the background */


      // Variables for the output tree
      float m_binLowerEdge = 0; /**< Lower edge of the amplitude bin in which this fit is performed */
      float m_binUpperEdge = 0; /**< Upper edge of the amplitude bin in which this fit is performed */
      short m_channel = 0; /**< Channel number (0-511)*/
      short m_slot = 0; /**< Slot ID (1-16)*/
      short m_row = 0; /**< Pixel row */
      short m_col = 0; /**< Pixel column*/
      float m_peakTime = 0; /**< Fitted time of the main (i.e. latest) peak */
      float m_deltaT =
        0; /**< Time difference between the main peak and the secondary peak. Can be either fixed to the MC value or fitted. */
      float m_sigma = 0.; /**< Gaussian time resolution, fitted */
      float m_fraction = 0.; /**< Fraction of events in the secondary peak*/
      float m_yieldLaser = 0.; /**< Total number of laser hits from the fitting function integral */
      float m_histoIntegral = 0.; /**< Integral of the fitted histogram */

      float m_peakTimeErr = 0; /**< Statistical error on peakTime*/
      float m_deltaTErr = 0; /**< Statistical error on deltaT */
      float m_sigmaErr = 0.; /**< Statistical error on sigma*/
      float m_fractionErr = 0.; /**< Statistical error on fraction*/
      float m_yieldLaserErr = 0.; /**< Statistical error on yield*/

      float m_timeExtra = 0.; /**< Position of the extra peak seen in the timing tail, w/ respect to peakTime */
      float m_sigmaExtra = 0.; /**< Gaussian sigma of the extra peak in the timing tail  */
      float m_yieldLaserExtra = 0.; /**< Integral of the extra peak */
      float m_alphaExtra = 0.; /**< alpha parameter of the tail of the extra peak. */
      float m_nExtra = 0.; /**< parameter n of the tail of the extra peak */

      float m_timeBackground = 0.; /**< Position of the gaussian used to describe the background, w/ respect to peakTime */
      float m_sigmaBackground = 0.; /**< Sigma of the gaussian used to describe the background */
      float m_yieldLaserBackground = 0.; /**< Integral of the background gaussian */

      float m_fractionMC = 0.; /**< Fraction of events in the secondary peak form the MC simulation */
      float m_deltaTMC = 0.; /**< Time difference between the main peak and the secondary peak in the MC simulation*/
      float m_peakTimeMC =
        0.; /**< Time of the main peak in teh MC simulation, i.e. time of propagation of the light in the prism. This factor is used to get the channelT0 calibration */

      float m_chi2 = 0; /**< Reduced chi2 of the fit*/
      float m_rms = 0; /**< RMS of the histogram used for the fit*/

      float m_channelT0 =
        0.; /**< Raw, channelT0 calibration, defined as peakTime-peakTimeMC. This constant is not yet normalized to the average constant in the slot, since that part is currently done by the DB importer. When the DB import functionalities will be added to this module, it will be set to the proper channeT0*/
      float m_channelT0Err = 0.; /**< Statistical error on channelT0*/

      float m_firstPulserTime = 0.; /**< Average time of the first electronic pulse respect to the reference pulse, from a Gaussian fit*/
      float m_firstPulserSigma = 0.; /**< Time resolution from the fit of the first electronic pulse, from a Gaussian fit*/

      float m_secondPulserTime =
        0.; /**< Average time of the second electronic pulse respect to the reference pulse, from a gaussian fit*/
      float m_secondPulserSigma = 0.;  /**< Time resolution from the fit of the first electronic pulse, from a Gaussian fit*/

      short m_fitStatus = 1; /**< Fit quality flag, propagated to the constants. 1 if fit did not converge, 0 if it is fine. */
    };
  }
} // namespace Belle2


