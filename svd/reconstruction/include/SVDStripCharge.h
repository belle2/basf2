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

#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Abstract Class representing the SVD strip charge
     */
    class SVDStripCharge {

    public:

      /**
       * Constructor to create a Strip from the SVDShaperDigit
       */
      SVDStripCharge(const SVDShaperDigit& shaper)
      {
        m_samples = shaper.getSamples();
        m_vxdID = shaper.getSensorID();
        m_isUside = shaper.isUStrip();
        m_cellID = shaper.getCellID();
        //        SVDELS3Charge helper;
        //        m_apvClockPeriod = helper.getAPVClockPeriod();
        //        m_ELS3tau = helper.getTau();
      };

      /**
       * Constructor to create a strip from a stripInRawCluster
       */
      SVDStripCharge(const Belle2::SVD::stripInRawCluster& aStrip, VxdID sensorID, bool isU, int cellID)
      {
        m_samples = aStrip.samples;
        m_vxdID = sensorID;
        m_isUside = isU;
        m_cellID = cellID;
        //        SVDELS3Charge helper;
        //        m_apvClockPeriod = helper.getAPVClockPeriod();
        //        m_ELS3tau = helper.getTau();
      };


      /**
       * virtual destructor
       */
      virtual ~SVDStripCharge() {};

      /**
       * @return the VxdID of the strip sensor
       */
      VxdID getSensorID() {return m_vxdID;}

      /**
       * @return true if the strip is on the U/P side
       */
      bool isUSide() {return m_isUside;}

      /** get strip charge as set in SVDRecoConfiguration payload*/
      double getStripCharge();
      /** get strip charge error as set in SVDRecoConfiguration payload*/
      double getStripChargeError();

      /** CoG6 Charge */
      double getMaxSampleCharge();
      /** CoG6 Charge Error*/
      double getMaxSampleChargeError();
      /** CoG3 Charge */
      double getSumSamplesCharge();
      /** CoG3 Charge Error*/
      double getSumSamplesChargeError();
      /** ELS3 Charge */
      double getELS3StripCharge() {return 0;}
      /** ELS3 Charge Error */
      double getELS3StripChargeError() {return 0;}

    protected:

      /** APV clock period*/
      double m_apvClockPeriod = 0;
      /** ELS3 tau constant*/
      double m_ELS3tau = 0;

      /** strip samples*/
      Belle2::SVDShaperDigit::APVFloatSamples m_samples; /** ADC of the acquired samples*/

      /** VxdID of the strip */
      VxdID m_vxdID = 0;

      /** side of the strip */
      bool m_isUside = 0;

      /**cell ID */
      int m_cellID = 0;

      SVDPulseShapeCalibrations m_PulseShapeCal; /**<SVDPulseShape calibration wrapper*/

    };

  }

}

