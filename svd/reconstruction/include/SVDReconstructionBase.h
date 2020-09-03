/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
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
       * Constructor with the SVDShaperDigit
       * for strip reconstruction by the SVDRecoDigitCreator
       */
      explicit SVDReconstructionBase(const SVDShaperDigit& shaper)
        : m_samples(shaper.getSamples())
        , m_vxdID(shaper.getSensorID())
        , m_isUside(shaper.isUStrip())
        , m_cellID(shaper.getCellID())
        , m_samplesInElectrons(false)
      {
        buildAlgorithmSets();
      };

      /**
       * Constructor with the stripInRawCluster,
       * for strip reconstruction in cluster reconstruction
       */
      SVDReconstructionBase(const Belle2::SVD::stripInRawCluster& aStrip, VxdID sensorID, bool isU, int cellID,
                            bool samplesInElectrons = false)
        : m_samples(aStrip.samples)
        , m_vxdID(sensorID)
        , m_isUside(isU)
        , m_cellID(cellID)
        , m_samplesInElectrons(samplesInElectrons)
      { };

      /**
       * Constructor with the SVDShaperDigit::APVFloatSamples,
       * for clustered sample reconstruction
       * cellID data member not significant
       */
      SVDReconstructionBase(const Belle2::SVDShaperDigit::APVFloatSamples samples, VxdID sensorID, bool isU,
                            bool samplesInElectrons = false)
        : m_samples(samples)
        , m_vxdID(sensorID)
        , m_isUside(isU)
        , m_cellID(-1)
        , m_samplesInElectrons(samplesInElectrons)
      { };


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
      void setAverageNoise(const int averageNoise)
      { m_averageNoise = averageNoise; };

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

    protected:

      /** APV clock period*/
      double m_apvClockPeriod = 16000. / 509;

      /** ELS3 tau constant*/
      double m_ELS3tau = 55;

      /** strip samples, can be in ADC or electrons, check m_sampleInElectrons*/
      Belle2::SVDShaperDigit::APVFloatSamples m_samples;

      /** VxdID of the strip */
      VxdID m_vxdID = 0;

      /** side of the strip */
      bool m_isUside = 0;

      /**cell ID */
      int m_cellID = 0;

      /** true is m_samples is in electrons*/
      bool m_samplesInElectrons = false;

      /** trigger bin */
      int m_triggerBin = -1;

      /** first frame */
      int m_firstFrame = -1;

      /** average noise */
      float m_averageNoise = -1;

      /** set containing the available time algorithms */
      std::set<TString> m_timeAlgorithms;

      /** build the set containing the available time algorithms */
      void buildAlgorithmSets()
      {
        m_timeAlgorithms.insert("CoG6");
        m_timeAlgorithms.insert("CoG3");
        m_timeAlgorithms.insert("ELS3");
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

