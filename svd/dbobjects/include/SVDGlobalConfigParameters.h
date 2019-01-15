/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Eugenio Paoloni                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING, ALBEIT INTENSE COHERENT LIGHT IS NOT USED AT ALL HERE:        *
 * PLEASE, PLEASE, PLEASE WEAR ALWAYS PROTECTIVE GLASSES                  *
 * WHEN LOOKING AT THIS FILE                                              *
 **************************************************************************/

#pragma once
#include "TObject.h"

namespace Belle2 {
  /**
   * This class defines the payload which stores to the central DB the
   * SVD global configuring parameters, and the methods to import and retrieve
   * these parameters.
   *
   */

  class SVDGlobalConfigParameters: public TObject {
  public:
    /**
    * Default constructor
    */
    SVDGlobalConfigParameters()
    {};


    /**
     * Returns the zero suppression threshold for global data taking in
     * ZS acquisition mode
     *
     * @param none
     * @return float corresponding to zero suppression cut applied during
     * data taking
     */
    float getZeroSuppression(void) const { return m_zeroSuppression; };

    /**
     * Returns the latency set for global data taking
     *
     * @param none
     * @return int corresponding to the latency set for global data taking
     */
    int getLatency(void) const { return m_latency; };




  private:

    /** zero suppression value which defines the S/N threshold of data mode acquisition during global run in ZS mode
         */
    float m_zeroSuppression;

    /** integer number of buffer cells to go back in the buffer ring to correctly sampling the signal pulse when the trigger is received.
     */
    int m_latency;

    ClassDef(SVDGlobalConfigParameters, 1);

  };

}
