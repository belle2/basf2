/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oskar Hartbrich                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  namespace TOP {

    /**
     * Feature extraction data
     */
    struct FeatureExtractionData {
      int sampleRise = 0; /**< sample number just before 50% CFD crossing */
      int samplePeak = 0; /**< sample number at maximum */
      int sampleFall = 0; /**< same for falling edge */
      short vRise0 = 0;   /**< ADC value at sampleRise */
      short vRise1 = 0;   /**< ADC value at sampleRise + 1 */
      short vPeak = 0;    /**< ADC value at samplePeak */
      short vFall0 = 0;   /**< ADC value at sampleFall */
      short vFall1 = 0;   /**< ADC value at sampleFall + 1 */
      int integral = 0;   /**< integral of a pulse (e.g. \propto charge) */
    };
  }
}
