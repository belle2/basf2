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
#include <svd/dataobjects/SVDShaperDigith>
#include <svd/dataobjects/RawCluster.h>
#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Abstract Class representing the SVD strip time
     */
    class SVDStripTime {

    public:

      /**
       * Constructor to create a Strip from the SVDShaperDigit
       */
      SVDStripTime(const SVDShaperDigit& shaper)
      {
        m_samples = shaper.getSamples();
        m_vxdID = shaper.getSensorID();
        m_isUside = shaper.isUStrip();
        m_cellID = shaper.getCellID();
        SVDELS3Time helper;
        m_apvClockPeriod = helper.getAPVClockPeriod();
        m_ELS3tau = helper.getTau();
      };

      /**
       * Constructor to create a strip from a stripInRawCluster
       */
      SVDStripTime(const Belle2::SVD::RawCluster::stripInRawCluster& aStrip, VxdID sensorID, bool isU, int cellID)
      {
        m_samples = aStrip.samples;
        m_vxdID = sensorID;
        m_isUside = isU;
        m_cellID = cellID;
        SVDELS3Time helper;
        m_apvClockPeriod = helper.getAPVClockPeriod();
        m_ELS3tau = helper.getTau();
      };

      /**
       * set the trigger bin
       */
      void setTriggerBin(const int triggerBin)
      { m_triggerBin = triggerBin; };

      /**
       * virtual destructor
       */
      virtual ~SVDStripTime() {};

      /**
       * @return the VxdID of the strip sensor
       */
      VxdID getSensorID() {return m_vxdID;}

      /**
       * @return true if the strip is on the U/P side
       */
      bool isUSide() {return m_isUside;}

      /** CoG6 Time */
      double getCoG6StripTime() {return 0;}
      double getCoG6StripTimeError() {return 0;}
      /** CoG3 Time */
      double getCoG3StripTime() {return 0;}
      double getCoG3StripTimeError() {return 0;}
      /** ELS3 Time */
      double getELS3StripTime() {return 0;}
      double getELS3StripTimeError() {return 0;}

    protected:

      /** APV clock period*/
      double m_apvClockPeriod = 0;
      /** ELS3 tau constant*/
      double m_ELS3tau = 0;

      /** trigger bin */
      int m_triggerBin = -1;

      /** strip samples*/
      Belle2::SVDShaperDigit::APVFloatSamples m_samples; /** ADC of the acquired samples*/

      /** VxdID of the strip */
      VxdID m_vxdID = 0;

      /** side of the strip */
      bool m_isUside = 0;

      /**cell ID */
      int m_cellID = 0;
    };

  }

}

