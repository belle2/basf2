/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Value of the threshold (in ADC counts) used for the pulse identification,
   * for all 512 channels of 16 modules.
   * From laser scans.
   */
  class TOPCalChannelThreshold: public TObject {
  public:

    /**
     * Default constructor.
     */
    TOPCalChannelThreshold() {}

    /**
     * Sets the threshold (in ADC counts) for a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param Thr channel threshold
     */
    void setThr(int moduleID, unsigned channel, short Thr)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than 16.");
        return;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Channel number greater than 511.");
        return;
      }
      m_Thr[module][channel] = Thr;
    }

    /**
     * Returns the threshold (in ADC counts) of a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return Thr
     */
    short getThr(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than 16. Returning 0.");
        return 0;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Channel number greater than 511. Returning 0.");
        return 0;
      }
      return m_Thr[module][channel];
    }


  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numChannels = 512 /**< number of channels per module */
         };

    short m_Thr[c_numModules][c_numChannels] = {{0}};    /**< threshold value in ADC counts. 0 by default*/

    ClassDef(TOPCalChannelThreshold, 1); /**< ClassDef */

  };

} // end namespace Belle2

