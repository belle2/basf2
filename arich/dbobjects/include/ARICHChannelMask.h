/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <arich/dbobjects/ARICHModulesInfo.h>

namespace Belle2 {

  //! The Class for ARICH HAPD channel mask
  /*! holds mask of non-active channels of all installed HAPDs
   *  By default all channels are active
   */

  class ARICHChannelMask : public TObject {

  public:

    //! Default constructor
    ARICHChannelMask();

    /**
     * Check if channel is active
     * @param modId module ID number
     * @param chNo asic channel number
     * @return true if active
     */
    bool isActive(unsigned modId, unsigned chNo) const;

    /**
     * Set active/non-active channel
     * @param modId module ID number
     * @param chNo asic channel number
     * @param active true if active
     */
    void setActiveCh(unsigned modId, unsigned chNo, bool active);

    /**
     * Set active/non-active HAPD module
     * @param modId module ID number
     * @param active true if active
     */
    void setActiveHAPD(unsigned modId, bool active);

    /**
     * Set active/non-active APD of HAPD module
     * @param modId module ID number
     * @param apdId apd chip ID number
     * @param active true if active
     */
    void setActiveAPD(unsigned modId, unsigned apdId, bool active);

    /**
     * dump content
     */
    void print() const;

  private:

    uint32_t  m_DetectorMask[N_HAPDS * N_CHANNELS / 32 + 1] = {0xFFFFFFFF}; /*!< Detector Mask of active channels */

    ClassDef(ARICHChannelMask, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
