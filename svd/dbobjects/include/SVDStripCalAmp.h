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

namespace Belle2 {

  struct SVDStripCalAmp {

    /** gain is expressed in ADC counts / # electrons injected in the channel
     */
    float gain;

    /** peakTimes (in ns) is the time at wich the shaper output reach the
     * maximum. TO BE CLARIFIED: where do we put the time origin of the injection?
     */
    float peakTime;

    /** pulseWidth (in ns) is the width of the shaper output
     *  TO BE CLARIFIED: how this width is defined?
     */
    float pulseWidth;

    ClassDef(SVDStripCalAmp, 1);

  };

}
