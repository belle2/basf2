/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <svd/reconstruction/SVDClusterCharge.h>
#include <svd/reconstruction/SVDMaxSumAlgorithm.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    void SVDClusterCharge::applyMaxSampleCharge(const Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR,
                                                double& seedCharge)
    {

      //get seed charge
      int seedCellID = rawCluster.getStripsInRawCluster().at(rawCluster.getSeedInternalIndex()).cellID;

      seedCharge = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), seedCellID,
                                                    rawCluster.getSeedMaxSample());

      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      //initialize noise and charge
      double noise = 0;
      charge = 0;


      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::StripInRawCluster strip = strips.at(i);

        double rawCharge = *std::max_element(begin(strip.samples), end(strip.samples));

        // calibrate (ADC -> electrons)
        double stripCharge = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), strip.cellID, rawCharge);

        double tmp_noise = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), strip.cellID, strip.noise);
        noise += tmp_noise * tmp_noise;

        charge += stripCharge;
      }

      SNR = charge / sqrt(noise);
    }

    void SVDClusterCharge::applySumSamplesCharge(const Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR,
                                                 double& seedCharge)
    {

      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      //initialize noise and charge
      double noise = 0;
      charge = 0;
      seedCharge = 0;

      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::StripInRawCluster strip = strips.at(i);

        double rawCharge = 0;
        for (auto sample : strip.samples)
          rawCharge += sample;

        // calibrate (ADC -> electrons)
        double stripCharge = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), strip.cellID, rawCharge);

        if (stripCharge > seedCharge)
          seedCharge = stripCharge;

        double tmp_noise = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), strip.cellID, strip.noise);
        noise += tmp_noise * tmp_noise;

        charge += stripCharge;
      }

      SNR = charge / sqrt(noise);

    }


    void SVDClusterCharge::applyELS3Charge(const Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR, double& seedCharge)
    {

      // ISSUES:
      // 1. samples always in electrons for charge
      // 2. hardcoded ELS3 tau
      // 3. seed charge is maxSample

      float   m_ELS3tau = 55;

      //get seed charge
      int seedCellID = rawCluster.getStripsInRawCluster().at(rawCluster.getSeedInternalIndex()).cellID;

      seedCharge = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), seedCellID,
                                                    rawCluster.getSeedMaxSample());

      //initialize noise and charge
      double noise = 0;
      charge = 0;

      //take the MaxSum 3 samples
      SVDMaxSumAlgorithm maxSum = SVDMaxSumAlgorithm(rawCluster.getClsSamples(false));
      std::vector<float> selectedSamples = maxSum.getSelectedSamples();

      auto begin = selectedSamples.begin();

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

      a0 = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), rawCluster.getStripsInRawCluster()[0].cellID,
                                            a0);
      a1 = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), rawCluster.getStripsInRawCluster()[0].cellID,
                                            a1);
      a2 = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), rawCluster.getStripsInRawCluster()[0].cellID,
                                            a2);


      double num = (1. / E - E3) * a1 + (2 + E2) * a2 - (1 + 2 * E2) * a0;
      double den =  m_apvClockPeriod / m_ELS3tau * std::exp(1 + rawtime / m_ELS3tau) * (1 + 4 * E2 + E4);

      charge = num / den;

      //compute Noise
      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::StripInRawCluster strip = strips.at(i);

        double tmp_noise = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), strip.cellID, strip.noise);
        noise += tmp_noise * tmp_noise;
      }

      SNR = charge / sqrt(noise);

    }


  }  //SVD namespace
} //Belle2 namespace
