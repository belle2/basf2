/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <cdc/dataobjects/ADCCountTranslatorBase.h>

namespace Belle2 {
  namespace CDC {
    /** This class simply assumes a linear translation through (0,0)*/
    class LinearGlobalADCCountTranslator : public ADCCountTranslatorBase {
    public:
      /** Constructor with a default calibration constant. */
      explicit LinearGlobalADCCountTranslator(const float conversionFactor = 3.2 * 1e-8) :
        m_conversionFactor(conversionFactor)
      {}

      /** Destructor. */
      ~LinearGlobalADCCountTranslator() {}

      /** just multiply with the conversion factor and return. */
      float getCharge(unsigned short adcCount,
                      const WireID&,
                      bool,
                      float, float)
      {
        //        const float EnergyLossOffset =  1.6 * 1e-8; // GeV
        //        return (adcCount * m_conversionFactor + EnergyLossOffset);

        //round-down -> round-up to be consistent with real adc module
        return (static_cast<float>(adcCount) - 0.5) * m_conversionFactor;
      }

    private:
      /** Conversion factor as determined from calibration. */
      float m_conversionFactor;
    };
  }
}
