/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * r.m.s. of noise for all 512 channels of 16 modules.
   *
   * The noise for masked channels is undefined.
   * It is the caller's responsibility to check for masked channels
   */
  class TOPCalChannelNoise: public TObject {
  public:

    /**
     * Default constructor.
     * Noises are set to 0 by default.
     */
    TOPCalChannelNoise() {}

    /**
     * Sets the noise r.m.s for a single channel.
     * If data for a given channel not available set noise to 0 (or just skip the call)
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param rmsNoise r.m.s. of noise [ADC counts]
     */
    void setNoise(int moduleID, unsigned channel, double rmsNoise)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules);
        return;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Channel number greater than " << c_numChannels - 1);
        return;
      }
      m_rmsNoise[module][channel] = rmsNoise;
    }

    /**
     * Returns the noise r.m.s of a single channel (0 or negative: data not available)
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return r.m.s. of noise [ADC counts]
     */
    double getNoise(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        return 0;
      }
      if (channel >= c_numChannels) {
        return 0;
      }
      return m_rmsNoise[module][channel];
    }

  private:

    /**
     * Sizes
     */
    enum {
      c_numModules = 16,  /**< number of modules */
      c_numChannels = 512 /**< number of channels per module */
    };

    float m_rmsNoise[c_numModules][c_numChannels] = {{0}};    /**< noise [ADC counts] */

    ClassDef(TOPCalChannelNoise, 1); /**< ClassDef */

  };

} // end namespace Belle2


