/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Analyze histograms of amplitudes for each ECL crystal from moun pair   *
 * events. Code can either find most-likely energy deposit for each       *
 * crystal (MC) or calibration constant for each crystal (data)           *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <top/calibration/TOPLocalCalFitter.h>
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace TOP {

    /** Calibrate ecl crystals using muon pair events */
    class TOPLocalCalFitter : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      TOPLocalCalFitter();

      /**..Destructor */
      virtual ~TOPLocalCalFitter() {}

      /** Parameters. Note thate they are public so you can set them in te steering file */
      int minEntries = 50; /**<  Minimum number of entries to perform the fit*/
      bool isMC = false; /** < The MC tructh requires a simplified PDF and different ranges */
      std::string m_output = "laserFitResult.root"; /**< Name of the output file */
      std::string m_laserCorrections = "LaserMCParameters.root"; /**< Name of the output file */
      std::string m_TTSData = "TTSParametrization.root"; /**< Name of the output file */
      bool isMonitoringFit =
        false; /**< Set to True if you are analyzing runs with low statistics and you want to fix the fit parameters from a high-stat run*/

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:
      int fitOK = 16; /**< fit is OK */

      //fit results
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

    };
  }
} // namespace Belle2


