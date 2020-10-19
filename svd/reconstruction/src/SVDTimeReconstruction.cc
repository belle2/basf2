/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <svd/reconstruction/SVDTimeReconstruction.h>
#include <svd/reconstruction/SVDMaxSumAlgorithm.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    std::pair<int, double> SVDTimeReconstruction::getFirstFrameAndStripTime(const TString& timeAlgo = "fromRecoDBObject")
    {

      StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
      std::string m_svdEventInfoName = "SVDEventInfo";
      if (!temp_eventinfo.isValid())
        m_svdEventInfoName = "SVDEventInfoSim";
      StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
      if (!eventinfo) B2ERROR("No SVDEventInfo!");


      //default
      TString stripTimeReco = "CoG6";

      if (m_recoConfig.isValid()) {
        if (eventinfo->getNSamples() == 6)
          stripTimeReco = m_recoConfig->getStripTimeRecoWith6Samples();
        else
          stripTimeReco = m_recoConfig->getStripTimeRecoWith3Samples();
      }

      if (! timeAlgo.EqualTo("fromRecoDBObject"))
        stripTimeReco = timeAlgo;

      if (m_timeAlgorithms.find(stripTimeReco) != m_timeAlgorithms.end()) {
        if (stripTimeReco.EqualTo("CoG6"))
          return std::make_pair(0, getCoG6Time());
        if (stripTimeReco.EqualTo("CoG3"))
          return getCoG3FirstFrameAndTime();
        if (stripTimeReco.EqualTo("ELS3"))
          return getELS3FirstFrameAndTime();
      }

      //we should NEVER get here, if we do, we should check better at the step before (SVDRecoDigitCreator or cluster reconstruction classes
      B2WARNING("strip time algorithm specified in SVDRecoConfiguration not found, using CoG6");
      return std::make_pair(0, getCoG6Time());

    }


    double SVDTimeReconstruction::getStripTimeError(const TString& timeAlgo = "fromRecoDBObject")
    {

      StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
      std::string m_svdEventInfoName = "SVDEventInfo";
      if (!temp_eventinfo.isValid())
        m_svdEventInfoName = "SVDEventInfoSim";
      StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
      if (!eventinfo) B2ERROR("No SVDEventInfo!");

      //default
      TString stripTimeReco = "CoG6";

      if (m_recoConfig.isValid()) {
        if (eventinfo->getNSamples() == 6)
          stripTimeReco = m_recoConfig->getStripTimeRecoWith6Samples();
        else
          stripTimeReco = m_recoConfig->getStripTimeRecoWith3Samples();
      }

      if (! timeAlgo.EqualTo("fromRecoDBObject"))
        stripTimeReco = timeAlgo;

      if (m_timeAlgorithms.find(stripTimeReco) != m_timeAlgorithms.end()) {
        if (stripTimeReco.EqualTo("CoG6"))
          return getCoG6TimeError();
        if (stripTimeReco.EqualTo("CoG3"))
          return getCoG3TimeError();
        if (stripTimeReco.EqualTo("ELS3"))
          return getELS3TimeError();
      }

      //we should NEVER get here, if we do, we should check better at the step before (SVDRecoDigitCreator or cluster reconstruction classes
      B2WARNING("strip time algorithm specified in SVDRecoConfiguration not found, using CoG6");
      return getCoG6TimeError();

    }

    double SVDTimeReconstruction::getCoG6Time()
    {

      //check if we can continue
      weCanContinue();

      //calculate weighted average
      float time = 0;
      float sumAmplitudes = 0;
      for (int k = 0; k < 6; k ++) {
        time += k * m_samples[k];
        sumAmplitudes += m_samples[k];
      }
      if (sumAmplitudes != 0) {
        time /= (sumAmplitudes);
        time *= m_apvClockPeriod;
      } else {
        time = -1;
        B2WARNING("Trying to divide by 0 (ZERO)! Sum of amplitudes is nullptr! Skipping this SVDShaperDigit!");
      }

      //check cellID
      if (isnan(m_cellID) || isnan(m_triggerBin))
        B2FATAL("OOPS, we can't continue, you are probably using the wrong SVDReconstructionBase constructor");

      // correct by the CalPeak
      time -= m_PulseShapeCal.getPeakTime(m_vxdID, m_isUside, m_cellID);

      // calibrate (cellID not used)
      time =  m_CoG6TimeCal.getCorrectedTime(m_vxdID, m_isUside, m_cellID, time, m_triggerBin);

      return time;
    }

    double SVDTimeReconstruction::getCoG6TimeError()
    {

      //check if we can continue
      weCanContinue();

      //assuming that:
      // 1. noise of the samples are totally UNcorrelated (correct in MC)
      // 2. error on sampling time is negligible

      //sum of samples amplitudes
      float Atot = 0;
      //sum of time residuals squared
      float tmpResSq = 0;

      for (int k = 0; k < 6; k ++) {
        Atot  += m_samples[k];
        tmpResSq += TMath::Power(k * m_apvClockPeriod -  getCoG6Time(), 2);
      }

      if (isnan(m_averageNoiseInADC))
        B2FATAL("OOPS, we can't continue, you have to set the average noise!");

      double rawTimeError = m_averageNoiseInADC / Atot * TMath::Sqrt(tmpResSq);

      //cellID not used for calibration
      if (isnan(m_cellID))
        m_cellID = 1;

      if (isnan(m_triggerBin))
        B2FATAL("OOPS, we can't continue, you have to set the trigger bin!");


      double timeError = m_CoG6TimeCal.getCorrectedTimeError(m_vxdID, m_isUside, m_cellID, getCoG6Time(),
                                                             rawTimeError, m_triggerBin);

      return timeError;

    }

    std::pair<int, double> SVDTimeReconstruction::getCoG3FirstFrameAndTime()
    {

      //check if we can continue
      weCanContinue();


      //take the MaxSum 3 samples
      SVDMaxSumAlgorithm maxSum = SVDMaxSumAlgorithm(m_samples);
      int firstFrame = maxSum.getFirstFrame();
      std::vector<float> selectedSamples = maxSum.getSelectedSamples();

      auto begin = selectedSamples.begin();
      const auto end = selectedSamples.end();

      auto retval = 0., norm = 0.;
      for (auto step = 0.; begin != end; ++begin, step += m_apvClockPeriod) {
        norm += static_cast<double>(*begin);
        retval += static_cast<double>(*begin) * step;
      }
      float rawtime = retval / norm;

      if (isnan(m_cellID))
        m_cellID = 1;

      if (isnan(m_triggerBin))
        B2FATAL("OOPS, we can't continue, you have to set the trigger bin!");

      //cellID not used for calibration
      double time = m_CoG3TimeCal.getCorrectedTime(m_vxdID, m_isUside, m_cellID, rawtime, m_triggerBin);

      return std::make_pair(firstFrame, time);

    }

    double SVDTimeReconstruction::getCoG3TimeError()
    {

      //check if we can continue
      weCanContinue();

      //NOTE: computed with the same algorithm as COG6 strip raw time error, does not take into account calibration!

      //take the MaxSum 3 samples
      SVDMaxSumAlgorithm maxSum = SVDMaxSumAlgorithm(m_samples);
      std::vector<float> selectedSamples = maxSum.getSelectedSamples();

      auto begin = selectedSamples.begin();
      const auto end = selectedSamples.end();


      //sum of samples amplitudes
      float Atot = 0;
      //sum of time residuals squared
      float tmpResSq = 0;

      for (auto step = 0.; begin != end; ++begin, step += m_apvClockPeriod) {
        Atot += static_cast<double>(*begin);
        tmpResSq += TMath::Power(step - getCoG3FirstFrameAndTime().second, 2);
      }

      //compute average noise
      /*      int aveNoise = 0;
      for (auto s : m_rawCluster.getStripsInRawCluster())
        aveNoise += s.noise * s.noise;

      aveNoise = TMath::Sqrt(aveNoise);
      */

      if (isnan(m_averageNoiseInADC))
        B2FATAL("OOPS, we can't continue, you have to set the average noise!");

      return m_averageNoiseInADC / Atot * TMath::Sqrt(tmpResSq);

    }


    std::pair<int, double> SVDTimeReconstruction::getELS3FirstFrameAndTime()
    {

      //check if we can continue
      weCanContinue();


      //take the MaxSum 3 samples
      SVDMaxSumAlgorithm maxSum = SVDMaxSumAlgorithm(m_samples);
      int firstFrame = maxSum.getFirstFrame();
      std::vector<float> selectedSamples = maxSum.getSelectedSamples();

      auto begin = selectedSamples.begin();

      //calculate 'raw' ELS hit-time
      const double E = std::exp(- m_apvClockPeriod / m_ELS3tau);
      const double E2 = E * E;
      const double E4 = E * E * E * E;
      const double a0 = (*begin);
      const double a1 = (*(begin + 1));
      const double a2 = (*(begin + 2));
      const double w = (a0 -  E2 * a2) / (2 * a0 + E * a1);

      auto num  = 2 * E4 + w * E2;
      auto den =  1 - E4 - w * (2 + E2);

      float rawtime = - m_apvClockPeriod * num / den;

      //cellID is not used for calibration
      if (isnan(m_cellID))
        m_cellID = 1;

      if (isnan(m_triggerBin))
        B2FATAL("OOPS, we can't continue, you have to set the trigger bin!");

      double time = m_ELS3TimeCal.getCorrectedTime(m_vxdID, m_isUside, m_cellID, rawtime, m_triggerBin);

      return std::make_pair(firstFrame, time);

    }


    double SVDTimeReconstruction::getELS3TimeError()
    {

      //check if we can continue
      weCanContinue();

      //take the MaxSum 3 samples
      SVDMaxSumAlgorithm maxSum = SVDMaxSumAlgorithm(m_samples);
      std::vector<float> selectedSamples = maxSum.getSelectedSamples();
      auto begin = selectedSamples.begin();

      //calculate 'raw' ELS hit-time error
      //neglecting errors from the calibration function

      const double E = std::exp(- m_apvClockPeriod / m_ELS3tau);
      const double E2 = E * E;
      const double E3 = E * E * E;
      const double E4 = E * E * E * E;
      const double a0 = (*begin);
      const double a1 = (*begin + 1);
      const double a2 = (*begin + 2);
      const double w = (a0 -  E2 * a2) / (2 * a0 + E * a1);

      //computing dT/dw:
      double num = E2 * (1 + 4 * E2 + E4);
      double den = TMath::Power(1 - E4 - w * (2 + E2), 2);
      double dTdw = - m_apvClockPeriod * num / den;

      //computing dw/da0
      num = E * a1 + 2 * E2 * a2;
      den = TMath::Power(2 * a0 + E * a1, 2);
      double dwda0 = num / den;

      //computing dw/da1
      num = -E * a0 + E3 * a2;
      den = TMath::Power(2 * a0 + E * a1, 2);
      double dwda1 = num / den;

      //computing dw/da2
      num = -E2;
      den = 2 * a0 + E * a1;
      double dwda2 = num / den;

      //error on a0,a1,a2 are equal (independent on the sample)
      //computing delta_a = sum in quadrature of the noise in electrons of the strips in the cluster = m_averageNoise

      if (isnan(m_averageNoiseInADC))
        B2FATAL("OOPS, we can't continue, you have to set the average noise!");

      double timeError = std::abs(m_averageNoiseInADC * dTdw) * std::sqrt(dwda0 * dwda0 + dwda1 * dwda1 + dwda2 * dwda2);
      //      B2INFO("m_averageNoise = " << m_averageNoiseInADC << ", dTdw = " << dTdw << ", dwda0 = " << dwda0 << ", dwda1 = " << dwda1 << ", dwda2 = " << dwda2);

      return  timeError;
    }
  }  //SVD namespace
} //Belle2 namespace
