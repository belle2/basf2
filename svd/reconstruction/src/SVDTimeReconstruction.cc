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
#include <svd/dataobjects/SVDEventInfo.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDTimeReconstruction::getStripTime()
    {

      StoreObjPtr<SVDEventInfo> eventInfo;

      //default
      TString stripTimeReco = "CoG6";

      if (m_recoConfig.isValid()) {
        if (eventInfo->getNSamples() == 6)
          stripTimeReco = m_recoConfig->getStripTimeRecoWith6Samples();
        else
          stripTimeReco = m_recoConfig->getStripTimeRecoWith3Samples();
      }

      if (m_timeAlgorithms.find(stripTimeReco) != m_timeAlgorithms.end()) {
        if (stripTimeReco.EqualTo("COG6"))
          return getCoG6Time();
        if (stripTimeReco.EqualTo("COG3"))
          return getCoG3Time();
        if (stripTimeReco.EqualTo("ELS3"))
          return getELS3Time();
      }

      B2WARNING("strip time algorithm specified in SVDRecoConfiguration not found, using CoG6");
      return getCoG6Time();

    }


    double SVDTimeReconstruction::getStripTimeError()
    {

      StoreObjPtr<SVDEventInfo> eventInfo;

      //default
      TString stripTimeReco = "CoG6";

      if (m_recoConfig.isValid()) {
        if (eventInfo->getNSamples() == 6)
          stripTimeReco = m_recoConfig->getStripTimeRecoWith6Samples();
        else
          stripTimeReco = m_recoConfig->getStripTimeRecoWith3Samples();
      }

      if (m_timeAlgorithms.find(stripTimeReco) != m_timeAlgorithms.end()) {
        if (stripTimeReco.EqualTo("COG6"))
          return getCoG6TimeError();
        if (stripTimeReco.EqualTo("COG3"))
          return getCoG3TimeError();
        if (stripTimeReco.EqualTo("ELS3"))
          return getELS3TimeError();
      }

      B2WARNING("strip time algorithm specified in SVDRecoConfiguration not found, using CoG6");
      return getCoG6TimeError();

    }

    double SVDTimeReconstruction::getCoG6Time()
    {

      m_firstFrame = 0;

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

      // correct by the CalPeak
      time -= m_PulseShapeCal.getPeakTime(m_vxdID, m_isUside, m_cellID);

      // calibrate
      time =  m_CoG6TimeCal.getCorrectedTime(m_vxdID, m_isUside, m_cellID, time, m_triggerBin);

      return time;
    }

    double SVDTimeReconstruction::getCoG6TimeError()
    {

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

      double noise = m_noiseCal.getNoise(m_vxdID, m_isUside, m_cellID);
      double rawTimeError = noise / Atot * TMath::Sqrt(tmpResSq);

      double timeError = m_CoG6TimeCal.getCorrectedTimeError(m_vxdID, m_isUside, m_cellID, getCoG6Time(),
                                                             rawTimeError, m_triggerBin);

      return timeError;

    }


  }  //SVD namespace
} //Belle2 namespace
