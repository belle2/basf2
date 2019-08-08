/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Analyze a tree contaninig laser hit timing and channel, returning      *
 * a tree  with the fit results and the histograms for each channel.      *
 * It can be used to produce both channelT0 calibrations and to analyze   *
 * the daily, low statistics laser runs                                   *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <top/calibration/TOPLocalCalFitter.h>
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace TOP {

    /** Fit the laser hits time distribution and caluculates the channelT0 */
    class TOPLocalCalFitter : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      TOPLocalCalFitter();

      /**..Destructor */
      virtual ~TOPLocalCalFitter() {}

      int minEntries = 50; /**<  Minimum number of entries to perform the fit*/
      std::string m_output = "laserFitResult.root"; /**< Name of the output file */
      std::string m_laserCorrections =
        "/group/belle2/group/detector/TOP/calibration/MCreferences/LaserMCParameters.root"; /**< Name of the output file */
      std::string m_TTSData =
        "/group/belle2/group/detector/TOP/calibration/MCreferences/TTSParametrization.root"; /**< Name of the output file */
      bool isMonitoringFit =
        false; /**< Set to True if you are analyzing runs with low statistics and you want to fix the fit parameters from a high-stat run*/

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

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:
      TTree* treeTTS = nullptr; /**< Input to the fitter. A tree containing the TTS parametrization for each channel */
      TTree* treeLaser =
        nullptr; /**< Input to the fitter. A tree containing the laser MC corrections and all the paraeters to be fixed in the fit*/
      TFile* histFile = nullptr; /**< Output of the fitter. The file containing the output trees and histograms*/
      TTree* fitTree = nullptr; /**< Output of the fitter. The tree containg the fit results. */


      // Variables for the TTS parametrization tree
      float mean2 = 0;  /**< Position of the second gaussian of the TTS parametrization with respect to the first one*/
      float sigma1 = 0; /**< Width of the first gaussian on the TTS parametrization */
      float sigma2 = 0; /**< Width of the second gaussian on the TTS parametrization */
      float f1 = 0; /**< Fraction of the first gaussian on the TTS parametrization */
      float f2 = 0; /**< Fraction of the second gaussian on the TTS parametrization */
      short pixelRow = 0; /**< Pixel row */
      short pixelCol = 0; /**< Pixel column */

      // Variables for the MC truth infos
      short channelLaser = 0; /**< Channel number (0-512) */
      float peakTimeLaser = 0; /**< Time of the main laser peak in the MC simulation (aka MC correction)  */
      float deltaTLaser = 0; /**< Distance between the main and the secondary laser peak */
      float fractionLaser = 0; /**< Fraction of the main peak*/
      float extraTimeLaser = 0; /**< Position of the guassian used to describe the extra peak on the timing distribution tail */
      float extraTimeSigma = 0; /**< Width of the guassian used to describe the extra peak on the timing distribution tail */
      float backgroundTimeLaser = 0; /**< Position of the guassian used to describe the long tail of the timing distribution */
      float backgroundSigmaLaser = 0; /**< Width of the guassian used to describe the long tail of the timing distribution */


      // Variables for the output tree
      short channel = 0;
      short slot = 0;
      short row = 0;
      short col = 0;
      float peakTime = 0;
      float deltaT = 0;
      float sigma = 0.;
      float fraction = 0.;
      float yieldLaser = 0.;

      float peakTimeErr = 0;
      float deltaTErr = 0;
      float sigmaErr = 0.;
      float fractionErr = 0.;
      float yieldLaserErr = 0.;

      float timeExtra = 0.;
      float sigmaExtra = 0.;
      float yieldLaserExtra = 0.;

      float fractionMC = 0.;
      float deltaTMC = 0.;
      float peakTimeMC = 0.;

      float chi2 = 0;
      float rms = 0;

      float channelT0 = 0.;
      float channelT0Err = 0.;

      float firstPulserTime = 0.;
      float firstPulserSigma = 0.;

      float secondPulserTime = 0.;
      float secondPulserSigma = 0.;

      short fitStatus = 1;
    };
  }
} // namespace Belle2


