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
   * Class to store the threshold efficiency (i.e. the efficiency for the
   * pulse identification, function of the threshold being used in the CFD
   * or template fit algorithm ) for all 512 channels of 16 modules.
   * From laser scans.
   */
  class TOPCalChannelThresholdEff: public TObject {
  public:

    /**
     * Default constructor.
     * Threshold efficiencies are set to 0 by default.
     */
    TOPCalChannelThresholdEff()
    {
      for (unsigned m = 0; m < c_numModules; m++) {
        for (unsigned c = 0; c < c_numChannels; c++) {
          m_ThrEff[m][c] = 0;
        }
      }
    }

    /**
     * Sets the threshold efficiency for a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param ThrEff channel threshold efficiency
     */
    void setThrEff(int moduleID, unsigned channel, float ThrEff)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than 16");
        return;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Channel number greater than 511");
        return;
      }
      m_ThrEff[module][channel] = ThrEff;
    }

    /**
     * Returns the threshold efficiency of a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return ThrEff
     */
    float getThrEff(int moduleID, unsigned channel) const
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
      return m_ThrEff[module][channel];
    }


  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numChannels = 512 /**< number of channels per module */
         };


    float m_ThrEff[c_numModules][c_numChannels];    /**< threshold efficiency value */

    ClassDef(TOPCalChannelThresholdEff, 1); /**< ClassDef */

  };

} // end namespace Belle2

