/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {

  /** contains the parameter of the APV pulse*/
  struct SVDStripCalAmp final {

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

    ClassDef(SVDStripCalAmp, 1); /**< needed by root*/

  };

}
