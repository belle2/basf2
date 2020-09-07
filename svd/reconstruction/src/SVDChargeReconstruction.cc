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
#include <svd/reconstruction/SVDChargeReconstruction.h>
#include <svd/reconstruction/SVDMaxSumAlgorithm.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDChargeReconstruction::getStripCharge(TString chargeAlgo = "fromRecoDBObject")
    {

      StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
      std::string m_svdEventInfoName = "SVDEventInfo";
      if (!temp_eventinfo.isValid())
        m_svdEventInfoName = "SVDEventInfoSim";
      StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
      if (!eventinfo) B2ERROR("No SVDEventInfo!");


      //default
      TString stripChargeReco = "MaxSample";

      if (m_recoConfig.isValid()) {
        if (eventinfo->getNSamples() == 6)
          stripChargeReco = m_recoConfig->getStripChargeRecoWith6Samples();
        else
          stripChargeReco = m_recoConfig->getStripChargeRecoWith3Samples();
      }

      if (! chargeAlgo.EqualTo("fromRecoDBObject"))
        stripChargeReco = chargeAlgo;

      if (m_chargeAlgorithms.find(stripChargeReco) != m_chargeAlgorithms.end()) {
        if (stripChargeReco.EqualTo("MaxSample"))
          return getMaxSampleCharge();
        if (stripChargeReco.EqualTo("SumSamples"))
          return getSumSamplesCharge();
        if (stripChargeReco.EqualTo("ELS3"))
          return getELS3Charge();
      }

      //we should NEVER get here, if we do, we should check better at the step before (SVDRecoDigitCreator or cluster reconstruction classes)
      B2WARNING("strip charge algorithm specified in SVDRecoConfiguration not found, using MaxSample");
      return getMaxSampleCharge();
    }

    double SVDChargeReconstruction::getStripChargeError(TString chargeAlgo = "fromRecoDBObject")
    {

      StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
      std::string m_svdEventInfoName = "SVDEventInfo";
      if (!temp_eventinfo.isValid())
        m_svdEventInfoName = "SVDEventInfoSim";
      StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
      if (!eventinfo) B2ERROR("No SVDEventInfo!");


      //default
      TString stripChargeReco = "MaxSample";

      if (m_recoConfig.isValid()) {
        if (eventinfo->getNSamples() == 6)
          stripChargeReco = m_recoConfig->getStripChargeRecoWith6Samples();
        else
          stripChargeReco = m_recoConfig->getStripChargeRecoWith3Samples();
      }

      if (! chargeAlgo.EqualTo("fromRecoDBObject"))
        stripChargeReco = chargeAlgo;

      if (m_chargeAlgorithms.find(stripChargeReco) != m_chargeAlgorithms.end()) {
        if (stripChargeReco.EqualTo("MaxSample"))
          return getMaxSampleChargeError();
        if (stripChargeReco.EqualTo("SumSamples"))
          return getSumSamplesChargeError();
        if (stripChargeReco.EqualTo("ELS3"))
          return getELS3ChargeError();
      }

      //we should NEVER get here, if we do, we should check better at the step before (SVDRecoDigitCreator or cluster reconstruction classes)
      B2WARNING("strip charge algorithm specified in SVDRecoConfiguration not found, using MaxSample");

      return getMaxSampleChargeError();

    }

    double SVDChargeReconstruction::getMaxSampleCharge()
    {
      double charge = 0;

      for (auto sample : m_samples)
        if (sample > charge) charge = sample;

      // calibrate (ADC -> electrons)
      charge = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, charge);

      return charge;
    }

    double SVDChargeReconstruction::getMaxSampleChargeError()
    {
      //the strip charge error is simply the noise
      return m_averageNoiseInElectrons;
    }

    double SVDChargeReconstruction::getSumSamplesCharge()
    {
      double charge = 0;

      for (auto sample : m_samples)
        charge += sample;

      // calibrate (ADC -> electrons)
      charge = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, charge);

      return charge;
    }

    double SVDChargeReconstruction::getSumSamplesChargeError()
    {
      //the strip charge error is simply the noise ?
      return m_averageNoiseInElectrons;
    }


    double SVDChargeReconstruction::getELS3Charge()
    {
      //take the MaxSum 3 samples
      SVDMaxSumAlgorithm maxSum = SVDMaxSumAlgorithm(m_samples);
      m_firstFrame = maxSum.getFirstFrame();
      std::vector<float> selectedSamples = maxSum.getSelectedSamples();

      auto begin = selectedSamples.begin();

      //calculate ELS hit-charge

      const double E = std::exp(- m_apvClockPeriod / m_ELS3tau);
      const double E2 = E * E;
      const double E3 = E * E * E;
      const double E4 = E * E * E * E;
      double a0 = (*begin);
      double a1 = (*(begin + 1));
      double a2 = (*(begin + 2));

      //compute raw time
      const double w = (a0 -  E2 * a2) / (2 * a0 + E * a1);
      auto rawtime_num  = 2 * E4 + w * E2;
      auto rawtime_den =  1 - E4 - w * (2 + E2);
      float rawtime = - m_apvClockPeriod * rawtime_num / rawtime_den;

      //convert samples in electrons
      a0 = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, a0);
      a1 = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, a1);
      a2 = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, a2);


      double num = (1. / E - E3) * a1 + (2 + E2) * a2 - (1 + 2 * E2) * a0;
      double den =  m_apvClockPeriod / m_ELS3tau * std::exp(1 + rawtime / m_ELS3tau) * (1 + 4 * E2 + E4);

      double charge = num / den;

      return charge;
    }

    double SVDChargeReconstruction::getELS3ChargeError()
    {
      //take the MaxSum 3 samples
      SVDMaxSumAlgorithm maxSum = SVDMaxSumAlgorithm(m_samples);
      m_firstFrame = maxSum.getFirstFrame();
      std::vector<float> selectedSamples = maxSum.getSelectedSamples();

      auto begin = selectedSamples.begin();

      //calculate ELS hit-charge error
      const double E = std::exp(- m_apvClockPeriod / m_ELS3tau);
      const double E2 = E * E;
      const double E3 = E * E * E;
      const double E4 = E * E * E * E;
      double a0 = (*begin);
      double a1 = (*(begin + 1));
      double a2 = (*(begin + 2));

      //compute raw time
      double w = (a0 -  E2 * a2) / (2 * a0 + E * a1);
      auto rawtime_num  = 2 * E4 + w * E2;
      auto rawtime_den =  1 - E4 - w * (2 + E2);
      float rawtime = - m_apvClockPeriod * rawtime_num / rawtime_den;

      //convert samples in electrons
      a0 = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, a0);
      a1 = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, a1);
      a2 = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, a2);

      //recompute w in electrons
      w = (a0 -  E2 * a2) / (2 * a0 + E * a1);

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


      //common pieces to compute dAda0,dAda1,dAda2
      double denominator =  m_apvClockPeriod / m_ELS3tau * std::exp(1 + rawtime / m_ELS3tau) * (1 + 4 * E2 + E4);
      double factor = getELS3Charge() / m_ELS3tau *  dTdw;

      //computing dAda0
      double dAda0 = - (1 + 2 * E2) / denominator - factor * dwda0;

      //computing dAda1
      double dAda1 = (1. / E - E3) / denominator - factor * dwda1;

      //computing dAda2
      double dAda2 = (2 + E2) / denominator - factor * dwda2;

      double chargeError = std::abs(m_averageNoiseInElectrons) * std::sqrt(dAda0 * dAda0 + dAda1 * dAda1 + dAda2 * dAda2);

      return  chargeError;
    }

  }  //SVD namespace
} //Belle2 namespace
