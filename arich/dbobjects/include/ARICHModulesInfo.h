/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <TObject.h>

// number of HAPDs in ARICH
#define N_HAPDS 420

//number of channels per HAPD
#define N_CHANNELS 144

namespace Belle2 {

  //! The Class for information on HAPD modules installed in ARICH
  /*! holds:
   *  - QEs of all channels of all installed HAPDs
   *  - list of installed modules. Only installed modules are placed in geant4 geometry.
   *  - list of active modules. Only SimHits (simulated data) or RawHits (measured data) of active modules are converted to ARICHDigits.
   */

  class ARICHModulesInfo : public TObject {

  public:

    //! Default constructor
    ARICHModulesInfo();

    /**
     * Get channel quantum efficiency
     * @param modId module ID number
     * @param chNo channel number (ASIC number)
     * @return channel QE
     */
    double getChannelQE(unsigned modId, unsigned chNo) const;

    /**
     * Set channel quantum efficiency
     * @param modId module ID number
     * @param chNo channel number (ASIC number)
     * @param qe channel quantum efficiency (0.35 for example)
     */
    void setChannelQE(unsigned modId, unsigned chId, double qe);

    /**
     * Add installed module
     * @param modId module ID number
     * @param qeList vector of channel QEs (in %!), arranged according to the channel ASIC numbers
     * @param active is module active
     */
    void addModule(unsigned modId, std::vector<float>& qeList, bool active);

    /**
     * Check if module is installed
     * @param modId module ID number
     * @return true if installed
     */
    bool isInstalled(unsigned modId) const;

    /**
     * Check if module is active
     * @param modId module ID number
     * @return true if active
     */
    bool isActive(unsigned modId) const;

    /**
     * Dump data
     */
    void print() const;

  private:

    std::vector<uint8_t>  m_ChannelQE;    /*!< Channel QE at 400nm */

    bool m_active[N_HAPDS];               /*!< array of active HAPDs */
    bool m_installed[N_HAPDS];            /*!< array of installed HAPDs */

    ClassDef(ARICHModulesInfo, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
