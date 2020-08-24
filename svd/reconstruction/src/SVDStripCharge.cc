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
#include <svd/reconstruction/SVDStripCharge.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDStripCharge::getStripCharge()
    {

      return getMaxSampleCharge();
    }

    double SVDStripCharge::getStripChargeError()
    {

      return getMaxSampleChargeError();
    }

    double SVDStripCharge::getMaxSampleCharge()
    {

      double charge = 0;

      for (auto sample : m_samples)
        if (sample > charge) charge = sample;

      // calibrate (ADC -> electrons)
      charge = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, charge);

      return charge;
    }

    double SVDStripCharge::getMaxSampleChargeError()
    {

      //the strip charge error is simply the noise

      double noise = 0;

      // calibrate (ADC -> electrons)
      noise = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, noise);
      return noise;

    }

    double SVDStripCharge::getSumSamplesCharge()
    {

      double charge = 0;

      for (auto sample : m_samples)
        charge += sample;

      // calibrate (ADC -> electrons)
      charge = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, charge);

      return charge;
    }

    double SVDStripCharge::getSumSamplesChargeError()
    {

      //the strip charge error is simply the noise ?

      double noise = 0;

      // calibrate (ADC -> electrons)
      noise = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, m_cellID, noise);
      return noise;

    }


  }  //SVD namespace
} //Belle2 namespace
