/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <framework/gearbox/GearDir.h>
#include <TObject.h>
#include <TTree.h>
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
