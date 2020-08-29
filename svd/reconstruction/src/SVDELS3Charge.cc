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
#include <svd/reconstruction/SVDELS3Charge.h>

#include <svd/reconstruction/SVDELS3Time.h>
#include <svd/calibration/SVDNoiseCalibrations.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDELS3Charge::getClusterCharge()
    {

      //take raw cluster charge
      double charge = getClusterRawCharge();

      //compute gain as weighted average, weighting strip gain (e-/ADC) with the strip max sample

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double weight = 0;
      double sumMaxSamples = 0;
      for (auto s : strips) {
        weight += s.maxSample * m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, s.cellID, 1);

        sumMaxSamples += s.maxSample;
      }
      weight = weight / sumMaxSamples;

      charge = weight * charge;

      return charge;
    }

    double SVDELS3Charge::getClusterRawCharge()
    {

      //take the MaxSum 3 samples
      std::vector<float> clustered3s = m_rawCluster.getMaxSum3Samples().second;
      auto begin = clustered3s.begin();

      //calculate 'raw' ELS hit-charge

      SVDELS3Time helper;
      helper.setRawCluster(m_rawCluster);

      double tau = helper.getTau();
      double apv = helper.getAPVClockPeriod();
      double rawtime = helper.getClusterRawTime();

      const double E = std::exp(- apv / tau);
      const double E2 = E * E;
      const double E3 = E * E * E;
      const double E4 = E * E * E * E;
      const double a0 = (*begin);
      const double a1 = (*(begin + 1));
      const double a2 = (*(begin + 2));

      double num = (1. / E - E3) * a1 + (2 + E2) * a2 - (1 + 2 * E2) * a0;
      double den =  apv / tau * std::exp(1 + rawtime / tau) * (1 + 4 * E2 + E4);

      double rawcharge = num / den;

      return  rawcharge;

    }


    double SVDELS3Charge::getClusterChargeError()
    {
      //take the MaxSum 3 samples
      std::vector<float> clustered3s = m_rawCluster.getMaxSum3Samples().second;
      auto begin = clustered3s.begin();

      //calculate  ELS amplitude error
      SVDELS3Time helper;
      helper.setRawCluster(m_rawCluster);
      double tau = helper.getTau();
      double apv = helper.getAPVClockPeriod();
      double rawtime = helper.getClusterRawTime();

      const double E = std::exp(- apv / tau);
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
      double dTdw = - apv * num / den;

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
      //computing delta_a = sum in quadrature of the noise in electrons of the strips in the cluster
      std::vector<stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();
      double deltaa = 0;
      SVDNoiseCalibrations noise;
      for (auto strip : strips)
        deltaa += noise.getNoiseInElectrons(m_vxdID, m_isUside, strip.cellID);
      deltaa = std::sqrt(deltaa);

      // up to here COPIED from SVDELS3Time::getClusterTimeError


      //common pieces to compute dAda0,dAda1,dAda2
      double denominator =  apv / tau * std::exp(1 + rawtime / tau) * (1 + 4 * E2 + E4);
      double factor = getClusterCharge() / tau *  dTdw;

      //computing dAda0
      double dAda0 = - (1 + 2 * E2) / denominator - factor * dwda0;

      //computing dAda1
      double dAda1 = - (1. / E - E3) / denominator - factor * dwda1;

      //computing dAda2
      double dAda2 = - (2 + E2) / denominator - factor * dwda2;

      double chargeError = std::abs(deltaa) * std::sqrt(dAda0 * dAda0 + dAda1 * dAda1 + dAda2 * dAda2);

      return  chargeError;
    }


    double SVDELS3Charge::getClusterSeedCharge()
    {

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double rawSeedCharge = m_rawCluster.getSeedMaxSample();
      double seedCellID = strips.at(m_rawCluster.getSeedInternalIndex()).cellID;

      double seedCharge = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, seedCellID, rawSeedCharge);

      return seedCharge;

    }

  }  //SVD namespace
} //Belle2 namespace
