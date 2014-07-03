/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LINEARGLOBALADCCOUNTTRANSLATOR_H
#define LINEARGLOBALADCCOUNTTRANSLATOR_H

#include <cdc/dataobjects/ADCCountTranslatorBase.h>

namespace Belle2 {
  namespace CDC {
    /** This class simply assumes a linear translation through (0,0)*/
    class LinearGlobalADCCountTranslator : public ADCCountTranslatorBase {
    public:
      /** Constructor with a default calibration constant. */
      LinearGlobalADCCountTranslator(const float conversionFactor = 3.2 * 1e-8) :
        m_conversionFactor(conversionFactor)
      {}

      /** Destructor. */
      ~LinearGlobalADCCountTranslator() {}

      const float EnergyLossOffset =  conversionFactor / 2.; // eV

      /** just multiply with the conversion factor and return. */
      float getCharge(unsigned short adcCount,
                      const WireID&,
                      bool,
                      float, float) {
        return (adcCount * m_conversionFactor + EnergyLossOffset);
      }

    private:
      /** Conversion factor as determined from calibration. */
      float m_conversionFactor;
    };
  }
}
#endif /* LINEARGLOBALADCCOUNTTRANSLATOR_H */
