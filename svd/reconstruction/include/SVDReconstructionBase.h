/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/reconstruction/RawCluster.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDCoGTimeCalibrations.h>
#include <svd/calibration/SVD3SampleCoGTimeCalibrations.h>
#include <svd/calibration/SVD3SampleELSTimeCalibrations.h>
#include <svd/dbobjects/SVDRecoConfiguration.h>

#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Abstract Class providing constructors to the SVD Reconstruction classes
     */
    class SVDReconstructionBase {

    public:

      /**
       * default copy constructor
       */
      SVDReconstructionBase(const SVDReconstructionBase&) = default;

      /**
       * default constructor
       */
      explicit SVDReconstructionBase() { buildAlgorithmSets(); };


      /**
       * Constructor with the SVDShaperDigit
       * for strip reconstruction by the SVDRecoDigitCreator
       */
      explicit SVDReconstructionBase(const SVDShaperDigit& shaper)
        : m_samples(shaper.getSamples())
        , m_vxdID(shaper.getSensorID())
        , m_isUside(shaper.isUStrip())
        , m_cellID(shaper.getCellID())
      {
        buildAlgorithmSets();
      };

      /**
       * Constructor with the StripInRawCluster,
       * for strip reconstruction in cluster reconstruction
       */
      explicit SVDReconstructionBase(const Belle2::SVD::StripInRawCluster& aStrip, VxdID sensorID, bool isU)
        : m_samples(aStrip.samples)
        , m_vxdID(sensorID)
        , m_isUside(isU)
        , m_cellID(aStrip.cellID)
      {
        buildAlgorithmSets();
      };

      /**
       * Constructor with the SVDShaperDigit::APVFloatSamples,
       * for clustered sample reconstruction
       * cellID data member not significant
       */
      explicit SVDReconstructionBase(const Belle2::SVDShaperDigit::APVFloatSamples samples, VxdID sensorID, bool isU)
        : m_samples(samples)
        , m_vxdID(sensorID)
        , m_isUside(isU)
        , m_cellID(std::numeric_limits<int>::quiet_NaN())
      {
        buildAlgorithmSets();
      };


      /**
       * virtual destructor
       */
      virtual ~SVDReconstructionBase() {};

      /**
       * set the trigger bin
       */
      void setTriggerBin(const int triggerBin)
      { m_triggerBin = triggerBin; };

      /**
       * set the average noise
       */
      void setAverageNoise(const int averageNoiseInADC, const int averageNoiseInElectrons)
      {
        m_averageNoiseInADC = averageNoiseInADC;
        m_averageNoiseInElectrons = averageNoiseInElectrons;
      };

      /**
       * @return the VxdID of the strip sensor
       */
      VxdID getSensorID() {return m_vxdID;}

      /**
       * @return true if the strip is on the U/P side
       */
      bool isUSide() {return m_isUside;}

      /**
       * @return true if the timeAlg is implemented and available for reconstruction
       */
      bool isTimeAlgorithmAvailable(TString timeAlg)
      {
        return m_timeAlgorithms.find(timeAlg) != m_timeAlgorithms.end();
      }

      /**
       * @return true if the chargeAlg is implemented and available for reconstruction
       */
      bool isChargeAlgorithmAvailable(TString chargeAlg)
      {
        return m_chargeAlgorithms.find(chargeAlg) != m_chargeAlgorithms.end();
      }

      /**
       * @return true if the positionAlg is implemented and available for reconstruction
       */
      bool isPositionAlgorithmAvailable(TString positionAlg)
      {
        return m_positionAlgorithms.find(positionAlg) != m_positionAlgorithms.end();
      }


    protected:

      /** check that the bare minimum set of informations are there */
      bool weCanContinue()
      {

        bool weCanContinue = true;
        if ((int)m_vxdID == 0) {
          weCanContinue = false;
          B2FATAL("OOPS, we can't continue, you are probably using the wrong SVDReconstructionBase constructor");
        }

        return weCanContinue;

      };



      /** APV clock period*/
      double m_apvClockPeriod = 16000. / 509;

      /** ELS3 tau constant*/
      double m_ELS3tau = 55;

      /** strip ADC sampless*/
      Belle2::SVDShaperDigit::APVFloatSamples m_samples;

      /** VxdID of the strip */
      VxdID m_vxdID = 0;

      /** side of the strip */
      bool m_isUside = 0;

      /**cell ID */
      int m_cellID = std::numeric_limits<int>::quiet_NaN();

      /** trigger bin */
      int m_triggerBin = std::numeric_limits<int>::quiet_NaN();

      /** average noise in ADC as sum in quadrature of noise of each strip*/
      float m_averageNoiseInADC = std::numeric_limits<int>::quiet_NaN();

      /** average noise in electrons as sum in quadrature of noise of each strip*/
      float m_averageNoiseInElectrons = std::numeric_limits<int>::quiet_NaN();

      /** set containing the available time algorithms */
      std::set<TString> m_timeAlgorithms;

      /** set containing the available charge algorithms */
      std::set<TString> m_chargeAlgorithms;

      /** set containing the available position algorithms */
      std::set<TString> m_positionAlgorithms;

      /** build the sets containing the available time, charge and position algorithms */
      void buildAlgorithmSets()
      {
        m_timeAlgorithms.insert("CoG6");
        m_timeAlgorithms.insert("CoG3");
        m_timeAlgorithms.insert("ELS3");

        m_chargeAlgorithms.insert("MaxSample");
        m_chargeAlgorithms.insert("SumSamples");
        m_chargeAlgorithms.insert("ELS3");

        m_positionAlgorithms.insert("CoGOnly");
        m_positionAlgorithms.insert("oldDefault");
      }


      /**Reconstruction Configuration dbobject*/
      DBObjPtr<SVDRecoConfiguration> m_recoConfig;

      /**SVDPulseShape calibration wrapper*/
      SVDPulseShapeCalibrations m_PulseShapeCal;
      /** Noise calibration wrapper*/
      SVDNoiseCalibrations m_NoiseCal;

      /** CoG6 time calibration wrapper*/
      SVDCoGTimeCalibrations m_CoG6TimeCal;
      /** CoG3 time calibration wrapper*/
      SVD3SampleCoGTimeCalibrations m_CoG3TimeCal;
      /** ELS3 time calibration wrapper*/
      SVD3SampleELSTimeCalibrations m_ELS3TimeCal;

    };

  }

}

